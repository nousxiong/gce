///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

namespace gce
{
class lualib_ut
{
public:
  static void run()
  {
    std::cout << "lualib_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      test_metatable();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "lualib_ut end." << std::endl;
  }

private:
  struct kvdb
  {
    std::map<std::string, std::string> data_;
    typedef std::map<std::string, std::string>::iterator iterator_t;

    static int make(lua_State* L)
    {
      void* block = lua_newuserdata(L, sizeof(kvdb));
      if (!block)
      {
        return luaL_error(L, "lua_newuserdata for kvdb failed");
      }
      kvdb* o = new (block) kvdb;

      gce::lualib::setmetatab(L, "kvdb");
      return 1;
    }

    static int dispose(lua_State* L)
    {
      kvdb* o = static_cast<kvdb*>(lua_touserdata(L, 1));
      if (o)
      {
        o->~kvdb();
      }
      return 0;
    }

    static int set(lua_State* L)
    {
      kvdb* o = static_cast<kvdb*>(lua_touserdata(L, 1));
      if (!o)
      {
        return luaL_error(L, "arg 1 must be kvdb!");
      }

      size_t klen = 0;
      char const* kstr = luaL_checklstring(L, 2, &klen);
      size_t vlen = 0;
      char const* vstr = luaL_checklstring(L, 3, &vlen);

      std::string key(kstr, klen);
      std::string value(vstr, vlen);
      std::pair<iterator_t, bool> pr = 
        o->data_.insert(std::make_pair(key, value));
      if (!pr.second)
      {
        pr.first->second = value;
      }
      return 0;
    }

    static int get(lua_State* L)
    {
      kvdb* o = static_cast<kvdb*>(lua_touserdata(L, 1));
      if (!o)
      {
        return luaL_error(L, "arg 1 must be kvdb!");
      }

      size_t klen = 0;
      char const* kstr = luaL_checklstring(L, 2, &klen);

      std::string key(kstr, klen);
      iterator_t it = o->data_.find(key);
      if (it != o->data_.end())
      {
        lua_pushlstring(L, it->second.c_str(), it->second.size());
      }
      else
      {
        lua_pushnil(L);
      }
      return 1;
    }

    static int del(lua_State* L)
    {
      kvdb* o = static_cast<kvdb*>(lua_touserdata(L, 1));
      if (!o)
      {
        return luaL_error(L, "arg 1 must be kvdb!");
      }

      size_t klen = 0;
      char const* kstr = luaL_checklstring(L, 2, &klen);

      std::string key(kstr, klen);
      o->data_.erase(key);
      return 0;
    }

    static int equals(lua_State* L)
    {
      kvdb* lhs = static_cast<kvdb*>(lua_touserdata(L, 1));
      if (!lhs)
      {
        return luaL_error(L, "arg 1 must be kvdb!");
      }

      kvdb* rhs = static_cast<kvdb*>(lua_touserdata(L, 2));
      if (!rhs)
      {
        return luaL_error(L, "arg 2 must be kvdb!");
      }
      lua_pushboolean(L, lhs->data_ == rhs->data_);
      return 1;
    }
  };

  struct aid
  {
    int id_;
    std::string name_;

    aid()
      : id_(0)
    {
    }

    static int make(lua_State* L)
    {
      void* block = lua_newuserdata(L, sizeof(aid));
      if (!block)
      {
        return luaL_error(L, "lua_newuserdata for aid failed");
      }
      aid* o = new (block) aid;

      gce::lualib::setmetatab(L, "aid");
      return 1;
    }

    static int dispose(lua_State* L)
    {
      aid* o = static_cast<aid*>(lua_touserdata(L, 1));
      if (o)
      {
        o->~aid();
      }
      return 0;
    }

    static int get(lua_State* L)
    {
      aid* o = static_cast<aid*>(lua_touserdata(L, 1));
      if (!o)
      {
        return luaL_error(L, "arg 1 must be aid!");
      }

      char const* member = luaL_checkstring(L, 2);
      if (strcmp(member, "id_") == 0)
      {
        lua_pushinteger(L, o->id_);
      }
      else if (strcmp(member, "name_") == 0)
      {
        lua_pushstring(L, o->name_.c_str());
      }
      else
      {
        return luaL_error(L, "aid\'s member %s not exist!", member);
      }

      return 1;
    }

    static int set(lua_State* L)
    {
      aid* o = static_cast<aid*>(lua_touserdata(L, 1));
      if (!o)
      {
        return luaL_error(L, "arg 1 must be aid!");
      }

      char const* member = luaL_checkstring(L, 2);
      if (strcmp(member, "id_") == 0)
      {
        o->id_ = (int)luaL_checkinteger(L, 3);
      }
      else if (strcmp(member, "name_") == 0)
      {
        o->name_ = luaL_checkstring(L, 3);
      }
      else
      {
        return luaL_error(L, "aid\'s member %s not exist!", member);
      }
      return 0;
    }

