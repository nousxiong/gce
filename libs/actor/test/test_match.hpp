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

  static void my_actor_child(self_t self)
  {
    aid_t aid = recv(self, 3);
    reply(self, aid);
  }

  static void my_actor(self_t self, aid_t base_id)
  {
    std::size_t size = 1;
    std::vector<response_t> res_list(size);
    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid = spawn(self, boost::bind(&match_ut::my_actor_child, _1));
      res_list[i] = request(self, aid, 3);
    }

    for (std::size_t i=0; i<size; ++i)
    {
      recv(self, res_list[i]);
    }
    send(self, base_id, 1);
  }

  static void my_thr(context& ctx, aid_t base_id)
  {
    mixin_t mix = spawn(ctx);
    for (std::size_t i=0; i<2; ++i)
    {
      spawn(mix, boost::bind(&match_ut::my_actor, _1, base_id));
    }
  }

  static void test_common()
  {
    try
    {
      std::size_t user_thr_num = 5;
      std::size_t my_actor_size = 10;
      attributes attrs;
      attrs.mixin_num_ = user_thr_num + 1;
      context ctx(attrs);

      mixin_t base = spawn(ctx);
      aid_t base_id = base.get_aid();

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

