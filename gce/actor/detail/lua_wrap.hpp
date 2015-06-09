///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_LUA_WRAP_HPP
#define GCE_ACTOR_DETAIL_LUA_WRAP_HPP

#include <gce/actor/config.hpp>
#include <gce/lualib/all.hpp>
#include <gce/actor/lua_object.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/pattern.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/actor/actor_id.adl.c2l.h>
#include <gce/actor/service_id.hpp>
#include <gce/actor/service_id.adl.c2l.h>
#include <gce/actor/net_option.hpp>
#include <gce/actor/net_option.adl.c2l.h>
#include <gce/actor/duration.hpp>
#include <gce/actor/duration.adl.c2l.h>
#include <boost/utility/string_ref.hpp>
#include <cstring>

namespace gce
{
namespace detail
{
namespace lua
{
enum
{
  ec_ok = 0,
  ec_timeout,
  ec_guard,
};

/// helper functions
template <typename Wrapper>
static Wrapper* to_wrap(lua_State* L, int arg)
{
  Wrapper* wrap = static_cast<Wrapper*>(lua_touserdata(L, arg));
  char const* name = Wrapper::name();
  size_t len = std::strlen(name);
  GCE_ASSERT(len < 128);

  char errmsg[128] = "'";
  std::strcpy(errmsg+1, name);
  std::strcpy(errmsg+len+1, "' expected");
  luaL_argcheck(L, wrap != 0, arg, errmsg);
  return wrap;
}

template <typename Wrapper>
static typename Wrapper::object_t* to_obj(lua_State* L, int arg)
{
  Wrapper* wrap = to_wrap<Wrapper>(L, arg);
  return wrap->object();
}

template <typename Object>
static Object* to_obj(lua_State* L, int arg, char const* name)
{
  Object* o = static_cast<Object*>(lua_touserdata(L, arg));
  size_t len = std::strlen(name);
  GCE_ASSERT(len < 128);

  char errmsg[128] = "'";
  std::strcpy(errmsg+1, name);
  std::strcpy(errmsg+len+1, "' expected");
  luaL_argcheck(L, o != 0, arg, errmsg);
  return o;
}
///------------------------------------------------------------------------------
/// service_id
///------------------------------------------------------------------------------
#if GCE_PACKER == GCE_ADATA
static void load(lua_State* L, int arg, gce::svcid_t& svcid)
{
  if (arg != -1)
  {
    lua_pushvalue(L, arg);
  }
  adata::lua::load(L, svcid);
  if (arg != -1)
  {
    lua_pop(L, 1);
  }
}
#endif

struct service_id
#if GCE_PACKER == GCE_AMSG
  : public gce::lua::basic_object
#endif
{
#if GCE_PACKER == GCE_AMSG
  typedef gce::svcid_t object_t;
  object_t obj_;

  object_t* object()
  {
    return &obj_;
  }

  static char const* name()
  {
    return "service_id";
  }

  virtual void pack(gce::message& msg)
  {
    msg << obj_;
  }

  virtual void unpack(gce::message& msg)
  {
    msg >> obj_;
  }

  virtual int gcety() const
  {
    return gce::lua::ty_service_id;
  }

  static int make(lua_State* L)
  {
    create(L);
    return 1;
  }

  static int gc(lua_State* L)
  {
    service_id* wrap = static_cast<service_id*>(lua_touserdata(L, 1));
    if (wrap)
    {
      wrap->~service_id();
    }
    return 0;
  }

  static int tostring(lua_State* L)
  {
    gce::svcid_t* o = to_obj<service_id>(L, 1);

    std::string str = gce::to_string(*o);
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
  }

  static int eq(lua_State* L)
  {
    service_id* lhs = to_wrap<service_id>(L, 1);
    service_id* rhs = to_wrap<service_id>(L, 2);

    lua_pushboolean(L, lhs->obj_ == rhs->obj_);
    return 1;
  }

  static int gcety(lua_State* L)
  {
    service_id* wrap = to_wrap<service_id>(L, 1);

    lua_pushinteger(L, wrap->gcety());
    return 1;
  }

  static gce::svcid_t* create(lua_State* L, gce::svcid_t const& svcid = gce::svcid_t())
  {
    void* block = lua_newuserdata(L, sizeof(service_id));
    if (!block)
    {
      luaL_error(L, "lua_newuserdata for service_id failed");
      return 0;
    }

    service_id* wrap = new (block) service_id;
    wrap->obj_ = svcid;
    gce::lualib::setmetatab(L, "service_id");
    return &wrap->obj_;
  }
#elif GCE_PACKER == GCE_ADATA
  static int tostring(lua_State* L)
  {
    luaL_argcheck(L, lua_istable(L, 1), 1, "table expected");

    gce::svcid_t o;
    load(L, 1, o);

    std::string str = gce::to_string(o);
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
  }
#endif
};

#if GCE_PACKER == GCE_AMSG
static void load(lua_State* L, int arg, gce::svcid_t& svcid)
{
  svcid = *to_obj<service_id>(L, arg);
}
#endif

static void push(lua_State* L, gce::svcid_t const& svcid)
{
#if GCE_PACKER == GCE_AMSG
  service_id::create(L, svcid);
#elif GCE_PACKER == GCE_ADATA
  adata::lua::push(L, svcid);
#endif
}
///------------------------------------------------------------------------------
/// actor_id
///------------------------------------------------------------------------------
#if GCE_PACKER == GCE_ADATA
static void load(lua_State* L, int arg, gce::aid_t& aid)
{
  if (arg != -1)
  {
    lua_pushvalue(L, arg);
  }
  adata::lua::load(L, aid);
  if (arg != -1)
  {
    lua_pop(L, 1);
  }
}
#endif

struct actor_id
#if GCE_PACKER == GCE_AMSG
  : public gce::lua::basic_object
#endif
{
#if GCE_PACKER == GCE_AMSG
  typedef gce::aid_t object_t;
  object_t obj_;

  object_t* object()
  {
    return &obj_;
  }

  static char const* name()
  {
    return "actor_id";
  }

  virtual void pack(gce::message& msg)
  {
    msg << obj_;
  }

  virtual void unpack(gce::message& msg)
  {
    msg >> obj_;
  }

  virtual int gcety() const
  {
    return gce::lua::ty_actor_id;
  }

  static int make(lua_State* L)
  {
    create(L);
    return 1;
  }

  static int gc(lua_State* L)
  {
    actor_id* wrap = static_cast<actor_id*>(lua_touserdata(L, 1));
    if (wrap)
    {
      wrap->~actor_id();
    }
    return 0;
  }

  static int tostring(lua_State* L)
  {
    gce::aid_t* o = to_obj<actor_id>(L, 1);

    std::string str = gce::to_string(*o);
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
  }

  static int eq(lua_State* L)
  {
    actor_id* lhs = to_wrap<actor_id>(L, 1);
    actor_id* rhs = to_wrap<actor_id>(L, 2);

    lua_pushboolean(L, lhs->obj_ == rhs->obj_);
    return 1;
  }

  static int gcety(lua_State* L)
  {
    actor_id* wrap = to_wrap<actor_id>(L, 1);

    lua_pushinteger(L, wrap->gcety());
    return 1;
  }

  static aid_t* create(lua_State* L, gce::aid_t const& aid = gce::aid_t())
  {
    void* block = lua_newuserdata(L, sizeof(actor_id));
    if (!block)
    {
      luaL_error(L, "lua_newuserdata for actor_id failed");
      return 0;
    }

    actor_id* wrap = new (block) actor_id;
    wrap->obj_ = aid;
    gce::lualib::setmetatab(L, "actor_id");
    return &wrap->obj_;
  }
#elif GCE_PACKER == GCE_ADATA
  static int tostring(lua_State* L)
  {
    luaL_argcheck(L, lua_istable(L, 1), 1, "table expected");

    /// using adata cpp2lua to load from lua
    gce::aid_t o;
    load(L, 1, o);

    std::string str = gce::to_string(o);
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
  }
#endif
};

#if GCE_PACKER == GCE_AMSG
static void load(lua_State* L, int arg, gce::aid_t& aid)
{
  aid = *to_obj<actor_id>(L, arg);
}
#endif

static void push(lua_State* L, aid_t const& aid)
{
#if GCE_PACKER == GCE_AMSG
  actor_id::create(L, aid);
#elif GCE_PACKER == GCE_ADATA
  adata::lua::push(L, aid);
#endif
}
///------------------------------------------------------------------------------
/// match
///------------------------------------------------------------------------------
#if GCE_PACKER == GCE_ADATA
static void load(lua_State* L, int arg, gce::match_t& mt)
{
  if (arg != -1)
  {
    lua_pushvalue(L, arg);
  }
  adata::lua::load(L, mt);
  if (arg != -1)
  {
    lua_pop(L, 1);
  }
}
#endif

struct match
#if GCE_PACKER == GCE_AMSG
  : public gce::lua::basic_object
#endif
{
#if GCE_PACKER == GCE_AMSG
  typedef gce::match_t object_t;
  object_t obj_;

  object_t* object()
  {
    return &obj_;
  }

  static char const* name()
  {
    return "match";
  }

  virtual void pack(gce::message& msg)
  {
    msg << obj_;
  }

  virtual void unpack(gce::message& msg)
  {
    msg >> obj_;
  }

  virtual int gcety() const
  {
    return gce::lua::ty_match;
  }

  static int make(lua_State* L)
  {
    int ty = lua_type(L, 1);
    gce::match_t mt = gce::match_nil;
    if (ty == LUA_TNUMBER)
    {
      mt.val_ = lua_tointeger(L, 1);
    }
    create(L, mt);
    return 1;
  }

  static int gc(lua_State* L)
  {
    match* wrap = static_cast<match*>(lua_touserdata(L, 1));
    if (wrap)
    {
      wrap->~match();
    }
    return 0;
  }

  static int tostring(lua_State* L)
  {
    gce::match_t* o = to_obj<match>(L, 1);

    std::string str = gce::to_string(*o);
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
  }

  static int eq(lua_State* L)
  {
    match* lhs = to_wrap<match>(L, 1);
    match* rhs = to_wrap<match>(L, 2);

    lua_pushboolean(L, lhs->obj_ == rhs->obj_);
    return 1;
  }

  static int gcety(lua_State* L)
  {
    match* wrap = to_wrap<match>(L, 1);

    lua_pushinteger(L, wrap->gcety());
    return 1;
  }

  static gce::match_t* create(lua_State* L, gce::match_t const& mt = gce::match_t())
  {
    void* block = lua_newuserdata(L, sizeof(match));
    if (!block)
    {
      luaL_error(L, "lua_newuserdata for match failed");
      return 0;
    }
    match* wrap = new (block) match;
    wrap->obj_ = mt;
    gce::lualib::setmetatab(L, "match");
    return &wrap->obj_;
  }
#elif GCE_PACKER == GCE_ADATA
  static int tostring(lua_State* L)
  {
    luaL_argcheck(L, lua_istable(L, 1), 1, "'match' expected");

    /// using adata cpp2lua to load from lua
    match_t o;
    load(L, 1, o);

    std::string str = gce::to_string(o);
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
  }
#endif 
};

#if GCE_PACKER == GCE_AMSG
static void load(lua_State* L, int arg, gce::match_t& mt)
{
  mt = *to_obj<match>(L, arg);
}
#endif

static void push(lua_State* L, match_t const& mt)
{
#if GCE_PACKER == GCE_AMSG
  match::create(L, mt);
#elif GCE_PACKER == GCE_ADATA
  adata::lua::push(L, mt);
#endif
}
///------------------------------------------------------------------------------
/// message
///------------------------------------------------------------------------------
struct message
  : public gce::lua::basic_object
{
  typedef gce::message object_t;
  object_t obj_;

  object_t* object()
  {
    return &obj_;
  }

  static char const* name()
  {
    return "message";
  }

  virtual void pack(gce::message& msg)
  {
    msg << obj_;
  }

  virtual void unpack(gce::message& msg)
  {
    msg >> obj_;
  }

  virtual int gcety() const
  {
    return gce::lua::ty_message;
  }

  static int make(lua_State* L)
  {
    create(L);
    return 1;
  }

  static int gc(lua_State* L)
  {
    message* wrap = static_cast<message*>(lua_touserdata(L, 1));
    if (wrap)
    {
      wrap->~message();
    }
    return 0;
  }

  static int setty(lua_State* L)
  {
    gce::message* o = to_obj<message>(L, 1);

    gce::match_t mt;
    load(L, 2, mt);

    o->set_type(mt);
    return 0;
  }

  static int getty(lua_State* L)
  {
    gce::message* o = to_obj<message>(L, 1);

    push(L, o->get_type());
    return 1;
  }

  static int size(lua_State* L)
  {
    gce::message* o = to_obj<message>(L, 1);
    lua_pushinteger(L, (lua_Integer)o->size());
    return 1;
  }

  static gce::message* create(lua_State* L, gce::message const& m = gce::message())
  {
    void* block = lua_newuserdata(L, sizeof(message));
    if (!block)
    {
      luaL_error(L, "lua_newuserdata for message failed");
    }
    message* wrap = new (block) message;
    wrap->obj_ = m;
    gce::lualib::setmetatab(L, "message");
    return &wrap->obj_;
  }

  static int gcety(lua_State* L)
  {
    message* wrap = to_wrap<message>(L, 1);

    lua_pushinteger(L, wrap->gcety());
    return 1;
  }

  static int tostring(lua_State* L)
  {
    gce::message* o = to_obj<message>(L, 1);

    std::string str = gce::to_string(*o);
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
  }
};
///------------------------------------------------------------------------------
/// response
///------------------------------------------------------------------------------
struct response
{
  static int make(lua_State* L)
  {
    create(L);
    return 1;
  }

