///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_CONN_HPP
#define GCE_ACTOR_DETAIL_CONN_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/basic_actor.hpp>

namespace gce
{
class context;
namespace detail
{
class cache_pool;
class conn
  : public object_pool<conn, context*>::object
  , public mpsc_queue<conn>::node
  , public basic_actor
{
  typedef basic_actor base_type;
  enum status
  {
    ready = 0,
    on
  };

public:
  explicit conn(context*);
  ~conn();

public:
  void init(cache_pool* user, cache_pool* owner);
  void start(aid_t acpr, aid_t master, aid_t skt);

public:
  void on_free();
  void on_recv(pack*);

private:
  void send(aid_t recver, message const&);
  void handle_recv(pack*);

private:
  void free_self(exit_code_t, std::string const&);

private:
  byte_t pad0_[GCE_CACHE_LINE_SIZE]; /// Ensure start from a new cache line.

  GCE_CACHE_ALIGNED_VAR(status, stat_)
  GCE_CACHE_ALIGNED_VAR(cache_pool*, user_)

  /// thread local vals
  aid_t acpr_;
  aid_t master_;
  aid_t skt_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_CONN_HPP
