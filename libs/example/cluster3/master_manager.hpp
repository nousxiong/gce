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
#include "node_quit.hpp"
#include <gce/actor/all.hpp>
#include <gce/log/all.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <vector>


struct node_stat
{
  enum status
  {
    offline = 0, /// 离线
    login, /// 登录，但还未bind成功，因为要建立全联通网络，所以每个node要先bind成功，再让其它node连接
    online, /// 已经bind完毕
  };

  node_stat()
    : stat_(offline)
  {
  }

  explicit node_stat(status stat, cluster3::node_info const& ni)
    : stat_(stat)
    , ni_(ni)
  {
  }

  char const* status2string() const
  {
    if (stat_ == offline)
    {
      return "offline";
    }
    else if (stat_ == login)
    {
      return "login";
    }
    else
    {
      return "online";
    }
  }

  status stat_;
  cluster3::node_info const ni_; /// 节点信息，包括其ctxid和网络地址（ip地址+端口）
};

typedef std::map<gce::svcid_t, node_stat> node_stat_list_t;

bool update_node_stat(node_stat_list_t& node_stat_list, gce::svcid_t node_id, node_stat::status stat)
{
  using namespace gce;

  node_stat_list_t::iterator itr = node_stat_list.find(node_id);
  if (itr != node_stat_list.end())
  {
    itr->second.stat_ = stat;
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

  node_stat_list_t node_stat_list;
  size_t node_num = boost::thread::hardware_concurrency();
  std::string node_ep = "tcp://127.0.0.1:";
  for (size_t i=0; i<node_num; ++i)
  {
    size_t port = 23334 + i;
    svcid_t svcid = make_svcid(to_match(i), "node_mgr");
    cluster3::node_info ni = cluster3::make_node_info(svcid, node_ep + boost::lexical_cast<std::string>(port));
    node_stat_list.insert(std::make_pair(ni.svcid_, node_stat(node_stat::offline, ni)));
  }

  /// 当前的处理node quit事务的aid
  aid_t node_quit_id = aid_nil;
  aid_t admin_id = aid_nil;

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
        if (update_node_stat(node_stat_list, node_id, node_stat::login))
        {
          std::string bind_ep;
          node_stat_list_t::const_iterator itr = node_stat_list.find(node_id);
          assert(itr != node_stat_list.end());

          /// 构建这个node的bind的网络地址，这里由于都是本机所以直接使用node_info中的ep即可
          bind_ep = itr->second.ni_.ep_;
          self->send(sender, "login_ret", true, bind_ep);
        }
        else
        {
          /// 返回错误
          self->send(sender, "login_ret", false, "node not found");
        }
      }
      else if (type == atom("ready"))
      {
        /// 某node bind完毕
        svcid_t node_id = sender.svc_;
        assert(node_id != svcid_nil);
        if (update_node_stat(node_stat_list, node_id, node_stat::online))
        {
          cluster3::node_info ready_ni;
          /// 让这个node连接其它online状态的node
          cluster3::node_info_list ni_list;
          ni_list.list_.reserve(node_stat_list.size() - 1);
          BOOST_FOREACH(node_stat_list_t::value_type const& pr, node_stat_list)
          {
            cluster3::node_info const& ni = pr.second.ni_;
            if (pr.second.stat_ == node_stat::online)
            {
              if (ni.svcid_ != node_id)
              {
                /// 不包括它自己
                ni_list.list_.push_back(pr.second.ni_);
              }
              else
              {
                ready_ni = ni;
              }
            }
          }
          self->send(sender, "ready_ret", true, ni_list);

          /// 让其它online状态的node连接这个node
          assert(cluster3::valid(ready_ni));
          BOOST_FOREACH(cluster3::node_info const& ni, ni_list.list_)
          {
            self->send(ni.svcid_, "conn", ready_ni);
          }
        }
        else
        {
          /// 返回错误
          self->send(sender, "ready_ret", false, "node not found");
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
          desc += pr.second.status2string();
          total_desc += desc;
        }

        self->send(sender, "status_ret", total_desc);
      }
      else if (type == atom("quit"))
      {
        admin_id = sender;

        if (node_quit_id != aid_nil)
        {
          /// 说明上一次quit还在进行中，返回错误
          self->send(admin_id, "error", "last node_quit not end yet");
        }
        else
        {
          /// 每次处理quit命令，单独使用一个actor来进行
          node_quit_id = spawn(self, boost::bind(&node_quit, _arg1));

          cluster3::node_info_list ni_list;
          /// 将当前所有online状态的node信息传给node_quit
          BOOST_FOREACH(node_stat_list_t::value_type const& pr, node_stat_list)
          {
            if (pr.second.stat_ == node_stat::online)
            {
              ni_list.list_.push_back(pr.second.ni_);
            }
          }
          self->send(node_quit_id, "init", ni_list);
        }
      }
      else if (type == atom("quit_ret"))
      {
        /// node_quit结束，返回结果给admin
        self.send(admin_id, msg);
        node_quit_id = aid_nil;
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
        update_node_stat(node_stat_list, node_id, node_stat::offline);

        /// 告之node_quit
        if (node_quit_id != aid_nil)
        {
          self->send(node_quit_id, "node_exit", node_id);
        }
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
