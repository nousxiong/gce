///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_SOCKET_ACTOR_HPP
#define GCE_ACTOR_DETAIL_SOCKET_ACTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/net_option.hpp>
#include <gce/actor/exception.hpp>
#include <gce/actor/detail/spawn_actor.hpp>
#include <gce/actor/detail/network_service.hpp>
#include <gce/actor/detail/stackful_actor.hpp>
#include <gce/actor/detail/stackless_actor.hpp>
#include <gce/actor/detail/heartbeat.hpp>
#include <gce/actor/detail/basic_actor.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/actor/detail/actor_function.hpp>
#include <gce/actor/detail/yielder.hpp>
#include <gce/actor/detail/internal.hpp>
#include <gce/actor/detail/tcp/socket.hpp>
#include <gce/detail/linked_queue.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/optional.hpp>
/// for compatibility with >= 1.59 (using boost::in_place)
#include <boost/utility/in_place_factory.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

#include <boost/variant/get.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/static_assert.hpp>
#include <vector>
#include <map>

#ifndef GCE_SOCKET_RECV_BUFFER_MIN_SIZE
# define GCE_SOCKET_RECV_BUFFER_MIN_SIZE 60000
#endif

#define GCE_MSG_HEADER_MAX_SIZE sizeof(gce::detail::header_t) * 3
#define GCE_SOCKET_RECV_BUFFER_COPY_SIZE GCE_MSG_HEADER_MAX_SIZE * 5

