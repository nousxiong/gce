///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef CLUSTER5_ADMIN_HPP
#define CLUSTER5_ADMIN_HPP

#include "param.hpp"
#include <gce/actor/all.hpp>
#include <gce/log/all.hpp>
#include <vector>
#include <algorithm>

static void admin(gce::log::logger_t lgr)
{
  using namespace gce;

  attributes attr;
  attr.id_ = to_match("admin");
  attr.thread_num_ = 1;
  attr.lg_ = lgr;
  context ctx(attr);
  threaded_actor base = spawn(ctx);
  aid_t admin_id = spawn(base, "admin.lua", monitored);

  log::logger_t lg = base.get_context().get_logger();
  while (true)
  {
    std::string line;
    std::getline(std::cin, line);
    if (!std::cin.good())
    {
      GCE_ERROR(lg) << "std::cin error";
      base->send(admin_id, "end");
      base->recv(gce::exit);
      break;
    }

    /// 使用空格分割命令极其参数
    std::stringstream cmd_str(line);
    std::string cmd;
    std::string tok;
    cluster5::param_list tok_list;
    if (std::getline(cmd_str, tok, ' '))
    {
      cmd = tok;
    }
    while (std::getline(cmd_str, tok, ' '))
    {
      tok_list.list_.push_back(tok);
    }

    if (cmd.empty())
    {
      continue;
    }

    /// 将这次命令极其参数列表发送给admin.lua
    base->send(admin_id, "cmd", cmd, tok_list);
    errcode_t ec;
    base->match("cmd_ret").guard(admin_id, ec).recv();
    if (ec)
    {
      break;
    }
  }

  GCE_INFO(lg) << "admin quit.";
}

#endif /// CLUSTER5_ADMIN_HPP
