///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/detail/socket.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/mixin.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/impl/tcp/socket.hpp>
#include <gce/actor/detail/mailbox.hpp>
#include <gce/actor/message.hpp>
#include <gce/detail/scope.hpp>
#include <gce/actor/impl/protocol.hpp>
#include <gce/amsg/amsg.hpp>
#include <gce/amsg/zerocopy.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <boost/variant/get.hpp>
#include <boost/static_assert.hpp>
#include <boost/foreach.hpp>

namespace gce
{
namespace detail
{
///----------------------------------------------------------------------------
socket::socket(context* ctx)
  : basic_actor(
      ctx->get_attributes().max_cache_match_size_,
      ctx->get_timestamp()
      )
  , stat_(ready)
  , hb_(ctx->get_io_service())
  , sync_(ctx->get_io_service())
  , recv_cache_(recv_buffer_, GCE_SOCKET_RECV_CACHE_SIZE)
  , conn_(false)
  , curr_reconn_(0)
  , is_router_(false)
{
}
///----------------------------------------------------------------------------
socket::~socket()
{
}
///----------------------------------------------------------------------------
void socket::init(cache_pool* user, cache_pool* owner, net_option opt)
{
  BOOST_ASSERT_MSG(stat_ == ready, "socket status error");
  user_ = user;
  owner_ = owner;
  opt_ = opt;
  curr_reconn_ = u32_nil;

  base_type::update_aid();
}
///----------------------------------------------------------------------------
void socket::connect(ctxid_t target, std::string const& ep, bool target_is_router)
{
  ctxid_pair_t ctxid_pr = std::make_pair(target, target_is_router);
  owner_->register_socket(ctxid_pr, get_aid());

  message m(msg_login);
  m << user_->get_ctxid();
  send(m);

  boost::asio::spawn(
    user_->get_strand(),
    boost::bind(
      &socket::run_conn,
      this, ctxid_pr, ep, _1
      ),
    boost::coroutines::attributes(default_stacksize())
    );
}
///----------------------------------------------------------------------------
void socket::start(
  std::map<match_t, actor_func_t> const& remote_list,
  socket_ptr skt, bool is_router
  )
{
  conn_ = true;
  remote_list_ = remote_list;
  is_router_ = is_router;

  boost::asio::spawn(
    user_->get_strand(),
    boost::bind(
      &socket::run, this, skt, _1
      ),
    boost::coroutines::attributes(default_stacksize())
    );
}
///----------------------------------------------------------------------------
void socket::stop()
{
  close();
}
///----------------------------------------------------------------------------
void socket::on_free()
{
  base_type::on_free();

  stat_ = ready;
  recv_cache_.clear();
  conn_ = false;
  conn_cache_.clear();
  curr_reconn_ = 0;
  straight_link_list_.clear();
  remote_list_.clear();
  is_router_ = false;
}
///----------------------------------------------------------------------------
void socket::on_recv(pack* pk)
{
  user_->get_strand().dispatch(
    boost::bind(
      &socket::handle_recv, this, pk
      )
    );
}
///----------------------------------------------------------------------------
ctxid_pair_t socket::handle_net_msg(message& msg, ctxid_pair_t curr_pr)
{
  if (msg.get_type() == msg_hb)
  {
    ctxid_pair_t ctxid_pr;
    msg >> ctxid_pr;
    return sync_ctxid(ctxid_pr, curr_pr);
  }

  pack* pk = base_type::alloc_pack(user_);
  ctxid_pair_t ctxid_pr;
  bool has_tag =
    msg.pop_tag(
      pk->tag_, pk->recver_, pk->skt_,
      pk->is_err_ret_, ctxid_pr
      );
  BOOST_ASSERT(has_tag);
  pk->msg_ = msg;

  curr_pr = sync_ctxid(ctxid_pr, curr_pr);

  if (link_t* link = boost::get<detail::link_t>(&pk->tag_))
  {
    if (is_router_)
    {
      sktaid_t skt = user_->select_straight_socket(pk->recver_.ctxid_);
      if (!skt)
      {
        /// no socket found, send already exit back
        base_type::send_already_exited(link->get_aid(), pk->recver_);
      }
      else
      {
        pk->tag_ = fwd_link_t(link->get_type(), link->get_aid(), get_aid());
        pk->skt_ = skt;
        if (link->get_type() == linked)
        {
          add_router_link(pk->recver_, link->get_aid(), skt);
        }
        base_type::send(pk->skt_, pk, user_);
      }
    }
    else
    {
      pk->skt_ = get_aid();
      if (link->get_type() == linked)
      {
        add_straight_link(pk->recver_, link->get_aid());
      }
      base_type::send(pk->recver_, pk, user_);
    }
  }
  else if (exit_t* ex = boost::get<exit_t>(&pk->tag_))
  {
    if (is_router_)
    {
      sktaid_t skt = remove_router_link(pk->recver_, ex->get_aid());
      BOOST_ASSERT(skt);
      pk->tag_ = fwd_exit_t(ex->get_code(), ex->get_aid(), get_aid());
      pk->skt_ = skt;
      base_type::send(skt, pk, user_);
    }
    else
    {
      remove_straight_link(pk->recver_, ex->get_aid());
      base_type::send(pk->recver_, pk, user_);
    }
  }
  else
  {
    if (is_router_)
    {
      sktaid_t skt = user_->select_straight_socket(pk->recver_.ctxid_);
      if (detail::request_t* req = boost::get<detail::request_t>(&pk->tag_))
      {
        if (!skt)
        {
          /// reply actor exit msg
          response_t res(req->get_id(), pk->recver_);
          base_type::send_already_exited(req->get_aid(), res);
        }
      }

      if (skt)
      {
        pk->skt_ = skt;
        base_type::send(pk->skt_, pk, user_);
      }
    }
    else
    {
      base_type::send(pk->recver_, pk, user_);
    }
  }

  return curr_pr;
}
///----------------------------------------------------------------------------
void socket::send(message const& m)
{
  match_t type = m.get_type();
  std::size_t size = m.size();
  if (conn_)
  {
    BOOST_ASSERT(skt_);
    while (!conn_cache_.empty())
    {
      message const& m = conn_cache_.front();
      send_msg(m);
      conn_cache_.pop_front();
    }
    send_msg(m);
  }
  else
  {
    conn_cache_.push_back(m);
  }
}
///----------------------------------------------------------------------------
void socket::send_msg(message const& m)
{
  BOOST_ASSERT(skt_);
  msg::header hdr;
  hdr.size_ = m.size();
  hdr.type_ = m.get_type();
  hdr.tag_offset_ = m.tag_offset_;

  byte_t buf[sizeof(msg::header)];
  boost::amsg::zero_copy_buffer zbuf(buf, sizeof(msg::header));
  boost::amsg::write(zbuf, hdr);
  skt_->send(
    buf, zbuf.write_length(),
    m.data(), hdr.size_
    );
}
///----------------------------------------------------------------------------
void socket::send_msg_hb()
{
  message m(detail::msg_hb);
  m << std::make_pair(user_->get_ctxid(), is_router_);
  send(m);
}
///----------------------------------------------------------------------------
void socket::run_conn(ctxid_pair_t target, std::string const& ep, yield_t yield)
{
  exit_code_t exc = exit_normal;
  std::string exit_msg("exit normal");
  ctxid_pair_t curr_pr = target;

  if (!user_->stopped())
  {
    context& ctx = user_->get_context();
    user_->add_socket(this);
    ctx.register_socket(target, get_aid(), user_);

    try
    {
      stat_ = on;
      skt_ = make_socket(ep);
      connect(yield);

      while (stat_ == on)
      {
        message msg;
        errcode_t ec = recv(msg, yield);
        if (ec)
        {
          on_neterr(ec);
          --curr_reconn_;
          if (curr_reconn_ == 0)
          {
            exc = exit_neterr;
            exit_msg = ec.message();
            close();
            break;
          }
          connect(yield);
        }
        else
        {
          match_t type = msg.get_type();
          if (type == msg_login_ret)
          {
            ctxid_pair_t ctxid_pr;
            msg >> ctxid_pr;
            curr_pr = sync_ctxid(ctxid_pr, curr_pr);
          }
          else
          {
            curr_pr = handle_net_msg(msg, curr_pr);
          }
          hb_.beat();
        }
      }
    }
    catch (std::exception& ex)
    {
      exc = exit_except;
      exit_msg = ex.what();
      close();
    }
    catch (...)
    {
      exc = exit_unknown;
      exit_msg = "unexpected exception";
      close();
    }
  }
  free_self(curr_pr, exc, exit_msg, yield);
}
///----------------------------------------------------------------------------
void socket::run(socket_ptr skt, yield_t yield)
{
  exit_code_t exc = exit_normal;
  std::string exit_msg("exit normal");
  ctxid_pair_t curr_pr = std::make_pair(ctxid_nil, false);

  if (!user_->stopped())
  {
    context& ctx = user_->get_context();
    user_->add_socket(this);

    try
    {
      stat_ = on;
      skt_ = skt;
      skt_->init(user_);
      start_heartbeat(boost::bind(&socket::close, this));

      while (stat_ == on)
      {
        message msg;
        errcode_t ec = recv(msg, yield);
        if (ec)
        {
          on_neterr(ec);
          close();
          exc = exit_neterr;
          exit_msg = ec.message();
          break;
        }
        else
        {
          match_t type = msg.get_type();
          if (type == msg_login)
          {
            ctxid_pair_t ctxid_pr = std::make_pair(u32_nil, false);
            msg >> ctxid_pr.first;
            curr_pr = sync_ctxid(ctxid_pr, curr_pr);
            message m(msg_login_ret);
            m << std::make_pair(user_->get_ctxid(), is_router_);
            send(m);
          }
          else
          {
            curr_pr = handle_net_msg(msg, curr_pr);
          }
          hb_.beat();
        }
      }
    }
    catch (std::exception& ex)
    {
      exc = exit_except;
      exit_msg = ex.what();
      close();
    }
    catch (...)
    {
      exc = exit_unknown;
      exit_msg = "unexpected exception";
      close();
    }
  }
  free_self(curr_pr, exc, exit_msg, yield);
}
///----------------------------------------------------------------------------
socket_ptr socket::make_socket(std::string const& ep)
{
  /// find protocol name
  std::size_t pos = ep.find("://");
  if (pos == std::string::npos)
  {
    throw std::runtime_error("protocol name parse failed");
  }

  std::string prot_name = ep.substr(0, pos);
  if (prot_name == "tcp")
  {
    /// parse address
    std::size_t begin = pos + 3;
    pos = ep.find(':', begin);
    if (pos == std::string::npos)
    {
      throw std::runtime_error("tcp address parse failed");
    }

    std::string address = ep.substr(begin, pos - begin);

    /// parse port
    begin = pos + 1;
    pos = ep.size();

    std::string port = ep.substr(begin, pos - begin);
    socket_ptr skt(
      new tcp::socket(
        user_->get_strand().get_io_service(),
        address, port
        )
      );
    skt->init(user_);
    return skt;
  }

  throw std::runtime_error("unsupported protocol");
}
///----------------------------------------------------------------------------
void socket::handle_recv(pack* pk)
{
  ctxid_pair_t ctxid_pr = std::make_pair(get_aid().ctxid_, is_router_);
  scope scp(boost::bind(&basic_actor::dealloc_pack, user_, pk));
  if (check(pk->skt_, ctxid_pr.first, user_->get_context().get_timestamp()))
  {
    BOOST_ASSERT(!check_local(pk->recver_, ctxid_pr.first));
    if (link_t* link = boost::get<link_t>(&pk->tag_))
    {
      add_straight_link(link->get_aid(), pk->recver_);
    }
    else if (exit_t* ex = boost::get<exit_t>(&pk->tag_))
    {
      remove_straight_link(ex->get_aid(), pk->recver_);
    }
    else if (fwd_link_t* link = boost::get<fwd_link_t>(&pk->tag_))
    {
      add_router_link(link->get_aid(), pk->recver_, link->get_skt());
      pk->tag_ = link_t(link->get_type(), link->get_aid());
    }
    else if (fwd_exit_t* ex = boost::get<fwd_exit_t>(&pk->tag_))
    {
      remove_router_link(ex->get_aid(), pk->recver_);
      pk->tag_ = exit_t(ex->get_code(), ex->get_aid());
    }
    pk->msg_.append_tag(pk->tag_, pk->recver_, pk->skt_, pk->is_err_ret_, ctxid_pr);
    send(pk->msg_);
  }
  else if (!pk->is_err_ret_)
  {
    if (detail::link_t* link = boost::get<detail::link_t>(&pk->tag_))
    {
      /// send actor exit msg
      base_type::send_already_exited(link->get_aid(), pk->recver_);
    }
    else if (detail::request_t* req = boost::get<detail::request_t>(&pk->tag_))
    {
      /// reply actor exit msg
      response_t res(req->get_id(), pk->recver_);
      base_type::send_already_exited(req->get_aid(), res);
    }
  }
}
///----------------------------------------------------------------------------
void socket::add_straight_link(aid_t src, aid_t des)
{
  std::pair<straight_link_list_t::iterator, bool> pr =
    straight_link_list_.insert(std::make_pair(src, straight_dummy_));
  pr.first->second.insert(des);
}
///----------------------------------------------------------------------------
void socket::remove_straight_link(aid_t src, aid_t des)
{
  straight_link_list_t::iterator itr(
    straight_link_list_.find(src)
    );
  if (itr != straight_link_list_.end())
  {
    itr->second.erase(des);
  }
}
///----------------------------------------------------------------------------
void socket::add_router_link(aid_t src, aid_t des, sktaid_t skt)
{
  std::pair<router_link_list_t::iterator, bool> pr =
    router_link_list_.insert(std::make_pair(src, router_dummy_));
  pr.first->second.insert(std::make_pair(des, skt));
}
///----------------------------------------------------------------------------
sktaid_t socket::remove_router_link(aid_t src, aid_t des)
{
  sktaid_t skt;
  router_link_list_t::iterator itr(
    router_link_list_.find(src)
    );
  if (itr != router_link_list_.end())
  {
    std::map<aid_t, sktaid_t>& skt_list = itr->second;
    std::map<aid_t, sktaid_t>::iterator skt_itr(skt_list.find(des));
    if (skt_itr != skt_list.end())
    {
      skt = skt_itr->second;
      skt_list.erase(skt_itr);
    }
  }
  return skt;
}
///----------------------------------------------------------------------------
void socket::on_neterr(errcode_t ec)
{
  conn_ = false;
  conn_cache_.clear();
  std::string errmsg("net error");
  if (ec)
  {
    errmsg = ec.message();
  }

  message m(exit);
  m << exit_neterr << errmsg;

  BOOST_FOREACH(straight_link_list_t::value_type& pr, straight_link_list_)
  {
    BOOST_FOREACH(aid_t const& des, pr.second)
    {
      detail::pack* pk = alloc_pack(user_);
      pk->tag_ = detail::exit_t(exit_neterr, des);
      pk->recver_ = pr.first;
      pk->skt_ = pr.first;
      pk->msg_ = m;

      base_type::send(pk->recver_, pk, user_);
    }
  }
  straight_link_list_.clear();

  BOOST_FOREACH(router_link_list_t::value_type& pr, router_link_list_)
  {
    BOOST_FOREACH(router_link_list_t::mapped_type::value_type& des, pr.second)
    {
      detail::pack* pk = alloc_pack(user_);
      pk->tag_ = fwd_exit_t(exit_neterr, des.first, get_aid());
      pk->recver_ = pr.first;
      pk->skt_ = des.second;
      pk->msg_ = m;

      base_type::send(pk->skt_, pk, user_);
    }
  }
  router_link_list_.clear();
}
///----------------------------------------------------------------------------
ctxid_pair_t socket::sync_ctxid(ctxid_pair_t new_pr, ctxid_pair_t curr_pr)
{
  if (new_pr != curr_pr)
  {
    context& ctx = user_->get_context();
    ctx.deregister_socket(curr_pr, get_aid(), user_);
    ctx.register_socket(new_pr, get_aid(), user_);
  }
  return new_pr;
}
///----------------------------------------------------------------------------
bool socket::parse_message(message& msg)
{
  msg::header hdr;
  byte_t* data = recv_cache_.get_read_data();
  std::size_t remain_size = recv_cache_.remain_read_size();
  boost::amsg::zero_copy_buffer zbuf(data, remain_size);
  boost::amsg::read(zbuf, hdr);
  if (zbuf.bad())
  {
    return false;
  }

  if (hdr.size_ > GCE_MAX_MSG_SIZE)
  {
    throw std::runtime_error("message overlength");
  }

  std::size_t header_size = zbuf.read_length();
  if (remain_size - header_size < hdr.size_)
  {
    return false;
  }

  recv_cache_.read(header_size + hdr.size_);
  msg = message(hdr.type_, data + header_size, hdr.size_, hdr.tag_offset_);

  /// reset read_cache
  if (recv_cache_.read_size() > GCE_SOCKET_RECV_MAX_SIZE)
  {
    BOOST_ASSERT(recv_cache_.write_size() >= recv_cache_.read_size());
    std::size_t copy_size =
      recv_cache_.write_size() - recv_cache_.read_size();
    std::memmove(recv_buffer_, recv_cache_.get_read_data(), copy_size);
    recv_cache_.clear();
    recv_cache_.write(copy_size);
  }
  return true;
}
///----------------------------------------------------------------------------
void socket::connect(yield_t yield)
{
  errcode_t ec;
  if (stat_ == on)
  {
    for (std::size_t i=0, retry=0; i<u32_nil; ++i, ++retry)
    {
      if (retry > opt_.reconn_try_)
      {
        retry = 0;
        on_neterr();
      }

      if (i > 0)
      {
        sync_.expires_from_now(opt_.reconn_period_);
        sync_.async_wait(yield[ec]);
        if (stat_ != on)
        {
          break;
        }
      }

      skt_->connect(yield[ec]);
      if (!ec || stat_ != on)
      {
        break;
      }
    }

    if (stat_ != on)
    {
      return;
    }

    if (ec)
    {
      throw std::runtime_error(ec.message());
    }

    conn_ = true;
    start_heartbeat(boost::bind(&socket::reconn, this));

    message m(msg_login);
    m << user_->get_ctxid();
    send(m);
  }
}
///----------------------------------------------------------------------------
errcode_t socket::recv(message& msg, yield_t yield)
{
  BOOST_STATIC_ASSERT((GCE_SOCKET_RECV_CACHE_SIZE > GCE_SOCKET_RECV_MAX_SIZE));

  errcode_t ec;
  while (stat_ != off && !parse_message(msg))
  {
    std::size_t size =
      skt_->recv(
        recv_cache_.get_write_data(),
        recv_cache_.remain_write_size(),
        yield[ec]
        );
    if (ec)
    {
      break;
    }

    recv_cache_.write(size);
  }

  return ec;
}
///----------------------------------------------------------------------------
void socket::close()
{
  stat_ = off;
  hb_.stop();
  if (skt_)
  {
    skt_->close();
  }
  errcode_t ignore_ec;
  sync_.cancel(ignore_ec);
}
///----------------------------------------------------------------------------
void socket::reconn()
{
  skt_->reset();
}
///----------------------------------------------------------------------------
template <typename F>
void socket::start_heartbeat(F f)
{
  hb_.init(
    user_,
    opt_.heartbeat_period_, opt_.heartbeat_count_,
    f, boost::bind(&socket::send_msg_hb, this)
    );
  hb_.start();
}
///----------------------------------------------------------------------------
void socket::free_self(
  ctxid_pair_t ctxid_pr, exit_code_t exc,
  std::string const& exit_msg, yield_t yield
  )
{
  try
  {
    hb_.wait_end(yield);
    if (skt_)
    {
      skt_->wait_end(yield);
    }
  }
  catch (...)
  {
  }

  skt_.reset();

  hb_.clear();
  if (ctxid_pr.first != ctxid_nil)
  {
    user_->deregister_socket(ctxid_pr, get_aid());
    user_->get_context().deregister_socket(ctxid_pr, get_aid(), user_);
  }

  user_->remove_socket(this);
  on_neterr();
  base_type::send_exit(exc, exit_msg);
  base_type::update_aid();
  user_->free_socket(owner_, this);
}
///----------------------------------------------------------------------------
}
}