namespace gce
{
namespace detail
{
template <typename Context>
class socket_actor
  : public basic_actor<Context>
{
public:
  typedef Context context_t;

private:
  typedef basic_actor<context_t> base_t;
  typedef socket_actor<context_t> self_t;
  typedef typename context_t::stackful_actor_t stackful_actor_t;
  typedef typename context_t::stackful_service_t stackful_service_t;
  typedef typename context_t::stackless_actor_t stackless_actor_t;
  typedef typename context_t::stackless_service_t stackless_service_t;
#ifdef GCE_LUA
  typedef typename context_t::lua_actor_t lua_actor_t;
  typedef typename context_t::lua_service_t lua_service_t;
#endif
  typedef network_service<self_t> service_t;
  typedef std::pair<match_t, remote_func<context_t> > remote_func_t;
  typedef std::map<match_t, remote_func<context_t> > remote_func_list_t;

  enum status
  {
    ready = 0,
    on,
    off,
  };

public:
  socket_actor(aid_t aid, service_t& svc)
    : base_t(svc.get_context(), svc, actor_socket, aid)
    , stat_(ready)
    , host_(this)
    , svc_(svc)
    , hb_(base_t::snd_)
    , sync_(base_t::ctx_.get_io_service())
    , recv_cache_(recv_buffer_.data(), recv_buffer_.size())
    , recving_header_(true)
    , recving_msg_(false)
    , conn_(false)
    , curr_reconn_(0)
    , is_router_(false)
    , lg_(base_t::ctx_.get_logger())
  {
  }

  ~socket_actor()
  {
    clear_conn_cache();
  }

public:
  void init(netopt_t const& opt)
  {
    GCE_ASSERT(stat_ == ready)(stat_).log(lg_, "socket_actor status error");
    opt_ = opt;
    curr_reconn_ = opt.reconn_max_try;
  }

  void connect(
    aid_t const& sire, std::vector<remote_func_t> const& remote_func_list,
    ctxid_t target, std::string const& ep, bool target_is_router
    )
  {
    BOOST_FOREACH(remote_func_t const& f, remote_func_list)
    {
      remote_func_list_.insert(std::make_pair(f.first, f.second));
    }
    ctxid_pair_t ctxid_pr =
      std::make_pair(
        target,
        target_is_router ? socket_router : socket_conn
        );
    svc_.register_socket(ctxid_pr, base_t::get_aid());

    send_login(false);

    boost::asio::spawn(
      base_t::snd_,
      run_conn_binder(*this, sire, ctxid_pr, ep), 
      boost::coroutines::attributes(default_stacksize())
      );

    if (stat_ != off)
    {
      base_t::ctx_.on_tick(svc_.get_index());
    }
  }

  void send(aid_t const& recver, message const& msg)
  {
    base_t::pri_send(recver, msg);
  }

public:
  typedef gce::socket type;

  static actor_type get_type()
  {
    return actor_socket;
  }

  static size_t get_pool_reserve_size(attributes const& attr)
  {
    return attr.socket_pool_reserve_size_;
  }

  static size_t get_pool_max_size(attributes const& attr)
  {
    return attr.socket_pool_max_size_;
  }

  void start(
    remote_func_list_t const& remote_func_list,
    socket_ptr skt, bool is_router
    )
  {
    conn_ = true;
    remote_func_list_ = remote_func_list;
    is_router_ = is_router;

    boost::asio::spawn(
      base_t::snd_,
      run_binder(*this, skt), 
      boost::coroutines::attributes(default_stacksize())
      );

    if (stat_ != off)
    {
      base_t::ctx_.on_tick(svc_.get_index());
    }
  }

  void stop()
  {
    close();
  }

  void on_recv(pack& pk)
  {
    handle_recv(pk);
  }

  void on_addon_recv(pack& pk)
  {
    base_t::snd_.dispatch(handle_recv_binder(*this, pk));
  }

  void link(aid_t const&) {}
  void monitor(aid_t const&) {}

private:
  struct run_conn_binder
  {
    run_conn_binder(self_t& self, aid_t const& sire, ctxid_pair_t const& ctxid_pr, std::string const& ep)
      : self_(self)
      , sire_(sire)
      , ctxid_pr_(ctxid_pr)
      , ep_(ep)
    {
    }

    void operator()(yield_t yld) const
    {
      self_.run_conn(sire_, ctxid_pr_, ep_, yld);
    }

    self_t& self_;
    aid_t const sire_;
    ctxid_pair_t const ctxid_pr_;
    std::string const ep_;
  };

  struct run_binder
  {
    run_binder(self_t& self, socket_ptr skt)
      : self_(self)
      , skt_(skt)
    {
    }

    void operator()(yield_t yld) const
    {
      self_.run(skt_, yld);
    }

    self_t& self_;
    socket_ptr skt_;
  };

  struct handle_recv_binder
  {
    handle_recv_binder(self_t& self, pack const& pk)
      : self_(self)
      , pk_(pk)
    {
    }

    void operator()()
    {
      self_.handle_recv(pk_);
    }

    self_t& self_;
    pack pk_;
  };

  void send_pack(aid_t const& target, pack& src, message const& msg)
  {
    pack& pk = base_t::basic_svc_.alloc_pack(target);
    pk.tag_ = src.tag_;
    pk.recver_ = src.recver_;
    pk.skt_ = src.skt_;
    pk.svc_ = src.svc_;
    pk.is_err_ret_ = src.is_err_ret_;
    //pk.msg_ = msg;
    pk.setmsg(msg);
    svc_.send(target, pk);
  }

  void handle_net_msg(message& msg)
  {
    pk_.on_free();
    pack& pk = pk_;

    bool has_tag =
      msg.pop_tag(
        pk.tag_, pk.recver_, pk.svc_,
        pk.skt_, pk.is_err_ret_
        );
    GCE_ASSERT(has_tag);
    //pk.msg_ = msg;

    if (link_t* link = boost::get<link_t>(&pk.tag_))
    {
      if (is_router_)
      {
        ctxid_t ctxid = pk.recver_ != aid_nil ? pk.recver_.ctxid_ : pk.recver_.svc_.ctxid_;
        sktaid_t skt = svc_.select_joint_socket(ctxid);
        if (skt == aid_nil)
        {
          /// no socket found, send already exit back
          svc_.send_already_exited(link->get_aid(), pk.recver_);
        }
        else
        {
          pk.tag_ = fwd_link_t(link->get_type(), link->get_aid(), base_t::get_aid());
          pk.skt_ = skt;
          if (link->get_type() == linked)
          {
            add_router_link(pk.recver_, link->get_aid(), skt);
          }
          send_pack(pk.skt_, pk, msg);
        }
      }
      else
      {
        pk.skt_ = base_t::get_aid();
        if (link->get_type() == linked)
        {
          add_straight_link(pk.recver_, link->get_aid());
        }
        aid_t target = 
          pk.recver_ != aid_nil ? 
          pk.recver_ : 
          base_t::basic_svc_.filter_svcid(pk.recver_.svc_);
        if (target != aid_nil)
        {
          send_pack(target, pk, msg);
        }
        else
        {
          svc_.send_already_exited(link->get_aid(), pk.recver_);
        }
      }
    }
    else if (exit_t* ex = boost::get<exit_t>(&pk.tag_))
    {
      if (is_router_)
      {
        sktaid_t skt = remove_router_link(pk.recver_, ex->get_aid());
        //GCE_ASSERT(skt != aid_nil)(pk.recver_)(ex->get_aid());
        if (skt != aid_nil)
        {
          pk.tag_ = fwd_exit_t(ex->get_code(), ex->get_aid(), base_t::get_aid());
          pk.skt_ = skt;
          send_pack(pk.skt_, pk, msg);
        }
      }
      else
      {
        if (remove_straight_link(pk.recver_, ex->get_aid()))
        {
          aid_t target = 
            pk.recver_ != aid_nil ? 
            pk.recver_ : 
            base_t::basic_svc_.filter_svcid(pk.recver_.svc_);
          if (target != aid_nil)
          {
            send_pack(target, pk, msg);
          }
        }
      }
    }
    else if (spawn_t* spw = boost::get<spawn_t>(&pk.tag_))
    {
      if (is_router_)
      {
        sktaid_t skt = svc_.select_joint_socket(spw->get_ctxid());
        if (skt == aid_nil)
        {
          send_spawn_ret(spw, pk, spawn_no_socket, aid_nil, true);
        }
        else
        {
          pk.skt_ = skt;
          send_pack(pk.skt_, pk, msg);
        }
      }
      else
      {
        /// spawn actor
        spawn_type sty = spw->get_type();
        if (sty == spw_stackful || sty == spw_stackless)
        {
          match_t func = gce::atom(spw->get_func().c_str());
          typename remote_func_list_t::iterator itr(remote_func_list_.find(func));
          if (itr == remote_func_list_.end())
          {
            send_spawn_ret(spw, pk, spawn_func_not_found, aid_nil, true);
          }
          else
          {
            context_t& ctx = svc_.get_context();
            if (sty == spw_stackful)
            {
              stackful_service_t& svc = ctx.select_service<stackful_service_t>();
              svc.get_strand().post(
                spawn_remote_stackful_actor_binder(
                  *this, svc, *spw, itr->second
                  )
                );
            }
            else if (sty == spw_stackless)
            {
              stackless_service_t& svc = ctx.select_service<stackless_service_t>();
              svc.get_strand().post(
                spawn_remote_stackless_actor_binder(
                  *this, svc, *spw, itr->second
                  )
                );
            }
          }
        }
        else
        {
  #ifdef GCE_LUA
          if (sty == spw_luaed)
          {
            context_t& ctx = svc_.get_context();
            lua_service_t& svc = ctx.select_service<lua_service_t>();
            svc.get_strand().post(
              spawn_remote_lua_actor_binder(
                *this, svc, *spw, spw->get_func()
                )
              );
          }
  #endif
        }
      }
    }
    else if (spawn_ret_t* spr = boost::get<spawn_ret_t>(&pk.tag_))
    {
      if (is_router_)
      {
        sktaid_t skt = svc_.select_joint_socket(pk.recver_.ctxid_);
        if (skt != aid_nil)
        {
          pk.skt_ = skt;
          send_pack(pk.skt_, pk, msg);
        }
      }
      else
      {
        /// fwd to spawner
        message m(msg_spawn_ret);
        m << (uint16_t)spr->get_error() << spr->get_id();
        aid_t aid = spr->get_aid();
        if (aid == aid_nil)
        {
          /// we should make sure no timeout miss.
          aid = base_t::get_aid();
        }
        pk.tag_ = aid;
        //pk.msg_ = m;

        send_pack(pk.recver_, pk, m);
      }
    }
    else
    {
      bool is_svc = pk.svc_ != svcid_nil;
      if (is_router_)
      {
        ctxid_t ctxid = is_svc ? pk.svc_.ctxid_ : pk.recver_.ctxid_;
        sktaid_t skt = svc_.select_joint_socket(ctxid);
        if (request_t* req = boost::get<request_t>(&pk.tag_))
        {
          if (skt == aid_nil && !is_svc)
          {
            /// reply actor exit msg
            resp_t res(req->get_id(), pk.recver_);
            svc_.send_already_exited(req->get_aid(), res);
          }
        }

        if (skt != aid_nil)
        {
          pk.skt_ = skt;
          send_pack(pk.skt_, pk, msg);
        }
      }
      else
      {
        if (is_svc)
        {
          pk.recver_ = svc_.find_service(pk.svc_.name_);
        }

        send_pack(pk.recver_, pk, msg);
      }
    }
  }

  struct spawn_remote_stackless_actor_binder
  {
    spawn_remote_stackless_actor_binder(
      self_t& self, 
      typename context_t::stackless_service_t& svc, 
      spawn_t spw, 
      remote_func<context_t> const& f
      )
      : self_(self)
      , svc_(svc)
      , spw_(spw)
      , f_(f)
    {
    }

    void operator()() const
    {
      self_.spawn_remote_stackless_actor(svc_, spw_, f_);
    }

    self_t& self_;
    typename context_t::stackless_service_t& svc_;
    spawn_t const spw_;
    remote_func<context_t> const f_;
  };

  struct spawn_remote_stackful_actor_binder
  {
    spawn_remote_stackful_actor_binder(
      self_t& self,
      typename context_t::stackful_service_t& svc, 
      spawn_t spw, 
      remote_func<context_t> const& f
      )
      : self_(self)
      , svc_(svc)
      , spw_(spw)
      , f_(f)
    {
    }

    void operator()() const
    {
      self_.spawn_remote_stackful_actor(svc_, spw_, f_);
    }

    self_t& self_;
    typename context_t::stackful_service_t& svc_;
    spawn_t const spw_;
    remote_func<context_t> const f_;
  };

  struct end_spawn_remote_actor_binder
  {
    end_spawn_remote_actor_binder(self_t& self, spawn_t spw, aid_t const& aid)
      : self_(self)
      , spw_(spw)
      , aid_(aid)
    {
    }

    void operator()() const
    {
      self_.end_spawn_remote_actor(spw_, aid_);
    }

    self_t& self_;
    spawn_t const spw_;
    aid_t const aid_;
  };

  void spawn_remote_stackful_actor(
    typename context_t::stackful_service_t& svc, spawn_t spw, remote_func<context_t> f
    )
  {
    spawn_type type = spw.get_type();
    GCE_ASSERT(type == spw_stackful)(type);
    aid_t aid = make_stackful_actor<context_t>(aid_nil, svc, f.af_, spw.get_stack_size(), no_link);
    base_t::snd_.post(end_spawn_remote_actor_binder(*this, spw, aid));
  }

  void spawn_remote_stackless_actor(
    typename context_t::stackless_service_t& svc, spawn_t spw, remote_func<context_t> f
    )
  {
    spawn_type type = spw.get_type();
    GCE_ASSERT(type == spw_stackless)(type);
    aid_t aid = make_stackless_actor<context_t>(aid_nil, svc, f.ef_, no_link);
    base_t::snd_.post(end_spawn_remote_actor_binder(*this, spw, aid));
  }

#ifdef GCE_LUA
  struct spawn_remote_lua_actor_binder
  {
    spawn_remote_lua_actor_binder(
      self_t& self, 
      lua_service_t& svc, 
      spawn_t spw, 
      std::string const& script
      )
      : self_(self)
      , svc_(svc)
      , spw_(spw)
      , script_(script)
    {
    }

    void operator()() const
    {
      self_.spawn_remote_lua_actor(svc_, spw_, script_);
    }

    self_t& self_;
    lua_service_t& svc_;
    spawn_t const spw_;
    std::string const script_;
  };

  void spawn_remote_lua_actor(lua_service_t& svc, spawn_t spw, std::string const& script)
  {
    aid_t aid = svc.spawn_actor(script, aid_nil, no_link);
    base_t::snd_.post(end_spawn_remote_actor_binder(*this, spw, aid));
  }
#endif

  void end_spawn_remote_actor(spawn_t spw, aid_t const& aid)
  {
    //pack pk;
    spw_pk_.on_free();
    spawn_error err = spawn_ok;
    if (aid == aid_nil)
    {
      err = spawn_func_not_found;
    }
    send_spawn_ret(&spw, spw_pk_, err, aid, false);
  }

  void send_spawn_ret(
    spawn_t* spw, pack& pk, spawn_error err, aid_t const& aid, bool is_err_ret
    )
  {
    pk.recver_ = spw->get_aid();
    pk.skt_ = spw->get_aid();
    pk.tag_ = spawn_ret_t(err, spw->get_id(), aid);
    pk.is_err_ret_ = is_err_ret;

    message* msg = base_t::alloc_msg();
    msg->set_type(msg_spawn_ret);
    msg->push_tag(
      pk.tag_, pk.recver_, pk.svc_,
      pk.skt_, pk.is_err_ret_
      );
    //send(pk.msg_);
    send(msg);
  }

  void send(message* msg)
  {
    GCE_ASSERT(msg->shared_size() == 0)(*msg);
    if (conn_)
    {
      GCE_ASSERT(skt_)(*msg);
      while (message* m = conn_cache_.pop())
      {
        send_msg(m);
      }
      send_msg(msg);
    }
    else
    {
      conn_cache_.push(msg);
    }
  }

  void send_msg(message* msg)
  {
    GCE_ASSERT(skt_)(*msg);
    skt_->send(msg);
  }

  struct send_msg_hb_binder
  {
    explicit send_msg_hb_binder(self_t& self)
      : self_(self)
    {
    }

    void operator()() const
    {
      self_.send_msg_hb();
    }

    self_t& self_;
  };

  void send_msg_hb()
  {
    message* msg = base_t::alloc_msg();
    msg->set_type(msg_hb);
    send(msg);
  }

  struct send_ret_binder
  {
    send_ret_binder(
      self_t& self, 
      aid_t const& sire, 
      ctxid_pair_t const& ctxid_pr, 
      std::string const& ep,
      errcode_t& ec
      )
      : self_(self)
      , sire_(sire)
      , ctxid_pr_(ctxid_pr)
      , ep_(ep)
      , ec_(ec)
    {
    }

    void operator()() const
    {
      self_.send_ret(sire_, ctxid_pr_, ep_, ec_);
    }

    self_t& self_;
    aid_t const& sire_;
    ctxid_pair_t const& ctxid_pr_;
    std::string const& ep_;
    errcode_t& ec_;
  };

  void send_ret(aid_t const& sire, ctxid_pair_t ctxid_pr, std::string const& ep, errcode_t& ec)
  {
    gce::detail::send(*this, sire, msg_new_conn, ctxid_pr, ec);
    send_conn_update(sire, ep, ec);
  }

  void send_conn_update(aid_t const& sire, std::string const& ep, errcode_t ec)
  {
    gce::detail::send(*this, sire, msg_conn_update, ep, ec);
  }

  void send_reconn_err(aid_t const& sire, std::string const& ep, errcode_t ec)
  {
    gce::detail::send(*this, sire, msg_reconn_err, ep, ec);
  }

  void run_conn(aid_t const& sire, ctxid_pair_t target, std::string const& ep, yield_t yld)
  {
    yld_ = boost::in_place(yld);
    exit_code_t exc = exit_normal;
    std::string exit_msg("exit normal");
    ctxid_pair_t curr_pr = target;

    if (!svc_.stopped())
    {
      context_t& ctx = svc_.get_context();
      svc_.add_actor(this);
      ctx.register_socket(target, base_t::get_aid(), actor_socket, svc_.get_index());

      try
      {
        stat_ = on;
        {
          errcode_t ec;
          scope_handler<send_ret_binder> scp(send_ret_binder(*this, sire, target, ep, ec));
          skt_ = make_socket(ep);
          ec = connect(true);
        }

        if (!conn_)
        {
          errcode_t ec = connect();
          if (ec)
          {
            send_reconn_err(sire, ep, ec);
          }
          if (conn_)
          {
            send_conn_update(sire, ep, ec);
          }
        }

        //GCE_INFO(lg_) << base_t::ctxid_ << " connected to " << target.first << " ok";

        message msg;
        while (stat_ == on)
        {
          msg.clear();
          errcode_t ec = recv(msg);
          if (ec)
          {
            if (conn_)
            {
              send_conn_update(sire, ep, ec);
            }
            on_neterr(base_t::get_aid(), ec);
            --curr_reconn_;
            if (curr_reconn_ == 0)
            {
              exc = exit_neterr;
              exit_msg = ec.message();
              close();
              break;
            }

            /// remove self from socket list
            aid_t const& skt = base_t::get_aid();
            svc_.disconn_socket(curr_pr, skt);
            ctx.disconn_socket(curr_pr, skt, actor_socket, svc_.get_index());

            /// try reconnect
            ec = connect();
            if (!ec)
            {
              curr_reconn_ = opt_.reconn_max_try;
              send_conn_update(sire, ep, ec);
              svc_.conn_socket(curr_pr, skt);
              ctx.conn_socket(curr_pr, skt, actor_socket, svc_.get_index());
            }
            else
            {
              send_reconn_err(sire, ep, ec);
            }
          }
          else
          {
            match_t type = msg.get_type();
            if (type == msg_login_ret)
            {
              ctxid_pair_t ctxid_pr;
              msg >> ctxid_pr;
              curr_pr = sync_ctxid(ctxid_pr, curr_pr);

              /// send cached msgs
              send_cache_msg();
            }
            else if (type != msg_hb)
            {
              handle_net_msg(msg);
            }
            hb_.beat();
          }
        }
      }
      catch (std::exception& ex)
      {
        exc = exit_except;
        exit_msg = ex.what();
        GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
        close();
      }
      catch (...)
      {
        exc = exit_except;
        exit_msg = boost::current_exception_diagnostic_information();
        GCE_ERROR(lg_)(__FILE__)(__LINE__) << exit_msg;
        close();
      }
    }
    else
    {
      errcode_t ec = 
        boost::asio::error::make_error_code(
          boost::asio::error::operation_aborted
          );
      gce::detail::send(*this, sire, msg_new_conn, target, ec);
    }
    quit(curr_pr, exc, exit_msg);
  }

  struct close_binder
  {
    explicit close_binder(self_t& self)
      : self_(self)
    {
    }

    void operator()() const
    {
      self_.close();
    }

    self_t& self_;
  };

  void run(socket_ptr skt, yield_t yld)
  {
    yld_ = boost::in_place(yld);
    exit_code_t exc = exit_normal;
    std::string exit_msg("exit normal");
    ctxid_pair_t curr_pr =
      std::make_pair(
        ctxid_nil,
        is_router_ ? socket_joint : socket_bind
        );

    if (!svc_.stopped())
    {
      svc_.add_actor(this);

      try
      {
        stat_ = on;
        skt_ = skt;
        skt_->init(svc_.get_strand(), base_t::get_msg_pool());
        start_heartbeat(close_binder(*this));

        message msg;
        while (stat_ == on)
        {
          msg.clear();
          errcode_t ec = recv(msg);
          if (ec)
          {
            on_neterr(base_t::get_aid(), ec);
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
              ctxid_pair_t ctxid_pr =
                std::make_pair(
                  ctxid_nil,
                  is_router_ ? socket_joint : socket_bind
                  );
              msg >> ctxid_pr.first;
              curr_pr = sync_ctxid(ctxid_pr, curr_pr);

              //message m(msg_login_ret);
              message* m = base_t::alloc_msg();
              m->set_type(msg_login_ret);
              *m << std::make_pair(
                base_t::ctxid_,
                is_router_ ? socket_router : socket_conn
                );

              send(m);
            }
            else if (type != msg_hb)
            {
              handle_net_msg(msg);
            }
            hb_.beat();
          }
        }
      }
      catch (std::exception& ex)
      {
        exc = exit_except;
        exit_msg = ex.what();
        GCE_ERROR(lg_)(__FILE__)(__LINE__) << exit_msg;
        close();
      }
      catch (...)
      {
        exc = exit_except;
        exit_msg = boost::current_exception_diagnostic_information();
        GCE_ERROR(lg_)(__FILE__)(__LINE__) << exit_msg;
        close();
      }
    }
    quit(curr_pr, exc, exit_msg);
  }

