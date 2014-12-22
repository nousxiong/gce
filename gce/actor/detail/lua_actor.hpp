///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_LUA_ACTOR_HPP
#define GCE_ACTOR_DETAIL_LUA_ACTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/net_option.hpp>
#include <gce/actor/exception.hpp>
#include <gce/actor/detail/remote.hpp>
#include <gce/actor/detail/service.hpp>
#include <gce/actor/detail/lua_service.hpp>
#include <gce/actor/detail/basic_actor.hpp>
#include <gce/actor/detail/actor_ref.hpp>
#include <gce/actor/pattern.hpp>

namespace gce
{
namespace detail
{
template <typename Context>
class lua_actor
  : public basic_actor<Context>
{
public:
  typedef Context context_t;

private:
  typedef basic_actor<context_t> base_t;
  typedef lua_actor<context_t> self_t;
  typedef lua_service<self_t> service_t;

public:
  lua_actor(aid_t aid, service_t& svc)
    : base_t(svc.get_context(), svc, actor_luaed, aid)
    , L_(svc.get_lua_state())
    , coro_(L_)
    , func_(L_)
    , svc_(svc)
    , yielding_(false)
    , recving_(false)
    , responsing_(false)
    , tmr_(base_t::ctx_.get_io_service())
    , tmr_sid_(0)
    , lg_(base_t::ctx_.get_logger())
  {
  }

  ~lua_actor()
  {
  }

public:
  void send(aid_t recver, message const& m)
  {
    base_t::pri_send(recver, m);
  }

  void send2svc(svcid_t recver, message const& m)
  {
    base_t::pri_send_svc(recver, m);
  }

  void relay(aid_t des, message& m)
  {
    base_t::pri_relay(des, m);
  }

  void relay2svc(svcid_t des, message& m)
  {
    base_t::pri_relay_svc(des, m);
  }

  resp_t request(aid_t recver, message const& m)
  {
    resp_t res(base_t::new_request(), base_t::get_aid(), recver);
    base_t::pri_request(res, recver, m);
    return res;
  }

  resp_t request2svc(svcid_t recver, message const& m)
  {
    resp_t res(base_t::new_request(), base_t::get_aid(), recver);
    base_t::pri_request_svc(res, recver, m);
    return res;
  }

  void reply(aid_t recver, message const& m)
  {
    base_t::pri_reply(recver, m);
  }

  void link(aid_t target)
  {
    base_t::pri_link(target);
  }

  void monitor(aid_t target)
  {
    base_t::pri_monitor(target);
  }

public:
  bool recv()
  {
    return recv_match(pattern());
  }

  bool recv_match(pattern const& patt)
  {
    aid_t sender;
    message msg;
    return pri_recv_match(patt, sender, msg);
  }

  bool recv_response(resp_t res)
  {
    return recv_response_timeout(res, seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC));
  }

  bool recv_response_timeout(resp_t res, duration_type tmo)
  {
    aid_t sender;
    message msg;
    duration_t dur(tmo.dur_);

    if (!base_t::mb_.pop(res, msg))
    {
      if (dur > zero)
      {
        if (dur < infin)
        {
          start_timer(dur);
        }
        responsing_ = true;
        recving_res_ = res;
        yield();
        return true;
      }
    }
    else
    {
      sender = end_recv(res);
    }

    set_recv_global(sender, msg);
    return false;
  }

  void sleep_for(duration_type dur)
  {
    start_timer(dur.dur_);
    yield();
  }

  bool bind(std::string const& ep, net_option opt)
  {
    typedef typename context_t::acceptor_service_t acceptor_service_t;
    context_t& ctx = base_t::get_context();
    acceptor_service_t& svc = ctx.select_service<acceptor_service_t>();

    gce::detail::bind<context_t>(base_t::get_aid(), svc, ep, opt);
    bool is_yield = recv_match(pattern(msg_new_bind));
    if (!is_yield)
    {
      handle_bind();
    }
    return is_yield;
  }

  bool connect(match_type target, std::string const& ep, net_option opt)
  {
    typedef typename context_t::socket_service_t socket_service_t;
    context_t& ctx = base_t::get_context();
    socket_service_t& svc = ctx.select_service<socket_service_t>();

    gce::detail::connect<context_t>(base_t::get_aid(), svc, target, ep, opt);
    message msg;
    aid_t sender;
    bool is_yield =  pri_recv_match(pattern(msg_new_conn), sender, msg);
    if (!is_yield)
    {
      handle_connect(sender, msg);
    }
    return is_yield;
  }

