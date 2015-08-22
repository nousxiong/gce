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
 *   在这个程序中（cluster2），从这个示例开始，我们一步步增加手游服务端中具体的服务逻辑，比如游戏服务器、DB服务器等；
 *
 *   这个示例先介绍如何在master的控制下建立node(s)之间的全联通(fully connected)，集群基本结构图就变成：
 *
 * M: master
 * N: node
 *
 *            M
 *        ____|______
 *       |    |      |
 *       N1   N2     Nx
 *       |____|______|
 *            |
 *      (fully connected)
 *
 *   每个node登录master的时候，master会根据其id来在登录回复消息中指明这个node去连接哪些node，
 * 以及任何其它需要处理的事情（比如启动何种actor），这部分是用户可以根据自己的需求而自定义，
 * 类似于集群的控制脚本，以后可以通过改为脚本（lua或js）来实现完全脚本化
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
  attributes attr;
  attr.id_ = to_match(id);
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
