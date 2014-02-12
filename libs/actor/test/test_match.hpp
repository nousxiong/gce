///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

namespace gce
{
class match_ut
{
public:
  static void run()
  {
    std::cout << "match_ut begin." << std::endl;
    test_common();
    std::cout << "match_ut end." << std::endl;
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
    GCE_REENTER(self)
    {
      GCE_YIELD
      {
        match mach;
        mach.match_list_.push_back(5);
        self.recv(s.sender, s.msg);
      }

      GCE_YIELD self.recv(s.sender, s.msg, match(seconds_t(3)));

      {
        s.msg = message(6);
        self.reply(s.sender, s.msg);
      }
    }
  }

  static void my_actor_child(self_t self)
  {
    aid_t aid = recv(self, 3);
    wait(self, seconds_t(3));
    reply(self, aid);
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
      {
        s.msg = message(1);
        self.send(base_id, s.msg);
      }
    }
  }

  static void my_actor(self_t self, aid_t base_id)
  {
    std::size_t size = 1;
    std::vector<response_t> res_list(size);
    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid = spawn<stackful>(self, boost::bind(&match_ut::my_actor_child, _1));
      res_list[i] = request(self, aid, 3);
    }

    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid;
      do
      {
        aid = recv(self, res_list[i], seconds_t(1));
      }
      while (!aid);
    }
    send(self, base_id, 1);
  }

  static void my_thr(context& ctx, aid_t base_id)
  {
    mixin_t mix = spawn(ctx);
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

      mixin_t base = spawn(ctx);
      aid_t base_id = base.get_aid();
      for (std::size_t i=0; i<1; ++i)
      {
        spawn<stackless>(
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

      for (std::size_t i=0; i<my_actor_size; ++i)
      {
        recv(base, 1);
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}