  socket_ptr make_socket(std::string const& ep)
  {
    /// find protocol name
    size_t pos = ep.find("://");
    GCE_VERIFY(pos != std::string::npos)(ep)
      .log(lg_, "protocol name parse failed");

    std::string prot_name = ep.substr(0, pos);
    if (prot_name == "tcp")
    {
      /// parse address
      size_t begin = pos + 3;
      pos = ep.find(':', begin);
      GCE_VERIFY(pos != std::string::npos)(ep)
        .log(lg_, "tcp address parse failed");

      std::string address = ep.substr(begin, pos - begin);

      /// parse port
      begin = pos + 1;
      pos = ep.size();

      std::string port = ep.substr(begin, pos - begin);
      strand_t& snd = svc_.get_strand();
      socket_ptr skt(
        new tcp::socket(
          snd.get_io_service(),
          address, port
          )
        );
      skt->init(snd, base_t::get_msg_pool());
      return skt;
    }
    else
    {
      GCE_VERIFY(false)(prot_name)
        .log(lg_, "gce::unsupported_protocol_exception")
        .except<unsupported_protocol_exception>();
      // just suppress vc's warning
      throw 1;
    }
  }

  void handle_recv(pack& pk)
  {
    /*message& msg = pk.getmsg();*/
    message* msg = handle_pack(pk);
    match_t ty = msg->get_type();
    //GCE_ASSERT(!check_local(pk.recver_, base_t::ctxid_))(pk.recver_)(base_t::ctxid_);
    if (link_t* link = boost::get<link_t>(&pk.tag_))
    {
      add_straight_link(link->get_aid(), pk.recver_);
    }
    else if (exit_t* ex = boost::get<exit_t>(&pk.tag_))
    {
      remove_straight_link(ex->get_aid(), pk.recver_);
    }
    else if (fwd_link_t* link = boost::get<fwd_link_t>(&pk.tag_))
    {
      add_router_link(link->get_aid(), pk.recver_, link->get_skt());
      pk.tag_ = link_t(link->get_type(), link->get_aid());
    }
    else if (fwd_exit_t* ex = boost::get<fwd_exit_t>(&pk.tag_))
    {
      remove_router_link(ex->get_aid(), pk.recver_);
      pk.tag_ = exit_t(ex->get_code(), ex->get_aid());
    }
    msg->push_tag(
      pk.tag_, pk.recver_, pk.svc_,
      pk.skt_, pk.is_err_ret_
      );

    send(msg);
    /*if (pk.pmsg_ != 0)
    {
      base_t::free_msg(pk.pmsg_);
    }*/
  }

