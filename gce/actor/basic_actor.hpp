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
#include <gce/actor/detail/request.hpp>
#include <gce/actor/detail/link.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/detail/cache_aligned_ptr.hpp>
#include <gce/actor/detail/pack.hpp>
#include <boost/function.hpp>
#include <set>

namespace gce
{
class message;
namespace detail
{
class mailbox;
class cache_pool;
}

class basic_actor
{
public:
  explicit basic_actor(std::size_t);
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
  inline void update_aid()
  {
    aid_ = aid_t(this, aid_.get_sid() + 1);
  }

  inline sid_t new_request()
  {
    return ++req_id_;
  }

  static detail::pack* alloc_pack(detail::cache_pool*);
  static void dealloc_pack(detail::cache_pool*, detail::pack*);
  void move_pack(detail::cache_pool*);
  void add_link(aid_t);
  void link(detail::link_t, detail::cache_pool* user = 0);
  void send_exit(exit_code_t, std::string const&, detail::cache_pool*);
  void remove_link(aid_t);

protected:
  detail::cache_aligned_ptr<detail::cache_pool, detail::cache_pool*> owner_;

  typedef detail::unique_ptr<detail::mailbox> mailbox_ptr;
  detail::cache_aligned_ptr<detail::mailbox, mailbox_ptr> mb_;

  typedef detail::unique_ptr<detail::pack_queue_t> pack_queue_ptr;
  detail::cache_aligned_ptr<detail::pack_queue_t, pack_queue_ptr> pack_que_;

private:
  aid_t aid_;
  byte_t pad1_[GCE_CACHE_LINE_SIZE - sizeof(aid_t)];

  /// local vals
  sid_t req_id_;
  std::set<aid_t> link_list_;
  std::set<aid_t> monitor_list_;
};

inline bool check(aid_t id)
{
  return id && id.get_actor_ptr()->get_aid() == id;
}
}

#endif /// GCE_ACTOR_BASIC_ACTOR_HPP
