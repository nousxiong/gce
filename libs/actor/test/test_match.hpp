///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/actor.hpp>
#include <gce/actor/thin.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/spawn.hpp>
#include <boost/atomic.hpp>

namespace gce
{
static boost::atomic_size_t match_count(0);
class match_ut
{
public:
  static void run()
  {
    for (std::size_t i=0; i<10; ++i)
    {
      test_common();
    }
    //test_raw();
  }

  struct stack
  {
    gce::aid_t sender;
    response_t res;
    message msg;
    std::size_t i;
    std::size_t size;
    std::vector<response_t> res_list;
  };

  static void my_child(thin_t self, stack& s)
  {
    //std::cout << "gce::thin: hello world!\n";
    GCE_REENTER(self)
    {
      //++actor_count;

      GCE_YIELD
      {
        match mach;
        mach.match_list_.push_back(5);
        self.recv(s.sender, s.msg);
      }

      GCE_YIELD self.recv(s.sender, s.msg, match(seconds_t(3)));

      //self.send(aid, msg);
      {
        s.msg = message(6);
        self.reply(s.sender, s.msg);
      }
//      for (s.i=0; s.i<msg_size; ++s.i)
//      {
//        GCE_YIELD self.recv(s.sender, s.msg);
//        self.send(s.sender, s.msg);
//      }
    }
  }

  static void my_actor_child(self_t self)
  {
    //std::cout << "gce::actor: hello world!\n";
//    ++actor_count;
    message msg(4);
    match mach;
    mach.match_list_.push_back(3);
    aid_t aid = self.recv(msg, mach);
    self.recv(msg, match(seconds_t(3)));
//    self.send(aid, msg);
    self.reply(aid, msg);
  }

  static void my_thin(thin_t self, aid_t base_id, stack& s)
  {
    GCE_REENTER(self)
    {
      s.size = 1;
      s.res_list.resize(s.size);
      for (std::size_t i=0; i<s.size; ++i)
      {
        aid_t aid = spawn<stackless>(
          self,
          boost::bind(&match_ut::my_child, _1, stack())
          );
        message m(5);
        s.res_list[i] = self.request(aid, m);
      }

      for (s.i=0; s.i<s.size; ++s.i)
      {
        do
        {
          s.sender = aid_t();
          GCE_YIELD self.recv(s.res_list[s.i], s.sender, s.msg, seconds_t(1));
        }
        while (!s.sender);
      }
//      s.sender = spawn<stackless>(
//        self,
//        boost::bind(&match_ut::my_child, _1, stack())
//        );
//
//      for (s.i=0; s.i<msg_size; ++s.i)
//      {
//        self.send(s.sender, s.msg);
//        GCE_YIELD self.recv(s.sender, s.msg);
//      }
      {
        s.msg = message(1);
        self.send(base_id, s.msg);
      }
      //std::cout << "end my_thin\n";
    }
  }

  static void my_actor(self_t self, aid_t base_id)
  {
    std::size_t size = 1;
    std::vector<response_t> res_list(size);
    message m(3);
    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid = spawn<stackful>(self, boost::bind(&match_ut::my_actor_child, _1));
      //self.send(aid, m);
      res_list[i] = self.request(aid, m);
    }

    message msg(1);
    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid;
      do
      {
        aid = self.recv(res_list[i], msg, seconds_t(1));
      }
      while (!aid);
      //self.recv(res_list[i], msg);
    }
//    for (std::size_t i=0; i<msg_size; ++i)
//    {
//      self.send(base_id, msg);
//      self.recv(msg);
//    }
//      aid_t aid = spawn<stackless>(self, boost::bind(&actor_ut::my_child, _1));
//
//      message msg;
//      for (std::size_t i=0; i<msg_size; ++i)
//      {
//        self.send(aid, msg);
//        self.recv(msg);
//      }
    self.send(base_id, message(1));
    //std::cout << "end my_actor\n";
  }

  static void my_thr(context& ctx, aid_t base_id)
  {
    mixin& mix = spawn(ctx);
    for (std::size_t i=0; i<2; ++i)
    {
      spawn<stackless>(mix, boost::bind(&match_ut::my_thin, _1, base_id, stack()));
      spawn<stackful>(mix, boost::bind(&match_ut::my_actor, _1, base_id));
    }
  }

  static void test_common()
  {
    try
    {
      std::size_t user_thr_num = 5;
      std::size_t my_actor_size = 21;
      attributes attrs;
      attrs.mixin_num_ = user_thr_num + 1;
      context ctx(attrs);

      mixin& base = spawn(ctx);
      aid_t base_id = base.get_aid();
      aid_t aid;
      for (std::size_t i=0; i<1; ++i)
      {
        aid = spawn<stackless>(
          base,
          boost::bind(
            &match_ut::my_thin,
            _1, base_id, stack()
            )
          );
      }

      boost::thread_group thrs;
      for (std::size_t i=0; i<user_thr_num; ++i)
      {
        thrs.create_thread(
          boost::bind(
            &match_ut::my_thr,
            boost::ref(ctx), base_id
            )
          );
      }

      thrs.join_all();

      message m;
      std::size_t i=0;
      {
        match mach;
        mach.match_list_.push_back(1);
        for (i=0; i<my_actor_size; ++i)
//        for (i=0; i<msg_size; ++i)
        {
          //std::cout << "i: " << i << std::endl;
          base.recv(m, mach);
//          base.send(aid, m);
        }
      }

      std::cout << std::endl;
//      std::cout << "actor_count: " << actor_count << std::endl;
      std::cout << "i: " << i << std::endl;
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}

