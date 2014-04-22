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
namespace detail
{
class cache_pool;
struct pack;
}

class basic_actor
{
public:
  basic_actor(std::size_t, timestamp_t const);
  virtual ~basic_actor();

public:
  inline aid_t get_aid() const { return aid_; }

protected:
  enum send_hint
  {
    async,
    sync
  };

  void pri_send(aid_t, message const&, send_hint hint = async);
  void pri_send_svc(svcid_t, message const&, send_hint hint = async);
  void pri_relay(aid_t, message&, send_hint hint = async);
  void pri_relay_svc(svcid_t, message&, send_hint hint = async);

  void pri_request(response_t, aid_t, message const&, send_hint hint = async);
  void pri_request_svc(response_t, svcid_t, message const&, send_hint hint = async);
  void pri_reply(aid_t, message const&, send_hint hint = async);

  void pri_link(aid_t, send_hint hint = async);
  void pri_monitor(aid_t, send_hint hint = async);

  void pri_spawn(
    sid_t, match_t func, match_t ctxid, 
    std::size_t stack_size, send_hint hint = async
    );

  virtual void on_recv(detail::pack&, send_hint hint) = 0;

public:
  /// internal use
  void on_free();
  inline void add_link(detail::link_t l)
  {
    link(l);
  }

protected:
  friend class mixin;

  void update_aid();
  sid_t new_request();

  void add_link(aid_t, sktaid_t skt = aid_t());
  void link(detail::link_t, send_hint hint = async, detail::cache_pool* user = 0);
  void send_exit(aid_t self_aid, exit_code_t, std::string const&);
  void remove_link(aid_t);
  void send_already_exited(aid_t recver, aid_t sender);
  void send_already_exited(aid_t recver, response_t res);
  static void send(
    aid_t const& recver, detail::pack&, 
    detail::cache_pool*, send_hint
    );

private:
  static aid_t filter_aid(aid_t const& src, detail::cache_pool*);
  static aid_t filter_svcid(svcid_t const& src, detail::cache_pool*);

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

protected:
  GCE_CACHE_ALIGNED_VAR(detail::cache_pool*, owner_)
  GCE_CACHE_ALIGNED_VAR(detail::cache_pool*, user_)
  GCE_CACHE_ALIGNED_VAR(detail::mailbox, mb_)

private:
  GCE_CACHE_ALIGNED_VAR(aid_t, aid_)

  /// local vals
  sid_t req_id_;
  typedef std::map<aid_t, sktaid_t> link_list_t;
  typedef std::set<aid_t> monitor_list_t;
  link_list_t link_list_;
  monitor_list_t monitor_list_;
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