  static int gc(lua_State* L)
  {
    gce::resp_t* o = static_cast<gce::resp_t*>(lua_touserdata(L, 1));
    if (o)
    {
      o->~resp_t();
    }
    return 0;
  }

  static gce::resp_t* create(lua_State* L, gce::resp_t const& resp = gce::resp_t())
  {
    void* block = lua_newuserdata(L, sizeof(gce::resp_t));
    if (!block)
    {
      luaL_error(L, "lua_newuserdata for response failed");
      return 0;
    }
    gce::resp_t* o = new (block) gce::resp_t(resp);
    gce::lualib::setmetatab(L, "response");
    return o;
  }

  static int gcety(lua_State* L)
  {
    lua_pushinteger(L, (int)gce::lua::ty_response);
    return 1;
  }

  static int tostring(lua_State* L)
  {
    gce::resp_t* o = to_obj<gce::resp_t>(L, 1, "response");

    std::string str = gce::to_string(*o);
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
  }
};
///------------------------------------------------------------------------------
/// duration
///------------------------------------------------------------------------------
#if GCE_PACKER == GCE_ADATA
static void load(lua_State* L, int arg, gce::duration_t& dur)
{
  if (arg != -1)
  {
    lua_pushvalue(L, arg);
  }
  adata::lua::load(L, dur);
  if (arg != -1)
  {
    lua_pop(L, 1);
  }
}
#endif

struct duration
#if GCE_PACKER == GCE_AMSG
  : public gce::lua::basic_object
#endif
{
  static void push(lua_State* L, gce::duration_t const& dur)
  {
#if GCE_PACKER == GCE_AMSG
    create(L, dur);
#elif GCE_PACKER == GCE_ADATA
    adata::lua::push(L, dur);
#endif
  }

#if GCE_PACKER == GCE_AMSG
  typedef gce::duration_t object_t;
  object_t obj_;

  object_t* object()
  {
    return &obj_;
  }

  static char const* name()
  {
    return "duration";
  }

  virtual void pack(gce::message& msg)
  {
    msg << obj_;
  }

  virtual void unpack(gce::message& msg)
  {
    msg >> obj_;
  }

  virtual int gcety() const
  {
    return gce::lua::ty_duration;
  }

  static int make_dur(lua_State* L)
  {
    lua_Integer val = luaL_checkinteger(L, 1);
    create(L, gce::duration(val));
    return 1;
  }

  static int make_millisecs(lua_State* L)
  {
    lua_Integer val = luaL_checkinteger(L, 1);
    create(L, gce::millisecs(val));
    return 1;
  }

  static int make_seconds(lua_State* L)
  {
    lua_Integer val = luaL_checkinteger(L, 1);
    create(L, gce::seconds(val));
    return 1;
  }

  static int make_minutes(lua_State* L)
  {
    lua_Integer val = luaL_checkinteger(L, 1);
    create(L, gce::minutes(val));
    return 1;
  }

  static int make_hours(lua_State* L)
  {
    lua_Integer val = luaL_checkinteger(L, 1);
    create(L, gce::hours(val));
    return 1;
  }

  static int gc(lua_State* L)
  {
    duration* wrap = static_cast<duration*>(lua_touserdata(L, 1));
    if (wrap)
    {
      wrap->~duration();
    }
    return 0;
  }

  static int type(lua_State* L)
  {
    gce::duration_t* o = to_obj<duration>(L, 1);

    lua_pushinteger(L, (lua_Integer)o->ty_);
    return 1;
  }

  static int tostring(lua_State* L)
  {
    gce::duration_t* o = to_obj<duration>(L, 1);

    std::string str = gce::to_string(*o);
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
  }

  static int eq(lua_State* L)
  {
    duration* lhs = to_wrap<duration>(L, 1);
    duration* rhs = to_wrap<duration>(L, 2);

    lua_pushboolean(L, lhs->obj_ == rhs->obj_);
    return 1;
  }

  static int lt(lua_State* L)
  {
    duration* lhs = to_wrap<duration>(L, 1);
    duration* rhs = to_wrap<duration>(L, 2);

    lua_pushboolean(L, lhs->obj_ < rhs->obj_);
    return 1;
  }

  static int le(lua_State* L)
  {
    duration* lhs = to_wrap<duration>(L, 1);
    duration* rhs = to_wrap<duration>(L, 2);

    lua_pushboolean(L, lhs->obj_ <= rhs->obj_);
    return 1;
  }

  static int add(lua_State* L)
  {
    duration* lhs = to_wrap<duration>(L, 1);
    duration* rhs = to_wrap<duration>(L, 2);

    gce::duration_t dur = lhs->obj_ + rhs->obj_;
    push(L, dur);
    return 1;
  }

  static int sub(lua_State* L)
  {
    duration* lhs = to_wrap<duration>(L, 1);
    duration* rhs = to_wrap<duration>(L, 2);

    gce::duration_t dur = lhs->obj_ - rhs->obj_;
    push(L, dur);
    return 1;
  }

  static int gcety(lua_State* L)
  {
    duration* wrap = to_wrap<duration>(L, 1);

    lua_pushinteger(L, wrap->gcety());
    return 1;
  }

  static gce::duration_t* create(lua_State* L, gce::duration_t const& dur = gce::zero)
  {
    void* block = lua_newuserdata(L, sizeof(duration));
    if (!block)
    {
      luaL_error(L, "lua_newuserdata for duration failed");
      return 0;
    }
    duration* wrap = new (block) duration;
    wrap->obj_ = dur;
    gce::lualib::setmetatab(L, "duration");
    return &wrap->obj_;
  }
#elif GCE_PACKER == GCE_ADATA
  static int tostring(lua_State* L)
  {
    luaL_argcheck(L, lua_istable(L, 1), 1, "table expected");

    /// using adata cpp2lua to load from lua
    gce::duration_t o;
    load(L, 1, o);

    std::string str = gce::to_string(o);
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
  }

  static int eq(lua_State* L)
  {
    luaL_argcheck(L, lua_istable(L, 1), 1, "'duration' expected");
    luaL_argcheck(L, lua_istable(L, 2), 2, "'duration' expected");

    gce::duration_t lhs;
    load(L, 1, lhs);

    gce::duration_t rhs;
    load(L, 2, lhs);

    lua_pushboolean(L, lhs == rhs);
    return 1;
  }

  static int lt(lua_State* L)
  {
    luaL_argcheck(L, lua_istable(L, 1), 1, "'duration' expected");
    luaL_argcheck(L, lua_istable(L, 2), 2, "'duration' expected");

    gce::duration_t lhs;
    load(L, 1, lhs);

    gce::duration_t rhs;
    load(L, 2, lhs);

    lua_pushboolean(L, lhs < rhs);
    return 1;
  }

  static int le(lua_State* L)
  {
    luaL_argcheck(L, lua_istable(L, 1), 1, "'duration' expected");
    luaL_argcheck(L, lua_istable(L, 2), 2, "'duration' expected");

    gce::duration_t lhs;
    load(L, 1, lhs);

    gce::duration_t rhs;
    load(L, 2, lhs);

    lua_pushboolean(L, lhs <= rhs);
    return 1;
  }

  static int add(lua_State* L)
  {
    luaL_argcheck(L, lua_istable(L, 1), 1, "'duration' expected");
    luaL_argcheck(L, lua_istable(L, 2), 2, "'duration' expected");

    gce::duration_t lhs;
    load(L, 1, lhs);

    gce::duration_t rhs;
    load(L, 2, lhs);

    gce::duration_t dur = lhs + rhs;
    push(L, dur);
    return 1;
  }

  static int sub(lua_State* L)
  {
    luaL_argcheck(L, lua_istable(L, 1), 1, "'duration' expected");
    luaL_argcheck(L, lua_istable(L, 2), 2, "'duration' expected");

    gce::duration_t lhs;
    load(L, 1, lhs);

    gce::duration_t rhs;
    load(L, 2, lhs);

    gce::duration_t dur = lhs - rhs;
    push(L, dur);
    return 1;
  }
#endif
};

#if GCE_PACKER == GCE_AMSG
static void load(lua_State* L, int arg, gce::duration_t& dur)
{
  dur = *to_obj<duration>(L, arg);
}
#endif

static void push(lua_State* L, gce::duration_t const& dur)
{
  duration::push(L, dur);
}
///------------------------------------------------------------------------------
/// net_option
///------------------------------------------------------------------------------
struct net_option
{
  static int make(lua_State* L)
  {
    create(L);
    return 1;
  }