  message* handle_pack(pack& pk)
  {
    if (pk.pmsg_ != 0)
    {
      return pk.pmsg_;
    }
    else
    {
      message* msg = base_t::alloc_msg();
      *msg = pk.msg_;
      return msg;
    }
  }

  void add_straight_link(aid_t const& src, aid_t const& des)
  {
    if (des != aid_nil)
    {
      if (src != aid_nil)
      {
        std::pair<typename straight_link_list_t::iterator, bool> pr =
          straight_link_list_.insert(std::make_pair(src, straight_dummy_));
        pr.first->second.aid_list_.insert(des);
      }
      else if (src.svc_ != svcid_nil)
      {
        std::pair<typename svc_straight_link_list_t::iterator, bool> pr =
          svc_straight_link_list_.insert(std::make_pair(src.svc_, straight_dummy_));
        pr.first->second.aid_list_.insert(des);
      }
    }
    else if (des.svc_ != svcid_nil)
    {
      if (src != aid_nil)
      {
        std::pair<typename straight_link_list_t::iterator, bool> pr =
          straight_link_list_.insert(std::make_pair(src, straight_dummy_));
        pr.first->second.svcid_list_.insert(des.svc_);
      }
      else if (src.svc_ != svcid_nil)
      {
        std::pair<typename svc_straight_link_list_t::iterator, bool> pr =
          svc_straight_link_list_.insert(std::make_pair(src.svc_, straight_dummy_));
        pr.first->second.svcid_list_.insert(des.svc_);
      }
    }
  }