    static int tostring(lua_State* L)
    {
      aid* o = static_cast<aid*>(lua_touserdata(L, 1));
      if (!o)
      {
        return luaL_error(L, "arg 1 must be aid!");
      }

      lua_pushfstring(L, "aid<%d><%s><%p>", o->id_, o->name_.c_str(), o);
      return 1;
    }

    static int equals(lua_State* L)
    {
      aid* lhs = static_cast<aid*>(lua_touserdata(L, 1));
      if (!lhs)
      {
        return luaL_error(L, "arg 1 must be aid!");
      }

      aid* rhs = static_cast<aid*>(lua_touserdata(L, 2));
      if (!rhs)
      {
        return luaL_error(L, "arg 2 must be aid!");
      }
      lua_pushboolean(L, lhs->id_ == rhs->id_ && lhs->name_ == rhs->name_);
      return 1;
    }

    static int copy_of(lua_State* L)
    {
      aid* o = static_cast<aid*>(lua_touserdata(L, 1));
      if (!o)
      {
        return luaL_error(L, "arg 1 must be aid!");
      }

      aid* other = static_cast<aid*>(lua_touserdata(L, 2));
      if (!other)
      {
        return luaL_error(L, "arg 2 must be aid!");
      }

      o->id_ = other->id_;
      o->name_ = other->name_;
      return 0;
    }

    static int func(lua_State* L)
    {
      aid* o = static_cast<aid*>(lua_touserdata(L, 1));
      if (!o)
      {
        return luaL_error(L, "arg 1 must be aid!");
      }
      o->id_--;
      o->name_.assign("changed", o->id_+1);
      return 0;
    }

    static int push_table(lua_State* L)
    {
      luaL_argcheck(L, lua_istable(L, 1), 1, "'table' expected");
      lua_pushvalue(L, 1);
      return 1;
    }

    static int push_udata(lua_State* L)
    {
      aid* o = static_cast<aid*>(lua_touserdata(L, 1));
      if (!o)
      {
        return luaL_error(L, "arg 1 must be aid!");
      }
      lua_pushvalue(L, 1);
      return 1;
    }

    static int test_lib(lua_State* L)
    {
      lua_getglobal(L, "libkv");
      lua_getfield(L, -1, "i");
      lua_Integer i = lua_tointeger(L, -1);
      std::cout << "libkv.i: " << i << std::endl;
      return 1;
    }
  };

