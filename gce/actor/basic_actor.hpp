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
#include <gce/actor/service_id.hpp>
#include <gce/actor/response.hpp>
#include <gce/actor/detail/mailbox.hpp>
#include <gce/actor/detail/request.hpp>
#include <gce/actor/detail/link.hpp>
#include <gce/actor/actor_id.hpp>
#include <boost/function.hpp>
#include <map>
#include <set>

namespace gce
{
class message;
class thread_mapped_actor;
class context;
namespace detail
{
class cache_pool;
struct pack;
}

class basic_actor
{
public:
  basic_actor(context* ctx, detail::cache_pool*, std::size_t cache_queue_index);
  virtual ~basic_actor();

public:
  inline context* get_context() { return ctx_; }
  inline strand_t& get_strand() { return snd_; }
  inline aid_t get_aid() const { return aid_; }
  inline void chain(bool flag) { chain_ = flag; }

public:
  /// internal use
  enum send_hint
  {
    async,
    sync
  };

  void pri_send(aid_t, message const&, send_hint hint = sync);
  void pri_send_svc(svcid_t, message const&, send_hint hint = sync);
  void pri_relay(aid_t, message&, send_hint hint = sync);
  void pri_relay_svc(svcid_t, message&, send_hint hint = sync);

  void pri_request(response_t, aid_t, message const&, send_hint hint = sync);
  void pri_request_svc(response_t, svcid_t, message const&, send_hint hint = sync);
  void pri_reply(aid_t, message const&, send_hint hint = sync);

  void pri_link(aid_t, send_hint hint = sync);
  void pri_monitor(aid_t, send_hint hint = sync);

  void pri_spawn(
    sid_t, detail::spawn_type, match_t func, match_t ctxid,
    std::size_t stack_size, send_hint hint = sync
    );

protected:
  virtual void on_recv(detail::pack&, send_hint hint) = 0;

public:
  inline detail::cache_pool* get_cache_pool() { return user_; }
  void on_free();

protected:
  void update_aid();
  sid_t new_request();

  void add_link(aid_t, sktaid_t skt = aid_t());
  void link(detail::link_t, send_hint hint = sync, detail::cache_pool* user = 0);
  void send_exit(aid_t self_aid, exit_code_t, std::string const&);
  void remove_link(aid_t);
  void send_already_exited(aid_t recver, aid_t sender);
  void send_already_exited(aid_t recver, response_t res);
  void send(aid_t const& recver, detail::pack&, send_hint);

private:
  aid_t filter_aid(aid_t const& src);
  aid_t filter_svcid(svcid_t const& src);

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

protected:
  GCE_CACHE_ALIGNED_VAR(context*, ctx_)
  GCE_CACHE_ALIGNED_VAR(detail::cache_pool*, user_)
  GCE_CACHE_ALIGNED_VAR(strand_t&, snd_)
  GCE_CACHE_ALIGNED_VAR(detail::mailbox, mb_)
  GCE_CACHE_ALIGNED_VAR(ctxid_t const, ctxid_)
  GCE_CACHE_ALIGNED_VAR(timestamp_t const, timestamp_)
  GCE_CACHE_ALIGNED_VAR(std::size_t const, cache_queue_index_)

private:
  GCE_CACHE_ALIGNED_VAR(aid_t, aid_)
  GCE_CACHE_ALIGNED_VAR(bool, chain_)

  /// local vals
  sid_t req_id_;
  typedef std::map<aid_t, sktaid_t> link_list_t;
  typedef std::set<aid_t> monitor_list_t;
  link_list_t link_list_;
  monitor_list_t monitor_list_;
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
