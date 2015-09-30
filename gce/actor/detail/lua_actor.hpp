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
    , guard_(base_t::ctx_.get_io_service())
  {
    guard_.expires_from_now(to_chrono(infin));
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

  void relay(aid_t const& des, message& m)
  {
    base_t::pri_relay(des, m);
  }

  void relay(svcid_t const& des, message& m)
  {
    base_t::pri_relay_svc(des, m);
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

  void link(svcid_t const& target)
  {
    base_t::pri_link_svc(target);
  }

  void monitor(svcid_t const& target)
  {
    base_t::pri_monitor_svc(target);
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
    message* pmsg = 0;

    if (!base_t::mb_.pop(res, pmsg))
    {
      if (tmo >= zero) /// change > to >= for yielding when timeout == 0
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

    tmp_msg_.clear();
    if (pmsg != 0)
    {
      tmp_msg_ = *pmsg;
    }

    int ec = check_result(tmp_msg_);
    set_recv_result(sender, tmp_msg_, ec);
    if (pmsg != 0)
    {
      base_t::free_msg(pmsg);
    }
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
    if (get_service().has_acceptor(ep))
    {
      return false;
    }

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
    if (opt.reuse_conn != 0 && get_service().has_socket(target))
    {
      return false;
    }

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
        spawn_stackful_binder(
          *this, base_t::get_aid(), svc, f, stack_size, (link_type)type
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
        spawn_stackless_binder(
          *this, base_t::get_aid(), svc, f, (link_type)type
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
        spawn_actor_binder(
          svc, func, base_t::get_aid(), (link_type)type
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
    sysclock_t::time_point begin_tp = sysclock_t::now();
    spw_hdr_ = spawn_handler_t(*this, (link_type)type, begin_tp, sid, tmo, curr_tmo);

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
    base_t::set_aid_svc(make_svcid(base_t::ctxid_, name));
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

  static size_t get_pool_max_size(attributes const& attr)
  {
    return attr.actor_pool_max_size_;
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
      guard_.async_wait(guard());
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
    base_t::snd_.dispatch(handle_recv_binder(*this, pk));
  }

  void quit(exit_code_t exc = exit_normal, std::string const& errmsg = std::string())
  {
    if (!yielding_)
    {
      aid_t self_aid = base_t::get_aid();
      base_t::snd_.post(stop_binder(*this, self_aid, exc, errmsg));
    }
    else
    {
      if (exc == exit_normal)
      {
        base_t::ctx_.on_tick(svc_.get_index());
      }
    }
  }

  class proxy
  {
  public:
    explicit proxy(self_t* p)
      : p_(p)
    {
    }

    void send(aid_t const& recver, message const& m)
    {
      BOOST_ASSERT(p_ != 0);
      p_->send(recver, m);
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
  struct spawn_stackful_binder
  {
    spawn_stackful_binder(
      self_t& self, 
      aid_t const& aid, 
      stackful_service_t* svc, 
      remote_func<context_t>* f, 
      size_t stacksize,
      link_type type
      )
      : self_(self)
      , aid_(aid)
      , svc_(svc)
      , f_(f)
      , stacksize_(stacksize)
      , type_(type)
    {
    }

    void operator()() const
    {
      self_.spawn_stackful(aid_, svc_, f_, stacksize_, type_);
    }

    self_t& self_;
    aid_t const aid_;
    stackful_service_t* svc_; 
    remote_func<context_t>* f_;
    size_t stacksize_;
    link_type type_;
  };

  struct spawn_stackless_binder
  {
    spawn_stackless_binder(
      self_t& self, 
      aid_t const& aid, 
      stackless_service_t* svc, 
      remote_func<context_t>* f, 
      link_type type
      )
      : self_(self)
      , aid_(aid)
      , svc_(svc)
      , f_(f)
      , type_(type)
    {
    }

    void operator()() const
    {
      self_.spawn_stackless(aid_, svc_, f_, type_);
    }

    self_t& self_;
    aid_t const aid_;
    stackless_service_t* svc_; 
    remote_func<context_t>* f_;
    link_type type_;
  };

  struct spawn_actor_binder
  {
    spawn_actor_binder(
      service_t* svc, 
      std::string const& func, 
      aid_t const& aid, 
      link_type type
      )
      : svc_(svc)
      , func_(func)
      , aid_(aid)
      , type_(type)
    {
    }

    void operator()() const
    {
      svc_->spawn_actor(func_, aid_, type_);
    }

    service_t* svc_; 
    std::string const func_;
    aid_t const aid_;
    link_type type_;
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
    message* pmsg = 0;

    if (!base_t::mb_.pop(rcv, pmsg, patt.match_list_, patt.recver_))
    {
      duration_t tmo = patt.timeout_;
      if (tmo >= zero) /// change > to >= for yielding when timeout == 0
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

    tmp_msg_.clear();
    if (pmsg != 0)
    {
      tmp_msg_ = *pmsg;
    }

    int ec = check_result(patt, tmp_msg_);
    set_recv_result(sender, tmp_msg_, ec);
    if (pmsg != 0)
    {
      base_t::free_msg(pmsg);
    }
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

  struct stop_binder
  {
    stop_binder(self_t& self, aid_t const& self_aid, exit_code_t ec, std::string const& exit_msg)
      : self_(self)
      , self_aid_(self_aid)
      , ec_(ec)
      , exit_msg_(exit_msg)
    {
    }

    void operator()() const
    {
      self_.stop(self_aid_, ec_, exit_msg_);
    }

    self_t& self_;
    aid_t const self_aid_;
    exit_code_t ec_;
    std::string const exit_msg_;
  };

  void stop(aid_t const& self_aid, exit_code_t ec, std::string const& exit_msg)
  {
    errcode_t ignored_ec;
    guard_.cancel(ignored_ec);

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
    base_t::ctx_.on_tick(svc_.get_index());
  }

  struct handle_timeout_binder
  {
    handle_timeout_binder(self_t& self, size_t tmr_sid)
      : self_(self)
      , tmr_sid_(tmr_sid)
    {
    }

    void operator()(errcode_t const& ec) const
    {
      self_.handle_timeout(ec, tmr_sid_);
    }

    self_t& self_;
    size_t tmr_sid_;
  };

  void start_timer(duration_t dur)
  {
    tmr_.expires_from_now(gce::to_chrono(dur));
    tmr_.async_wait(base_t::snd_.wrap(handle_timeout_binder(*this, ++tmr_sid_)));
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
        message msg;
        pri_handle_remote_spawn(aid_nil, msg);
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
    message* msg = base_t::handle_pack(pk);

    if (aid_t* aid = boost::get<aid_t>(&pk.tag_))
    {
      base_t::mb_.push(*aid, msg);
    }
    else if (request_t* req = boost::get<request_t>(&pk.tag_))
    {
      base_t::mb_.push(*req, msg);
    }
    else if (link_t* link = boost::get<link_t>(&pk.tag_))
    {
      base_t::add_link(link->get_aid(), pk.skt_);
      return;
    }
    else if (exit_t* ex = boost::get<exit_t>(&pk.tag_))
    {
      base_t::mb_.push(*ex, msg);
      base_t::remove_link(ex->get_aid());
    }
    else if (resp_t* res = boost::get<resp_t>(&pk.tag_))
    {
      is_response = true;
      base_t::mb_.push(*res, msg);
    }

    match_t ty = msg->get_type();
    msg = 0;
    //message msg;
    aid_t sender;
    int ec = lua::ec_ok;
    bool need_resume = false;
    bool recving = recving_ && !is_response;
    bool resping = responsing_ && (is_response || ty == exit);

    if (recving || resping)
    {
      if (recving)
      {
        recv_t rcv;
        if (!base_t::mb_.pop(rcv, msg, curr_pattern_.match_list_, curr_pattern_.recver_))
        {
          return;
        }
        sender = end_recv(rcv, *msg);
        ec = check_result(curr_pattern_, *msg);
        curr_pattern_.clear();
        need_resume = true;
        recving_ = false;
      }

      if (resping)
      {
        GCE_ASSERT(recving_res_.valid());
        if (!base_t::mb_.pop(recving_res_, msg))
        {
          return;
        }
        sender = end_recv(recving_res_);
        ec = check_result(*msg);
        need_resume = true;
        responsing_ = false;
        recving_res_ = nil_resp_;
      }

      ++tmr_sid_;
      errcode_t ignored_ec;
      tmr_.cancel(ignored_ec);

      match_t msg_type = msg->get_type();
      if (msg_type == msg_new_actor)
      {
        need_resume = false;
        handle_spawn(sender, *msg);
      }
      else if (msg_type == msg_spawn_ret)
      {
        need_resume = false;
        pri_handle_remote_spawn(sender, *msg);
      }
      else if (msg_type == msg_new_bind)
      {
        need_resume = false;
        handle_bind();
      }
      else if (msg_type == msg_new_conn)
      {
        need_resume = false;
        handle_connect(sender, *msg);
      }

      if (need_resume)
      {
        set_recv_result(sender, *msg, ec);
        base_t::free_msg(msg);
        resume();
      }
      else
      {
        base_t::free_msg(msg);
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

  struct handle_remote_spawn_binder
  {
    handle_remote_spawn_binder()
      : self_(0)
    {
    }

    handle_remote_spawn_binder(
      self_t& self, 
      link_type type, 
      sysclock_t::time_point const& begin_tp,
      sid_t sid, 
      duration_t const& tmo,
      duration_t const& curr_tmo
      )
      : self_(&self)
      , type_(type)
      , begin_tp_(begin_tp)
      , sid_(sid)
      , tmo_(tmo)
      , curr_tmo_(curr_tmo)
    {
    }

    /*void operator=(handle_remote_spawn_binder const& rhs)
    {
      if (this != &rhs)
      {
        type_ = rhs.type_;
        begin_tp_ = rhs.begin_tp_;
        sid_ = rhs.sid_;
        tmo_ = rhs.tmo_;
        curr_tmo_ = rhs.curr_tmo_;
      }
    }*/

    bool operator()(aid_t const& aid, message& msg) const
    {
      return self_->handle_remote_spawn(aid, msg, type_, begin_tp_, sid_, tmo_, curr_tmo_);
    }

    void clear()
    {
      self_ = 0;
    }

    operator bool() const
    {
      return self_ != 0;
    }

    self_t* self_;
    link_type type_;
    sysclock_t::time_point begin_tp_;
    sid_t sid_;
    duration_t tmo_;
    duration_t curr_tmo_;
  };

  bool handle_remote_spawn(
    aid_t aid,
    message& msg, link_type type,
    sysclock_t::time_point begin_tp,
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
          duration_t pass_time = from_chrono(sysclock_t::now() - begin_tp);
          curr_tmo = curr_tmo - pass_time;
        }

        begin_tp = sysclock_t::now();
        pattern patt;
        patt.add_match(msg_spawn_ret);
        patt.timeout_ = curr_tmo;
        bool is_yield = pri_recv_match(patt, sender, msg);
        if (is_yield)
        {
          spw_hdr_ = spawn_handler_t(*this, type, begin_tp, sid, tmo, curr_tmo);
            /*boost::bind(
              &self_t::handle_remote_spawn, this, _arg1, _arg2,
              type, begin_tp, sid, tmo, curr_tmo
              );*/
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

  bool pri_handle_remote_spawn(aid_t const& sender, message& msg)
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

  struct guard
  {
    void operator()(errcode_t const&) const {}
  };

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
  message tmp_msg_;

  //typedef boost::function<bool (aid_t, message)> spawn_handler_t;
  typedef handle_remote_spawn_binder spawn_handler_t;
  spawn_handler_t spw_hdr_;

  std::vector<addon> addon_list_;

  recv_t const nil_rcv_;
  resp_t const nil_resp_;
  message const nil_msg_;
  log::logger_t& lg_;

  timer_t guard_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_LUA_ACTOR_HPP
