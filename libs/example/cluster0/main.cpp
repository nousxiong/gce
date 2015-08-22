///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

/**
 *   这是一个系列示例程序，从cluster0开始，到最后的cluster，由简单到复杂构建一个基于gce的集群架构
 * 
 *   这个系列最后会完成一个基本的手机网游的服务端（cluster），但这个架构在前N个示例程序中完全是在
 * 构建通用的部分，完全可以以此为基础进行其它类型服务端的开发
 */

/**
 *   集群基本思路介绍
 *
 *   1、在物理上，是master + node(s)结构，master是根节点，node(s)是叶节点
 *   2、master统一管理node(s)，包括在其上要运行的actor，和其需要的配置信息，这样只用维护1个地方即可管理整个集群
 *   3、node(s)唯一需要的配置信息，就是master的地址
 *
 *   集群基本结构图
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
 *         (optional)
 *
 *   注：node(s)之间可以根据需要自由进行互相连接，这是上图中(optional)的含义
 *
 *   在这个程序中（cluster0），主要介绍master和node(s)如何互相连接
 */

/// gce依赖
#include <gce/actor/all.hpp>

/// boost依赖
#include <boost/thread.hpp>

/// stl依赖
#include <iostream>


using namespace gce;

/// master的服务actor
void master_service(stackful_actor self, size_t const node_num)
{
  /**
   *   将此actor注册为gce中的服务，服务拥有人类可读的id（well-known)，
   * 可以让其他actor直接使用服务id来访问到服务actor
   *   这里使用"master_svc"名字来标识这个服务
   */
  register_service(self, "master_svc");

  for (size_t i=0; i<node_num; ++i)
  {
    /// 每个node发送一次hi的消息，然后返回一个ok消息，来确认已经连通
    aid_t sender = self->match("hi").recv();
    self->send(sender, "ok");
  }

  /// 在actor结束的时候取消这个服务
  deregister_service(self, "master_svc");
}

/// master线程
void master(size_t const node_num)
{
  attributes attr;
  /*
   *   设置master的ctxid，同一个集群内的所有gce的ctxid都必须不同，保持唯一性
   *   注：gce::to_match的字符串长度必须<=13；亦可直接使用整数
   */
  attr.id_ = to_match("master");
  context ctx(attr);

  /// 首先创建一个threaded_actor，所有gce的功能都是由它开始
  threaded_actor base = spawn(ctx);

  /// bind一个地址端口，用于接受来自node(s)的连接；0.0.0.0表示接受任何主机的连接
  gce::bind(base, "tcp://0.0.0.0:23333");

  /**
   *   创建一个stackful_actor来接收连接上来的node(s)发来的消息
   *   参数gce::monitored表示base监视新产生的actor的情况，
   * 一旦其退出，base会收到gce::exit类型的消息
   */ 
  spawn(base, boost::bind(&master_service, _arg1, node_num), monitored);

  /// 等待master_service退出，从而结束master
  base->recv(gce::exit);
}

/// node线程
void node(size_t const id)
{
  attributes attr;
  /// 设置node的ctxid，直接使用整数序列号
  attr.id_ = to_match(id);

  /// 默认gce会使用硬件并发数来设置内部线程数量，这里设置1
  attr.thread_num_ = 1;
  context ctx(attr);
  threaded_actor base = spawn(ctx);

  /// connect到master，指定master的ctxid和其bind的地址端口
  connect(base, "master", "tcp://127.0.0.1:23333");

  /// 通过master的服务"master_svc"来访问；svcid_t由ctxid和服务名字组成
  svcid_t svc = make_svcid("master", "master_svc");

  /// 发送hi消息给master的服务
  base->send(svc, "hi");

  /// 等待回应ok消息，如果收到则说明成功和master连通；然后退出
  base->match("ok").recv();
}

int main()
{
  /// 我们用1个线程模拟master进程，其他N个线程模拟N个node(s)进程
  size_t const node_num = boost::thread::hardware_concurrency();

  /// 1个master
  boost::thread thr_master(boost::bind(&master, node_num));

  /// N个node，使用硬件物理并发数
  boost::thread_group thrs_node;
  for (size_t i=0; i<node_num; ++i)
  {
    thrs_node.create_thread(boost::bind(&node, i));
  }

  thrs_node.join_all();
  thr_master.join();

  /// 如果到这步，说明一切正常，示例结束
  std::cout << "done." << std::endl;
  return 0;
}
