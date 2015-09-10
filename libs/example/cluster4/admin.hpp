///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef CLUSTER4_ADMIN_HPP
#define CLUSTER4_ADMIN_HPP

#include "endpoint.hpp"
#include <gce/actor/all.hpp>
#include <gce/log/all.hpp>
#include <vector>
#include <algorithm>

void admin(gce::log::logger_t lgr)
{
  using namespace gce;

  attributes attr;
  attr.id_ = to_match("admin");
  attr.thread_num_ = 1;
  attr.lg_ = lgr;
  context ctx(attr);
  threaded_actor base = spawn(ctx);
  connect(base, "master", "tcp://127.0.0.1:23333");

  log::logger_t lg = base.get_context().get_logger();
  svcid_t master_mgr = make_svcid("master", "master_mgr");

  base.monitor(master_mgr);

  char const* help_desc = 
    "\nadmin usage: \n  status: get node(s) state\n  quit: stop all nodes\n  shutdown: stop master and admin\n  help: display this info";

  GCE_INFO(lg) << help_desc;

  while (true)
  {
    std::string cmd;
    std::getline(std::cin, cmd);
    if (!std::cin.good())
    {
      GCE_ERROR(lg) << "std::cin error";
      break;
    }

    /// 使用空格分割命令极其参数
    std::stringstream cmd_str(cmd);
    std::string tok;
    std::vector<std::string> tok_list;
    while (std::getline(cmd_str, tok, ' '))
    {
      tok_list.push_back(tok);
    }

    if (tok_list.empty())
    {
      continue;
    }
    cmd = tok_list[0];

    errcode_t ec;
    match_t type;
    message msg;
    std::string errmsg;
    if (cmd == "status")
    {
      base->send(master_mgr, "status");
      base->match("status_ret", "error", type).guard(master_mgr, ec).raw(msg).recv();
      if (ec)
      {
        GCE_ERROR(lg) << cmd << " error";
        break;
      }

      if (type == atom("status_ret"))
      {
        std::string total_desc;
        msg >> total_desc;
        std::string node_status = "node(s) status: ";
        node_status += total_desc;
        GCE_INFO(lg) << node_status;
      }
    }
    else if (cmd == "add")
    {
      /// 根据用户输入添加新的node(s)到集群，此命令仅为告诉master新node(s)的身份，之后需要手动启动新node(s)来真正加入
      if (tok_list.size() < 2)
      {
        GCE_ERROR(lg) << "not enough params; Usage: add <conn_ep> [conn_ep] ... ";
        continue;
      }
      cluster4::endpoint_list ep_list;
      ep_list.list_.resize(tok_list.size() - 1);
      std::copy(++tok_list.begin(), tok_list.end(), ep_list.list_.begin());
      base->send(master_mgr, "add", ep_list);
      base->match("add_ret", "error", type).guard(master_mgr, ec).raw(msg).recv();
      if (ec)
      {
        GCE_ERROR(lg) << cmd << " error";
        break;
      }

      if (type == atom("add_ret"))
      {
        std::string desc;
        msg >> desc;
        GCE_INFO(lg) << cmd << " " << desc;
      }
    }
    else if (cmd == "rmv")
    {
      /// 根据用户输入将指定的node(s)从集群中移除，此命令仅为告诉master要移除的node(s)的身份，在此之前需要用quit命令来让其退出
      if (tok_list.size() < 2)
      {
        GCE_ERROR(lg) << "not enough params; Usage: rmv <conn_ep> [conn_ep] ... ";
        continue;
      }

      cluster4::endpoint_list ep_list;
      ep_list.list_.resize(tok_list.size() - 1);
      std::copy(++tok_list.begin(), tok_list.end(), ep_list.list_.begin());
      base->send(master_mgr, "rmv", ep_list);
      base->match("rmv_ret", "error", type).guard(master_mgr, ec).raw(msg).recv();
      if (ec)
      {
        GCE_ERROR(lg) << cmd << " error";
        break;
      }

      if (type == atom("rmv_ret"))
      {
        std::string desc;
        msg >> desc;
        GCE_INFO(lg) << cmd << " " << desc;
      }
    }
    else if (cmd == "quit")
    {
      /// 可以指定要退出的node(s)列表，如果不指定，就是全部node(s)
      cluster4::endpoint_list ep_list;
      ep_list.list_.resize(tok_list.size() - 1);
      std::copy(++tok_list.begin(), tok_list.end(), ep_list.list_.begin());
      base->send(master_mgr, "quit", ep_list);
      base->match("quit_ret", "error", type).guard(master_mgr, ec).raw(msg).recv();
      if (ec)
      {
        GCE_ERROR(lg) << cmd << " error";
        break;
      }

      if (type == atom("quit_ret"))
      {
        std::string total_desc;
        msg >> total_desc;
        GCE_INFO(lg) << cmd << " ok, " << total_desc;
      }
    }
    else if (cmd == "shutdown")
    {
      base->send(master_mgr, "shutdown");
      base->match("shutdown_ret", "error", type).guard(master_mgr, ec).raw(msg).recv();
      if (ec)
      {
        GCE_ERROR(lg) << cmd << " error";
        break;
      }

      if (type == atom("shutdown_ret"))
      {
        GCE_INFO(lg) << cmd << " ok";
        break;
      }
    }
    else if (cmd == "help")
    {
      GCE_INFO(lg) << help_desc;
    }

    if (type == atom("error"))
    {
      msg >> errmsg;
      GCE_ERROR(lg) << errmsg;
    }
  }

  GCE_INFO(lg) << "admin quit.";
}

#endif /// CLUSTER4_ADMIN_HPP
