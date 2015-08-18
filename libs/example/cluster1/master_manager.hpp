///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/all.hpp>
#include <gce/log/all.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <vector>

/// node的状态
struct node_stat
{
  node_stat()
    : online_(false)
  {
  }

  explicit node_stat(bool online)
    : online_(online)
  {
  }

  bool online_; /// 是否在线
};

typedef std::map<gce::svcid_t, node_stat> node_stat_list_t;
void update_node_stat(node_stat_list_t& node_stat_list, gce::svcid_t node_id, node_stat const& stat)
{
  using namespace gce;

  std::pair<node_stat_list_t::iterator, bool> pr = 
    node_stat_list.insert(std::make_pair(node_id, stat));
  if (!pr.second)
  {
    pr.first->second = stat;
  }
}

/// master的管理actor
static void master_manager(gce::stackful_actor self)
{
  using namespace gce;

  register_service(self, "master_mgr");
  context& ctx = self.get_context();
  log::logger_t lg = ctx.get_logger();

  /// 用于保存node(s)的状态
  node_stat_list_t node_stat_list;

  /// 当前的admin actor的aid
  aid_t admin_id = aid_nil;

  /// 当前准备退出的node数量
  size_t ready_quit_num = 0;

  /// 当前已经退出的node列表
  std::set<svcid_t> quited_node_list;

  GCE_INFO(lg) << "master start";

  /// 主消息循环
  while (true)
  {
    try
    {
      message msg;
      errcode_t ec;

      /**
       *   接收来自admin和node的所有消息，有任何错误发生，打印错误，继续循环
       */
      aid_t sender = self.recv(msg);
      match_t type = msg.get_type();
      if (type == atom("login"))
      {
        /// 服务actor发来的消息，aid中都会自带svcid
        svcid_t node_id = sender.svc_;
        assert(node_id != svcid_nil);

        /// 设置对应的node本地状态为online
        update_node_stat(node_stat_list, node_id, node_stat(true));
        self->send(sender, "login_ret");
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
        update_node_stat(node_stat_list, node_id, node_stat(false));
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
