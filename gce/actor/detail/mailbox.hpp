///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_MAILBOX_HPP
#define GCE_ACTOR_DETAIL_MAILBOX_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/response.hpp>
#include <gce/actor/detail/request.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/detail/mailbox_fwd.hpp>
#include <boost/variant/variant.hpp>
#include <vector>
#include <deque>
#include <list>
#include <map>

namespace gce
{
namespace detail
{
class mailbox
{
  typedef std::pair<recv_t, message> recv_pair_t;
  typedef std::pair<response_t, message> res_msg_pair_t;
  typedef std::map<sid_t, res_msg_pair_t> res_msg_list_t;

public:
  explicit mailbox(std::size_t);
  ~mailbox();

  void clear();

public:
  bool pop(recv_t&, message&, match_list_t const&);
  bool pop(response_t&, message&);
  bool pop(aid_t, request_t&);

  void push(aid_t, message const&);
  void push(exit_t, message const&);
  void push(request_t, message const&);
  bool push(response_t, message const&);

private:
  void add_match_msg(recv_t const&, message const&);
  bool fetch_match_msg(match_t, recv_t&, message&);

private:
  typedef std::list<recv_pair_t> recv_queue_t;
  typedef recv_queue_t::iterator recv_itr;
  recv_queue_t recv_que_;

  typedef std::deque<recv_itr> match_queue_t;
  std::vector<match_queue_t> cache_match_list_;

  typedef std::map<match_t, match_queue_t> match_queue_list_t;
  match_queue_list_t match_queue_list_;

  res_msg_list_t res_msg_list_;

  typedef std::deque<request_t> req_queue_t;
  typedef std::map<aid_t, req_queue_t> wait_reply_list_t;
  wait_reply_list_t wait_reply_list_;

  req_queue_t dummy_;
  match_queue_t dummy2_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_MAILBOX_HPP
