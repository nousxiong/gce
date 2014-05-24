///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_ACCEPTOR_HPP
#define GCE_ACTOR_DETAIL_ACCEPTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/basic_actor.hpp>
#include <gce/actor/net_option.hpp>
#include <gce/actor/detail/basic_socket.hpp>
#include <gce/actor/detail/object_pool.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/actor/match.hpp>
#include <boost/scoped_ptr.hpp>
#include <map>

namespace gce
{
class thread_mapped_actor;
class context;
namespace detail
{
class cache_pool;
class basic_acceptor;

class acceptor
  : public object_pool<acceptor, cache_pool*>::object
  , public basic_actor
{
  typedef basic_actor base_type;

  enum status
  {
    ready = 0,
    on,
    off,
  };

public:
  explicit acceptor(cache_pool*);
  ~acceptor();

public:
  void init(net_option);
  void bind(
    aid_t sire, remote_func_list_t const&, 
    std::string const&, bool is_router
    );

public:
  void stop();
  void on_free();
  void on_recv(pack&, base_type::send_hint);
  inline void send(aid_t recver, message const& m)
  {
    base_type::pri_send(recver, m);
  }

  void link(aid_t) {}
  void monitor(aid_t) {}

private:
  void run(aid_t sire, std::string const&, yield_t);
  void spawn_socket(cache_pool*, socket_ptr);
  basic_acceptor* make_acceptor(std::string const&);
  void handle_recv(pack&);

private:
  void close();
  void free_self(exit_code_t, std::string const&, yield_t);

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(status, stat_)
  GCE_CACHE_ALIGNED_VAR(net_option, opt_)

  /// thread local vals
  boost::scoped_ptr<basic_acceptor> acpr_;
  std::map<match_t, remote_func> remote_func_list_;
  bool is_router_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_ACCEPTOR_HPP

