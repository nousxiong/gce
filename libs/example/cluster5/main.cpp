///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

/**
 *   简介
 *
 *   这个示例（cluster5）将node、master、node_quit和admin都改为lua，分别为：
 * node.lua master.lua node_quit.lua admin.lua；
 *
 *   展示lua下的gce的用法
 */

///----------------------------------------------------------------------------
/// node登录master
///----------------------------------------------------------------------------
///
/// 成功：
///    node                    master
///     |                        |
///     | login                  |
///     |----------------------->|
///     |                        |
///     |              login_ret |
///     |<-----------------------|
///
/// 失败：
///    node                    master
///     |                        |
///     | login                  |
///     |----------------------->|
///     |                        |
///     |              login_ret |
///     |<-----------------------|
///
/// master宕机：
///       node                    master
///        |                        |
/// |----->| login                  |
/// |      |----------------------->|
/// |      |                        |
/// |      |              gce::exit |
/// |      |<-----------------------|
/// |<-----|

///----------------------------------------------------------------------------
/// node告诉master自己准备好接受其它node连接（在“node登录master”之后）
///----------------------------------------------------------------------------
///
/// 成功：
///    node                master            node(s)
///     |                    |                 |
///     | ready              |                 |
///     |------------------->|                 |
///     |                    |                 |
///     |          ready_ret | conn(s)         |
///     |<-------------------|---------------->|
///
/// 失败：
///    node             master 
///     |                 |
///     | ready           |
///     |---------------->|
///     |                 |
///     |       ready_ret |
///     |<----------------|
///
/// master宕机：
///       node                    master
///        |                        |
/// |----->| login                  |
/// |      |----------------------->|
/// |      |              login_ret |
/// |      |<-----------------------|
/// |      |                        |
/// |      | ready                  |
/// |      |----------------------->|
/// |      |              gce::exit |
/// |      |<-----------------------|
/// |      |
/// |<-----|

///----------------------------------------------------------------------------
/// admin查询master中所有node状态时序图
///----------------------------------------------------------------------------
///
/// 正常：
/// admin              master
///   |                  |
///   | status           |
///   |----------------->|
///   |                  |
///   |       status_ret |
///   |<-----------------|
///
/// master宕机：
/// admin              master
///   |                  |
///   | status           |
///   |----------------->|
///   |                  |
///   |        gce::exit |
///   |<-----------------|

///----------------------------------------------------------------------------
/// admin添加新node(s)进入集群时序图
///----------------------------------------------------------------------------
///
/// 正常：
/// admin              master
///   |                  |
///   | add              |
///   |----------------->|
///   |                  |
///   |          add_ret |
///   |<-----------------|
///
/// master宕机：
/// admin              master
///   |                  |
///   | add              |
///   |----------------->|
///   |                  |
///   |        gce::exit |
///   |<-----------------|

///----------------------------------------------------------------------------
/// admin移除指定node(s)从集群时序图
///----------------------------------------------------------------------------
///
/// 正常：
/// admin              master
///   |                  |
///   | rmv              |
///   |----------------->|
///   |                  |
///   |          rmv_ret |
///   |<-----------------|
///
/// master宕机：
/// admin              master
///   |                  |
///   | rmv              |
///   |----------------->|
///   |                  |
///   |        gce::exit |
///   |<-----------------|

///----------------------------------------------------------------------------
/// admin告诉master关闭node(s)
///
/// 正常：
/// admin              master                                node(s)
///   |                  |                                     |
///   | quit             |                                     |
///   |----------------->|                                     |
///   |                  | spawn                               |
///   |                  |--------------->node_quit            |
///   |                  | init               |                |
///   |                  |------------------->|                |
///   |                  |                    | quit(s)        |
///   |                  |                    |--------------->|
///   |                  |                    |                |
///   |                  |                    |    quit_ret(s) |
///   |                  |                    |<---------------|
///   |                  |                        gce::exit(s) |
///   |                  |<------------------------------------|
///   |                  | node_exit(s)       |
///   |                  |------------------->|
///   |                  |                    |
///   |                  |           quit_ret |
///   |                  |<-------------------|
///   |         quit_ret |
///   |<-----------------|
///
/// master拒绝quit命令，上一次还未结束：
/// admin              master
///   |                  |
///   | quit             |
///   |----------------->|
///   |                  |
///   |            error |
///   |<-----------------|
///
/// master宕机（admin发送quit时）：
/// admin              master
///   |                  |
///   | quit             |
///   |----------------->|
///   |                  |
///   |        gce::exit |
///   |<-----------------|
///
/// master宕机（node_quit处理过程中）：
/// admin              master                           node(s)
///   |                  |                                |
///   | quit             |                                |
///   |----------------->|                                |
///   |                  | spawn                          |
///   |                  |--------------->node_quit       |
///   |                  | init               |           |
///   |                  |------------------->|           |
///   |                  |                    |           |
///   |        gce::exit | gce::exit          |           |
///   |<-----------------|------------------->|           |
///   |                  | gce::exit(s)                   |
///   |                  |------------------------------->|
///   |                                                   |

/// include these before include gce/actor/message.hpp
#include "endpoint.adl.h"
#include "node_info.adl.h"
#include "param.adl.h"

#include "util.hpp"
#include "admin.hpp"

#include <gce/actor/all.hpp>
#include <gce/log/all.hpp>
#include <boost/thread.hpp>
#include <vector>
#include <iostream>

using namespace gce;

void master(log::logger_t lg)
{
  attributes attr;
  attr.id_ = to_match("master");
  attr.lg_ = lg;
  attr.lua_reg_list_.push_back(
    boost::bind(&cluster5::make_libutil, _arg1)
    );
  context ctx(attr);
  threaded_actor base = spawn(ctx);

  lua_Number node_num = (lua_Number)boost::thread::hardware_concurrency();
  aid_t master_id = spawn(base, "master.lua", monitored);
  /// 告之节点数量
  base->send(master_id, "init", node_num);
  base->recv(gce::exit);
}

void node(size_t const id, log::logger_t lg)
{
  /// 配置node的连接网络地址，hash后用于ctxid
  std::string conn_ep = "tcp://127.0.0.1:";
  conn_ep += boost::lexical_cast<std::string>(23334 + id);
  uint64_t hash = string_hash(conn_ep.c_str());

  attributes attr;
  attr.id_ = to_match(hash);
  attr.lg_ = lg;
  attr.thread_num_ = 1;
  attr.lua_reg_list_.push_back(
    boost::bind(&cluster5::make_libutil, _arg1)
    );
  context ctx(attr);
  threaded_actor base = spawn(ctx);

  spawn(base, "node.lua", monitored);
  base->recv(gce::exit);
}

int main()
{
  log::asio_logger lgr;
  log::logger_t lg = boost::bind(&log::asio_logger::output, &lgr, _arg1, "");

  size_t const node_num = boost::thread::hardware_concurrency();
  boost::thread thr_master(boost::bind(&master, lg));

  /// 等待一会儿，让master启动
  boost::this_thread::sleep_for(boost::chrono::milliseconds(300));
  boost::thread_group thrs_node;
  for (size_t i=0; i<node_num; ++i)
  {
    thrs_node.create_thread(boost::bind(&node, i, lg));
  }

  boost::thread thr_admin(boost::bind(&admin, lg));

  thrs_node.join_all();
  thr_master.join();
  thr_admin.join();

  GCE_INFO(lg) << "done.";
  return 0;
}