  static void test_common()
  {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    try
    {
      gce::lualib::open(L)
        .begin("libkv")
          .add_function("make_kvdb", kvdb::make)
          .begin_userdata("kvdb")
            .add_function("set", kvdb::set)
            .add_function("get", kvdb::get)
            .add_function("del", kvdb::del)
            .add_function("__gc", kvdb::dispose)
            .add_function("__eq", kvdb::equals)
          .end_userdata()
          .add_function("make_aid", aid::make)
          .begin_userdata("aid")
            .add_function("get", aid::get)
            .add_function("copy_of", aid::copy_of)
            .add_function("__newindex", aid::set)
            .add_function("__tostring", aid::tostring)
            .add_function("__gc", aid::dispose)
            .add_function("__eq", aid::equals)
          .end_userdata()
          .add_function("func", aid::func)
          .add_function("push_table", aid::push_table)
          .add_function("push_udata", aid::push_udata)
          .add_function("test_lib", aid::test_lib)
        .end()
        ;

      char const* kvdb_scr = 
        "local libkv = require('libkv') \
         libkv.i = 12\
         print(libkv.i)\
         local kv = libkv.make_kvdb() \
         kv:set('k1', 'v1') \
         kv:set('k2', 'v2') \
         \
         local v \
         v = kv:get('k1') \
         print(v) \
         assert(v == 'v1') \
         \
         kv:set('k1', 'v1_1') \
         v = kv:get('k1') \
         print(v) \
         assert(v == 'v1_1')\
         \
         kv:del('k2') \
         v = kv:get('k2') \
         assert(v == nil) \
         \
         print('done.') \
         ";

      if (luaL_dostring(L, kvdb_scr) != 0)
      {
        throw std::runtime_error(lua_tostring(L, -1));
      }

      char const* aid_scr = 
        "collectgarbage('collect')\
         local libkv_other = require('libkv') \
         print(libkv_other.i)\
         print(libkv.i)\
         local a = libkv.make_aid() \
         a.id_ = 1 \
         a.name_ = 'nousxiong' \
         print(a) \
         \
         local a_other = libkv.make_aid() \
         a_other.id_ = a:get('id_') + 2 \
         a_other.name_ = a:get('name_') \
         print(a_other) \
         a:copy_of(a_other) \
         print(a) \
         assert(a == a_other) \
         a.id_ = 5\
         function f(o)\
           if o:get('id_') == 0 then \
             return\
           else\
             libkv.func(o)\
             print(o)\
             return f(o)\
           end\
         end\
         f(a)\
         local tab = {id=0}\
         local tab_other = libkv.push_table(tab)\
         print(tab)\
         print(tab_other)\
         local a_other = libkv.push_udata(a)\
         print(a)\
         print(a_other)\
         libkv.test_lib()\
         print('done.') \
        ";

      if (luaL_dostring(L, aid_scr) != 0)
      {
        throw std::runtime_error(lua_tostring(L, -1));
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << "test_common: " << ex.what() << std::endl;
    }

    if (L)
    {
      lua_close(L);
    }
  }

  struct meta
  {
    int i_;

    meta()
      : i_(0)
    {
    }

    static int make(lua_State* L)
    {
      void* block = lua_newuserdata(L, sizeof(meta));
      if (!block)
      {
        return luaL_error(L, "lua_newuserdata for meta failed");
      }
      meta* o = new (block) meta;

      gce::lualib::setmetatab(L, "meta");
      return 1;
    }

    static int dispose(lua_State* L)
    {
      meta* o = static_cast<meta*>(lua_touserdata(L, 1));
      if (o)
      {
        o->~meta();
      }
      return 0;
    }

    static int memfunc(lua_State* L)
    {
      meta* o = static_cast<meta*>(lua_touserdata(L, 1));
      luaL_argcheck(L, o != 0, 1, "'meta' expected");

      lua_pushinteger(L, ++o->i_);
      return 1;
    }

    static int func(lua_State* L)
    {
      meta* o = static_cast<meta*>(lua_touserdata(L, 1));
      luaL_argcheck(L, o != 0, 1, "'meta' expected");

      /// get obj's metatable
      if (lua_getmetatable(L, 1) == 0)
      {
        return luaL_error(L, "meta obj must have a metatable!");
      }

      /// then get its 'memfunc' method and call it
      lua_getfield(L, -1, "memfunc");
      lua_pushvalue(L, 1);

      if (lua_pcall(L, 1, 1, 0) != 0)
      {
        /// pop obj's metatable
        lua_pop(L, 1);
        return luaL_error(L, lua_tostring(L, -1));
      }

      lua_Integer i = lua_tointeger(L, -1);
      lua_pop(L, 1);

      /// pop metatable
      lua_pop(L, 1);
      std::cout << "func i: " << i << ", stack size: " << lua_gettop(L) << std::endl;
      return 0;
    }

    static int test_ref(lua_State* L)
    {
      int begin_stack = lua_gettop(L);

      lua_pushinteger(L, 13);
      int k = gce::lualib::make_ref(L, "libmeta");
      BOOST_ASSERT(begin_stack == lua_gettop(L));

      gce::lualib::get_ref(L, "libmeta", k);

      int i = (int)luaL_checkinteger(L, -1);
      BOOST_ASSERT(i == 13);
      std::cout << "ref<key:" << k << "><value:" << i << ">" << std::endl;

      lua_pop(L, 1);
      BOOST_ASSERT(begin_stack == lua_gettop(L));

      gce::lualib::get_ref(L, "libmeta", k);
      i = (int)luaL_checkinteger(L, -1);
      BOOST_ASSERT(i == 13);
      lua_pop(L, 1);

      gce::lualib::rmv_ref(L, "libmeta", k);
      int end_stack = lua_gettop(L);
      BOOST_ASSERT(begin_stack == end_stack);

      return 0;
    }
  };

  static void test_metatable()
  {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    try
    {
      gce::lualib::open(L)
        .begin("libmeta")
          .add_function("make", meta::make)
          .begin_userdata("meta")
            .add_function("memfunc", meta::memfunc)
            .add_function("__gc", meta::dispose)
          .end_userdata()
          .add_function("func", meta::func)
          .add_function("test_ref", meta::test_ref)
        .end()
        ;

      char const* scr = 
        "local libmeta = require('libmeta') \
         libmeta.reftab = {}\
         local m = libmeta.make() \
         local m_other = libmeta.make() \
         assert(getmetatable(m) == getmetatable(m_other))\
         libmeta.func(m) \
         libmeta.func(m) \
         libmeta.func(m) \
         libmeta.func(m) \
         libmeta.func(m) \
         libmeta.test_ref() \
         print('all done.')\
         ";

      if (luaL_dostring(L, scr) != 0)
      {
        throw std::runtime_error(lua_tostring(L, -1));
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << "test_metatable: " << ex.what() << std::endl;
    }

    if (L)
    {
      lua_close(L);
    }
  }
};
}