  bool spawn(std::string const& script, bool sync_sire, int type)
  {
    service_t* svc = 0;
    if (sync_sire)
    {
      svc = &svc_;
    }
    else
    {
      svc = &base_t::ctx_.select_service<service_t>();
    }

    svc->get_strand().post(
      boost::bind(
        &service_t::spawn_actor, svc, 
        script, base_t::get_aid(), (link_type)type
        )
      );

    message msg;
    aid_t sender;
    bool is_yield =  pri_recv_match(pattern(msg_new_actor), sender, msg);
    if (!is_yield)
    {
      handle_spawn(sender, msg);
    }
    return is_yield;
  }

  bool spawn_remote(
    spawn_type sty, std::string const& func, match_type ctxid, 
    int type, std::size_t stack_size, seconds_t tmo
    )
  {
    aid_t aid;
    sid_t sid = base_t::new_request();
    base_t::pri_spawn(sid, sty, func, ctxid, stack_size);

    duration_t curr_tmo = tmo;
    typedef boost::chrono::system_clock clock_t;
    clock_t::time_point begin_tp = clock_t::now();
    spw_hdr_ = 
      boost::bind(
        &self_t::handle_remote_spawn, this, _1, _2,
        (link_type)type, begin_tp, sid, tmo, curr_tmo
        );

    aid_t sender;
    message msg;
    bool is_yield =  pri_recv_match(pattern(msg_spawn_ret, curr_tmo), sender, msg);
    if (!is_yield)
    {
      is_yield = pri_handle_remote_spawn(sender, msg);
    }
    return is_yield;
  }

  void register_service(match_type name)
  {
    gce::detail::register_service(base_t::get_aid(), svc_, name);
  }

  void deregister_service(match_type name)
  {
    gce::detail::deregister_service(base_t::get_aid(), svc_, name);
  }

  void log_debug(std::string const& str)
  {
    GCE_DEBUG(lg_) << str;
  }

  void log_info(std::string const& str)
  {
    GCE_INFO(lg_) << str;
  }

  void log_warn(std::string const& str)
  {
    GCE_WARN(lg_) << str;
  }

  void log_error(std::string const& str)
  {
    GCE_ERROR(lg_) << str;
  }

  void log_fatal(std::string const& str)
  {
    GCE_FATAL(lg_) << str;
  }

public:
  /// internal use
  typedef gce::luaed type;

  static actor_type get_type()
  {
    return actor_luaed;
  }

  aid_t get_aid() const
  {
    return base_t::get_aid();
  }

  static std::size_t get_pool_reserve_size(attributes const& attr)
  {
    return attr.actor_pool_reserve_size_;
  }

  service_t& get_service()
  {
    return svc_;
  }

  void set_coro(luabridge::LuaRef co)
  {
    coro_ = co;
  }

  void set_resume(luabridge::LuaRef func)
  {
    func_ = func;
  }

  void init(std::string const& script)
  {
    script_ = script;
  }

  void start()
  {
    try
    {
      luabridge::setGlobal(L_, this, "self");
      luabridge::LuaRef nil(L_);
      luabridge::setGlobal(L_, nil, "gce_curr_co");

      luabridge::LuaRef scr = svc_.get_script(script_);
      if (scr.isNil())
      {
        std::string errmsg;
        errmsg += "gce::lua_exception: ";
        errmsg += lua_tostring(L_, -1);
        GCE_VERIFY(false)(script_)
          .log(lg_, errmsg.c_str()).except<lua_exception>();
      }
      scr();
      quit();
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
      quit(exit_except, ex.what());
    }
  }

  void on_recv(pack& pk)
  {
    handle_recv(pk);
  }

  void quit(exit_code_t exc = exit_normal, std::string const& errmsg = std::string())
  {
    if (!yielding_)
    {
      aid_t self_aid = base_t::get_aid();
      base_t::snd_.post(boost::bind(&self_t::stop, this, self_aid, exc, errmsg));
    }
  }

private:
  bool pri_recv_match(pattern const& patt, aid_t& sender, message& msg)
  {
    recv_t rcv;

    if (!base_t::mb_.pop(rcv, msg, patt.match_list_, patt.recver_))
    {
      duration_t tmo = patt.timeout_;
      if (tmo > zero)
      {
        if (tmo < infin)
        {
          start_timer(tmo);
        }
        recving_ = true;
        curr_pattern_ = patt;
        yield();
        return true;
      }
    }
    else
    {
      sender = end_recv(rcv, msg);
    }
  
    set_recv_global(sender, msg);
    return false;
  }