  static void create(lua_State* L, netopt_t const& netopt = make_netopt())
  {
#if GCE_PACKER == GCE_AMSG
    adata::lua::push(L, netopt, false);
#elif GCE_PACKER == GCE_ADATA
    adata::lua::push(L, netopt);
#endif
  }
};

static void load(lua_State* L, int arg, netopt_t& netopt)
{
  if (arg != -1)
  {
    lua_pushvalue(L, arg);
  }
  adata::lua::load(L, netopt);
  if (arg != -1)
  {
    lua_pop(L, 1);
  }
}

static void push(lua_State* L, netopt_t const& netopt)
{
  net_option::create(L, netopt);
}
///------------------------------------------------------------------------------
/// pattern
///------------------------------------------------------------------------------
struct pattern
{
  static int make(lua_State* L)
  {
    void* block = lua_newuserdata(L, sizeof(gce::pattern));
    if (!block)
    {
      return luaL_error(L, "lua_newuserdata for pattern failed");
    }
    new (block) gce::pattern;
    gce::lualib::setmetatab(L, "pattern");
    return 1;
  }

  static int gc(lua_State* L)
  {
    gce::pattern* o = static_cast<gce::pattern*>(lua_touserdata(L, 1));
    if (o)
    {
      o->~pattern();
    }
    return 0;
  }