  bool remove_straight_link(aid_t const& src, aid_t const& des)
  {
    bool rt = false;
    if (des != aid_nil)
    {
      if (src != aid_nil)
      {
        typename straight_link_list_t::iterator itr(
          straight_link_list_.find(src)
          );
        if (itr != straight_link_list_.end())
        {
          std::set<aid_t>& aid_list = itr->second.aid_list_;
          std::set<aid_t>::iterator it(aid_list.find(des));
          if (it != aid_list.end())
          {
            aid_list.erase(it);
            rt = true;
          }
        }
      }

      if (src.svc_ != svcid_nil)
      {
        typename svc_straight_link_list_t::iterator itr(
          svc_straight_link_list_.find(src.svc_)
          );
        if (itr != svc_straight_link_list_.end())
        {
          std::set<aid_t>& aid_list = itr->second.aid_list_;
          std::set<aid_t>::iterator it(aid_list.find(des));
          if (it != aid_list.end())
          {
            aid_list.erase(it);
            rt = true;
          }
        }
      }
    }

    if (des.svc_ != svcid_nil)
    {
      if (src != aid_nil)
      {
        typename straight_link_list_t::iterator itr(
          straight_link_list_.find(src)
          );
        if (itr != straight_link_list_.end())
        {
          std::set<svcid_t>& svcid_list = itr->second.svcid_list_;
          std::set<svcid_t>::iterator it(svcid_list.find(des.svc_));
          if (it != svcid_list.end())
          {
            svcid_list.erase(it);
            rt = true;
          }
        }
      }

      if (src.svc_ != svcid_nil)
      {
        typename svc_straight_link_list_t::iterator itr(
          svc_straight_link_list_.find(src.svc_)
          );
        if (itr != svc_straight_link_list_.end())
        {
          std::set<svcid_t>& svcid_list = itr->second.svcid_list_;
          std::set<svcid_t>::iterator it(svcid_list.find(des.svc_));
          if (it != svcid_list.end())
          {
            svcid_list.erase(it);
            rt = true;
          }
        }
      }
    }
    return rt;
  }

  void add_router_link(aid_t const& src, aid_t const& des, sktaid_t skt)
  {
    if (des != aid_nil)
    {
      if (src != aid_nil)
      {
        std::pair<typename router_link_list_t::iterator, bool> pr =
          router_link_list_.insert(std::make_pair(src, router_dummy_));
        pr.first->second.aid_list_.insert(std::make_pair(des, skt));
      }
      else if (src.svc_ != svcid_nil)
      {
        std::pair<typename svc_router_link_list_t::iterator, bool> pr =
          svc_router_link_list_.insert(std::make_pair(src.svc_, router_dummy_));
        pr.first->second.aid_list_.insert(std::make_pair(des, skt));
      }
    }
    else if (des.svc_ != svcid_nil)
    {
      if (src != aid_nil)
      {
        std::pair<typename router_link_list_t::iterator, bool> pr =
          router_link_list_.insert(std::make_pair(src, router_dummy_));
        pr.first->second.svcid_list_.insert(std::make_pair(des.svc_, skt));
      }
      else if (src.svc_ != svcid_nil)
      {
        std::pair<typename svc_router_link_list_t::iterator, bool> pr =
          svc_router_link_list_.insert(std::make_pair(src.svc_, router_dummy_));
        pr.first->second.svcid_list_.insert(std::make_pair(des.svc_, skt));
      }
    }
  }

  sktaid_t remove_router_link(aid_t const& src, aid_t const& des)
  {
    sktaid_t skt = aid_nil;
    if (des != aid_nil)
    {
      if (src != aid_nil)
      {
        typename router_link_list_t::iterator itr(
          router_link_list_.find(src)
          );
        if (itr != router_link_list_.end())
        {
          std::map<aid_t, sktaid_t>& skt_list = itr->second.aid_list_;
          std::map<aid_t, sktaid_t>::iterator skt_itr(skt_list.find(des));
          if (skt_itr != skt_list.end())
          {
            skt = skt_itr->second;
            skt_list.erase(skt_itr);
          }
        }
      }

      if (src.svc_ != svcid_nil)
      {
        typename svc_router_link_list_t::iterator itr(
          svc_router_link_list_.find(src.svc_)
          );
        if (itr != svc_router_link_list_.end())
        {
          std::map<aid_t, sktaid_t>& skt_list = itr->second.aid_list_;
          std::map<aid_t, sktaid_t>::iterator skt_itr(skt_list.find(des));
          if (skt_itr != skt_list.end())
          {
            skt = skt_itr->second;
            skt_list.erase(skt_itr);
          }
        }
      }
    }

    if (des.svc_ != svcid_nil)
    {
      if (src != aid_nil)
      {
        typename router_link_list_t::iterator itr(
          router_link_list_.find(src)
          );
        if (itr != router_link_list_.end())
        {
          std::map<svcid_t, sktaid_t>& skt_list = itr->second.svcid_list_;
          std::map<svcid_t, sktaid_t>::iterator skt_itr(skt_list.find(des.svc_));
          if (skt_itr != skt_list.end())
          {
            skt = skt_itr->second;
            skt_list.erase(skt_itr);
          }
        }
      }

      if (src.svc_ != svcid_nil)
      {
        typename svc_router_link_list_t::iterator itr(
          svc_router_link_list_.find(src.svc_)
          );
        if (itr != svc_router_link_list_.end())
        {
          std::map<svcid_t, sktaid_t>& skt_list = itr->second.svcid_list_;
          std::map<svcid_t, sktaid_t>::iterator skt_itr(skt_list.find(des.svc_));
          if (skt_itr != skt_list.end())
          {
            skt = skt_itr->second;
            skt_list.erase(skt_itr);
          }
        }
      }
    }
    return skt;
  }

