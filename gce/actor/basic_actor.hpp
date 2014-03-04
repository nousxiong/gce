///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_BASIC_ACTOR_HPP
#define GCE_ACTOR_BASIC_ACTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/response.hpp>
#include <gce/actor/detail/mailbox.hpp>
#include <gce/actor/detail/request.hpp>
#include <gce/actor/detail/link.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/actor/detail/pack.hpp>
#include <boost/function.hpp>
#include <set>

namespace gce
{
class message;
class mixin;
class slice;
namespace detail
{
class cache_pool;
}

class basic_actor
{
public:
  basic_actor(std::size_t, timestamp_t const);
  virtual ~basic_actor();

public:
  virtual void on_recv(detail::pack*) = 0;
  virtual void link(aid_t) = 0;
  virtual void monitor(aid_t) = 0;
  void on_free();

  inline aid_t get_aid() const
  {
    return aid_;
  }

  inline void add_link(detail::link_t l)
  {
    link(l);
  }

protected:
  friend class mixin;
  friend class slice;

  inline void update_aid(ctxid_t ctxid)
  {
    aid_ = aid_t(ctxid, aid_.timestamp_, this, aid_.sid_ + 1);
  }

  inline sid_t new_request()
  {
    return ++req_id_;
  }

  static detail::pack* alloc_pack(detail::cache_pool*);
  static void dealloc_pack(detail::cache_pool*, detail::pack*);
  void add_link(aid_t);
  void link(detail::link_t, detail::cache_pool* user = 0);
  void send_exit(exit_code_t, std::string const&, detail::cache_pool*);
  void remove_link(aid_t);
  void send_already_exited(aid_t recver, aid_t sender, detail::cache_pool*);
  void send_already_exited(aid_t recver, response_t res, detail::cache_pool*);

private:
  byte_t pad0_[GCE_CACHE_LINE_SIZE]; /// Ensure start from a new cache line.

protected:
  GCE_CACHE_ALIGNED_VAR(detail::cache_pool*, owner_)
  GCE_CACHE_ALIGNED_VAR(detail::mailbox, mb_)
  GCE_CACHE_ALIGNED_VAR(detail::pack_queue_t, pack_que_)

private:
  GCE_CACHE_ALIGNED_VAR(aid_t, aid_)

  /// local vals
  sid_t req_id_;
  std::set<aid_t> link_list_;
  std::set<aid_t> monitor_list_;
  timestamp_t const timestamp_;
};

inline bool check_local(aid_t const& id, ctxid_t ctxid)
{
  return id.ctxid_ == ctxid;
}

inline bool check_local_valid(aid_t const& id, ctxid_t ctxid, timestamp_t timestamp)
{
  BOOST_ASSERT(id.ctxid_ == ctxid);
  return id.timestamp_ == timestamp;
}

inline bool check(aid_t const& id, ctxid_t ctxid, timestamp_t timestamp)
{
  if (ctxid != id.ctxid_ || timestamp != id.timestamp_)
  {
    return false;
  }

  return id && id.get_actor_ptr(ctxid, timestamp)->get_aid() == id;
}
}

#endif /// GCE_ACTOR_BASIC_ACTOR_HPP
