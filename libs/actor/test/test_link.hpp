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

  static void my_actor_child(self_t self)
  {
    aid_t aid = recv(self, 3);
  }

  static void my_actor(self_t self, aid_t base_id)
  {
    detail::cache_pool* cac_pool = self.get_cache_pool();
    std::size_t size = 1;
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
          cac_pool->get_ctxid(),
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

  static void my_thr(context& ctx, aid_t base_id)
  {
    mixin_t mix = spawn(ctx);
    for (std::size_t i=0; i<2; ++i)
    {
      spawn(
        mix,
        boost::bind(&link_ut::my_actor, _1, base_id),
        monitored
        );
    }

    for (std::size_t i=0; i<2; ++i)
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
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