  static int set_timeout(lua_State* L)
  {
    gce::pattern* patt = to_obj<gce::pattern>(L, 1, "pattern");

    gce::duration_t dur;
    load(L, 2, dur);
    patt->timeout_ = dur;
    return 0;
  }

  static int add_match(lua_State* L)
  {
    gce::pattern* patt = to_obj<gce::pattern>(L, 1, "pattern");

    int ty = lua_type(L, 2);
    switch (ty)
    {
    case LUA_TNUMBER: 
    {
      lua_Integer i = luaL_checkinteger(L, 2);
      patt->add_match(i);
    }break;
    case LUA_TSTRING: 
    {
      char const* str = luaL_checkstring(L, 2);
      patt->add_match(str);
    }break;
    default:
    {
      match_t mt;
      load(L, 2, mt);
      patt->add_match(mt);
    }break;
    }

    return 0;
  }

  static int set_match_aid(lua_State* L)
  {
    gce::pattern* patt = to_obj<gce::pattern>(L, 1, "pattern");

    gce::aid_t aid;
    load(L, 2, aid);

    patt->recver_ = aid;
    return 0;
  }

  static int set_match_svcid(lua_State* L)
  {
    gce::pattern* patt = to_obj<gce::pattern>(L, 1, "pattern");

    gce::svcid_t svcid;
    load(L, 2, svcid);

    patt->recver_ = svcid;
    return 0;
  }

  static int gcety(lua_State* L)
  {
    lua_pushinteger(L, (int)gce::lua::ty_pattern);
    return 1;
  }

