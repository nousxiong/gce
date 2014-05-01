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
class mixin;
class thread;
class context;
namespace detail
{
struct pack;
}

class basic_actor
{
public:
  explicit basic_actor(thread*);
  virtual ~basic_actor();

public:
  inline aid_t get_aid() const { return aid_; }
  inline context* get_context() { return ctx_; }

protected:
  friend class mixin;

  void pri_send(aid_t, message const&);
  void pri_send_svc(svcid_t, message const&);
  void pri_relay(aid_t, message&);
  void pri_relay_svc(svcid_t, message&);

  void pri_request(response_t, aid_t, message const&);
  void pri_request_svc(response_t, svcid_t, message const&);
  void pri_reply(aid_t, message const&);

  void pri_link(aid_t);
  void pri_monitor(aid_t);

  void pri_spawn(sid_t, match_t func, match_t ctxid, std::size_t stack_size);

public:
  /// internal use
  virtual void on_recv(detail::pack*) = 0;
  void on_free();

  inline thread* get_thread() { return thr_; }
  inline void add_link(detail::link_t l)
  {
    link(l);
  }

protected:
  void update_aid();
  sid_t new_request();

  void add_link(aid_t, sktaid_t skt = aid_t());
  void link(detail::link_t, thread* thr = 0);
  void send_exit(aid_t self_aid, exit_code_t, std::string const&);
  void remove_link(aid_t);
  void send_already_exited(aid_t recver, aid_t sender);
  void send_already_exited(aid_t recver, response_t res);
  void send(aid_t const& recver, detail::pack*);

private:
  aid_t filter_aid(aid_t const& src);
  aid_t filter_svcid(svcid_t const& src);

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(context*, ctx_)

protected:
  GCE_CACHE_ALIGNED_VAR(thread*, thr_)
  GCE_CACHE_ALIGNED_VAR(detail::mailbox, mb_)

private:
  GCE_CACHE_ALIGNED_VAR(aid_t, aid_)
  GCE_CACHE_ALIGNED_VAR(sid_t, req_id_)

  /// local vals
protected:
  ctxid_t const ctxid_;
  timestamp_t const timestamp_;

private:
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