  void yield()
  {
    if (!yielding_)
    {
      yielding_ = true;
    }
  }

  void resume()
  {
    GCE_ASSERT(yielding_);
    yielding_ = false;
    luabridge::setGlobal(L_, this, "self");
    luabridge::setGlobal(L_, coro_, "gce_curr_co");
    func_();
  }

  void stop(aid_t self_aid, exit_code_t ec, std::string const& exit_msg)
  {
    base_t::send_exit(self_aid, ec, exit_msg);
    svc_.free_actor(this);
  }

  void start_timer(duration_t dur)
  {
    tmr_.expires_from_now(dur);
    tmr_.async_wait(
      base_t::snd_.wrap(
        boost::bind(
          &self_t::handle_timeout, this,
          boost::asio::placeholders::error, ++tmr_sid_
          )
        )
      );
  }

  void handle_timeout(errcode_t const& ec, std::size_t tmr_sid)
  {
    if (!ec && tmr_sid == tmr_sid_)
    {
      recving_ = false;
      responsing_ = false;
      curr_pattern_.clear();
      try
      {
        if (spw_hdr_)
        {
          pri_handle_remote_spawn(nil_aid_, nil_msg_);
        }
        else
        {
          set_recv_global(nil_aid_, nil_msg_);
          resume();
          quit();
        }
      }
      catch (std::exception& ex)
      {
        GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
        quit(exit_except, ex.what());
      }
    }
  }

  aid_t end_recv(recv_t& rcv, message& msg)
  {
    aid_t sender;
    if (aid_t* aid = boost::get<aid_t>(&rcv))
    {
      sender = *aid;
      msg.set_relay(*aid);
    }
    else if (request_t* req = boost::get<request_t>(&rcv))
    {
      sender = req->get_aid();
      msg.set_relay(*req);
    }
    else if (exit_t* ex = boost::get<exit_t>(&rcv))
    {
      sender = ex->get_aid();
    }
    return sender;
  }

  aid_t end_recv(resp_t& res)
  {
    return res.get_aid();
  }

  void handle_recv(pack& pk)
  {
    bool is_response = false;

    if (aid_t* aid = boost::get<aid_t>(&pk.tag_))
    {
      base_t::mb_.push(*aid, pk.msg_);
    }
    else if (request_t* req = boost::get<request_t>(&pk.tag_))
    {
      base_t::mb_.push(*req, pk.msg_);
    }
    else if (link_t* link = boost::get<link_t>(&pk.tag_))
    {
      base_t::add_link(link->get_aid(), pk.skt_);
      return;
    }
    else if (exit_t* ex = boost::get<exit_t>(&pk.tag_))
    {
      base_t::mb_.push(*ex, pk.msg_);
      base_t::remove_link(ex->get_aid());
    }
    else if (resp_t* res = boost::get<resp_t>(&pk.tag_))
    {
      is_response = true;
      base_t::mb_.push(*res, pk.msg_);
    }

    recv_t rcv;
    message msg;
    aid_t sender;
    bool need_resume = false;

    if (
      (recving_ && !is_response) ||
      (responsing_ && is_response)
      )
    {
      if (recving_ && !is_response)
      {
        bool ret = base_t::mb_.pop(rcv, msg, curr_pattern_.match_list_, curr_pattern_.recver_);
        if (!ret)
        {
          return;
        }
        sender = end_recv(rcv, msg);
        curr_pattern_.clear();
        need_resume = true;
        recving_ = false;
      }

      if (responsing_ && is_response)
      {
        GCE_ASSERT(recving_res_.valid());
        bool ret = base_t::mb_.pop(recving_res_, msg);
        if (!ret)
        {
          return;
        }
        sender = end_recv(recving_res_);
        need_resume = true;
        responsing_ = false;
        recving_res_ = nil_resp_;
      }

      ++tmr_sid_;
      errcode_t ec;
      tmr_.cancel(ec);

      match_t msg_type = msg.get_type();
      if (msg_type == msg_new_actor)
      {
        need_resume = false;
        handle_spawn(sender, msg);
      }
      else if (msg_type == msg_spawn_ret)
      {
        need_resume = false;
        pri_handle_remote_spawn(sender, msg);
      }
      else if (msg_type == msg_new_bind)
      {
        need_resume = false;
        handle_bind();
      }
      else if (msg_type == msg_new_conn)
      {
        need_resume = false;
        handle_connect(sender, msg);
      }

      if (need_resume)
      {
        try
        {
          
          set_recv_global(sender, msg);
          resume();
          quit();
        }
        catch (std::exception& ex)
        {
          GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
          quit(exit_except, ex.what());
        }
      }
    }
  }