  static int tostring(lua_State* L)
  {
    gce::pattern* o = to_obj<gce::pattern>(L, 1, "pattern");

    std::string str = gce::to_string(*o);
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
  }
};
///------------------------------------------------------------------------------
/// chunk
///------------------------------------------------------------------------------
struct chunk
  : public gce::lua::basic_object
{
  typedef gce::message::chunk object_t;
  object_t obj_;

  object_t* object()
  {
    return &obj_;
  }

  static char const* name()
  {
    return "chunk";
  }

  virtual void pack(gce::message& msg)
  {
    msg << obj_;
  }

  virtual void unpack(gce::message& msg)
  {
    msg >> obj_;
  }

  virtual int gcety() const
  {
    return gce::lua::ty_chunk;
  }

  static int make(lua_State* L)
  {
    void* block = lua_newuserdata(L, sizeof(chunk));
    if (!block)
    {
      return luaL_error(L, "lua_newuserdata for chunk failed");
    }

    chunk* wrap = new (block) chunk;
    int luaty = lua_type(L, 1);
    if (luaty == LUA_TNONE || luaty == LUA_TNIL)
    {
      wrap->obj_ = gce::message::chunk();
    }
    else if (luaty == LUA_TSTRING)
    {
      size_t len = 0;
      byte_t const* data = (byte_t const*)lua_tolstring(L, 1, &len);
      wrap->obj_ = gce::message::chunk(data, len);
    }
    else if (luaty == LUA_TNUMBER)
    {
      size_t len = (size_t)lua_tointeger(L, 1);
      wrap->obj_ = gce::message::chunk(len);
    }

    gce::lualib::setmetatab(L, "chunk");
    return 1;
  }

  static int gc(lua_State* L)
  {
    chunk* wrap = static_cast<chunk*>(lua_touserdata(L, 1));
    if (wrap)
    {
      wrap->~chunk();
    }
    return 0;
  }

  static int gcety(lua_State* L)
  {
    chunk* wrap = to_wrap<chunk>(L, 1);

    lua_pushinteger(L, wrap->gcety());
    return 1;
  }

  static int tostring(lua_State* L)
  {
    gce::message::chunk* o = to_obj<chunk>(L, 1);

    std::string str = gce::to_string(*o);
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
  }

  static int to_string(lua_State* L)
  {
    gce::message::chunk* o = to_obj<chunk>(L, 1);

    lua_pushlstring(L, (char const*)o->data(), o->size());
    return 1;
  }
  
  static int from_string(lua_State* L)
  {
    gce::message::chunk* o = to_obj<chunk>(L, 1);

    o->data_ = (byte_t const*)luaL_checklstring(L, 2, &o->len_);
    return 0;
  }
};
///------------------------------------------------------------------------------
/// errcode
///------------------------------------------------------------------------------
struct errcode
  : public gce::lua::basic_object
{
  typedef gce::errcode_t object_t;
  object_t obj_;

  object_t* object()
  {
    return &obj_;
  }

  static char const* name()
  {
    return "errcode";
  }

  virtual void pack(gce::message& msg)
  {
    msg << obj_;
  }

  virtual void unpack(gce::message& msg)
  {
    msg >> obj_;
  }

  virtual int gcety() const
  {
    return gce::lua::ty_errcode;
  }

  static int make(lua_State* L)
  {
    create(L);
    return 1;
  }

  static int gc(lua_State* L)
  {
    errcode* wrap = static_cast<errcode*>(lua_touserdata(L, 1));
    if (wrap)
    {
      wrap->~errcode();
    }
    return 0;
  }

  static int value(lua_State* L)
  {
    gce::errcode_t* o = to_obj<errcode>(L, 1);

    lua_pushinteger(L, o->value());
    return 1;
  }

  static int errmsg(lua_State* L)
  {
    gce::errcode_t* o = to_obj<errcode>(L, 1);

    std::string msg = o->message();
    lua_pushlstring(L, msg.c_str(), msg.size());
    return 1;
  }

  static gce::errcode_t* create(lua_State* L, gce::errcode_t const& ec = gce::errcode_t())
  {
    void* block = lua_newuserdata(L, sizeof(errcode));
    if (!block)
    {
      luaL_error(L, "lua_newuserdata for errcode failed");
      return 0;
    }
    errcode* wrap = new (block) errcode;
    wrap->obj_ = ec;
    gce::lualib::setmetatab(L, "errcode");
    return &wrap->obj_;
  }

  static int gcety(lua_State* L)
  {
    errcode* wrap = to_wrap<errcode>(L, 1);

    lua_pushinteger(L, wrap->gcety());
    return 1;
  }

  static int tostring(lua_State* L)
  {
    gce::errcode_t* o = to_obj<errcode>(L, 1);

    std::string str = gce::to_string(*o);
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
  }

  static int eq(lua_State* L)
  {
    errcode* lhs = to_wrap<errcode>(L, 1);
    errcode* rhs = to_wrap<errcode>(L, 2);

    lua_pushboolean(L, lhs->obj_ == rhs->obj_);
    return 1;
  }
};
///------------------------------------------------------------------------------
/// atom/deatom
///------------------------------------------------------------------------------
inline int s2i(lua_State* L)
{
  char const* str = luaL_checkstring(L, 1);
  push(L, gce::atom(str));
  return 1;
}
///------------------------------------------------------------------------------
inline int i2s(lua_State* L)
{
  match_t mt;
  load(L, 1, mt);
  std::string str = gce::atom(mt);
  lua_pushstring(L, str.c_str());
  return 1;
}
///------------------------------------------------------------------------------
/// actor
///------------------------------------------------------------------------------
template <typename Impl>
struct actor
{
  typedef Impl impl_t;
  typedef typename impl_t::proxy proxy_t;

  static int create(lua_State* L, impl_t* impl)
  {
    void* block = lua_newuserdata(L, sizeof(proxy_t));
    if (!block)
    {
      return luaL_error(L, "lua_newuserdata for actor failed");
    }
    new (block) proxy_t(impl);
    gce::lualib::setmetatab(L, "actor");
    return gce::lualib::make_ref(L, "libgce");
  }

  static int gc(lua_State* L)
  {
    proxy_t* o = static_cast<proxy_t*>(lua_touserdata(L, 1));
    if (o)
    {
      o->~proxy_t();
    }
    return 0;
  }

  static int get_aid(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

    proxy_t& a = *o;

    aid_t const& aid = a->get_aid();
    push(L, aid);
    return 1;
  }

  static int init_coro(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");
    luaL_argcheck(L, lua_isthread(L, 2), 2, "'thread' expected");
    luaL_argcheck(L, lua_isfunction(L, 3), 3, "'function' expected");

    proxy_t& a = *o;

    lua_pushvalue(L, 2);
    int c = gce::lualib::make_ref(L, "libgce");
    lua_pushvalue(L, 3);
    int f = gce::lualib::make_ref(L, "libgce");
    a->init_coro(c, f);
    return 0;
  }

