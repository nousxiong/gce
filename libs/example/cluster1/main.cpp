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
 *   在这个程序中（cluster1），主要介绍master和node(s)各自的管理actor，以及它们之间的actor链接
 *
 *   master和每个node进程在存活期内都会一直存在一个独一无二的服务，被称为“管理actor”，用于管理自己进程内的所有事务，
 * 比如读取配置文件，启动actor、管理actor，处理进程的启动和关闭等；
 *   另外，为了错误宕机，或者网络错误，master和node(s)之间的管理actor需要建立“actor链接”，以便在错误发生时，能及时侦测到；
 *
 *   在这个程序中，我们建立master和node各自的管理actor，并将其链接起来，建立一个初步的master-node(s)的管理actor树形结构；
 * 这个树形结构能处理简单的宕机（比如master或者某node进程挂掉，能及时检测到，并不会发生逻辑错误）
 *
 * 注1：示例中的注释，为了清晰，如果是之前示例中已经注释过的，在之后的示例中就会不再进行注释，如果对某些代码有疑问，请查阅之前的示例
 * 注2：注1和2也在以后示例中不会再出现
 */

#include "master_manager.hpp"
#include "node_manager.hpp"
#include "admin.hpp"

#include <gce/actor/all.hpp>
#include <gce/log/all.hpp>
#include <boost/thread.hpp>
#include <vector>
#include <iostream>

using namespace gce;

void master(size_t node_num, log::logger_t lg)
{
  attributes attr;
  attr.id_ = to_match("master");
  attr.lg_ = lg; /// 设置logger
  context ctx(attr);
  threaded_actor base = spawn(ctx);
  gce::bind(base, "tcp://0.0.0.0:23333");

  spawn(base, boost::bind(&master_manager, _arg1), monitored);
  base->recv(gce::exit);
}

void node(size_t const id, log::logger_t lg)
{
  attributes attr;
  attr.id_ = to_match(id);
  attr.lg_ = lg; /// 设置logger
  attr.thread_num_ = 1;
  context ctx(attr);
  threaded_actor base = spawn(ctx);
  connect(base, "master", "tcp://127.0.0.1:23333");

  spawn(base, boost::bind(&node_manager, _arg1), monitored);
  base->recv(gce::exit);
}

int main()
{
  /// 使用gce提供的基于asio的异步logger来输出日志；此示例需要一些输出
  log::asio_logger lgr;
  log::logger_t lg = boost::bind(&log::asio_logger::output, &lgr, _arg1, "");

  size_t const node_num = boost::thread::hardware_concurrency();
  boost::thread thr_master(boost::bind(&master, node_num, lg));
  boost::thread_group thrs_node;
  for (size_t i=0; i<node_num; ++i)
  {
    thrs_node.create_thread(boost::bind(&node, i, lg));
  }

  /**
   *   这里我们用一个线程模拟管理进程，管理进程能让集群管理员通过交互命令来直接控制集群；
   * 这些交互命令发送给master，由其处理
   */
  boost::thread thr_admin(boost::bind(&admin, lg));

  thrs_node.join_all();
  thr_master.join();
  thr_admin.join();

  GCE_INFO(lg) << "done.";
  return 0;
}