  void on_neterr(aid_t const& self_aid, errcode_t ec = errcode_t())
  {
    conn_ = false;
    //conn_cache_.clear();
    //clear_conn_cache();
    std::string errmsg("net error");
    if (ec)
    {
      errmsg = ec.message();
    }

    message m(exit);
    m << exit_neterr << errmsg;

    BOOST_FOREACH(typename straight_link_list_t::value_type& pr, straight_link_list_)
    {
      BOOST_FOREACH(aid_t const& des, pr.second.aid_list_)
      {
        aid_t const& target = pr.first;
        pack& pk = base_t::basic_svc_.alloc_pack(target);
        pk.tag_ = exit_t(exit_neterr, des);
        pk.recver_ = target;
        pk.skt_ = pr.first;
        //pk.msg_ = m;
        pk.setmsg(m);

        svc_.send(target, pk);
      }

      BOOST_FOREACH(svcid_t const& des, pr.second.svcid_list_)
      {
        aid_t const& target = pr.first;
        pack& pk = base_t::basic_svc_.alloc_pack(target);
        aid_t aid = aid_nil;
        aid.svc_ = des;
        pk.tag_ = exit_t(exit_neterr, aid);
        pk.recver_ = aid;
        pk.skt_ = pr.first;
        //pk.msg_ = m;
        pk.setmsg(m);

        svc_.send(target, pk);
      }
    }
    straight_link_list_.clear();

    BOOST_FOREACH(typename svc_straight_link_list_t::value_type& pr, svc_straight_link_list_)
    {
      aid_t target = base_t::basic_svc_.filter_svcid(pr.first);
      if (target != aid_nil)
      {
        BOOST_FOREACH(aid_t const& des, pr.second.aid_list_)
        {
          //svcid_t const& target = pr.first;
          pack& pk = base_t::basic_svc_.alloc_pack(target);
          pk.tag_ = exit_t(exit_neterr, des);
          pk.recver_ = target;
          pk.skt_ = target;
          //pk.msg_ = m;
          pk.setmsg(m);

          svc_.send(target, pk);
        }

        BOOST_FOREACH(svcid_t const& des, pr.second.svcid_list_)
        {
          pack& pk = base_t::basic_svc_.alloc_pack(target);
          aid_t aid = aid_nil;
          aid.svc_ = des;
          pk.tag_ = exit_t(exit_neterr, aid);
          pk.recver_ = aid;
          pk.skt_ = target;
          //pk.msg_ = m;
          pk.setmsg(m);

          svc_.send(target, pk);
        }
      }
    }
    svc_straight_link_list_.clear();

    typedef std::map<aid_t, sktaid_t> aidskt_list_t;
    typedef std::map<svcid_t, sktaid_t> svcidskt_list_t;
    BOOST_FOREACH(typename router_link_list_t::value_type& pr, router_link_list_)
    {
      BOOST_FOREACH(aidskt_list_t::value_type& des, pr.second.aid_list_)
      {
        aid_t const& target = des.second;
        pack& pk = base_t::basic_svc_.alloc_pack(target);
        pk.tag_ = fwd_exit_t(exit_neterr, des.first, self_aid);
        pk.recver_ = pr.first;
        pk.skt_ = target;
        //pk.msg_ = m;
        pk.setmsg(m);

        svc_.send(target, pk);
      }

      BOOST_FOREACH(svcidskt_list_t::value_type& des, pr.second.svcid_list_)
      {
        aid_t const& target = des.second;
        pack& pk = base_t::basic_svc_.alloc_pack(target);
        aid_t aid = aid_nil;
        aid.svc_ = des.first;
        pk.tag_ = fwd_exit_t(exit_neterr, aid, self_aid);
        pk.recver_ = pr.first;
        pk.skt_ = target;
        //pk.msg_ = m;
        pk.setmsg(m);

        svc_.send(target, pk);
      }
    }
    router_link_list_.clear();

    BOOST_FOREACH(typename svc_router_link_list_t::value_type& pr, svc_router_link_list_)
    {
      BOOST_FOREACH(aidskt_list_t::value_type& des, pr.second.aid_list_)
      {
        aid_t const& target = des.second;
        pack& pk = base_t::basic_svc_.alloc_pack(target);
        aid_t aid = aid_nil;
        aid.svc_ = pr.first;
        pk.tag_ = fwd_exit_t(exit_neterr, des.first, self_aid);
        pk.recver_ = aid;
        pk.skt_ = target;
        //pk.msg_ = m;
        pk.setmsg(m);

        svc_.send(target, pk);
      }

      BOOST_FOREACH(svcidskt_list_t::value_type& des, pr.second.svcid_list_)
      {
        aid_t const& target = des.second;
        pack& pk = base_t::basic_svc_.alloc_pack(target);
        aid_t aid = aid_nil;
        aid.svc_ = des.first;
        pk.tag_ = fwd_exit_t(exit_neterr, aid, self_aid);
        pk.recver_ = aid;
        pk.skt_ = target;
        //pk.msg_ = m;
        pk.setmsg(m);

        svc_.send(target, pk);
      }
    }
    svc_router_link_list_.clear();
  }

  ctxid_pair_t sync_ctxid(ctxid_pair_t new_pr, ctxid_pair_t curr_pr)
  {
    if (new_pr != curr_pr)
    {
      context_t& ctx = svc_.get_context();
      aid_t skt = base_t::get_aid();
      svc_.deregister_socket(curr_pr, skt);
      svc_.register_socket(new_pr, skt);
      ctx.deregister_socket(curr_pr, skt, actor_socket, svc_.get_index());
      ctx.register_socket(new_pr, skt, actor_socket, svc_.get_index());
    }
    return new_pr;
  }

  void clear_conn_cache()
  {
    while (message* msg = conn_cache_.pop())
    {
      base_t::free_msg(msg);
    }
  }

private:
  struct recv_buffer
  {
    recv_buffer()
      : data_((byte_t*)std::malloc(GCE_SOCKET_RECV_BUFFER_MIN_SIZE))
      , size_(GCE_SOCKET_RECV_BUFFER_MIN_SIZE)
    {
    }