  void handle_bind()
  {
    try
    {
      if (yielding_)
      {
        resume();
        quit();
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
      quit(exit_except, ex.what());
    }
  }

  void handle_connect(aid_t skt, message& msg)
  {
    try
    {
      ctxid_pair_t ctxid_pr;
      errcode_t ec;
      msg >> ctxid_pr >> ec;

      if (skt)
      {
        svc_.register_socket(ctxid_pr, skt);
      }
      
      luabridge::setGlobal(L_, ec.value(), "gce_conn_ret");
      luabridge::setGlobal(L_, ec.message(), "gce_conn_errmsg");
      if (yielding_)
      {
        resume();
        quit();
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
      quit(exit_except, ex.what());
    }
  }

  void handle_spawn(aid_t aid, message& msg)
  {
    try
    {
      boost::uint16_t ty = u16_nil;
      msg >> ty;
      link_type type = (link_type)ty;

      if (aid)
      {
        if (type == linked)
        {
          link(aid);
        }
        else if (type == monitored)
        {
          monitor(aid);
        }
      }
      
      luabridge::setGlobal(L_, aid, "gce_spawn_aid");
      if (yielding_)
      {
        resume();
        quit();
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
      quit(exit_except, ex.what());
    }
  }

  bool handle_remote_spawn(
    aid_t aid,
    message msg, link_type type,
    boost::chrono::system_clock::time_point begin_tp,
    sid_t sid, seconds_t tmo, duration_t curr_tmo
    )
  {
    boost::uint16_t err = 0;
    sid_t ret_sid = sid_nil;
    if (msg.get_type() != match_nil)
    {
      aid_t sender;
      while (true)
      {
        msg >> err >> ret_sid;
        if (err != 0 || (aid && sid == ret_sid))
        {
          break;
        }

        if (tmo != infin)
        {
          duration_t pass_time = boost::chrono::system_clock::now() - begin_tp;
          curr_tmo -= pass_time;
        }

        begin_tp = boost::chrono::system_clock::now();
        bool is_yield = pri_recv_match(pattern(msg_spawn_ret, curr_tmo), sender, msg);
        if (is_yield)
        {
          spw_hdr_ = 
            boost::bind(
              &self_t::handle_remote_spawn, this, _1, _2,
              type, begin_tp, sid, tmo, curr_tmo
              );
          return true;
        }
      }

      spawn_error error = (spawn_error)err;
      if (error != spawn_ok)
      {
        aid = nil_aid_;
      }

      if (aid)
      {
        if (type == linked)
        {
          link(aid);
        }
        else if (type == monitored)
        {
          monitor(aid);
        }
      }
    }

    if (yielding_)
    {
      try
      {
        luabridge::setGlobal(L_, aid, "gce_spawn_aid");
        resume();
        quit();
      }
      catch (std::exception& ex)
      {
        GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
        quit(exit_except, ex.what());
      }
    }
    return false;
  }

  bool pri_handle_remote_spawn(aid_t const& sender, message const& msg)
  {
    if (spw_hdr_)
    {
      spawn_handler_t hdr(spw_hdr_);
      spw_hdr_.clear();
      return hdr(sender, msg);
    }
    return false;
  }

  void set_recv_global(aid_t const& sender, message const& msg)
  {
    luabridge::setGlobal(L_, sender, "gce_recv_sender");
    luabridge::setGlobal(L_, msg, "gce_recv_msg");
  }

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(lua_State*, L_)
  GCE_CACHE_ALIGNED_VAR(std::string, script_)
  GCE_CACHE_ALIGNED_VAR(luabridge::LuaRef, coro_)
  GCE_CACHE_ALIGNED_VAR(luabridge::LuaRef, func_)

  /// thread local
  service_t& svc_;
  bool yielding_;
  bool recving_;
  bool responsing_;
  resp_t recving_res_;
  pattern curr_pattern_;
  timer_t tmr_;
  std::size_t tmr_sid_;

  typedef boost::function<bool (aid_t, message)> spawn_handler_t;
  spawn_handler_t spw_hdr_;

  aid_t const nil_aid_;
  recv_t const nil_rcv_;
  resp_t const nil_resp_;
  message const nil_msg_;
  log::logger_t& lg_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_LUA_ACTOR_HPP
