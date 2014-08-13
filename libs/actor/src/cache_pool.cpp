///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/send.hpp>
#include <gce/actor/coroutine_stackful_actor.hpp>
#include <gce/actor/coroutine_stackless_actor.hpp>
#include <gce/actor/service.hpp>
#include <gce/actor/lua_actor.hpp>
#include <gce/actor/detail/socket.hpp>
#include <gce/actor/detail/acceptor.hpp>
#include <boost/foreach.hpp>

namespace gce
{
namespace detail
{
typedef actor_pool<coroutine_stackful_actor> stackful_actor_pool_t;
typedef actor_pool<coroutine_stackless_actor> stackless_actor_pool_t;
#ifdef GCE_LUA
  typedef actor_pool<lua_actor> lua_actor_pool_t;
#endif
typedef actor_pool<socket> socket_pool_t;
typedef actor_pool<acceptor> acceptor_pool_t;

#ifdef GCE_LUA
static std::string const init_lua_script = 
  "local p = gce_path \
  local package_path = package.path \
  package.path = string.format(\"%s;%s\", package_path, p)";
#endif
///------------------------------------------------------------------------------
/// cache_pool::pool_impl
///------------------------------------------------------------------------------
struct cache_pool::pool_impl
{
  GCE_CACHE_ALIGNED_VAR(boost::optional<stackful_actor_pool_t>, stackful_actor_pool_)
  GCE_CACHE_ALIGNED_VAR(boost::optional<stackless_actor_pool_t>, stackless_actor_pool_)
#ifdef GCE_LUA
  GCE_CACHE_ALIGNED_VAR(boost::optional<lua_actor_pool_t>, lua_actor_pool_)
#endif
  GCE_CACHE_ALIGNED_VAR(boost::optional<socket_pool_t>, socket_pool_)
  GCE_CACHE_ALIGNED_VAR(boost::optional<acceptor_pool_t>, acceptor_pool_)
};
#ifdef GCE_LUA
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
#endif
///------------------------------------------------------------------------------
/// cache_pool
///------------------------------------------------------------------------------
cache_pool::cache_pool(context& ctx, std::size_t index, bool is_slice)
  : ctx_(&ctx)
  , index_(index)
  , snd_(ctx.get_io_service())
#ifdef GCE_LUA
  , L_(is_slice ? 0 : luaL_newstate(), lua_state_deletor())
#endif
  , pool_list_(new pool_impl)
  , ctxid_(ctx_->get_attributes().id_)
  , timestamp_(ctx_->get_timestamp())
  , curr_router_list_(router_list_.end())
  , curr_socket_list_(conn_list_.end())
  , curr_joint_list_(joint_list_.end())
  , stopped_(false)
{
  ctxid_t ctxid = ctx.get_attributes().id_;
  timestamp_t timestamp = ctx.get_timestamp();
  boost::uint16_t idx = (boost::uint16_t)index_;
  pool_list_->stackful_actor_pool_ = 
    boost::in_place(
      ctxid, timestamp, idx,
      is_slice ? 0 : ctx.get_attributes().actor_pool_reserve_size_
      );
  pool_list_->stackless_actor_pool_ = 
    boost::in_place(
      ctxid, timestamp, idx,
      is_slice ? 0 : ctx.get_attributes().actor_pool_reserve_size_
      );
#ifdef GCE_LUA
  pool_list_->lua_actor_pool_ = 
    boost::in_place(
      ctxid, timestamp, idx,
      is_slice ? 0 : ctx.get_attributes().actor_pool_reserve_size_
      );
#endif
  pool_list_->socket_pool_ = 
    boost::in_place(
      ctxid, timestamp, idx,
      is_slice ? 0 : ctx.get_attributes().socket_pool_reserve_size_
      );
  pool_list_->acceptor_pool_ = 
    boost::in_place(
      ctxid, timestamp, idx,
      is_slice ? 0 : ctx.get_attributes().acceptor_pool_reserve_size_
      );
}
///------------------------------------------------------------------------------
cache_pool::~cache_pool()
{
}
///------------------------------------------------------------------------------
#ifdef GCE_LUA
inline void serialize_number(msg_t& msg, int src)
{
  msg << (boost::int32_t)src;
}
inline void serialize_string(msg_t& msg, std::string const& src)
{
  msg << src;
}
inline void serialize_boolean(msg_t& msg, bool src)
{
  msg << src;
}
///------------------------------------------------------------------------------
inline int deserialize_number(msg_t& m)
{
  boost::int32_t des;
  m >> des;
  return (int)des;
}
inline std::string deserialize_string(msg_t& m)
{
  std::string des;
  m >> des;
  return des;
}
inline bool deserialize_boolean(msg_t& m)
{
  bool des;
  m >> des;
  return des;
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
void cache_pool::init_lua(std::string const& gce_path)
{
  lua_State* L = L_.get();
  luaL_openlibs(L);

  luabridge::getGlobalNamespace(L)
    .beginNamespace("detail")
      .beginClass<aid_t>("aid_t")
        .addFunction("get_overloading_type", &aid_t::get_overloading_type)
        .addFunction("is_nil", &aid_t::is_nil)
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
        .addFunction("set_type", &msg_t::set_type)
        .addFunction("to_string", &msg_t::to_string)
        GCE_LUA_REG_SERIALIZE_FUNC(msg_t)
      .endClass()
      .beginClass<match_type>("match_t")
        .addFunction("get_overloading_type", &match_type::get_overloading_type)
        .addFunction("to_string", &match_type::to_string)
      .endClass()
      .beginClass<resp_t>("resp_t")
        .addFunction("get_overloading_type", &resp_t::get_overloading_type)
        .addFunction("to_string", &resp_t::to_string)
      .endClass()
      .beginClass<pattern>("pattern_t")
        .addFunction("get_overloading_type", &pattern::get_overloading_type)
        .addFunction("set_timeout", &pattern::set_timeout)
        .addFunction("add_match", &pattern::add_match)
        .addFunction("to_string", &pattern::to_string)
      .endClass()
      .beginClass<duration_type>("duration_t")
        .addFunction("to_string", &duration_type::to_string)
        GCE_LUA_REG_SERIALIZE_FUNC(duration_type)
      .endClass()
      .beginClass<basic_actor>("basic_actor")
        .addFunction("get_aid", &basic_actor::get_aid)
      .endClass()
      .deriveClass<lua_actor, basic_actor>("actor")
        .addFunction("set_coro", &lua_actor::set_coro)
        .addFunction("send", &lua_actor::send)
        .addFunction("send2svc", &lua_actor::send2svc)
        .addFunction("relay", &lua_actor::relay)
        .addFunction("relay2svc", &lua_actor::relay2svc)
        .addFunction("request", &lua_actor::request)
        .addFunction("request2svc", &lua_actor::request2svc)
        .addFunction("reply", &lua_actor::reply)
        .addFunction("link", &lua_actor::link)
        .addFunction("monitor", &lua_actor::monitor)
        .addFunction("recv", &lua_actor::recv)
        .addFunction("recv_match", &lua_actor::recv_match)
        .addFunction("recv_response", &lua_actor::recv_response)
        .addFunction("recv_response_timeout", &lua_actor::recv_response_timeout)
        .addFunction("wait", &lua_actor::wait)
        .addFunction("spawn", &lua_actor::spawn)
        .addFunction("spawn_remote", &lua_actor::spawn_remote)
      .endClass()
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

  luabridge::setGlobal(L, gce_path, "gce_path");
  if (luaL_dostring(L, init_lua_script.c_str()) != 0)
  {
    throw std::runtime_error(lua_tostring(L, -1));
  }
}
///------------------------------------------------------------------------------
luabridge::LuaRef cache_pool::get_script(std::string const& file)
{
  script_list_t::iterator itr(script_list_.find(file));
  if (itr != script_list_.end())
  {
    return itr->second;
  }
  else
  {
    lua_State* L = L_.get();
    luabridge::LuaRef sf(L);
    if (luaL_loadfile(L, file.c_str()) != 0)
    {
      return sf;
    }
    sf.pop(L);
    script_list_.insert(std::make_pair(file, sf));
    return sf;
  }
}
#endif
///------------------------------------------------------------------------------
coroutine_stackful_actor* cache_pool::make_stackful_actor()
{
  return pool_list_->stackful_actor_pool_->make(this);
}
///------------------------------------------------------------------------------
coroutine_stackless_actor* cache_pool::make_stackless_actor()
{
  return pool_list_->stackless_actor_pool_->make(this);
}
#ifdef GCE_LUA
///------------------------------------------------------------------------------
lua_actor* cache_pool::make_lua_actor()
{
  return pool_list_->lua_actor_pool_->make(this);
}
///------------------------------------------------------------------------------
aid_t cache_pool::spawn_lua_actor(std::string const& script, aid_t sire, link_type type)
{
  lua_actor* a = make_lua_actor();
  a->init(script);
  if (sire)
  {
    gce::send(*a, sire, msg_new_actor, (boost::uint16_t)type);
  }
  aid_t aid = a->get_aid();
  a->start();
  return aid;
}
#endif
///------------------------------------------------------------------------------
socket* cache_pool::make_socket()
{
  return pool_list_->socket_pool_->make(this);
}
///------------------------------------------------------------------------------
acceptor* cache_pool::make_acceptor()
{
  return pool_list_->acceptor_pool_->make(this);
}
///------------------------------------------------------------------------------
void cache_pool::free_actor(coroutine_stackful_actor* a)
{
  pool_list_->stackful_actor_pool_->free(a);
}
///------------------------------------------------------------------------------
void cache_pool::free_actor(coroutine_stackless_actor* a)
{
  pool_list_->stackless_actor_pool_->free(a);
}
#ifdef GCE_LUA
///------------------------------------------------------------------------------
void cache_pool::free_actor(lua_actor* a)
{
  pool_list_->lua_actor_pool_->free(a);
}
#endif
///------------------------------------------------------------------------------
void cache_pool::free_socket(socket* skt)
{
  pool_list_->socket_pool_->free(skt);
}
///------------------------------------------------------------------------------
void cache_pool::free_acceptor(acceptor* acpr)
{
  pool_list_->acceptor_pool_->free(acpr);
}
///------------------------------------------------------------------------------
void cache_pool::on_recv(
  detail::actor_index i, sid_t sid, 
  detail::pack& pk, detail::send_hint hint
  )
{
  if (hint == detail::sync)
  {
    snd_.dispatch(
      boost::bind(
        &cache_pool::handle_recv, this, i, sid, pk, hint
        )
      );
  }
  else
  {
    snd_.post(
      boost::bind(
        &cache_pool::handle_recv, this, i, sid, pk, hint
        )
      );
  }
}
///------------------------------------------------------------------------------
void cache_pool::register_service(match_t name, aid_t svc)
{
  service_list_.insert(std::make_pair(name, svc));
}
///------------------------------------------------------------------------------
aid_t cache_pool::find_service(match_t name)
{
  aid_t svc;
  service_list_t::iterator itr(service_list_.find(name));
  if (itr != service_list_.end())
  {
    svc = itr->second;
  }
  return svc;
}
///------------------------------------------------------------------------------
void cache_pool::deregister_service(match_t name, aid_t svc)
{
  service_list_t::iterator itr(service_list_.find(name));
  if (itr != service_list_.end() && itr->second == svc)
  {
    service_list_.erase(itr);
  }
}
///------------------------------------------------------------------------------
void cache_pool::register_socket(ctxid_pair_t ctxid_pr, aid_t skt)
{
  if (ctxid_pr.second == socket_router)
  {
    std::pair<conn_list_t::iterator, bool> pr =
      router_list_.insert(std::make_pair(ctxid_pr.first, dummy_));
    skt_list_t& skt_list = pr.first->second.skt_list_;
    skt_list.insert(skt);
    if (skt_list.size() == 1)
    {
      pr.first->second.curr_skt_ = skt_list.begin();
    }

    if (router_list_.size() == 1)
    {
      curr_router_list_ = router_list_.begin();
    }
  }
  else if (ctxid_pr.second == socket_joint)
  {
    std::pair<conn_list_t::iterator, bool> pr =
      joint_list_.insert(std::make_pair(ctxid_pr.first, dummy_));
    skt_list_t& skt_list = pr.first->second.skt_list_;
    skt_list.insert(skt);
    if (skt_list.size() == 1)
    {
      pr.first->second.curr_skt_ = skt_list.begin();
    }

    if (joint_list_.size() == 1)
    {
      curr_joint_list_ = joint_list_.begin();
    }
  }
  else
  {
    std::pair<conn_list_t::iterator, bool> pr =
      conn_list_.insert(std::make_pair(ctxid_pr.first, dummy_));
    skt_list_t& skt_list = pr.first->second.skt_list_;
    skt_list.insert(skt);
    if (skt_list.size() == 1)
    {
      pr.first->second.curr_skt_ = skt_list.begin();
    }

    if (conn_list_.size() == 1)
    {
      curr_socket_list_ = conn_list_.begin();
    }
  }
}
///------------------------------------------------------------------------------
aid_t cache_pool::select_socket(ctxid_t ctxid, ctxid_t* target)
{
  aid_t skt = select_straight_socket(ctxid, target);
  if (!skt)
  {
    skt = select_router(target);
  }
  return skt;
}
///------------------------------------------------------------------------------
aid_t cache_pool::select_straight_socket(ctxid_t ctxid, ctxid_t* target)
{
  aid_t skt;
  skt_list_t* skt_list = 0;
  skt_list_t::iterator* curr_skt = 0;

  if (ctxid != ctxid_nil)
  {
    conn_list_t::iterator itr(conn_list_.find(ctxid));
    if (itr != conn_list_.end())
    {
      skt_list = &itr->second.skt_list_;
      curr_skt = &itr->second.curr_skt_;
    }
  }
  else
  {
    if (!conn_list_.empty())
    {
      if (curr_socket_list_ != conn_list_.end())
      {
        skt_list = &curr_socket_list_->second.skt_list_;
        curr_skt = &curr_socket_list_->second.curr_skt_;
        if (target)
        {
          *target = curr_socket_list_->first;
        }
      }

      ++curr_socket_list_;
      if (curr_socket_list_ == conn_list_.end())
      {
        curr_socket_list_ = conn_list_.begin();
      }
    }
  }

  if (skt_list && !skt_list->empty())
  {
    BOOST_ASSERT(curr_skt);
    skt_list_t::iterator& itr = *curr_skt;
    if (itr != skt_list->end())
    {
      skt = *itr;
    }
    ++itr;
    if (itr == skt_list->end())
    {
      itr = skt_list->begin();
    }
  }

  return skt;
}
///------------------------------------------------------------------------------
aid_t cache_pool::select_joint_socket(ctxid_t ctxid, ctxid_t* target)
{
  aid_t skt;
  skt_list_t* skt_list = 0;
  skt_list_t::iterator* curr_skt = 0;

  if (ctxid != ctxid_nil)
  {
    conn_list_t::iterator itr(joint_list_.find(ctxid));
    if (itr != joint_list_.end())
    {
      skt_list = &itr->second.skt_list_;
      curr_skt = &itr->second.curr_skt_;
    }
  }
  else
  {
    if (!joint_list_.empty())
    {
      if (curr_joint_list_ != joint_list_.end())
      {
        skt_list = &curr_joint_list_->second.skt_list_;
        curr_skt = &curr_joint_list_->second.curr_skt_;
        if (target)
        {
          *target = curr_joint_list_->first;
        }
      }

      ++curr_joint_list_;
      if (curr_joint_list_ == joint_list_.end())
      {
        curr_joint_list_ = joint_list_.begin();
      }
    }
  }

  if (skt_list && !skt_list->empty())
  {
    BOOST_ASSERT(curr_skt);
    skt_list_t::iterator& itr = *curr_skt;
    if (itr != skt_list->end())
    {
      skt = *itr;
    }
    ++itr;
    if (itr == skt_list->end())
    {
      itr = skt_list->begin();
    }
  }

  return skt;
}
///------------------------------------------------------------------------------
aid_t cache_pool::select_router(ctxid_t* target)
{
  aid_t skt;
  if (!router_list_.empty())
  {
    if (curr_router_list_ != router_list_.end())
    {
      skt_list_t& skt_list = curr_router_list_->second.skt_list_;
      skt_list_t::iterator& curr_skt = curr_router_list_->second.curr_skt_;
      if (target)
      {
        *target = curr_router_list_->first;
      }

      if (!skt_list.empty())
      {
        if (curr_skt != skt_list.end())
        {
          skt = *curr_skt;
        }
        ++curr_skt;
        if (curr_skt == skt_list.end())
        {
          curr_skt = skt_list.begin();
        }
      }
    }

    ++curr_router_list_;
    if (curr_router_list_ == router_list_.end())
    {
      curr_router_list_ = router_list_.begin();
    }
  }
  return skt;
}
///------------------------------------------------------------------------------
void cache_pool::deregister_socket(ctxid_pair_t ctxid_pr, aid_t skt)
{
  if (ctxid_pr.second == socket_router)
  {
    conn_list_t::iterator itr(router_list_.find(ctxid_pr.first));
    if (itr != router_list_.end())
    {
      socket_list& skt_list = itr->second;
      skt_list.skt_list_.erase(skt);
      skt_list.curr_skt_ = skt_list.skt_list_.begin();
    }
  }
  else if (ctxid_pr.second == socket_joint)
  {
    conn_list_t::iterator itr(joint_list_.find(ctxid_pr.first));
    if (itr != joint_list_.end())
    {
      socket_list& skt_list = itr->second;
      skt_list.skt_list_.erase(skt);
      skt_list.curr_skt_ = skt_list.skt_list_.begin();
    }
  }
  else
  {
    conn_list_t::iterator itr(conn_list_.find(ctxid_pr.first));
    if (itr != conn_list_.end())
    {
      socket_list& skt_list = itr->second;
      skt_list.skt_list_.erase(skt);
      skt_list.curr_skt_ = skt_list.skt_list_.begin();
    }
  }
}
///------------------------------------------------------------------------------
void cache_pool::add_socket(socket* s)
{
  socket_list_.insert(s);
}
///------------------------------------------------------------------------------
void cache_pool::add_acceptor(acceptor* a)
{
  acceptor_list_.insert(a);
}
///------------------------------------------------------------------------------
void cache_pool::remove_socket(socket* s)
{
  socket_list_.erase(s);
}
///------------------------------------------------------------------------------
void cache_pool::remove_acceptor(acceptor* a)
{
  acceptor_list_.erase(a);
}
///------------------------------------------------------------------------------
void cache_pool::stop()
{
  stopped_ = true;
  BOOST_FOREACH(socket* s, socket_list_)
  {
    s->stop();
  }
  socket_list_.clear();

  BOOST_FOREACH(acceptor* a, acceptor_list_)
  {
    a->stop();
  }
  acceptor_list_.clear();
}
///------------------------------------------------------------------------------
void cache_pool::send_already_exited(aid_t recver, aid_t sender)
{
  aid_t target = filter_aid(recver);
  if (target)
  {
    message m(exit);
    std::string exit_msg("already exited");
    m << exit_already << exit_msg;

    detail::pack pk;
    pk.tag_ = sender;
    pk.recver_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;
    pk.is_err_ret_ = true;

    send(target, pk, detail::async);
  }
}
///------------------------------------------------------------------------------
void cache_pool::send_already_exited(aid_t recver, resp_t res)
{
  aid_t target = filter_aid(recver);
  if (target)
  {
    message m(exit);
    std::string exit_msg("already exited");
    m << exit_already << exit_msg;

    detail::pack pk;
    pk.tag_ = res;
    pk.recver_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;
    pk.is_err_ret_ = true;

    send(target, pk, detail::async);
  }
}
///------------------------------------------------------------------------------
void cache_pool::send(aid_t const& recver, detail::pack& pk, detail::send_hint hint)
{
  pk.cache_queue_index_ = index_;
  bool already_exit = true;
  if (recver.in_pool())
  {
    detail::actor_index i = recver.get_actor_index(ctxid_, timestamp_);
    if (i)
    {
      already_exit = false;
      detail::cache_pool* cac = ctx_->get_cache_pool(i.cac_id_);
      cac->on_recv(i, recver.sid_, pk, hint);
    }
  }
  else
  {
    basic_actor* a = recver.get_actor_ptr(ctxid_, timestamp_);
    if (a)
    {
      already_exit = false;
      a->on_recv(pk, hint);
    }
  }

  if (already_exit)
  {
    send_already_exit(pk);
  }
}
///------------------------------------------------------------------------------
aid_t cache_pool::filter_aid(aid_t const& src)
{
  aid_t target;

  bool is_local = check_local(src, ctxid_);
  if (is_local && check_local_valid(src, ctxid_, timestamp_))
  {
    target = src;
  }
  else
  {
    if (!is_local)
    {
      target = select_socket(src.ctxid_);
    }
  }
  return target;
}
///------------------------------------------------------------------------------
aid_t cache_pool::filter_svcid(svcid_t const& src)
{
  aid_t target;

  if (src.ctxid_ == ctxid_nil || src.ctxid_ == ctxid_)
  {
    target = find_service(src.name_);
  }
  else
  {
    target = select_socket(src.ctxid_);
  }
  return target;
}
///------------------------------------------------------------------------------
void cache_pool::handle_recv(
  detail::actor_index i, sid_t sid, 
  detail::pack& pk, detail::send_hint hint
  )
{
  basic_actor* a = 0;
  switch (i.type_)
  {
  case detail::actor_stackful:
    {
      a = pool_list_->stackful_actor_pool_->get(i, sid);
    }break;
  case detail::actor_stackless:
    {
      a = pool_list_->stackless_actor_pool_->get(i, sid);
    }break;
#ifdef GCE_LUA
  case detail::actor_lua:
    {
      a = pool_list_->lua_actor_pool_->get(i, sid);
    }break;
#endif
  case detail::actor_socket:
    {
      a = pool_list_->socket_pool_->get(i, sid);
    }break;
  case detail::actor_acceptor:
    {
      a = pool_list_->acceptor_pool_->get(i, sid);
    }break;
  }

  if (a)
  {
    a->on_recv(pk, hint);
  }
  else
  {
    send_already_exit(pk);
  }
}
///------------------------------------------------------------------------------
void cache_pool::send_already_exit(detail::pack& pk)
{
  if (!pk.is_err_ret_)
  {
    if (detail::link_t* link = boost::get<detail::link_t>(&pk.tag_))
    {
      /// send actor exit msg
      send_already_exited(link->get_aid(), pk.recver_);
    }
    else if (detail::request_t* req = boost::get<detail::request_t>(&pk.tag_))
    {
      /// reply actor exit msg
      resp_t res(req->get_id(), pk.recver_);
      send_already_exited(req->get_aid(), res);
    }
  }
}
///------------------------------------------------------------------------------
}
}
