///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_SESSION_HPP
#define GCE_ASIO_SESSION_HPP

#include <gce/asio/config.hpp>
#include <gce/asio/detail/session_impl.hpp>
#include <boost/scoped_array.hpp>

namespace gce
{
namespace asio
{
template <typename Parser, typename Socket, typename Actor>
class session
  : public addon_t
{
  typedef addon_t base_t;
  typedef Parser parser_t;
  typedef Socket socket_t;
  typedef Actor actor_t;
  typedef session<parser_t, socket_t, actor_t> self_t;
  typedef base_t::scope<self_t> scope_t;
  typedef typename scope_t::guard_ptr guard_ptr;
  typedef boost::asio::ip::tcp::resolver resolver_t;
  typedef detail::session_impl<parser_t, socket_t> session_impl_t;

public:
  session(
    actor_t& a, 
    boost::shared_ptr<parser_t> parser, 
    boost::shared_ptr<socket_t> skt, 
    snopt_t opt = make_snopt()
    )
    : base_t(a)
    , a_(a)
    , sa_(spawn_session_actor(a, parser, skt, resolver_t::iterator(), opt))
    , scp_(this)
  {
    a_.send(sa_, message("init"));
  }

  session(
    actor_t& a, 
    boost::shared_ptr<parser_t> parser, 
    boost::shared_ptr<socket_t> skt, 
    resolver_t::iterator eitr, 
    snopt_t opt = make_snopt()
    )
    : base_t(a)
    , a_(a)
    , sa_(spawn_session_actor(a, parser, skt, eitr, opt))
    , scp_(this)
  {
    a_.send(sa_, message("init"));
  }

  ~session()
  {
    dispose();
  }

  void dispose()
  {
    scp_.notify();
    a_.send(sa_, message(exit));
  }

public:
  void open()
  {
    a_.send(sa_, message(detail::api_open));
  }

  void send(message const& m)
  {
    a_.send(sa_, m);
  }

  void close(bool grateful = true)
  {
    message m(detail::api_close);
    m << grateful;
    a_.send(sa_, m);
  }

private:
  aid_t spawn_session_actor(
    threaded_actor& a,
    boost::shared_ptr<parser_t> parser, 
    boost::shared_ptr<socket_t> skt, 
    resolver_t::iterator eitr,
    snopt_t opt
    )
  {
    boost::shared_ptr<session_impl_t> sn = boost::make_shared<session_impl_t>(parser, eitr, opt);
    return gce::spawn<stackless>(a, boost::bind(&session_impl_t::run, sn, _arg1, skt));
  }

  aid_t spawn_session_actor(
    stackful_actor& a,
    boost::shared_ptr<parser_t> parser, 
    boost::shared_ptr<socket_t> skt, 
    resolver_t::iterator eitr,
    snopt_t opt
    )
  {
    boost::shared_ptr<session_impl_t> sn = boost::make_shared<session_impl_t>(parser, eitr, opt);
    return gce::spawn<stackless>(a, boost::bind(&session_impl_t::run, sn, _arg1, skt), no_link, true);
  }

#ifdef GCE_LUA
  aid_t spawn_session_actor(
    gce::lua::actor_proxy& a,
    boost::shared_ptr<parser_t> parser, 
    boost::shared_ptr<socket_t> skt, 
    resolver_t::iterator eitr,
    snopt_t opt
    )
  {
    typedef typename context::stackless_service_t stackless_service_t;
    typedef typename context::lua_service_t lua_service_t;
    lua_service_t& sire_svc = a.get_service();
    context& ctx = sire_svc.get_context();

    stackless_service_t& svc = ctx.select_service<stackless_service_t>(sire_svc.get_index());
    boost::shared_ptr<session_impl_t> sn = boost::make_shared<session_impl_t>(parser, eitr, opt);
    gce::detail::actor_func<stackless, context> ef(boost::bind(&session_impl_t::run, sn, _arg1, skt));
    return gce::detail::make_stackless_actor<context>(aid_nil, svc, ef, no_link);
  }
#endif

private:
  void pri_send2actor(message& m)
  {
    message msg(m);
    m = msg_nil_;
    base_t::send2actor(msg);
  }

private:
  actor_t& a_;
  aid_t sa_;

  /// for quit
  scope_t scp_;
};
}
}

#endif /// GCE_ASIO_SESSION_HPP
