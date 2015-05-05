///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_LUALIB_USERLIB_HPP
#define GCE_LUALIB_USERLIB_HPP

#include <gce/lualib/config.hpp>
#include <boost/thread/tss.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>

namespace gce
{
namespace lualib
{
/// helper functions
inline std::string metatab_name(char const* userdata_name)
{
  return std::string(userdata_name) + "_metatab";
}

class userlib
{
  class userdata
  {
  public:
    userdata(userlib& ul, lua_State* L, char const* name)
      : ul_(ul)
      , L_(L)
      , name_(name)
      , metaname_(metatab_name(name))
    {
    }

  public:
    userdata& add_function(char const* name, lua_CFunction f)
    {
      luaL_Reg reg;
      reg.name = name;
      reg.func = f;
      metareg_list_.push_back(reg);
      return *this;
    }

    userlib& end_userdata()
    {
      if (!metareg_list_.empty())
      {
        luaL_Reg reg;
        reg.name = 0;
        reg.func = 0;
        metareg_list_.push_back(reg);

        luaL_Reg const* lib = &metareg_list_.front();
        luaL_newmetatable(L_, metaname_.c_str());
#if LUA_VERSION_NUM == 501
        adata::lua::luaL_setfuncs(L_, lib, 0);
#else
        luaL_setfuncs(L_, lib, 0);
#endif
        lua_pushvalue(L_, -1);
        lua_setfield(L_, -2, "__index");
      }
      return ul_;
    }

  private:
    userlib& ul_;
    lua_State* L_;
    std::string name_;
    std::string metaname_;
    std::vector<luaL_Reg> metareg_list_;
  };

public:
  explicit userlib(lua_State* L)
    : L_(L)
  {
  }

public:
  userlib& begin(char const* name)
  {
    name_ = name;
    return *this;
  }

  userdata begin_userdata(char const* name)
  {
    return userdata(*this, L_, name);
  }

  userlib& add_function(char const* name, lua_CFunction f)
  {
    luaL_Reg reg;
    reg.name = name;
    reg.func = f;
    reg_list_.push_back(reg);
    return *this;
  }

#if LUA_VERSION_NUM > 501
  struct lua_reg_ptr
  {
    explicit lua_reg_ptr(luaL_Reg* p)
      : p_(p)
    {
    }

    luaL_Reg* p_;
  };

  static int regist_lib(lua_State * L)
  {
    lua_reg_ptr* lib = lib_.get();
    luaL_checkversion(L);
    luaL_newlib(L, lib->p_);
    return 1;
  }
#endif

  void end()
  {
    if (!reg_list_.empty())
    {
      luaL_Reg reg;
      reg.name = 0;
      reg.func = 0;
      reg_list_.push_back(reg);

      luaL_Reg* lib = &reg_list_.front();
#if LUA_VERSION_NUM == 501
      luaL_register(L_, name_.c_str(), lib);
#else
      lib_.reset(new lua_reg_ptr(lib));
      luaL_requiref(L_, name_.c_str(), userlib::regist_lib, 1);
#endif
      lua_newtable(L_);
      lua_setfield(L_, -2, "reftab");
    }
  }

private:
  lua_State* L_;
  std::string name_;
  std::vector<luaL_Reg> reg_list_;

#if LUA_VERSION_NUM > 501
public:
  static boost::thread_specific_ptr<lua_reg_ptr> lib_;
#endif
};

#if LUA_VERSION_NUM > 501
boost::thread_specific_ptr<userlib::lua_reg_ptr> userlib::lib_;
#endif

/// open a userlib
inline userlib open(lua_State* L)
{
  return userlib(L);
}

/// set userdata's metatable
inline void setmetatab(lua_State* L, char const* userdata_name)
{
  std::string metatab = gce::lualib::metatab_name(userdata_name);
  luaL_getmetatable(L, metatab.c_str());
  lua_setmetatable(L, -2);
}

/// make sure push your object that want to be ref first
/// after calling, the ref object will be poped
inline int make_ref(lua_State* L, char const* libname)
{
  lua_getglobal(L, libname);
  lua_getfield(L, -1, "reftab");

  lua_pushvalue(L, -3);
  int r = luaL_ref(L, -2);
  lua_pop(L, 3);
  return r;
}

/// push ref object that k pointed to stack
/// return 0 for not found and nothing on the stack
inline int get_ref(lua_State* L, char const* libname, int k)
{
  lua_getglobal(L, libname);
  lua_getfield(L, -1, "reftab");
  lua_rawgeti(L, -1, k);
  if (lua_isnil(L, -1))
  {
    lua_pop(L, 3);
    return 0;
  }
  else
  {
    lua_replace(L, -3);
    lua_pop(L, 1);
    return 1;
  }
}

/// remove ref
inline void rmv_ref(lua_State* L, char const* libname, int k)
{
  lua_getglobal(L, libname);
  lua_getfield(L, -1, "reftab");
  luaL_unref(L, -1, k);
  lua_pop(L, 2);
}
} /// namespace lualib
} /// namespace gce

#endif /// GCE_LUALIB_USERLIB_HPP