  static int send(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

#if GCE_PACKER == GCE_AMSG
    int arg2check = lua_isuserdata(L, 2);
#elif GCE_PACKER == GCE_ADATA
    int arg2check = lua_istable(L, 2);
#endif
    luaL_argcheck(L, arg2check, 2, "'actor_id' expected");

    gce::message* msg = to_obj<message>(L, 3);

    gce::aid_t aid;
    load(L, 2, aid);

    proxy_t& a = *o;

    a->send(aid, *msg);
    return 0;
  }
  
  static int send2svc(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

#if GCE_PACKER == GCE_AMSG
    int arg2check = lua_isuserdata(L, 2);
#elif GCE_PACKER == GCE_ADATA
    int arg2check = lua_istable(L, 2);
#endif
    luaL_argcheck(L, arg2check, 2, "'service_id' expected");

    gce::message* msg = to_obj<message>(L, 3);

    gce::svcid_t svcid;
    load(L, 2, svcid);

    proxy_t& a = *o;

    a->send(svcid, *msg);
    return 0;
  }
  
  static int send2svcs(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

#if GCE_PACKER == GCE_AMSG
    int arg2check = lua_isuserdata(L, 2);
#elif GCE_PACKER == GCE_ADATA
    int arg2check = lua_istable(L, 2);
#endif
    luaL_argcheck(L, arg2check, 2, "'match' expected");

    gce::message* msg = to_obj<message>(L, 3);

    gce::match_t name;
    load(L, 2, name);

    proxy_t& a = *o;

    a->send(name, *msg);
    return 0;
  }

  static int relay(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

#if GCE_PACKER == GCE_AMSG
    int arg2check = lua_isuserdata(L, 2);
#elif GCE_PACKER == GCE_ADATA
    int arg2check = lua_istable(L, 2);
#endif
    luaL_argcheck(L, arg2check, 2, "'actor_id' expected");

    gce::message* msg = to_obj<message>(L, 3);

    gce::aid_t aid;
    load(L, 2, aid);

    proxy_t& a = *o;

    a->relay(aid, *msg);
    return 0;
  }
  
  static int relay2svc(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

#if GCE_PACKER == GCE_AMSG
    int arg2check = lua_isuserdata(L, 2);
#elif GCE_PACKER == GCE_ADATA
    int arg2check = lua_istable(L, 2);
#endif
    luaL_argcheck(L, arg2check, 2, "'service_id' expected");

    gce::message* msg = to_obj<message>(L, 3);

    gce::svcid_t svcid;
    load(L, 2, svcid);

    proxy_t& a = *o;

    a->relay(svcid, *msg);
    return 0;
  }
  
  static int relay2svcs(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

#if GCE_PACKER == GCE_AMSG
    int arg2check = lua_isuserdata(L, 2);
#elif GCE_PACKER == GCE_ADATA
    int arg2check = lua_istable(L, 2);
#endif
    luaL_argcheck(L, arg2check, 2, "'match' expected");

    gce::message* msg = to_obj<message>(L, 3);

    gce::match_t name;
    load(L, 2, name);

    proxy_t& a = *o;

    a->relay(name, *msg);
    return 0;
  }

  static int request(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

#if GCE_PACKER == GCE_AMSG
    int arg2check = lua_isuserdata(L, 2);
#elif GCE_PACKER == GCE_ADATA
    int arg2check = lua_istable(L, 2);
#endif
    luaL_argcheck(L, arg2check, 2, "'actor_id' expected");

    gce::message* msg = to_obj<message>(L, 3);

    gce::aid_t aid;
    load(L, 2, aid);

    proxy_t& a = *o;

    gce::resp_t resp = a->request(aid, *msg);
    response::create(L, resp);
    return 1;
  }
  
  static int request2svc(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

#if GCE_PACKER == GCE_AMSG
    int arg2check = lua_isuserdata(L, 2);
#elif GCE_PACKER == GCE_ADATA
    int arg2check = lua_istable(L, 2);
#endif
    luaL_argcheck(L, arg2check, 2, "'service_id' expected");

    gce::message* msg = to_obj<message>(L, 3);

    gce::svcid_t svcid;
    load(L, 2, svcid);

    proxy_t& a = *o;

    gce::resp_t resp = a->request(svcid, *msg);
    response::create(L, resp);
    return 1;
  }
  
  static int request2svcs(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

#if GCE_PACKER == GCE_AMSG
    int arg2check = lua_isuserdata(L, 2);
#elif GCE_PACKER == GCE_ADATA
    int arg2check = lua_istable(L, 2);
#endif
    luaL_argcheck(L, arg2check, 2, "'match' expected");

    gce::message* msg = to_obj<message>(L, 3);

    gce::match_t name;
    load(L, 2, name);

    proxy_t& a = *o;

    gce::resp_t resp = a->request(name, *msg);
    response::create(L, resp);
    return 1;
  }

  static int reply(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

#if GCE_PACKER == GCE_AMSG
    int arg2check = lua_isuserdata(L, 2);
#elif GCE_PACKER == GCE_ADATA
    int arg2check = lua_istable(L, 2);
#endif
    luaL_argcheck(L, arg2check, 2, "'actor_id' expected");

    gce::message* msg = to_obj<message>(L, 3);

    aid_t aid;
    load(L, 2, aid);

    proxy_t& a = *o;

    a->reply(aid, *msg);
    return 0;
  }

  static int link(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

#if GCE_PACKER == GCE_AMSG
    int arg2check = lua_isuserdata(L, 2);
#elif GCE_PACKER == GCE_ADATA
    int arg2check = lua_istable(L, 2);
#endif
    luaL_argcheck(L, arg2check, 2, "'actor_id' expected");

    aid_t aid;
    load(L, 2, aid);

    proxy_t& a = *o;

    a->link(aid);
    return 0;
  }

  static int monitor(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

#if GCE_PACKER == GCE_AMSG
    int arg2check = lua_isuserdata(L, 2);
#elif GCE_PACKER == GCE_ADATA
    int arg2check = lua_istable(L, 2);
#endif
    luaL_argcheck(L, arg2check, 2, "'actor_id' expected");

    aid_t aid;
    load(L, 2, aid);

    proxy_t& a = *o;

    a->monitor(aid);
    return 0;
  }

  static int recv(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

    proxy_t& a = *o;

    bool r = a->recv();
    push_coro(L, a, r);
    return 1;
  }

