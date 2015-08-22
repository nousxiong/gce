///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef CLUSTER2_ADMIN_HPP
#define CLUSTER2_ADMIN_HPP

#include <gce/actor/all.hpp>
#include <gce/log/all.hpp>
#include <vector>

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

  /// 链接master，这里不需要master关注admin的状态，所以用monitor
  base.monitor(master_mgr);

  /// 帮助信息
  char const* help_desc = 
    "\nadmin usage: \n  status: get node(s) state\n  quit: stop all nodes\n  shutdown: stop master and admin\n  help: display this info";

  GCE_INFO(lg) << help_desc;

  /// 主循环，用于用户命令输入和查看结果，有任何异常即退出admin
  while (true)
  {
    std::string cmd;
    std::getline(std::cin, cmd);
    if (!std::cin.good())
    {
      GCE_ERROR(lg) << "std::cin error";
      break;
    }

    errcode_t ec;
    match_t type;
    message msg;
    std::string errmsg;
    if (cmd == "status")
    {
      base->send(master_mgr, "status");
      std::string total_desc;
      base->match("status_ret", "error", type).guard(master_mgr, ec).raw(msg).recv(total_desc);
      if (ec)
      {
        GCE_ERROR(lg) << cmd << " error";
        break;
      }

      if (type == atom("status_ret"))
      {
        /// 输出所有node的状态
        std::string node_status = "node(s) status: ";
        node_status += total_desc;
        GCE_INFO(lg) << node_status;
      }
    }
    else if (cmd == "quit")
    {
      base->send(master_mgr, "quit");
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

#endif /// CLUSTER2_ADMIN_HPP
