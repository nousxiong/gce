///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/detail/mailbox.hpp>
#include <gce/actor/pattern.hpp>
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
  exit_list_.clear();
  svc_exit_list_.clear();
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
    return fetch_match_msg(rp.second.get_type(), src, msg);
  }

  BOOST_FOREACH(match_t type, match_list)
  {
    if (fetch_match_msg(type, src, msg))
    {
      return true;
    }
  }

  return false;
}
///------------------------------------------------------------------------------
bool mailbox::pop(resp_t& res, message& msg)
{
  res_msg_list_t::iterator itr(res_msg_list_.find(res.get_id()));
  if (itr != res_msg_list_.end())
  {
    res = itr->second.first;
    msg = itr->second.second;
    res_msg_list_.erase(itr);
    return true;
  }
  else
  {
    aid_t recver = res.get_recver();
    svcid_t svc = res.get_svcid();
    if (recver)
    {
      exit_list_t::iterator itr(exit_list_.find(recver));
      if (itr != exit_list_.end())
      {
        recv_itr rtr = *itr->second;
        msg = rtr->second;
        res = resp_t(res.get_id(), recver);
        return true;
      }
    }
    else
    {
      svc_exit_list_t::iterator itr(svc_exit_list_.find(svc));
      if (itr != svc_exit_list_.end())
      {
        recv_itr rtr = *itr->second.second;
        msg = rtr->second;
        res = resp_t(res.get_id(), itr->second.first);
        return true;
      }
    }
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
  add_match_msg(rcv, sender, msg);
  scp.reset();
}
///------------------------------------------------------------------------------
void mailbox::push(exit_t ex, message const& msg)
{
  recv_t rcv(ex);
  scope scp(boost::bind(&recv_queue_t::pop_back, &recv_que_));
  add_match_msg(rcv, ex.get_aid(), msg);
  scp.reset();
}
///------------------------------------------------------------------------------
void mailbox::push(request_t req, message const& msg)
{
  scope scp(boost::bind(&recv_queue_t::pop_back, &recv_que_));
  recv_t rcv(req);
  add_match_msg(rcv, aid_t(), msg);
  std::pair<wait_reply_list_t::iterator, bool> pr =
    wait_reply_list_.insert(std::make_pair(req.get_aid(), dummy_));
  pr.first->second.push_back(req);
  scp.reset();
}
///------------------------------------------------------------------------------
bool mailbox::push(resp_t res, message const& msg)
{
  res_msg_list_.insert(std::make_pair(res.get_id(), std::make_pair(res, msg)));
  return false;
}
///------------------------------------------------------------------------------
void mailbox::add_match_msg(recv_t const& rcv, aid_t sender, message const& msg)
{
  recv_itr itr = recv_que_.insert(recv_que_.end(), std::make_pair(rcv, msg));
  match_t type = msg.get_type();
  match_itr mtr;
  match_queue_t* match_que = 0;

  if (type >= 0 && type < (match_t)cache_match_list_.size())
  {
    match_que = &cache_match_list_[type];
  }
  else
  {
    std::pair<match_queue_list_t::iterator, bool> pr =
      match_queue_list_.insert(std::make_pair(type, dummy2_));
    match_que = &pr.first->second;
  }
  mtr = match_que->insert(match_que->end(), itr);

  if (sender && type == exit)
  {
    if (sender.svc_)
    {
      std::pair<aid_t, match_itr> p = std::make_pair(sender, mtr);
      std::pair<svc_exit_list_t::iterator, bool> pr = 
        svc_exit_list_.insert(std::make_pair(sender.svc_, p));
      if (!pr.second)
      {
        recv_que_.erase(*pr.first->second.second);
        match_que->erase(pr.first->second.second);
        pr.first->second = p;
      }
    }
    else
    {
      std::pair<exit_list_t::iterator, bool> pr = 
        exit_list_.insert(std::make_pair(sender, mtr));
      if (!pr.second)
      {
        recv_que_.erase(*pr.first->second);
        match_que->erase(pr.first->second);
        pr.first->second = mtr;
      }
    }
  }
}
///------------------------------------------------------------------------------
bool mailbox::fetch_match_msg(match_t type, recv_t& src, message& msg)
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
    if (type == exit)
    {
      aid_t sender;
      if (aid_t* aid = boost::get<aid_t>(&src))
      {
        sender = *aid;
      }
      else if (exit_t* ex = boost::get<exit_t>(&src))
      {
        sender = ex->get_aid();
      }

      if (sender)
      {
        if (sender.svc_)
        {
          svc_exit_list_.erase(sender.svc_);
        }
        else
        {
          exit_list_.erase(sender);
        }
      }
    }
    recv_que_.erase(itr);
    return true;
  }

  return false;
}
///------------------------------------------------------------------------------
}
}
