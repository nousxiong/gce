///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef CLUSTER2_MASTER_MANAGER_HPP
#define CLUSTER2_MASTER_MANAGER_HPP

#include "node_info.hpp"
#include <gce/actor/all.hpp>
#include <gce/log/all.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <vector>


struct node_stat
{
  node_stat()
    : online_(false)
  {
  }

  explicit node_stat(bool online, cluster2::node_info const& ni)
    : online_(online)
    , ni_(ni)
  {
  }

  bool online_;
  cluster2::node_info const ni_; /// 节点信息，包括其ctxid和网络地址（ip地址+端口）
};

typedef std::map<gce::svcid_t, node_stat> node_stat_list_t;

/// 更新节点状态，如果找不到节点，返回false
bool update_node_stat(node_stat_list_t& node_stat_list, gce::svcid_t node_id, bool online)
{
  using namespace gce;

  node_stat_list_t::iterator itr = node_stat_list.find(node_id);
  if (itr != node_stat_list.end())
  {
    itr->second.online_ = online;
    return true;
  }
  else
  {
    return false;
  }
}

static void master_manager(gce::stackful_actor self)
{
  using namespace gce;

  register_service(self, "master_mgr");
  context& ctx = self.get_context();
  log::logger_t lg = ctx.get_logger();

  /** 
   *   构建所有node的信息，用于node登录时，告之其它node的node_info，
   * 使其能连接到其它node，建立node之间的全联通网络；此处未来可以通过配置（脚本）进行构建
   */
  node_stat_list_t node_stat_list;
  size_t node_num = boost::thread::hardware_concurrency();
  std::string node_ep = "tcp://127.0.0.1:";
  for (size_t i=0; i<node_num; ++i)
  {
    /// 根据i来决定node的端口，由于这里示例所有node均在一个计算机上，所以ip相同，只有端口不同
    size_t port = 23334 + i;
    cluster2::node_info ni = cluster2::make_node_info(to_match(i), node_ep + boost::lexical_cast<std::string>(port));
    node_stat_list.insert(std::make_pair(make_svcid(ni.ctxid_, "node_mgr"), node_stat(false, ni)));
  }

  aid_t admin_id = aid_nil;
  size_t ready_quit_num = 0;
  std::set<svcid_t> quited_node_list;

  GCE_INFO(lg) << "master start";

  while (true)
  {
    try
    {
      message msg;
      errcode_t ec;

      aid_t sender = self.recv(msg);
      match_t type = msg.get_type();
      if (type == atom("login"))
      {
        svcid_t node_id = sender.svc_;
        assert(node_id != svcid_nil);
        if (update_node_stat(node_stat_list, node_id, true))
        {
          /// 告诉node需要连接的其它node(s)的信息
          std::string bind_ep;
          cluster2::node_info_list ni_list;
          ni_list.list_.reserve(node_stat_list.size() - 1);
          BOOST_FOREACH(node_stat_list_t::value_type const& pr, node_stat_list)
          {
            /// 不包括它自己
            if (pr.second.ni_.ctxid_ != node_id.ctxid_)
            {
              ni_list.list_.push_back(pr.second.ni_);
            }
            else
            {
              /// 构建这个node的bind的网络地址， 这里由于都是本机所以直接使用node_info中的ep即可
              bind_ep = pr.second.ni_.ep_;
            }
          }

          self->send(sender, "login_ret", true, bind_ep, ni_list);
        }
        else
        {
          /// 返回错误
          self->send(sender, "login_ret", false, "node not found");
        }
      }
      else if (type == atom("status"))
      {
        admin_id = sender;
        std::string total_desc;
        /// 将当前node状态列表返回
        BOOST_FOREACH(node_stat_list_t::value_type const& pr, node_stat_list)
        {
          std::string desc = "\n  node<";
          desc += to_string(pr.first.ctxid_);
          desc += ">: ";
          desc += pr.second.online_ ? "online" : "offline";
          total_desc += desc;
        }

        self->send(sender, "status_ret", total_desc);
      }
      else if (type == atom("quit"))
      {
        admin_id = sender;

        /// 清理退出用数据结构
        ready_quit_num = 0;
        quited_node_list.clear();

        /// 向online状态的node发送退出命令
        BOOST_FOREACH(node_stat_list_t::value_type const& pr, node_stat_list)
        {
          if (pr.second.online_)
          {
            ++ready_quit_num;
            self.send(pr.first, msg);
          }
        }

        /// 如果已经全部非online状态，则立刻返回
        if (ready_quit_num == 0)
        {
          std::string total_desc = "0 node quited";
          self->send(admin_id, "quit_ret", total_desc);
        }
      }
      else if (type == atom("quit_ret"))
      {
        svcid_t node_id = sender.svc_;
        assert(node_id != svcid_nil);

        /// 将退出的node插入列表
        quited_node_list.insert(node_id);
        if (--ready_quit_num == 0)
        {
          /// 如果请求的node全部退出，则告之admin
          std::string total_desc = boost::lexical_cast<std::string>(quited_node_list.size());
          total_desc += " node(s) quited: ";
          BOOST_FOREACH(svcid_t const& svcid, quited_node_list)
          {
            total_desc += "\n";
            total_desc += "  node<";
            total_desc += to_string(svcid.ctxid_);
            total_desc += ">";
          }
          self->send(admin_id, "quit_ret", total_desc);
        }
      }
      else if (type == atom("shutdown"))
      {
        admin_id = sender;
        self->send(admin_id, "shutdown_ret");
        break;
      }
      else if (type == atom("error"))
      {
        svcid_t node_id = sender.svc_;
        assert(node_id != svcid_nil);

        /// 将错误转发给admin
        self.send(admin_id, msg);
      }
      else if (type == gce::exit)
      {
        /// 当前除了node之外，没有其他actor和master链接（admin使用的是单向的），所以肯定是node
        svcid_t node_id = sender.svc_;
        assert(node_id != svcid_nil);
        update_node_stat(node_stat_list, node_id, false);
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << "master_manager except: " << ex.what();
    }
  }

  deregister_service(self, "master_mgr");
  GCE_INFO(lg) << "master quit";
}

#endif /// CLUSTER2_MASTER_MANAGER_HPP
