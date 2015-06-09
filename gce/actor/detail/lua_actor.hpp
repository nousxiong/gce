///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
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
#include <gce/actor/detail/spawn_actor.hpp>
#include <gce/actor/detail/basic_addon.hpp>
#include <gce/actor/detail/remote.hpp>
#include <gce/actor/detail/service.hpp>
#include <gce/actor/detail/lua_service.hpp>
#include <gce/actor/detail/basic_actor.hpp>
#include <gce/actor/detail/actor_ref.hpp>
#include <gce/actor/pattern.hpp>
#include <boost/utility/string_ref.hpp>

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
  typedef typename context_t::stackful_service_t stackful_service_t;
  typedef typename context_t::stackless_service_t stackless_service_t;

  struct addon
  {
    std::string lib_;
    int k_;
    basic_addon<context_t>* ao_;
  };

public:
  lua_actor(aid_t aid, service_t& svc)
    : base_t(svc.get_context(), svc, actor_luaed, aid)
    , L_(svc.get_lua_state())
    , a_(LUA_REFNIL)
    , co_(LUA_REFNIL)
    , rf_(LUA_REFNIL)
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
  void send(aid_t const& recver, message const& m)
  {
    base_t::pri_send(recver, m);
  }

  void send(svcid_t const& recver, message const& m)
  {
    base_t::pri_send_svc(recver, m);
  }

  void send(match_t recver, message const& m)
  {
    base_t::pri_send_svcs(recver, m);
  }

  void relay(aid_t const& des, message& m)
  {
    base_t::pri_relay(des, m);
  }

  void relay(svcid_t const& des, message& m)
  {
    base_t::pri_relay_svc(des, m);
  }

  void relay(match_t des, message& m)
  {
    base_t::pri_relay_svcs(des, m);
  }

  resp_t request(aid_t const& recver, message const& m)
  {
    resp_t res(base_t::new_request(), base_t::get_aid(), recver);
    base_t::pri_request(res, recver, m);
    return res;
  }

  resp_t request(svcid_t const& recver, message const& m)
  {
    resp_t res(base_t::new_request(), base_t::get_aid(), recver);
    base_t::pri_request_svc(res, recver, m);
    return res;
  }

  resp_t request(match_t recver, message const& m)
  {
    resp_t res(base_t::new_request(), base_t::get_aid());
    base_t::pri_request_svcs(res, recver, m);
    return res;
  }

  void reply(aid_t const& recver, message const& m)
  {
    base_t::pri_reply(recver, m);
  }

  void link(aid_t const& target)
  {
    base_t::pri_link(target);
  }

  void monitor(aid_t const& target)
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

  bool recv_response(resp_t const& res)
  {
    return recv_response_timeout(res, seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC));
  }

  bool recv_response_timeout(resp_t res, duration_t tmo)
  {
    aid_t sender;
    message msg;

    if (!base_t::mb_.pop(res, msg))
    {
      if (tmo > zero)
      {
        if (tmo < infin)
        {
          start_timer(tmo);
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

    int ec = check_result(msg);
    set_recv_result(sender, msg, ec);
    return false;
  }

  bool sleep_for(duration_t dur)
  {
    start_timer(dur);
    yield();
    return true;
  }

  bool bind(std::string const& ep, netopt_t opt)
  {
    typedef typename context_t::acceptor_service_t acceptor_service_t;
    context_t& ctx = base_t::get_context();
    acceptor_service_t& svc = ctx.select_service<acceptor_service_t>();

    gce::detail::bind<context_t>(base_t::get_aid(), svc, ep, opt);
    pattern patt;
    patt.add_match(msg_new_bind);
    bool is_yield = recv_match(patt);
    if (!is_yield)
    {
      handle_bind();
    }
    return is_yield;
  }

  bool connect(ctxid_t target, std::string const& ep, netopt_t opt)
  {
    typedef typename context_t::socket_service_t socket_service_t;
    context_t& ctx = base_t::get_context();
    socket_service_t& svc = ctx.select_service<socket_service_t>();

    gce::detail::connect<context_t>(base_t::get_aid(), svc, target, ep, opt);
    message msg;
    aid_t sender;

    pattern patt;
    patt.add_match(msg_new_conn);
    bool is_yield =  pri_recv_match(patt, sender, msg);
    if (!is_yield)
    {
      handle_connect(sender, msg);
    }
    return is_yield;
  }

  bool spawn(int spw_ty, std::string const& func, bool sync_sire, int type, size_t stack_size)
  {
    spawn_type spw_type = (spawn_type)spw_ty;
    if (spw_type == spw_stackful)
    {
      stackful_service_t* svc = 0;
      if (sync_sire)
      {
        svc = &base_t::ctx_.select_service<stackful_service_t>(svc_.get_index());
      }
      else
      {
        svc = &base_t::ctx_.select_service<stackful_service_t>();
      }

      remote_func<context_t>* f = svc_.get_native_func(func);
      if (f == 0)
      {
        message msg;
        handle_spawn(aid_nil, msg);
        return false;
      }

      svc->get_strand().post(
        boost::bind(
          &self_t::spawn_stackful,
          base_t::get_aid(), svc, f, stack_size, (link_type)type
          )
        );
    }
    else if (spw_type == spw_stackless)
    {
      stackless_service_t* svc = 0;
      if (sync_sire)
      {
        svc = &base_t::ctx_.select_service<stackless_service_t>(svc_.get_index());
      }
      else
      {
        svc = &base_t::ctx_.select_service<stackless_service_t>();
      }

      remote_func<context_t>* f = svc_.get_native_func(func);
      if (f == 0)
      {
        message msg;
        handle_spawn(aid_nil, msg);
        return false;
      }

      svc->get_strand().post(
        boost::bind(
          &self_t::spawn_stackless,
          base_t::get_aid(), svc, f, (link_type)type
          )
        );
    }
    else
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
          func, base_t::get_aid(), (link_type)type
          )
        );
    }

    message msg;
    aid_t sender;
    pattern patt;
    patt.add_match(msg_new_actor);
    bool is_yield =  pri_recv_match(patt, sender, msg);
    if (!is_yield)
    {
      handle_spawn(sender, msg);
    }
    return is_yield;
  }

  bool spawn_remote(
    spawn_type sty, std::string const& func, match_t ctxid, 
    int type, size_t stack_size, duration_t tmo
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
        &self_t::handle_remote_spawn, this, _arg1, _arg2,
        (link_type)type, begin_tp, sid, tmo, curr_tmo
        );

    aid_t sender;
    message msg;
    pattern patt;
    patt.add_match(msg_spawn_ret);
    patt.timeout_ = curr_tmo;
    bool is_yield =  pri_recv_match(patt, sender, msg);
    if (!is_yield)
    {
      is_yield = pri_handle_remote_spawn(sender, msg);
    }
    return is_yield;
  }

  void register_service(match_t name)
  {
    gce::detail::register_service(base_t::get_aid(), svc_, name);
  }

  void deregister_service(match_t name)
  {
    gce::detail::deregister_service(base_t::get_aid(), svc_, name);
  }

  void log_debug(boost::string_ref str)
  {
    GCE_DEBUG(lg_) << str;
  }

  void log_info(boost::string_ref str)
  {
    GCE_INFO(lg_) << str;
  }

  void log_warn(boost::string_ref str)
  {
    GCE_WARN(lg_) << str;
  }

  void log_error(boost::string_ref str)
  {
    GCE_ERROR(lg_) << str;
  }

  void log_fatal(boost::string_ref str)
  {
    GCE_FATAL(lg_) << str;
  }

  void add_addon(char const* lib, int k, basic_addon<context_t>* a)
  {
    addon ao;
    ao.lib_ = lib;
    ao.k_ = k;
    ao.ao_ = a;
    addon_list_.push_back(ao);
  }

