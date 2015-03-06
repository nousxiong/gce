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
#include <gce/actor/detail/inpool_service.hpp>
#include <gce/actor/detail/actor_pool.hpp>
#include <gce/actor/detail/send.hpp>
#include <gce/detail/unique_ptr.hpp>
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
inline msg_t serialize_number(msg_t& msg, int src)
{
  msg << (boost::int32_t)src;
  return msg;
}
inline msg_t serialize_string(msg_t& msg, std::string const& src)
{
  msg << src;
  return msg;
}
inline msg_t serialize_boolean(msg_t& msg, bool src)
{
  msg << src;
  return msg;
}
///------------------------------------------------------------------------------
template <typename T>
struct deserialize_result
{
  T r_;
  msg_t m_;
};
///------------------------------------------------------------------------------
inline deserialize_result<int> deserialize_number(msg_t& m)
{
  deserialize_result<int> res;
  boost::int32_t des;
  m >> des;
  res.r_ = (int)des;
  res.m_ = m;
  return res;
}
inline deserialize_result<std::string> deserialize_string(msg_t& m)
{
  deserialize_result<std::string> res;
  m >> res.r_;
  res.m_ = m;
  return res;
}
inline deserialize_result<bool> deserialize_boolean(msg_t& m)
{
  deserialize_result<bool> res;
  m >> res.r_;
  res.m_ = m;
  return res;
}
///------------------------------------------------------------------------------
inline void print(std::string const& str)
{
  if (!str.empty())
  {
    std::printf("%s\n", str.c_str());
  }
  else
  {
    std::printf("\n", str.c_str());
  }
}
///------------------------------------------------------------------------------
inline duration_type lua_millisecs(int val)
{
  return millisecs_t(val);
}

inline duration_type lua_seconds(int val)
{
  return seconds_t(val);
}

inline duration_type lua_minutes(int val)
{
  return minutes_t(val);
}

inline duration_type lua_hours(int val)
{
  return hours_t(val);
}

inline duration_type make_zero()
{
  return duration_t(zero);
}

inline duration_type make_infin()
{
  return duration_t(infin);
}

inline net_option make_net_option()
{
  return net_option();
}
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

public:
  lua_service(context_t& ctx, strand_t& snd, std::size_t index)
    : base_t(ctx, snd, index)
    , init_lua_script_(
        "local p = gce_path \
        local package_path = package.path \
        package.path = string.format(\"%s;%s\", package_path, p)"
        )
    , L_(luaL_newstate(), lua_state_deletor())
    , actor_pool_(
        base_t::ctxid_, base_t::timestamp_, (boost::uint16_t)base_t::index_,
        actor_t::get_pool_reserve_size(base_t::ctx_.get_attributes())
        )
    , lg_(ctx.get_logger())
  {
  }

  ~lua_service()
  {
  }

