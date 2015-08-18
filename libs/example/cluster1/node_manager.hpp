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

static void login(gce::stackful_actor& self)
{
  using namespace gce;

  context& ctx = self.get_context();
  ctxid_t ctxid = ctx.get_ctxid();
  log::logger_t lg = ctx.get_logger();

  svcid_t master_mgr = make_svcid("master", "master_mgr");
  errcode_t ec;

  do
  {
    GCE_INFO(lg) << "node<" << ctxid << "> login master...";
    /// 建立和master管理actor的链接
    self.link(master_mgr);

    /// 发送登录消息给master
    self->send(master_mgr, "login");

    /**
     *   等待master的确认消息"login_ret"，这里使用了match - recv的语法糖模式：
     *
     *   1、match表示要匹配的特定类型的消息（可以指定多个），只有接收到指定类型的消息才会返回；
     *   2、guard表示如果指定svcid/aid的actor退出，并且在之前link/monitor过它，
     * 则会捕获到其发来的gce::exit消息，然后返回，ec返回错误的代码（如果不指定ec，则会抛出异常）；
     *   3、recv用于反序列化消息中的参数，可以不/只指定部分参数，但必须要按照参数序列化的顺序；
     *
     *   以上3种分句，match和recv是必须有的，其他可选；match必须在开头，recv必须在结尾
     */
    self->match("login_ret").guard(master_mgr, ec).recv();
    if (ec)
    {
      GCE_INFO(lg) << "node<" << ctxid << "> login master error: " << ec;
    }
    else
    {
      GCE_INFO(lg) << "node<" << ctxid << "> login master success";
    }
  }
  while (ec); /// 如果ec返回错误，则循环继续尝试登录，由于node的关闭也是由master来控制的，所以这里一直进行尝试；
}

static void node_manager(gce::stackful_actor self)
{
  using namespace gce;
  context& ctx = self.get_context();
  ctxid_t ctxid = ctx.get_ctxid();
  /// 取得在创建context时候设置的logger
  log::logger_t lg = ctx.get_logger();
  register_service(self, "node_mgr");

  /// master管理actor的svcid
  svcid_t master_mgr = make_svcid("master", "master_mgr");

  /// 自己的管理actor的svcid
  svcid_t node_mgr = make_svcid(ctxid, "node_mgr");

  /**
   *   node的主循环，用于处理来自master的管理消息
   */
  while (true)
  {
    try
    {
      /// 登录
      login(self);

      message msg;
      errcode_t ec;
      match_t type;
      /**
       *   等待master的指示，这里使用了match - recv的语法糖中另外一个分句：raw
       *
       *   raw分句，会返回匹配到的消息对象，多用于一次同时匹配多个类型消息的情况，
       * 一般这时候，消息的参数可能不同，所以无法使用recv直接反序列化消息参数；
       *
       *   match分句中，type用于返回收到的消息的类型，也可以直接从raw返回的消息中调用get_type来获取；
       *
       *   注意：当此方法返回时，一定要先检测ec是否有错误，如果有任何错误，那么类似raw、type一般都不会返回任何参数
       */
      self->match("quit", type).guard(master_mgr, ec).raw(msg).recv();
      if (!ec)
      {
        /// 如果正常，根据type类型分开处理；有任何错误直接循环重新登录继续；
        if (type == atom("quit"))
        {
          /// 退出命令，告诉master已经收到，然后尽快退出
          self->send(master_mgr, "quit_ret");
          break;
        }
        else
        {
          /// 发送错误消息返回，告之master不可用的命令
          std::string errmsg = "not available command: ";
          errmsg += atom(type);
          self->send(master_mgr, "error", errmsg);
        }
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << "node_manager<" << ctxid << "> except: " << ex.what();
    }
  }

  deregister_service(self, "node_mgr");
  GCE_INFO(lg) << "node<" << ctxid << "> quit";
}