public:
  /// internal use
  typedef gce::luaed type;

  static actor_type get_type()
  {
    return actor_luaed;
  }

  aid_t const& get_aid() const
  {
    return base_t::get_aid();
  }

  static size_t get_pool_reserve_size(attributes const& attr)
  {
    return attr.actor_pool_reserve_size_;
  }

  service_t& get_service()
  {
    return svc_;
  }

  void init_coro(int co, int rf)
  {
    co_ = co;
    rf_ = rf;
  }

  int get_coro() const
  {
    return co_;
  }

  void init(std::string const& script)
  {
    script_ = script;
  }

  void start()
  {
    try
    {
      make_self();
      set_self();
      svc_.run_script(script_);
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

  void on_addon_recv(pack& pk)
  {
    base_t::snd_.dispatch(
      boost::bind(
        &self_t::handle_recv, this, pk
        )
      );
  }

  void quit(exit_code_t exc = exit_normal, std::string const& errmsg = std::string())
  {
    if (!yielding_)
    {
      aid_t self_aid = base_t::get_aid();
      base_t::snd_.post(boost::bind(&self_t::stop, this, self_aid, exc, errmsg));
    }
  }

  class proxy
  {
  public:
    explicit proxy(self_t* p)
      : p_(p)
    {
    }

    self_t* operator->() const
    {
      BOOST_ASSERT(p_ != 0);
      return p_;
    }

    /// compatible with actor_ref
    aid_t const& get_aid() const
    {
      BOOST_ASSERT(p_ != 0);
      return p_->get_aid();
    }

    listener* get_listener()
    {
      BOOST_ASSERT(p_ != 0);
      return p_;
    }

    service_t& get_service()
    {
      BOOST_ASSERT(p_ != 0);
      return p_->get_service();
    }

  private:
    self_t* p_;
  };

private:
  void make_self()
  {
    a_ = lua::actor<self_t>::create(L_, this);
    GCE_VERIFY(a_ != LUA_REFNIL).except<lua_exception>();
  }

  void set_self()
  {
    lua_getglobal(L_, "libgce");
    GCE_VERIFY(gce::lualib::get_ref(L_, "libgce", a_) != 0).except<lua_exception>();
    lua_setfield(L_, -2, "self");
    lua_pop(L_, 1);
  }

  int check_result(pattern const& patt, message const& msg)
  {
    bool has_exit = check_exit(patt.match_list_);
    if (!has_exit && msg.get_type() == exit)
    {
      BOOST_ASSERT(!patt.recver_.empty());
      return lua::ec_guard;
    }
    return lua::ec_ok;
  }

  int check_result(message const& msg)
  {
    if (msg.get_type() == exit)
    {
      return lua::ec_guard;
    }
    return lua::ec_ok;
  }

  static void spawn_stackful(
    aid_t const& sire, stackful_service_t* svc, 
    remote_func<context_t>* f, size_t stack_size, link_type type
    )
  {
    make_stackful_actor<context_t>(sire, *svc, f->af_, stack_size, type);
  }

  static void spawn_stackless(
    aid_t const& sire, stackless_service_t* svc, 
    remote_func<context_t>* f, link_type type
    )
  {
    make_stackless_actor<context_t>(sire, *svc, f->ef_, type);
  }

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

    int ec = check_result(patt, msg);
    set_recv_result(sender, msg, ec);
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
    set_self();

    int rf = gce::lualib::get_ref(L_, "libgce", rf_);
    GCE_ASSERT(rf != 0);
    GCE_ASSERT(lua_type(L_, -1) == LUA_TFUNCTION);
    int co = gce::lualib::get_ref(L_, "libgce", co_);
    GCE_ASSERT(co != 0);

    exit_code_t exc = exit_normal;
    std::string errmsg;
    if (lua_pcall(L_, 1, 0, 0) != 0)
    {
      exc = exit_except;
      errmsg = lua_tostring(L_, -1);
    }
    quit(exc, errmsg);
  }

  void stop(aid_t self_aid, exit_code_t ec, std::string const& exit_msg)
  {
    gce::lualib::rmv_ref(L_, "libgce", a_);
    gce::lualib::rmv_ref(L_, "libgce", co_);
    gce::lualib::rmv_ref(L_, "libgce", rf_);

    BOOST_FOREACH(addon& ao, addon_list_)
    {
      ao.ao_->dispose();
      gce::lualib::rmv_ref(L_, ao.lib_.c_str(), ao.k_);
    }

    base_t::send_exit(self_aid, ec, exit_msg);
    svc_.free_actor(this);
  }

  void start_timer(duration_t dur)
  {
    tmr_.expires_from_now(gce::to_chrono(dur));
    tmr_.async_wait(
      base_t::snd_.wrap(
        boost::bind(
          &self_t::handle_timeout, this,
          boost::asio::placeholders::error, ++tmr_sid_
          )
        )
      );
  }

  void handle_timeout(errcode_t const& ec, size_t tmr_sid)
  {
    if (!ec && tmr_sid == tmr_sid_)
    {
      recving_ = false;
      responsing_ = false;
      curr_pattern_.clear();

      if (spw_hdr_)
      {
        pri_handle_remote_spawn(aid_nil, nil_msg_);
      }
      else
      {
        set_recv_result(aid_nil, nil_msg_, lua::ec_timeout);
        resume();
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

    match_t ty = pk.msg_.get_type();
    recv_t rcv;
    message msg;
    aid_t sender;
    int ec = lua::ec_ok;
    bool need_resume = false;

    if (
      (recving_ && !is_response) ||
      (responsing_ && (is_response || ty == exit))
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
        ec = check_result(curr_pattern_, msg);
        curr_pattern_.clear();
        need_resume = true;
        recving_ = false;
      }

      if (responsing_ && (is_response || ty == exit))
      {
        GCE_ASSERT(recving_res_.valid());
        bool ret = base_t::mb_.pop(recving_res_, msg);
        if (!ret)
        {
          return;
        }
        sender = end_recv(recving_res_);
        ec = check_result(msg);
        need_resume = true;
        responsing_ = false;
        recving_res_ = nil_resp_;
      }

      ++tmr_sid_;
      errcode_t ignored_ec;
      tmr_.cancel(ignored_ec);

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
        set_recv_result(sender, msg, ec);
        resume();
      }
    }
  }

  void handle_bind()
  {
    if (yielding_)
    {
      resume();
    }
  }

  void handle_connect(aid_t skt, message& msg)
  {
    ctxid_pair_t ctxid_pr;
    errcode_t ec;
    msg >> ctxid_pr >> ec;

    if (skt != aid_nil)
    {
      svc_.register_socket(ctxid_pr, skt);
    }

    lua_getglobal(L_, "libgce");
    lua_pushinteger(L_, ec.value());
    lua_setfield(L_, -2, "conn_ret");
    std::string errmsg = ec.message();
    lua_pushlstring(L_, errmsg.c_str(), errmsg.size());
    lua_setfield(L_, -2, "conn_errmsg");
    lua_pop(L_, 1);

    if (yielding_)
    {
      resume();
    }
  }

  void handle_spawn(aid_t aid, message& msg)
  {
    if (aid != aid_nil)
    {
      uint16_t ty = u16_nil;
      msg >> ty;
      link_type type = (link_type)ty;

      if (type == linked)
      {
        link(aid);
      }
      else if (type == monitored)
      {
        monitor(aid);
      }
    }

    set_spawn_result(aid);
    if (yielding_)
    {
      resume();
    }
  }

  bool handle_remote_spawn(
    aid_t aid,
    message msg, link_type type,
    boost::chrono::system_clock::time_point begin_tp,
    sid_t sid, duration_t tmo, duration_t curr_tmo
    )
  {
    uint16_t err = 0;
    sid_t ret_sid = sid_nil;
    if (msg.get_type() != match_nil)
    {
      aid_t sender;
      while (true)
      {
        msg >> err >> ret_sid;
        if (err != 0 || (aid != aid_nil && sid == ret_sid))
        {
          break;
        }

        if (tmo != infin)
        {
          duration_t pass_time = from_chrono(boost::chrono::system_clock::now() - begin_tp);
          curr_tmo = curr_tmo - pass_time;
        }

        begin_tp = boost::chrono::system_clock::now();
        pattern patt;
        patt.add_match(msg_spawn_ret);
        patt.timeout_ = curr_tmo;
        bool is_yield = pri_recv_match(patt, sender, msg);
        if (is_yield)
        {
          spw_hdr_ = 
            boost::bind(
              &self_t::handle_remote_spawn, this, _arg1, _arg2,
              type, begin_tp, sid, tmo, curr_tmo
              );
          return true;
        }
      }

      spawn_error error = (spawn_error)err;
      if (error != spawn_ok)
      {
        aid = aid_nil;
      }

      if (aid != aid_nil)
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
      set_spawn_result(aid);
      resume();
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

  void set_recv_result(gce::aid_t const& sender, message const& msg, int ec)
  {
    lua_getglobal(L_, "libgce");
    lua::push(L_, sender);
    lua_setfield(L_, -2, "recv_sender");
    lua::message::create(L_, msg);
    lua_setfield(L_, -2, "recv_msg");
    lua_pushinteger(L_, ec);
    lua_setfield(L_, -2, "recv_ec");
    lua_pop(L_, 1);
  }

  void set_spawn_result(gce::aid_t const& aid)
  {
    lua_getglobal(L_, "libgce");
    lua::push(L_, aid);
    lua_setfield(L_, -2, "spawn_aid");
    lua_pop(L_, 1);
  }

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(lua_State*, L_)
  GCE_CACHE_ALIGNED_VAR(std::string, script_)
  GCE_CACHE_ALIGNED_VAR(int, a_)
  GCE_CACHE_ALIGNED_VAR(int, co_)
  GCE_CACHE_ALIGNED_VAR(int, rf_)

  /// coro local
  service_t& svc_;
  bool yielding_;
  bool recving_;
  bool responsing_;
  resp_t recving_res_;
  pattern curr_pattern_;
  timer_t tmr_;
  size_t tmr_sid_;

  typedef boost::function<bool (aid_t, message)> spawn_handler_t;
  spawn_handler_t spw_hdr_;

  std::vector<addon> addon_list_;

  recv_t const nil_rcv_;
  resp_t const nil_resp_;
  message const nil_msg_;
  log::logger_t& lg_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_LUA_ACTOR_HPP
