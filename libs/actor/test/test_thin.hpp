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
class thin_ut
{
public:
  static void run()
  {
    std::cout << "thin_ut begin." << std::endl;
    test_common();
    std::cout << "thin_ut end." << std::endl;
  }

  struct stack
  {
    stack()
      : msg(1)
    {
    }

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
      GCE_YIELD self.recv(s.sender, s.msg);
      GCE_YIELD self.wait(seconds_t(3));
      self.reply(s.sender, s.msg);
    }
  }

  static void my_thin(thin_t self, aid_t base_id, stack& s)
  {
    GCE_REENTER(self)
    {
      s.size = 5;
      s.res_list.resize(s.size);
      for (std::size_t i=0; i<s.size; ++i)
      {
        aid_t aid = spawn<stackless>(
          self,
          boost::bind(&thin_ut::my_child, _1, stack())
          );
        message m;
        s.res_list[i] = self.request(aid, m);
      }

      for (s.i=0; s.i<s.size; ++s.i)
      {
        do
        {
          s.sender = aid_t();
          GCE_YIELD self.recv(
            s.res_list[s.i], s.sender, s.msg, seconds_t(1)
            );
        }
        while (!s.sender);
      }

      self.send(base_id, message(2));
    }
  }

  static void my_thr(context& ctx, aid_t base_id)
  {
    mixin_t mix = spawn(ctx);
    for (std::size_t i=0; i<2; ++i)
    {
      spawn<stackless>(
        mix,
        boost::bind(
          &thin_ut::my_thin, _1,
          base_id, stack()
          )
        );
    }
  }

  static void test_common()
  {
    try
    {
      std::size_t free_thin_num = 5;
      std::size_t user_thr_num = 5;
      std::size_t my_actor_size = free_thin_num + user_thr_num * 2;
      attributes attrs;
      attrs.mixin_num_ = user_thr_num + 1;
      context ctx(attrs);

      mixin_t base = spawn(ctx);
      aid_t base_id = base.get_aid();
      for (std::size_t i=0; i<free_thin_num; ++i)
      {
        spawn<stackless>(
          base,
          boost::bind(
            &thin_ut::my_thin,
            _1, base_id, stack()
            )
          );
      }

      boost::thread_group thrs;
      for (std::size_t i=0; i<user_thr_num; ++i)
      {
        thrs.create_thread(
          boost::bind(
            &thin_ut::my_thr,
            boost::ref(ctx), base_id
            )
          );
      }

      thrs.join_all();

      for (std::size_t i=0; i<my_actor_size; ++i)
      {
        recv(base);
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
