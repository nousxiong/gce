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
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "link_ut end." << std::endl;
  }

  static void my_actor_child(stackful_actor self)
  {
    aid_t aid = self->recv(3);
  }

  static void my_actor(stackful_actor self)
  {
    std::size_t size = 10;
    std::vector<resp_t> res_list(size);
    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid =
        spawn(
          self,
          boost::bind(
            &link_ut::my_actor_child, _arg1
            ),
          linked
          );

      aid_t tmp = aid;
      tmp.sid_ += 100000;
      ++tmp.sid_;

      message msg;

      self.link(tmp);
      self.recv(msg);
      BOOST_ASSERT(msg.get_type() == exit);

      resp_t res = self->request(tmp);
      self.respond(res, msg);
      BOOST_ASSERT(msg.get_type() == exit);

      res_list[i] = self->request(aid, 3);
    }

    self->recv(exit);
  }

  static void my_thr(context& ctx)
  {
    threaded_actor a = spawn(ctx);
    for (std::size_t i=0; i<100; ++i)
    {
      spawn(
        a,
        boost::bind(&link_ut::my_actor, _arg1),
        monitored
        );
    }

    for (std::size_t i=0; i<100; ++i)
    {
      a->recv();
    }
  }

  static void my_root(stackful_actor self)
  {
    for (std::size_t i=0; i<100; ++i)
    {
      spawn(
        self,
        boost::bind(&link_ut::my_actor, _arg1),
        monitored
        );
    }

    for (std::size_t i=0; i<100; ++i)
    {
      self->recv();
    }
  }

  static void test_common()
  {
    try
    {
      std::size_t user_thr_num = 5;
      //std::size_t my_actor_size = 21;
      attributes attrs;
      context ctx(attrs);
      threaded_actor base = spawn(ctx);

      boost::thread_group thrs;
      for (std::size_t i=0; i<user_thr_num; ++i)
      {
        thrs.create_thread(
          boost::bind(
            &link_ut::my_thr,
            boost::ref(ctx)
            )
          );
        /*spawn(
          base,
          boost::bind(&link_ut::my_root, _arg1),
          monitored
          );*/
      }

      /*for (std::size_t i=0; i<user_thr_num; ++i)
      {
        recv(base);
      }*/

      thrs.join_all();
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