    ~recv_buffer()
    {
      if (data_)
      {
        std::free(data_);
      }
    }

    void resize(size_t size)
    {
      if (size_ < size)
      {
        void* p = std::realloc(data_, size);
        if (!p)
        {
          throw std::bad_alloc();
        }
        data_ = (byte_t*)p;
      }

      if (size > GCE_SOCKET_RECV_BUFFER_MIN_SIZE)
      {
        size_ = size;
      }
      else
      {
        size_ = GCE_SOCKET_RECV_BUFFER_MIN_SIZE;
      }
    }

    void reset(size_t size)
    {
      if (data_)
      {
        std::free(data_);
        data_ = 0;
      }

      if (size < GCE_SOCKET_RECV_BUFFER_MIN_SIZE)
      {
        size = GCE_SOCKET_RECV_BUFFER_MIN_SIZE;
      }

      void* p = std::malloc(size);
      if (!p)
      {
        throw std::bad_alloc();
      }
      data_ = (byte_t*)p;
      size_ = size;
    }

    byte_t* data()
    {
      return data_;
    }

    size_t size() const
    {
      return size_;
    }

    byte_t* data_;
    size_t size_;
  };

  struct resume_binder
  {
    explicit resume_binder(self_t& self)
      : self_(self)
    {
    }

    void operator()() const
    {
      self_.resume();
    }

    self_t& self_;
  };

  struct host
    : public yielder::basic_host
  {
    explicit host(self_t* self)
      : self_(self)
    {
    }

    void yield()
    {
      self_->yield();
    }

    void resume()
    {
      self_->snd_.dispatch(resume_binder(*self_));
    }

    self_t* self_;
  };

  typedef boost::asio::detail::async_result_init<yield_t, void ()> async_result_init_t;

  void resume()
  {
    GCE_ASSERT(yld_cb_);
    yld_cb_();

    base_t::ctx_.on_tick(svc_.get_index());
  }

  void yield()
  {
    GCE_ASSERT(yld_);
    async_result_init_t init(BOOST_ASIO_MOVE_CAST(yield_t)(*yld_));

    yld_cb_ = init.handler;
    return init.result.get();
  }

  void adjust_recv_buffer(size_t expect_size)
  {
    size_t const remain_size = recv_cache_.remain_read_size();
    if (remain_size <= GCE_SOCKET_RECV_BUFFER_COPY_SIZE)
    {
      /// copy unread data to recv buffer's head, if total size > buffer size reset it
      size_t const copy_size = remain_size;
      if (expect_size <= recv_buffer_.size())
      {
        std::memmove(recv_buffer_.data(), recv_cache_.get_read_data(), copy_size);
      }
      else
      {
        byte_t tmp[GCE_SOCKET_RECV_BUFFER_COPY_SIZE];
        std::memcpy(tmp, recv_cache_.get_read_data(), copy_size);
        recv_buffer_.reset(expect_size);
        std::memcpy(recv_buffer_.data(), tmp, copy_size);
      }
      recv_cache_.clear();
      recv_cache_.write(copy_size);
    }
    else
    {
      /// extend recv buffer
      size_t const new_size = recv_cache_.read_size() + expect_size;
      GCE_ASSERT(new_size >= recv_cache_.write_size())(new_size)(recv_cache_.write_size());
      recv_buffer_.resize(new_size);
      recv_cache_.reset(recv_buffer_.data(), recv_buffer_.size());
    }
  }

  bool parse_message(message& msg)
  {
    if (recving_header_)
    {
      byte_t* data = recv_cache_.get_read_data();
      size_t const remain_size = recv_cache_.remain_read_size();

      pkr_.clear();
      pkr_.set_read(data, remain_size);
      packer::error_code_t ec = packer::ok();
      pkr_.read(curr_hdr_, ec);
      if (ec != packer::ok())
      {
        pkr_.clear();
        if (recv_cache_.size() - recv_cache_.read_size() < GCE_MSG_HEADER_MAX_SIZE)
        {
          adjust_recv_buffer(GCE_MSG_HEADER_MAX_SIZE);
        }
        return false;
      }
      recving_header_ = false;
      recv_cache_.read(pkr_.read_length());
    }

    size_t const remain_size = recv_cache_.remain_read_size();
    if (remain_size < curr_hdr_.size_)
    {
      if (!recving_msg_)
      {
        if (curr_hdr_.size_ - remain_size >= GCE_SOCKET_BIG_MSG_SIZE)
        {
          msg = message(curr_hdr_.type_, curr_hdr_.tag_offset_);

          /// change msg to large msg
          msg.to_large(curr_hdr_.size_ + GCE_MSG_HEADER_MAX_SIZE);

          /// copy already recved data to msg
          msg << message::chunk(recv_cache_.get_read_data(), remain_size);

          /// reset recv_cache_ to msg buffer
          recv_cache_.clear();
          recv_cache_.reset(const_cast<byte_t*>(msg.data()), curr_hdr_.size_ + GCE_MSG_HEADER_MAX_SIZE);
          recv_cache_.write(remain_size);

          /// prepare for writing last data
          pre_write_size_ = curr_hdr_.size_ - remain_size;
          body_pkr_.clear();
          msg.pre_write(body_pkr_, pre_write_size_);

          /// switch to recving_msg_ mode
          recving_msg_ = true;
        }
      }

      if (!recving_msg_)
      {
        if (recv_cache_.size() - recv_cache_.read_size() < curr_hdr_.size_)
        {
          adjust_recv_buffer(curr_hdr_.size_);
        }
      }
      return false;
    }

    if (!recving_msg_)
    {
      byte_t* data = recv_cache_.get_read_data();
      recv_cache_.read(curr_hdr_.size_);
      msg = message(curr_hdr_.type_, data, curr_hdr_.size_, curr_hdr_.tag_offset_);
    }
    else
    {
      //packer& pkr = msg.get_packer();
      body_pkr_.skip_write(pre_write_size_);
      msg.end_write(body_pkr_);

      recv_cache_.read(curr_hdr_.size_);
      byte_t* data = recv_cache_.get_read_data();
      size_t const remain_size = recv_cache_.remain_read_size();
      GCE_ASSERT(remain_size < recv_buffer_.size())(remain_size);

      recv_cache_.clear();
      recv_cache_.reset(recv_buffer_.data(), recv_buffer_.size());
      std::memcpy(recv_cache_.get_write_data(), data, remain_size);
      recv_cache_.write(remain_size);
      recving_msg_ = false;
    }
    recving_header_ = true;
    return true;
  }

  struct reconn_binder
  {
    explicit reconn_binder(self_t& self)
      : self_(self)
    {
    }

