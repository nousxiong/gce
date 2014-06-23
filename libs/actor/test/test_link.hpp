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

  static void my_actor_child(actor<stackful>& self)
  {
    aid_t aid = recv(self, 3);
  }

  static void my_actor(actor<stackful>& self)
  {
    detail::cache_pool* cac_pool = self.get_cache_pool();
    std::size_t size = 10;
    std::vector<response_t> res_list(size);
    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid =
        spawn(
          self,
          boost::bind(
            &link_ut::my_actor_child, _1
            ),
          linked
          );

      basic_actor* a =
        aid.get_actor_ptr(
          cac_pool->get_context().get_attributes().id_,
          cac_pool->get_context().get_timestamp()
          );
      sid_t sid = aid.sid_;
      sid += 100000;
      ++sid;

      message msg;

      self.link(aid_t(ctxid_nil, aid.timestamp_, a, sid));
      self.recv(msg);
      BOOST_ASSERT(msg.get_type() == exit);

      response_t res = request(self, aid_t(ctxid_nil, aid.timestamp_, a, sid));
      self.recv(res, msg);
      BOOST_ASSERT(msg.get_type() == exit);

      res_list[i] = request(self, aid, 3);
    }

    recv(self, exit);
  }

  static void my_thr(context& ctx)
  {
    actor<threaded> a = spawn(ctx);
    for (std::size_t i=0; i<100; ++i)
    {
      spawn(
        a,
        boost::bind(&link_ut::my_actor, _1),
        monitored
        );
    }

    for (std::size_t i=0; i<100; ++i)
    {
      recv(a);
    }
  }

  static void my_root(actor<stackful>& self)
  {
    for (std::size_t i=0; i<100; ++i)
    {
      spawn(
        self,
        boost::bind(&link_ut::my_actor, _1),
        monitored
        );
    }

    for (std::size_t i=0; i<100; ++i)
    {
      recv(self);
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
      actor<threaded> base = spawn(ctx);

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
          boost::bind(&link_ut::my_root, _1),
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