  static int recv_match(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");
    gce::pattern* patt = to_obj<gce::pattern>(L, 2, "pattern");

    proxy_t& a = *o;

    bool r = a->recv_match(*patt);
    push_coro(L, a, r);
    return 1;
  }

  static int recv_response(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");
    gce::resp_t* resp = to_obj<gce::resp_t>(L, 2, "response");

    proxy_t& a = *o;

    bool r = a->recv_response(*resp);
    push_coro(L, a, r);
    return 1;
  }

  static int recv_response_timeout(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");
    gce::resp_t* resp = to_obj<gce::resp_t>(L, 2, "response");

    gce::duration_t tmo;
    load(L, 3, tmo);

    proxy_t& a = *o;

    bool r = a->recv_response_timeout(*resp, tmo);
    push_coro(L, a, r);
    return 1;
  }

  static int sleep_for(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

    gce::duration_t dur;
    load(L, 2, dur);

    proxy_t& a = *o;

    bool r = a->sleep_for(dur);
    push_coro(L, a, r);
    return 1;
  }

  static int bind(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

    size_t len = 0;
    char const* ep = luaL_checklstring(L, 2, &len);

    netopt_t netopt;
    load(L, 3, netopt);

    proxy_t& a = *o;

    bool r = a->bind(std::string(ep, len), netopt);
    push_coro(L, a, r);
    return 1;
  }

  static int connect(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

    match_t target;
    load(L, 2, target);

    size_t len = 0;
    char const* ep = luaL_checklstring(L, 3, &len);

    netopt_t netopt;
    load(L, 4, netopt);

    proxy_t& a = *o;

    bool r = a->connect(target, std::string(ep, len), netopt);
    push_coro(L, a, r);
    return 1;
  }

  static int spawn(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

    int spw_ty = (int)luaL_checkinteger(L, 2);

    size_t len = 0;
    char const* func = luaL_checklstring(L, 3, &len);

    luaL_argcheck(L, lua_isboolean(L, 4), 4, "'boolean' expected");
    bool sync_sire = lua_toboolean(L, 4) != 0;

    int type = (int)luaL_checkinteger(L, 5);
    size_t stack_size = (size_t)luaL_checkinteger(L, 6);

    proxy_t& a = *o;

    bool r = a->spawn(spw_ty, std::string(func, len), sync_sire, type, stack_size);
    push_coro(L, a, r);
    return 1;
  }

  static int spawn_remote(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

    spawn_type sty = (spawn_type)luaL_checkinteger(L, 2);

    size_t len = 0;
    char const* func = luaL_checklstring(L, 3, &len);

    match_t ctxid;
    load(L, 4, ctxid);

    int type = (int)luaL_checkinteger(L, 5);

    size_t stack_size = luaL_checkinteger(L, 6);

    gce::duration_t tmo;
    load(L, 7, tmo);

    proxy_t& a = *o;

    bool r = a->spawn_remote(sty, std::string(func, len), ctxid, type, stack_size, tmo);
    push_coro(L, a, r);
    return 1;
  }

  static int register_service(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

    match_t name;
    load(L, 2, name);

    proxy_t& a = *o;

    a->register_service(name);
    return 0;
  }

  static int deregister_service(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

    match_t name;
    load(L, 2, name);

    proxy_t& a = *o;

    a->deregister_service(name);
    return 0;
  }

  static int log_debug(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

    size_t len = 0;
    char const* str = luaL_checklstring(L, 2, &len);

    proxy_t& a = *o;

    a->log_debug(boost::string_ref(str, len));
    return 0;
  }

  static int log_info(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

    size_t len = 0;
    char const* str = luaL_checklstring(L, 2, &len);

    proxy_t& a = *o;

    a->log_info(boost::string_ref(str, len));
    return 0;
  }

  static int log_warn(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

    size_t len = 0;
    char const* str = luaL_checklstring(L, 2, &len);

    proxy_t& a = *o;

    a->log_warn(boost::string_ref(str, len));
    return 0;
  }

  static int log_error(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

    size_t len = 0;
    char const* str = luaL_checklstring(L, 2, &len);

    proxy_t& a = *o;

    a->log_info(boost::string_ref(str, len));
    return 0;
  }

  static int log_fatal(lua_State* L)
  {
    proxy_t* o = to_obj<proxy_t>(L, 1, "actor");

    size_t len = 0;
    char const* str = luaL_checklstring(L, 2, &len);

    proxy_t& a = *o;
    
    a->log_fatal(boost::string_ref(str, len));
    return 0;
  }