public:
  void initialize(std::string const& lua_gce_path)
  {
    lua_State* L = L_.get();
    luaL_openlibs(L);

    typedef actor_t lua_actor_t;

    luabridge::getGlobalNamespace(L)
      .beginNamespace("detail")
        .beginClass<aid_t>("aid_t")
          .addFunction("get_overloading_type", &aid_t::get_overloading_type)
          .addFunction("is_nil", &aid_t::is_nil)
          .addFunction("equals", &aid_t::equals)
          .addFunction("to_string", &aid_t::to_string)
          GCE_LUA_REG_SERIALIZE_FUNC(aid_t)
        .endClass()
        .beginClass<svcid_t>("svcid_t")
          .addFunction("get_overloading_type", &svcid_t::get_overloading_type)
          .addFunction("to_string", &svcid_t::to_string)
          GCE_LUA_REG_SERIALIZE_FUNC(svcid_t)
        .endClass()
        .beginClass<msg_t>("msg_t")
          .addFunction("get_overloading_type", &msg_t::get_overloading_type)
          .addFunction("set_type", &msg_t::set_match_type)
          .addFunction("get_type", &msg_t::get_match_type)
          .addFunction("to_string", &msg_t::to_string)
          .addFunction("enable_copy_read_size", &msg_t::enable_copy_read_size)
          .addFunction("disable_copy_read_size", &msg_t::disable_copy_read_size)
          GCE_LUA_REG_SERIALIZE_FUNC(msg_t)
        .endClass()
        .beginClass<match_type>("match_t")
          .addFunction("get_overloading_type", &match_type::get_overloading_type)
          .addFunction("equals", &match_type::equals)
          .addFunction("to_string", &match_type::to_string)
        .endClass()
        .beginClass<resp_t>("resp_t")
          .addFunction("to_string", &resp_t::to_string)
        .endClass()
        .beginClass<pattern>("pattern_t")
          .addFunction("get_overloading_type", &pattern::get_overloading_type)
          .addFunction("set_timeout", &pattern::set_timeout)
          .addFunction("add_match", &pattern::add_match_type)
          .addFunction("set_match_aid", &pattern::set_match_aid)
          .addFunction("set_match_svcid", &pattern::set_match_svcid)
          .addFunction("to_string", &pattern::to_string)
        .endClass()
        .beginClass<duration_type>("duration_t")
          .addFunction("get_overloading_type", &duration_type::get_overloading_type)
          .addFunction("to_string", &duration_type::to_string)
          GCE_LUA_REG_SERIALIZE_FUNC(duration_type)
        .endClass()
        .beginClass<net_option>("net_option_t")
          .addData("is_router", &net_option::is_router_)
          .addData("heartbeat_period", &net_option::heartbeat_period_)
          .addData("heartbeat_count", &net_option::heartbeat_count_)
          .addData("init_reconn_period", &net_option::init_reconn_period_)
          .addData("init_reconn_try", &net_option::init_reconn_try_)
          .addData("reconn_period", &net_option::reconn_period_)
          .addData("reconn_try", &net_option::reconn_try_)
          .addData("rebind_period", &net_option::rebind_period_)
          .addData("rebind_try", &net_option::rebind_try_)
        .endClass()
        .beginClass<deserialize_result<int> >("unpack_number")
          .addData("rt", &deserialize_result<int>::r_)
          .addData("ms", &deserialize_result<int>::m_)
        .endClass()
        .beginClass<deserialize_result<std::string> >("unpack_string")
          .addData("rt", &deserialize_result<std::string>::r_)
          .addData("ms", &deserialize_result<std::string>::m_)
        .endClass()
        .beginClass<deserialize_result<bool> >("unpack_boolean")
          .addData("rt", &deserialize_result<bool>::r_)
          .addData("ms", &deserialize_result<bool>::m_)
        .endClass()
        .beginClass<lua_actor_t>("actor")
          .addFunction("get_aid", &lua_actor_t::get_aid)
          .addFunction("set_coro", &lua_actor_t::set_coro)
          .addFunction("set_resume", &lua_actor_t::set_resume)
          .addFunction("send", &lua_actor_t::send)
          .addFunction("send2svc", &lua_actor_t::send2svc)
          .addFunction("relay", &lua_actor_t::relay)
          .addFunction("relay2svc", &lua_actor_t::relay2svc)
          .addFunction("request", &lua_actor_t::request)
          .addFunction("request2svc", &lua_actor_t::request2svc)
          .addFunction("reply", &lua_actor_t::reply)
          .addFunction("link", &lua_actor_t::link)
          .addFunction("monitor", &lua_actor_t::monitor)
          .addFunction("recv", &lua_actor_t::recv)
          .addFunction("recv_match", &lua_actor_t::recv_match)
          .addFunction("recv_response", &lua_actor_t::recv_response)
          .addFunction("recv_response_timeout", &lua_actor_t::recv_response_timeout)
          .addFunction("sleep_for", &lua_actor_t::sleep_for)
          .addFunction("bind", &lua_actor_t::bind)
          .addFunction("connect", &lua_actor_t::connect)
          .addFunction("spawn", &lua_actor_t::spawn)
          .addFunction("spawn_remote", &lua_actor_t::spawn_remote)
          .addFunction("register_service", &lua_actor_t::register_service)
          .addFunction("deregister_service", &lua_actor_t::deregister_service)
          .addFunction("debug", &lua_actor_t::log_debug)
          .addFunction("info", &lua_actor_t::log_info)
          .addFunction("warn", &lua_actor_t::log_warn)
          .addFunction("error", &lua_actor_t::log_error)
          .addFunction("fatal", &lua_actor_t::log_fatal)
        .endClass()
        .addFunction("overloading_0", &lua_overloading_0)
        .addFunction("overloading_1", &lua_overloading_1)
        .addFunction("overloading_2", &lua_overloading_2)
        .addFunction("infin", &make_infin)
        .addFunction("zero", &make_zero)
        .addFunction("millisecs", &lua_millisecs)
        .addFunction("seconds", &lua_seconds)
        .addFunction("minutes", &lua_minutes)
        .addFunction("hours", &lua_hours)
        .addFunction("msg", &lua_msg)
        .addFunction("aid", &lua_aid)
        .addFunction("svcid", &lua_svcid)
        .addFunction("pattern", &lua_pattern)
        .addFunction("make_match", &make_match)
        .addFunction("net_option", &make_net_option)
        .addFunction("atom", &s2i)
        .addFunction("deatom", &i2s)
        .addFunction("default_stacksize", &default_stacksize)
        .addFunction("serialize_number", &serialize_number)
        .addFunction("serialize_string", &serialize_string)
        .addFunction("serialize_boolean", &serialize_boolean)
        .addFunction("deserialize_number", &deserialize_number)
        .addFunction("deserialize_string", &deserialize_string)
        .addFunction("deserialize_boolean", &deserialize_boolean)
        .addFunction("print", &print)
      .endNamespace()
      ;

    luabridge::setGlobal(L, lua_gce_path, "gce_path");
    if (luaL_dostring(L, init_lua_script_.c_str()) != 0)
    {
      std::string errmsg("gce::lua_exception: ");
      errmsg += lua_tostring(L, -1);
      GCE_VERIFY(false)(lua_gce_path)
        .log(lg_, errmsg.c_str()).except<lua_exception>();
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

  luabridge::LuaRef get_script(
    std::string const& name, 
    std::string const& script = std::string()
    )
  {
    script_list_t::iterator itr(script_list_.find(name));
    if (itr != script_list_.end())
    {
      return itr->second;
    }
    else
    {
      lua_State* L = L_.get();
      luabridge::LuaRef sf(L);
      if (script.empty())
      {
        if (luaL_loadfile(L, name.c_str()) != 0)
        {
          return sf;
        }
      }
      else
      {
        if (luaL_loadstring(L, script.c_str()) != 0)
        {
          return sf;
        }
      }
      sf.pop(L);
      script_list_.insert(std::make_pair(name, sf));
      return sf;
    }
  }

  luabridge::LuaRef set_script(
    std::string const& name, 
    std::string const& script = std::string()
    )
  {
    lua_State* L = L_.get();
    luabridge::LuaRef sf(L);
    if (script.empty())
    {
      if (luaL_loadfile(L, name.c_str()) != 0)
      {
        return sf;
      }
    }
    else
    {
      if (luaL_loadstring(L, script.c_str()) != 0)
      {
        return sf;
      }
    }
    sf.pop(L);
    std::pair<script_list_t::iterator, bool> pr =
      script_list_.insert(std::make_pair(name, sf));
    if (!pr.second)
    {
      pr.first->second = sf;
    }
    return sf;
  }

  lua_State* get_lua_state()
  {
    return L_.get();
  }

  aid_t spawn_actor(std::string const& script, aid_t sire, link_type type)
  {
    actor_t* a = make_actor();
    a->init(script);
    if (sire)
    {
      gce::detail::send(*a, sire, msg_new_actor, (boost::uint16_t)type);
    }
    aid_t aid = a->get_aid();
    a->start();
    return aid;
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

  typedef std::map<std::string, luabridge::LuaRef> script_list_t;
  script_list_t script_list_;
  log::logger_t& lg_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_LUA_SERVICE_HPP
