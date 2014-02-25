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
class link_ut
{
public:
  static void run()
  {
    std::cout << "link_ut begin." << std::endl;
    test_common();
    std::cout << "link_ut end." << std::endl;
  }

  struct stack
  {
    gce::aid_t sender;
    gce::aid_t aid;
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
    }
  }

  static void my_actor_child(self_t self)
  {
    aid_t aid = recv(self, 3);
  }

  static void my_thin(thin_t self, aid_t base_id, stack& s)
  {
    GCE_REENTER(self)
    {
      s.size = 1;
      s.res_list.resize(s.size);
      for (s.i=0; s.i<s.size; ++s.i)
      {
        s.aid =
          spawn<stackless>(
            self,
            boost::bind(&link_ut::my_child, _1, stack()),
            linked
            );

        {
          basic_actor* a = s.aid.get_actor_ptr();
          sid_t sid = s.aid.get_sid();
          sid += 100000;
          ++sid;
          self.link(aid_t(a, sid));
        }
        GCE_YIELD self.recv(s.sender, s.msg);
        BOOST_ASSERT(s.msg.get_type() == exit);

        {
          basic_actor* a = s.aid.get_actor_ptr();
          sid_t sid = s.aid.get_sid();
          sid += 100000;
          ++sid;
          s.res = self.request(aid_t(a, sid), message());
        }
        GCE_YIELD self.recv(s.res, s.sender, s.msg);
        BOOST_ASSERT(s.msg.get_type() == exit);

        message m(5);
        s.res_list[s.i] = self.request(s.aid, m);
      }

      GCE_YIELD
      {
        match mach;
        mach.match_list_.push_back(exit);
        self.recv(s.sender, s.msg, mach);
      }
    }
  }

  static void my_actor(self_t self, aid_t base_id)
  {
    std::size_t size = 1;
    std::vector<response_t> res_list(size);
    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid =
        spawn<stackful>(
          self,
          boost::bind(
            &link_ut::my_actor_child, _1
            ),
          linked
          );

      basic_actor* a = aid.get_actor_ptr();
      sid_t sid = aid.get_sid();
      sid += 100000;
      ++sid;

      message msg;

      self.link(aid_t(a, sid));
      self.recv(msg);
      BOOST_ASSERT(msg.get_type() == exit);

      response_t res = request(self, aid_t(a, sid));
      self.recv(res, msg);
      BOOST_ASSERT(msg.get_type() == exit);

      res_list[i] = request(self, aid, 3);
    }

    recv(self, exit);
  }

  static void my_thr(context& ctx, aid_t base_id)
  {
    mixin_t mix = spawn(ctx);
    for (std::size_t i=0; i<2; ++i)
    {
      spawn<stackful>(mix, boost::bind(&link_ut::my_actor, _1, base_id), monitored);
      spawn<stackless>(mix, boost::bind(&link_ut::my_thin, _1, base_id, stack()), monitored);
    }

    for (std::size_t i=0; i<4; ++i)
    {
      recv(mix, exit);
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

      for (std::size_t i=0; i<1; ++i)
      {
        recv(base, exit);
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
