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
 *   这个示例（cluster3）改进了cluster2中node登录master之后建立全联通的过程，通过增加一次交互，
 * 不仅解决了node之间互联的等待问题，使得程序更加健壮，而且还能处理node和master宕机重启之后的全联通恢复
 *
 *   在node登录（login和login_ret消息）master成功后，node不再等待一段时间，而是发送ready消息给master，
 * 告之自己已经bind完毕，然后master会返回ready_ret消息来告之这个node去连接已经处于bind完毕状态的node(s)；
 * 同时也会把这个新bind完毕的node告之（conn消息）其它bind完毕的node(s)来让它们连接；
 *
 *   另外，改进quit命令的执行方式，现在每个quit命令会单独建立一个actor来专门处理，这里展示了需要多次消息交互
 * 的事务时，使用单独actor处理的优势
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
