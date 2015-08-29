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
 *   这个示例（cluster4）改进了cluster3中node的ctxid的配置方式，cluster3之前是需要手动直接配置每个node
 * 的ctxid，这样做在部署node进程的时候，容易出错，而cluster4开始，改为配置每个node的网络地址，
 * 即让其它node进行连接的地址，node在启动的时候用字符串哈希函数处理这个地址，取得的结果为ctxid；
 * 
 *   master这边配置所有的node的网络地址，如果哈希结果有碰撞（几乎不可能），则master启动的时候会报错，让用户
 * 修改碰撞的node的地址，然后继续直到正常；
 *
 *   另外一个改进是增加新node的加入和旧node的移除从集群中；
 */

///----------------------------------------------------------------------------
/// node登录master
///----------------------------------------------------------------------------
///
/// 成功：
/// node_manager           master_manager
///     |                        |
///     | login                  |
///     |----------------------->|
///     |                        |
///     |              login_ret |
///     |<-----------------------|
///
/// 失败：
/// node_manager           master_manager
///     |                        |
///     | login                  |
///     |----------------------->|
///     |                        |
///     |              login_ret |
///     |<-----------------------|
///
/// master宕机：
///    node_manager           master_manager
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
/// node_manager       master_manager      node_manager(s)
///     |                    |                 |
///     | ready              |                 |
///     |------------------->|                 |
///     |                    |                 |
///     |          ready_ret | conn(s)         |
///     |<-------------------|---------------->|
///
/// 失败：
/// node_manager    master_manager 
///     |                 |
///     | ready           |
///     |---------------->|
///     |                 |
///     |       ready_ret |
///     |<----------------|
///
/// master宕机：
///    node_manager           master_manager
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
/// admin          master_manager
///   |                  |
///   | status           |
///   |----------------->|
///   |                  |
///   |       status_ret |
///   |<-----------------|
///
/// master宕机：
/// admin          master_manager
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
/// admin          master_manager
///   |                  |
///   | add              |
///   |----------------->|
///   |                  |
///   |          add_ret |
///   |<-----------------|
///
/// master宕机：
/// admin          master_manager
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
/// admin          master_manager
///   |                  |
///   | rmv              |
///   |----------------->|
///   |                  |
///   |          rmv_ret |
///   |<-----------------|
///
/// master宕机：
/// admin          master_manager
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
/// admin          master_manager                          node_manager(s)
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
/// admin          master_manager
///   |                  |
///   | quit             |
///   |----------------->|
///   |                  |
///   |            error |
///   |<-----------------|
///
/// master宕机（admin发送quit时）：
/// admin          master_manager
///   |                  |
///   | quit             |
///   |----------------->|
///   |                  |
///   |        gce::exit |
///   |<-----------------|
///
/// master宕机（node_quit处理过程中）：
/// admin          master_manager                     node_manager(s)
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


#include "string_hash.hpp"
#include "master_manager.hpp"
#include "node_manager.hpp"
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
  context ctx(attr);
  threaded_actor base = spawn(ctx);
  gce::bind(base, "tcp://0.0.0.0:23333");

  spawn(base, boost::bind(&master_manager, _arg1), monitored);
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
  context ctx(attr);
  threaded_actor base = spawn(ctx);
  connect(base, "master", "tcp://127.0.0.1:23333");

  spawn(base, boost::bind(&node_manager, _arg1), monitored);
  base->recv(gce::exit);
}

int main()
{
  log::asio_logger lgr;
  log::logger_t lg = boost::bind(&log::asio_logger::output, &lgr, _arg1, "");

  size_t const node_num = boost::thread::hardware_concurrency();
  boost::thread thr_master(boost::bind(&master, lg));
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
