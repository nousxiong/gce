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
static boost::atomic_size_t link_count(0);
class link_ut
{
public:
  static void run()
  {
    for (std::size_t i=0; i<10; ++i)
    {
      test_common();
    }
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
      ++link_count;

      GCE_YIELD
      {
        match mach;
        mach.match_list_.push_back(5);
        self.recv(s.sender, s.msg);
      }

      //GCE_YIELD self.recv(s.sender, s.msg, match(seconds_t(3)));
    }
  }

  static void my_actor_child(self_t self)
  {
    //std::cout << "gce::actor: hello world!\n";
    ++link_count;
    message msg(4);
    match mach;
    mach.match_list_.push_back(3);
    aid_t aid = self.recv(msg, mach);
    //self.recv(msg, match(seconds_t(3)));
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
          boost::bind(&link_ut::my_child, _1, stack()),
          linked
          );
        message m(5);
        s.res_list[i] = self.request(aid, m);
      }

      GCE_YIELD
      {
        match mach;
        mach.match_list_.push_back(exit_normal);
        self.recv(s.sender, s.msg, mach);
      }
    }
  }

  static void my_actor(self_t self, aid_t base_id)
  {
    std::size_t size = 1;
    std::vector<response_t> res_list(size);
    message m(3);
    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid = spawn<stackful>(self, boost::bind(&link_ut::my_actor_child, _1), linked);
      //self.send(aid, m);
      res_list[i] = self.request(aid, m);
    }

    message msg;
    match mach;
    mach.match_list_.push_back(exit_normal);
    self.recv(msg, mach);
  }

  static void my_thr(context& ctx, aid_t base_id)
  {
    mixin& mix = spawn(ctx);
    for (std::size_t i=0; i<2; ++i)
    {
      spawn<stackless>(mix, boost::bind(&link_ut::my_thin, _1, base_id, stack()), monitored);
      spawn<stackful>(mix, boost::bind(&link_ut::my_actor, _1, base_id), monitored);
    }

    match mach;
    mach.match_list_.push_back(exit_normal);
    message m;
    for (std::size_t i=0; i<4; ++i)
    {
      mix.recv(m, mach);
    }
  }

  static void test_common()
  {
    try
    {
      std::size_t user_thr_num = 5;
      //std::size_t my_actor_size = 21;
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
            &link_ut::my_thin,
            _1, base_id, stack()
            ),
          monitored
          );
      }

      boost::thread_group thrs;
      for (std::size_t i=0; i<user_thr_num; ++i)
      {
        thrs.create_thread(
          boost::bind(
            &link_ut::my_thr,
            boost::ref(ctx), base_id
            )
          );
      }

      thrs.join_all();

      message m;
      std::size_t i=0;
      {
        match mach;
        mach.match_list_.push_back(exit_normal);
        for (i=0; i<1; ++i)
//        for (i=0; i<msg_size; ++i)
        {
          //std::cout << "i: " << i << std::endl;
          base.recv(m, mach);
//          base.send(aid, m);
        }
      }

      std::cout << std::endl;
      std::cout << "link_count: " << link_count << std::endl;
      //std::cout << "i: " << i << std::endl;
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
