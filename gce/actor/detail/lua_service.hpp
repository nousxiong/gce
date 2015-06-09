///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_LUA_SERVICE_HPP
#define GCE_ACTOR_DETAIL_LUA_SERVICE_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/net_option.hpp>
#include <gce/actor/duration.hpp>
#include <gce/actor/detail/lua_wrap.hpp>
#include <gce/actor/detail/inpool_service.hpp>
#include <gce/actor/detail/actor_pool.hpp>
#include <gce/actor/detail/send.hpp>
#include <gce/actor/detail/actor_function.hpp>
#include <gce/detail/unique_ptr.hpp>
#include <sstream>
#include <map>

namespace gce
{
namespace detail
{
///------------------------------------------------------------------------------
/// lua_state_deletor
///------------------------------------------------------------------------------
struct lua_state_deletor
{
  void operator()(lua_State* L)
  {
    if (L)
    {
      lua_close(L);
    }
  }
};
///------------------------------------------------------------------------------
template <typename Actor>
class lua_service
  : public inpool_service<Actor>
{
  typedef Actor actor_t;
  typedef inpool_service<actor_t> base_t;
  typedef lua_service<actor_t> self_t;
  typedef actor_pool<actor_t> actor_pool_t;

public:
  typedef typename actor_t::type type;
  typedef typename actor_t::context_t context_t;
  typedef std::map<match_t, remote_func<context_t> > native_func_list_t;

public:
  lua_service(
    context_t& ctx, 
    strand_t& snd, 
    size_t index, 
    native_func_list_t const& native_func_list = native_func_list_t()
    )
    : base_t(ctx, snd, index)
    , init_lua_script_("\
        local p = gce_path \
        local package_path = package.path \
        package.path = string.format(\"%s;%s\", package_path, p)\
        ")
    , L_(luaL_newstate(), lua_state_deletor())
    , actor_pool_(
        base_t::ctxid_, base_t::timestamp_, (uint16_t)base_t::index_,
        actor_t::get_pool_reserve_size(base_t::ctx_.get_attributes())
        )
    , lg_(ctx.get_logger())
    , native_func_list_(native_func_list)
  {
  }

  ~lua_service()
  {
  }

public:
  void make_libgce(std::string const& lua_gce_path)
  {
    lua_State* L = L_.get();
    luaL_openlibs(L);

#if GCE_PACKER == GCE_ADATA
    adata::lua::init_adata_corec(L);
#endif

    typedef lua::actor<actor_t> lua_actor_t;

    /// register libgce
    gce::lualib::open(L)
      .begin("libgce")
        .add_function("make_msg", lua::message::make)
        .begin_userdata("message")
          .add_function("setty", lua::message::setty)
          .add_function("getty", lua::message::getty)
          .add_function("gcety", lua::message::gcety)
          .add_function("size", lua::message::size)
          .add_function("__tostring", lua::message::tostring)
          .add_function("__gc", lua::message::gc)
        .end_userdata()
        .add_function("make_resp", lua::response::make)
        .begin_userdata("response")
          .add_function("gcety", lua::response::gcety)
          .add_function("__tostring", lua::response::tostring)
          .add_function("__gc", lua::response::gc)
        .end_userdata()
        .add_function("make_patt", lua::pattern::make)
        .begin_userdata("pattern")
          .add_function("set_timeout", lua::pattern::set_timeout)
          .add_function("gcety", lua::pattern::gcety)
          .add_function("add_match", lua::pattern::add_match)
          .add_function("set_match_aid", lua::pattern::set_match_aid)
          .add_function("set_match_svcid", lua::pattern::set_match_svcid)
          .add_function("__tostring", lua::pattern::tostring)
          .add_function("__gc", lua::pattern::gc)
        .end_userdata()
        .add_function("make_chunk", lua::chunk::make)
        .begin_userdata("chunk")
          .add_function("gcety", lua::chunk::gcety)
          .add_function("to_string", lua::chunk::to_string)
          .add_function("from_string", lua::chunk::from_string)
          .add_function("__tostring", lua::chunk::tostring)
          .add_function("__gc", lua::chunk::gc)
        .end_userdata()
        .add_function("make_errcode", lua::errcode::make)
        .begin_userdata("errcode")
          .add_function("value", lua::errcode::value)
          .add_function("errmsg", lua::errcode::errmsg)
          .add_function("gcety", lua::errcode::gcety)
          .add_function("__tostring", lua::errcode::tostring)
          .add_function("__eq", lua::errcode::eq)
          .add_function("__gc", lua::errcode::gc)
        .end_userdata()
        .add_function("make_netopt", lua::net_option::make)
#if GCE_PACKER == GCE_AMSG
        .add_function("make_match", lua::match::make)
        .begin_userdata("match")
          .add_function("gcety", lua::match::gcety)
          .add_function("__tostring", lua::match::tostring)
          .add_function("__gc", lua::match::gc)
          .add_function("__eq", lua::match::eq)
        .end_userdata()
        .add_function("make_svcid", lua::service_id::make)
        .begin_userdata("service_id")
          .add_function("gcety", lua::service_id::gcety)
          .add_function("__tostring", lua::service_id::tostring)
          .add_function("__eq", lua::service_id::eq)
          .add_function("__gc", lua::service_id::gc)
        .end_userdata()
        .add_function("make_aid", lua::actor_id::make)
        .begin_userdata("actor_id")
          .add_function("gcety", lua::actor_id::gcety)
          .add_function("__tostring", lua::actor_id::tostring)
          .add_function("__eq", lua::actor_id::eq)
          .add_function("__gc", lua::actor_id::gc)
        .end_userdata()
        .add_function("make_dur", lua::duration::make_dur)
        .add_function("make_millisecs", lua::duration::make_millisecs)
        .add_function("make_seconds", lua::duration::make_seconds)
        .add_function("make_minutes", lua::duration::make_minutes)
        .add_function("make_hours", lua::duration::make_hours)
        .begin_userdata("duration")
          .add_function("type", lua::duration::type)
          .add_function("gcety", lua::duration::gcety)
          .add_function("__tostring", lua::duration::tostring)
          .add_function("__eq", lua::duration::eq)
          .add_function("__lt", lua::duration::lt)
          .add_function("__le", lua::duration::le)
          .add_function("__add", lua::duration::add)
          .add_function("__sub", lua::duration::sub)
          .add_function("__gc", lua::duration::gc)
        .end_userdata()
#elif GCE_PACKER == GCE_ADATA
        .add_function("mt_tostring", lua::match::tostring)
        .add_function("svcid_tostring", lua::service_id::tostring)
        .add_function("aid_tostring", lua::actor_id::tostring)
        .add_function("dur_tostring", lua::duration::tostring)
        .add_function("dur_eq", lua::duration::eq)
        .add_function("dur_lt", lua::duration::lt)
        .add_function("dur_le", lua::duration::le)
        .add_function("dur_add", lua::duration::add)
        .add_function("dur_sub", lua::duration::sub)
#endif
        .begin_userdata("actor")
          .add_function("get_aid", lua_actor_t::get_aid)
          .add_function("init_coro", lua_actor_t::init_coro)
          .add_function("send", lua_actor_t::send)
          .add_function("send2svc", lua_actor_t::send2svc)
          .add_function("send2svcs", lua_actor_t::send2svcs)
          .add_function("relay", lua_actor_t::relay)
          .add_function("relay2svc", lua_actor_t::relay2svc)
          .add_function("relay2svcs", lua_actor_t::relay2svcs)
          .add_function("request", lua_actor_t::request)
          .add_function("request2svc", lua_actor_t::request2svc)
          .add_function("request2svcs", lua_actor_t::request2svcs)
          .add_function("reply", lua_actor_t::reply)
          .add_function("link", lua_actor_t::link)
          .add_function("monitor", lua_actor_t::monitor)
          .add_function("recv", lua_actor_t::recv)
          .add_function("recv_match", lua_actor_t::recv_match)
          .add_function("recv_response", lua_actor_t::recv_response)
          .add_function("recv_response_timeout", lua_actor_t::recv_response_timeout)
          .add_function("sleep_for", lua_actor_t::sleep_for)
          .add_function("bind", lua_actor_t::bind)
          .add_function("connect", lua_actor_t::connect)
          .add_function("spawn", lua_actor_t::spawn)
          .add_function("spawn_remote", lua_actor_t::spawn_remote)
          .add_function("register_service", lua_actor_t::register_service)
          .add_function("deregister_service", lua_actor_t::deregister_service)
          .add_function("log_debug", lua_actor_t::log_debug)
          .add_function("log_info", lua_actor_t::log_info)
          .add_function("log_warn", lua_actor_t::log_warn)
          .add_function("log_error", lua_actor_t::log_error)
          .add_function("log_fatal", lua_actor_t::log_fatal)
          .add_function("__gc", lua_actor_t::gc)
        .end_userdata()
        .add_function("atom", lua::s2i)
        .add_function("deatom", lua::i2s)
        .add_function("pack_number", lua::pack_number)
        .add_function("pack_string", lua::pack_string)
        .add_function("pack_boolean", lua::pack_boolean)
        .add_function("pack_object", lua::pack_object)
        .add_function("unpack_number", lua::unpack_number)
        .add_function("unpack_string", lua::unpack_string)
        .add_function("unpack_boolean", lua::unpack_boolean)
        .add_function("unpack_object", lua::unpack_object)
        .add_function("print", lua::print)
        .add_function("init_nil", lua::init_nil)
      .end()
      ;

    lua_pushlstring(L, lua_gce_path.c_str(), lua_gce_path.size());
    lua_setglobal(L, "gce_path");
    if (luaL_dostring(L, init_lua_script_.c_str()) != 0)
    {
      std::string errmsg("gce::lua_exception: ");
      errmsg += lua_tostring(L, -1);
      GCE_VERIFY(false)(lua_gce_path).msg(errmsg.c_str()).except<gce::lua_exception>();
    }

    /// init libgce
    std::ostringstream oss;
    oss << "local libgce = require('libgce')" << std::endl;
    oss << "libgce.pkr_amsg = " << GCE_AMSG << std::endl;
    oss << "libgce.pkr_adata = " << GCE_ADATA << std::endl;
#if GCE_PACKER == GCE_AMSG
    oss << "libgce.packer = libgce.pkr_amsg" << std::endl;
#elif GCE_PACKER == GCE_ADATA
    oss << "libgce.packer = libgce.pkr_adata" << std::endl;
#endif
#ifdef GCE_OPENSSL
    oss << "libgce.openssl = 1" << std::endl;
#else
    oss << "libgce.openssl = 0" << std::endl;
#endif
    oss << "libgce.ty_pattern = " << gce::lua::ty_pattern << std::endl;
    oss << "libgce.ty_message = " << gce::lua::ty_message << std::endl;
    oss << "libgce.ty_response = " << gce::lua::ty_response << std::endl;
    oss << "libgce.ty_chunk = " << gce::lua::ty_chunk << std::endl;
    oss << "libgce.ty_errcode = " << gce::lua::ty_errcode << std::endl;
    oss << "libgce.ty_match = " << gce::lua::ty_match << std::endl;
    oss << "libgce.ty_duration = " << gce::lua::ty_duration << std::endl;
    oss << "libgce.ty_actor_id = " << gce::lua::ty_actor_id << std::endl;
    oss << "libgce.ty_service_id = " << gce::lua::ty_service_id << std::endl;
    oss << "libgce.ty_userdef = " << gce::lua::ty_userdef << std::endl;
    oss << "libgce.ty_lua = " << gce::lua::ty_lua << std::endl;
    oss << "libgce.ty_other = " << gce::lua::ty_other << std::endl;

    oss << "libgce.ec_ok = " << lua::ec_ok << std::endl;
    oss << "libgce.ec_timeout = " << lua::ec_timeout << std::endl;
    oss << "libgce.ec_guard = " << lua::ec_guard << std::endl;

    oss << "libgce.dur_raw = " << gce::dur_raw << std::endl;
    oss << "libgce.dur_microsec = " << gce::dur_microsec << std::endl;
    oss << "libgce.dur_millisec = " << gce::dur_millisec << std::endl;
    oss << "libgce.dur_second = " << gce::dur_second << std::endl;
    oss << "libgce.dur_minute = " << gce::dur_minute << std::endl;
    oss << "libgce.dur_hour = " << gce::dur_hour << std::endl;

    oss << "libgce.no_link = " << gce::no_link << std::endl;
    oss << "libgce.linked = " << gce::linked << std::endl;
    oss << "libgce.monitored = " << gce::monitored << std::endl;

    oss << "libgce.stackful = " << gce::detail::spw_stackful << std::endl;
    oss << "libgce.stackless = " << gce::detail::spw_stackless << std::endl;
    oss << "libgce.luaed = " << gce::detail::spw_luaed<< std::endl;

    oss << "libgce.stacksize = " << default_stacksize() << std::endl;

#if GCE_LUA_VERSION == GCE_LUA51
    oss << "libgce.luaver = '5.1'" << std::endl;
#elif GCE_LUA_VERSION == GCE_LUA52
    oss << "libgce.luaver = '5.2'" << std::endl;
#elif GCE_LUA_VERSION == GCE_LUA53
    oss << "libgce.luaver = '5.3'" << std::endl;
#endif

    std::string init_libgce_script = oss.str();
    if (luaL_dostring(L, init_libgce_script.c_str()) != 0)
    {
      std::string errmsg("gce::lua_exception: ");
      errmsg += lua_tostring(L, -1);
      GCE_VERIFY(false).msg(errmsg.c_str()).except<gce::lua_exception>();
    }
  }

  actor_t* make_actor()
  {
    return actor_pool_.make(boost::ref(*this));
  }

  void free_actor(actor_t* a)
  {
    actor_pool_.free(a);
  }

  void run_script(
    std::string const& name, 
    std::string const& script = std::string()
    )
  {
    int scr = LUA_REFNIL;
    std::string errmsg;
    script_list_t::iterator itr(script_list_.find(name));
    if (itr != script_list_.end())
    {
      scr = itr->second;
    }
    else
    {
      std::pair<int, std::string> pr = set_script(name, script);
      scr = pr.first;
      errmsg = pr.second;
    }
    GCE_VERIFY(scr != LUA_REFNIL)(name)(errmsg).except<gce::lua_exception>();

    lua_State* L = L_.get();
    GCE_VERIFY(gce::lualib::get_ref(L, "libgce", scr) != 0)(name).except<gce::lua_exception>();

    if (lua_pcall(L, 0, LUA_MULTRET, 0) != 0)
    {
      std::string errmsg("gce::lua_exception: ");
      errmsg += lua_tostring(L, -1);
      GCE_VERIFY(false).msg(errmsg.c_str()).except<gce::lua_exception>();
    }
  }

  std::pair<int, std::string> set_script(
    std::string const& name, 
    std::string const& script = std::string()
    )
  {
    lua_State* L = L_.get();
    if (script.empty())
    {
      if (luaL_loadfile(L, name.c_str()) != 0)
      {
        return std::make_pair(LUA_REFNIL, lua_tostring(L, -1));
      }
    }
    else
    {
      if (luaL_loadstring(L, script.c_str()) != 0)
      {
        return std::make_pair(LUA_REFNIL, lua_tostring(L, -1));
      }
    }
    int r = gce::lualib::make_ref(L, "libgce");
    std::pair<script_list_t::iterator, bool> pr =
      script_list_.insert(std::make_pair(name, r));
    if (!pr.second)
    {
      pr.first->second = r;
    }
    return std::make_pair(r, "");
  }

  lua_State* get_lua_state()
  {
    return L_.get();
  }

  aid_t spawn_actor(std::string const& func, aid_t sire, link_type type)
  {
    actor_t* a = make_actor();
    a->init(func);
    if (sire != aid_nil)
    {
      gce::detail::send(*a, sire, msg_new_actor, (uint16_t)type);
    }
    aid_t aid = a->get_aid();
    a->start();
    return aid;
  }

  remote_func<context_t>* get_native_func(std::string const& func)
  {
    typename native_func_list_t::iterator itr(native_func_list_.find(to_match(func)));
    if (itr == native_func_list_.end())
    {
      return 0;
    }

    return &itr->second;
  }

protected:
  actor_t* find_actor(actor_index ai, sid_t sid)
  {
    return actor_pool_.find(ai, sid);
  }

private:
  std::string const init_lua_script_;
  detail::unique_ptr<lua_State> L_;
  actor_pool_t actor_pool_;

  typedef std::map<std::string, int> script_list_t;
  script_list_t script_list_;
  log::logger_t& lg_;

  native_func_list_t native_func_list_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_LUA_SERVICE_HPP