    void operator()() const
    {
      self_.reconn();
    }

    self_t& self_;
  };

  errcode_t connect(bool init = false)
  {
    errcode_t ec;
    if (stat_ == on)
    {
      if (!init && opt_.reconn_wait_period > zero)
      {
        errcode_t ignored_ec;
        sync_.expires_from_now(to_chrono(opt_.reconn_wait_period));
        //sync_.async_wait(yield[ignored_ec]);
        yielder ylder(host_, ignored_ec);
        sync_.async_wait(ylder);
        ylder.yield();
        if (stat_ != on)
        {
          if (!ec)
          {
            ec = boost::system::errc::make_error_code(boost::system::errc::state_not_recoverable);
          }
          return ec;
        }
      }

      duration_t reconn_period = 
        init ? opt_.init_reconn_period : opt_.reconn_period;
      size_t const reconn_try = 
        init ? (size_t)opt_.init_reconn_try : (size_t)opt_.reconn_try;
      for (size_t i=0, retry=0; i<u32_nil; ++i, ++retry)
      {
        if (retry > reconn_try)
        {
          retry = 0;
          on_neterr(base_t::get_aid());
          //if (init)
          {
            if (!ec)
            {
              ec = boost::system::errc::make_error_code(boost::system::errc::state_not_recoverable);
            }
            return ec;
          }
        }

        if (i > 0)
        {
          errcode_t ignored_ec;
          sync_.expires_from_now(to_chrono(reconn_period));
          //sync_.async_wait(yield[ignored_ec]);
          yielder ylder(host_, ignored_ec);
          sync_.async_wait(ylder);
          ylder.yield();
          if (stat_ != on)
          {
            break;
          }
        }

        //skt_->connect(yield[ec]);
        skt_->connect(yielder(host_, ec));
        if (!ec || stat_ != on)
        {
          recv_cache_.clear();
          recving_header_ = true;
          recving_msg_ = false;
          break;
        }
      }

      if (stat_ != on)
      {
        if (!ec)
        {
          ec = boost::system::errc::make_error_code(boost::system::errc::state_not_recoverable);
        }
        return ec;
      }

      GCE_VERIFY(!ec)(ec.value()).msg(ec.message().c_str());

      conn_ = true;
      start_heartbeat(reconn_binder(*this));

      send_login();
    }
    return ec;
  }

  errcode_t recv(message& msg)
  {
    errcode_t ec;
    yielder ylder(host_, ec);
    while (stat_ != off && !parse_message(msg))
    {
      size_t size =
        skt_->recv(
          recv_cache_.get_write_data(),
          recv_cache_.remain_write_size(),
          ylder
          );
      if (ec)
      {
        break;
      }

      recv_cache_.write(size);
    }

    if (stat_ == off && !ec)
    {
      ec =
        boost::asio::error::make_error_code(
          boost::asio::error::operation_aborted
          );
    }

    return ec;
  }

  void send_login(bool immediate = true)
  {
    //message m(msg_login);
    message* m = base_t::alloc_msg();
    m->set_type(msg_login);
    *m << base_t::ctxid_;

    if (immediate)
    {
      send_msg(m);
    }
    else
    {
      send(m);
    }
  }

  void send_cache_msg()
  {
    if (!conn_)
    {
      return;
    }

    while (message* m = conn_cache_.pop())
    {
      send_msg(m);
    }
  }

  void close()
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

  void reconn()
  {
    skt_->reset();
  }

  template <typename F>
  void start_heartbeat(F const& f)
  {
    hb_.init(
      opt_.heartbeat_period, (size_t)opt_.heartbeat_count,
      f, send_msg_hb_binder(*this)
      );
    hb_.start();
  }

  void quit(ctxid_pair_t ctxid_pr, exit_code_t exc, std::string const& exit_msg)
  {
    yielder ylder(host_);
    hb_.wait_end(ylder);
    if (skt_)
    {
      skt_->wait_end(ylder);
    }

    skt_.reset();

    hb_.clear();
    if (ctxid_pr.first != ctxid_nil)
    {
      svc_.deregister_socket(ctxid_pr, base_t::get_aid());
      base_t::ctx_.deregister_socket(ctxid_pr, base_t::get_aid(), actor_socket, svc_.get_index());
    }

    svc_.remove_actor(this);
    aid_t self_aid = base_t::get_aid();
    on_neterr(self_aid);
    base_t::send_exit(self_aid, exc, exit_msg);
    base_t::snd_.post(free_self_binder(*this));
  }

  struct free_self_binder
  {
    explicit free_self_binder(self_t& self)
      : self_(self)
    {
    }

    void operator()() const
    {
      self_.free_self();
    }

    self_t& self_;
  };

  void free_self()
  {
    svc_.free_actor(this);
    base_t::ctx_.on_tick(svc_.get_index());
  }

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(status, stat_)
  GCE_CACHE_ALIGNED_VAR(netopt_t, opt_)

  /// coro local vars
  host host_;
  service_t& svc_;
  socket_ptr skt_;
  heartbeat hb_;
  timer_t sync_;
  size_t tmr_sid_;

  typedef boost::function<void ()> yield_cb_t;
  //typedef typename boost::asio::handler_type<yield_t, void ()>::type yield_cb_t;
  boost::optional<yield_t> yld_;
  yield_cb_t yld_cb_;

  recv_buffer recv_buffer_;
  buffer_ref recv_cache_;
  bool recving_header_;
  bool recving_msg_;
  size_t pre_write_size_;
  header_t curr_hdr_;

  bool conn_;
  //std::deque<message> conn_cache_;
  linked_queue<message> conn_cache_;
  size_t curr_reconn_;

  /// remote links
  struct straight_set
  {
    std::set<aid_t> aid_list_;
    std::set<svcid_t> svcid_list_;
  };
  typedef std::map<aid_t, straight_set> straight_link_list_t;
  straight_link_list_t straight_link_list_;
  typedef std::map<svcid_t, straight_set> svc_straight_link_list_t;
  svc_straight_link_list_t svc_straight_link_list_;
  straight_set const straight_dummy_;

  struct router_set
  {
    std::map<aid_t, sktaid_t> aid_list_;
    std::map<svcid_t, sktaid_t> svcid_list_;
  };
  typedef std::map<aid_t, router_set> router_link_list_t;
  router_link_list_t router_link_list_;
  typedef std::map<svcid_t, router_set> svc_router_link_list_t;
  svc_router_link_list_t svc_router_link_list_;
  router_set const router_dummy_;

  /// remote spawn's funcs
  remote_func_list_t remote_func_list_;

  bool is_router_;
  log::logger_t& lg_;

  packer pkr_;
  packer body_pkr_;

  pack pk_;
  pack spw_pk_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_SOCKET_ACTOR_HPP