  static void push_coro(lua_State* L, proxy_t& a, bool r)
  {
    if (r)
    {
      int co = a->get_coro();
      if (gce::lualib::get_ref(L, "libgce", co) == 0)
      {
        luaL_error(L, "actor coro missing");
      }
    }
    else
    {
      lua_pushnil(L);
    }
  }
};
///------------------------------------------------------------------------------
/// serialize
///------------------------------------------------------------------------------
inline int pack_number(lua_State* L)
{
  gce::message* msg = to_obj<message>(L, 1);

  lua_Number num = lua_tonumber(L, 2);
  try
  {
    *msg << num;
  }
  catch (std::exception& ex)
  {
    return luaL_error(L, ex.what());
  }
  return 0;
}
///------------------------------------------------------------------------------
inline int pack_string(lua_State* L)
{
  gce::message* msg = to_obj<message>(L, 1);

  size_t len = 0;
  char const* str = luaL_checklstring(L, 2, &len);
  try
  {
    *msg << boost::string_ref(str, len);
  }
  catch (std::exception& ex)
  {
    return luaL_error(L, ex.what());
  }
  return 0;
}
///------------------------------------------------------------------------------
inline int pack_boolean(lua_State* L)
{
  gce::message* msg = to_obj<message>(L, 1);

  bool b = lua_toboolean(L, 2) != 0;
  try
  {
    *msg << b;
  }
  catch (std::exception& ex)
  {
    return luaL_error(L, ex.what());
  }
  return 0;
}
///------------------------------------------------------------------------------
inline int pack_object(lua_State* L)
{
  gce::message* msg = to_obj<message>(L, 1);

  int ty = lua_type(L, 2);
#if GCE_PACKER == GCE_AMSG
  luaL_argcheck(
    L, (ty == LUA_TUSERDATA || ty == LUA_TLIGHTUSERDATA), 
    2, "'userdata' or 'lightuserdata' expected"
    );
#elif GCE_PACKER == GCE_ADATA
  luaL_argcheck(
    L, (ty == LUA_TUSERDATA || ty == LUA_TLIGHTUSERDATA || ty == LUA_TTABLE), 
    2, "'table' or 'userdata'/'lightuserdata' expected"
    );
#endif

  if (ty == LUA_TTABLE)
  {
#if GCE_PACKER == GCE_ADATA
    /// adata obj
    /// call 'size_of' to get serilaizing size
    if (luaL_callmeta(L, 2, "size_of") == 0)
    {
      return luaL_error(L, "metatable not found!");
    }

    lua_Integer len = lua_tointeger(L, -1);
    lua_pop(L, 1);

    /// get obj's metatable
    if (lua_getmetatable(L, 2) == 0)
    {
      return luaL_error(L, "object must have a metatable!");
    }

    try
    {
      /// adata raw write
      msg->pre_write(len);
      lua_getfield(L, -1, "write");
      lua_pushvalue(L, 2);
      lua_pushlightuserdata(L, &msg->get_packer().get_stream());
      if (lua_pcall(L, 2, 1, 0) != 0)
      {
        return luaL_error(L, lua_tostring(L, -1));
      }

      lua_Integer ec = lua_tointeger(L, -1);
      lua_pop(L, 1);
      if (ec != adata::success)
      {
        adata::zero_copy_buffer& stream = msg->get_packer().get_stream();
        return luaL_error(L, stream.message());
      }
      msg->end_write();

      /// pop obj's metatable
      lua_pop(L, 1);
    }
    catch (std::exception& ex)
    {
      return luaL_error(L, ex.what());
    }
#endif
  }
  else
  {
    /// gce obj
    /// call virtual method pack
    try
    {
      gce::lua::basic_object* o = to_obj<gce::lua::basic_object>(L, 2, "lua basic_object");
      o->pack(*msg);
    }
    catch (std::exception& ex)
    {
      return luaL_error(L, ex.what());
    }
  }
  return 0;
}
///------------------------------------------------------------------------------
/// deserialize
///------------------------------------------------------------------------------
inline int unpack_number(lua_State* L)
{
  gce::message* msg = to_obj<message>(L, 1);

  lua_Number num;
  try
  {
    *msg >> num;
  }
  catch (std::exception& ex)
  {
    return luaL_error(L, ex.what());
  }
  lua_pushnumber(L, num);
  return 1;
}
///------------------------------------------------------------------------------
inline int unpack_string(lua_State* L)
{
  gce::message* msg = to_obj<message>(L, 1);

  boost::string_ref str;
  try
  {
    *msg >> str;
  }
  catch (std::exception& ex)
  {
    return luaL_error(L, ex.what());
  }
  lua_pushlstring(L, str.data(), str.size());
  return 1;
}
///------------------------------------------------------------------------------
inline int unpack_boolean(lua_State* L)
{
  gce::message* msg = to_obj<message>(L, 1);

  bool b;
  try
  {
    *msg >> b;
  }
  catch (std::exception& ex)
  {
    return luaL_error(L, ex.what());
  }
  lua_pushboolean(L, b);
  return 1;
}
///------------------------------------------------------------------------------
inline int unpack_object(lua_State* L)
{
  gce::message* msg = to_obj<message>(L, 1);

  int ty = lua_type(L, 2);
#if GCE_PACKER == GCE_AMSG
  luaL_argcheck(
    L, (ty == LUA_TUSERDATA || ty == LUA_TLIGHTUSERDATA), 
    2, "'userdata' or 'lightuserdata' expected"
    );
#elif GCE_PACKER == GCE_ADATA
  luaL_argcheck(
    L, (ty == LUA_TUSERDATA || ty == LUA_TLIGHTUSERDATA || ty == LUA_TTABLE), 
    2, "'table' or 'userdata'/'lightuserdata' expected"
    );
#endif

  if (ty == LUA_TTABLE)
  {
#if GCE_PACKER == GCE_ADATA
    /// get obj's metatable
    if (lua_getmetatable(L, 2) == 0)
    {
      return luaL_error(L, "object must have a metatable!");
    }

    try
    {
      /// adata obj
      /// adata raw read
      msg->pre_read();
      lua_getfield(L, -1, "read");
      lua_pushvalue(L, 2);
      lua_pushlightuserdata(L, &msg->get_packer().get_stream());
      if (lua_pcall(L, 2, 1, 0) != 0)
      {
        return luaL_error(L, lua_tostring(L, -1));
      }

      lua_Integer ec = lua_tointeger(L, -1);
      lua_pop(L, 1);
      if (ec != adata::success)
      {
        adata::zero_copy_buffer& stream = msg->get_packer().get_stream();
        return luaL_error(L, stream.message());
      }
      msg->end_read();

      /// pop obj's metatable
      lua_pop(L, 1);
    }
    catch (std::exception& ex)
    {
      return luaL_error(L, ex.what());
    }
#endif
  }
  else
  {
    /// gce obj
    /// call virtual method unpack
    try
    {
      gce::lua::basic_object* o = to_obj<gce::lua::basic_object>(L, 2, "lua basic_object");
      o->unpack(*msg);
    }
    catch (std::exception& ex)
    {
      return luaL_error(L, ex.what());
    }
  }

  return 0;
}
///------------------------------------------------------------------------------
/// print
///------------------------------------------------------------------------------
inline int print(lua_State* L)
{
  char const* str = luaL_checkstring(L, 1);
  if (str != 0)
  {
    std::printf("%s\n", str);
  }
  else
  {
    std::printf("\n");
  }
  return 0;
}
///------------------------------------------------------------------------------
/// init_nil
///------------------------------------------------------------------------------
inline int init_nil(lua_State* L)
{
  /// set nil
  lua_getglobal(L, "libgce");
  lua::push(L, gce::zero);
  lua_setfield(L, -2, "zero");
  lua::push(L, gce::infin);
  lua_setfield(L, -2, "infin");
  lua::push(L, gce::aid_nil);
  lua_setfield(L, -2, "aid_nil");
  lua::push(L, gce::svcid_nil);
  lua_setfield(L, -2, "svcid_nil");
  lua::push(L, gce::match_nil);
  lua_setfield(L, -2, "match_nil");
  lua::errcode::create(L);
  lua_setfield(L, -2, "err_nil");
  lua_pop(L, 1);
  return 0;
}
///------------------------------------------------------------------------------
} /// namespace lua
} /// namespace detail
} /// namespace gce

#endif /// GCE_ACTOR_DETAIL_LUA_WRAP_HPP
