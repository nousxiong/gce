///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef CLUSTER2_NODE_QUIT_HPP
#define CLUSTER2_NODE_QUIT_HPP

#include "node_info.hpp"
#include <gce/actor/all.hpp>
#include <boost/foreach.hpp>

/// 我们使用一个actor来处理node quit的事务
static void node_quit(gce::stackful_actor self)
{
  using namespace gce;

  context& ctx = self.get_context();
  ctxid_t ctxid = ctx.get_ctxid();
  log::logger_t lg = ctx.get_logger();

  svcid_t master_mgr = make_svcid("master", "master_mgr");
  self.monitor(master_mgr);

  size_t ready_quit_num = 0;
  std::set<svcid_t> quited_node_list;

  cluster3::node_info_list ni_list;
  errcode_t ec;
  self->match("init").guard(master_mgr, ec).recv(ni_list);
  if (ec)
  {
    GCE_ERROR(lg) << "master quited, no quit will handle";
    return;
  }

  /// 根据要quit的node信息，发送quit给node(s)
  BOOST_FOREACH(cluster3::node_info const& ni, ni_list.list_)
  {
    ++ready_quit_num;
    self->send(ni.svcid_, "quit");
  }

  /// 如果已经全部非online状态，则立刻返回
  if (ready_quit_num == 0)
  {
    std::string total_desc = "0 node quited";
    self->send(master_mgr, "quit_ret", total_desc);
    return;
  }

  /// 依次接收所有node的quit_ret消息
  for (size_t i=0; i<ready_quit_num; )
  {
    match_t type;
    errcode_t ec;
    message msg;
    aid_t sender = self->match("quit_ret", "node_exit", type).guard(master_mgr, ec).raw(msg).recv();
    if (ec)
    {
      GCE_ERROR(lg) << "master quited, can't continue, quit asap";
      return;
    }
    else
    {
      if (type == atom("quit_ret"))
      {
        svcid_t node_id = sender.svc_;
        assert(node_id != svcid_nil);
        /// 将退出的node插入列表
        quited_node_list.insert(node_id);
        ++i;
      }
      else
      {
        svcid_t node_id;
        msg >> node_id;
        if (quited_node_list.find(node_id) == quited_node_list.end())
        {
          /// 这种情况表明node非正常退出（master转发的node的exit消息），不加入quited列表；
          ++i;
        }
      }
    }
  }

  /// 全部结束，返回结果给master
  std::string total_desc = boost::lexical_cast<std::string>(quited_node_list.size());
  total_desc += " node(s) quited: ";
  BOOST_FOREACH(svcid_t const& svcid, quited_node_list)
  {
    total_desc += "\n";
    total_desc += "  node<";
    total_desc += to_string(svcid.ctxid_);
    total_desc += ">";
  }

  self->send(master_mgr, "quit_ret", total_desc);
}

#endif /// CLUSTER2_NODE_QUIT_HPP
