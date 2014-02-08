#include <gce/actor/detail/mailbox.hpp>
#include <gce/detail/scope.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>

namespace gce
{
namespace detail
{
///------------------------------------------------------------------------------
mailbox::mailbox(std::size_t cache_match_size)
  : cache_match_list_(cache_match_size)
{
}
///------------------------------------------------------------------------------
mailbox::~mailbox()
{
}
///------------------------------------------------------------------------------
void mailbox::clear()
{
  recv_que_.clear();
  BOOST_FOREACH(match_queue_t& que, cache_match_list_)
  {
    que.clear();
  }
  match_queue_list_.clear();

  res_msg_list_.clear();
  wait_reply_list_.clear();
}
///------------------------------------------------------------------------------
bool mailbox::pop(recv_t& src, message& msg, match_list_t const& match_list)
{
  if (recv_que_.empty())
  {
    return false;
  }

  if (match_list.empty())
  {
    recv_pair_t& rp = recv_que_.front();
    src = rp.first;
    msg = rp.second;
    recv_que_.pop_front();
    return true;
  }

  BOOST_FOREACH(match_t type, match_list)
  {
    match_queue_t* match_que = 0;
    match_queue_list_t::iterator mq_itr(match_queue_list_.end());
    if (type >= 0 && type < (match_t)cache_match_list_.size())
    {
      match_que = &cache_match_list_[type];
    }
    else
    {
      mq_itr = match_queue_list_.find(type);
      if (mq_itr != match_queue_list_.end())
      {
        match_que = &mq_itr->second;
      }
    }

    if (match_que && !match_que->empty())
    {
      recv_itr itr = match_que->front();
      match_que->pop_front();
      if (match_que->empty() && mq_itr != match_queue_list_.end())
      {
        match_queue_list_.erase(mq_itr);
      }
      src = itr->first;
      msg = itr->second;
      recv_que_.erase(itr);
      return true;
    }
  }

  return false;
}
///------------------------------------------------------------------------------
bool mailbox::pop(response_t& res, message& msg)
{
  res_msg_list_t::iterator itr(res_msg_list_.find(res.get_id()));
  if (itr != res_msg_list_.end())
  {
    res = itr->second.first;
    msg = itr->second.second;
    res_msg_list_.erase(itr);
    return true;
  }

  return false;
}
///------------------------------------------------------------------------------
bool mailbox::pop(aid_t aid, request_t& req)
{
  wait_reply_list_t::iterator itr(wait_reply_list_.find(aid));
  if (itr != wait_reply_list_.end())
  {
    req_queue_t& req_que = itr->second;
    if (!req_que.empty())
    {
      req = req_que.front();
      req_que.pop_front();
      return true;
    }
  }
  return false;
}
///------------------------------------------------------------------------------
void mailbox::push(aid_t sender, message const& msg)
{
  recv_t rcv(sender);
  scope scp(boost::bind(&recv_queue_t::pop_back, &recv_que_));
  add_match(rcv, msg);
  scp.reset();
}
///------------------------------------------------------------------------------
void mailbox::push(exit_t ex, message const& msg)
{
  recv_t rcv(ex);
  scope scp(boost::bind(&recv_queue_t::pop_back, &recv_que_));
  add_match(rcv, msg);
  scp.reset();
}
///------------------------------------------------------------------------------
void mailbox::push(request_t req, message const& msg)
{
  scope scp(boost::bind(&recv_queue_t::pop_back, &recv_que_));
  recv_t rcv(req);
  add_match(rcv, msg);
  std::pair<wait_reply_list_t::iterator, bool> pr =
    wait_reply_list_.insert(std::make_pair(req.get_aid(), dummy_));
  pr.first->second.push_back(req);
  scp.reset();
}
///------------------------------------------------------------------------------
bool mailbox::push(response_t res, message const& msg)
{
  res_msg_list_.insert(std::make_pair(res.get_id(), std::make_pair(res, msg)));
  return false;
}
///------------------------------------------------------------------------------
void mailbox::add_match(recv_t const& rcv, message const& msg)
{
  recv_itr itr = recv_que_.insert(recv_que_.end(), std::make_pair(rcv, msg));
  match_t type = msg.get_type();
  if (type >= 0 && type < (match_t)cache_match_list_.size())
  {
    cache_match_list_[type].push_back(itr);
  }
  else
  {
    std::pair<match_queue_list_t::iterator, bool> pr =
      match_queue_list_.insert(std::make_pair(type, dummy2_));
    pr.first->second.push_back(itr);
  }
}
///------------------------------------------------------------------------------
}
}
