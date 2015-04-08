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
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "match_ut end." << std::endl;
  }

  static void my_actor_child(stackful_actor self)
  {
    aid_t aid = self->recv(3);
    self->reply(aid);
  }

  static void my_actor(stackful_actor self, aid_t base_id)
  {
    std::size_t size = 1;
    std::vector<resp_t> res_list(size);
    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid = spawn(self, boost::bind(&match_ut::my_actor_child, _arg1));
      res_list[i] = self->request(aid, 3);
    }

    for (std::size_t i=0; i<size; ++i)
    {
      self->respond(res_list[i]);
    }
    self->send(base_id, 1);
  }

  static void my_thr(context& ctx, aid_t base_id)
  {
    threaded_actor a = spawn(ctx);
    for (std::size_t i=0; i<2; ++i)
    {
      spawn(a, boost::bind(&match_ut::my_actor, _arg1, base_id));
    }
  }

  static void test_common()
  {
    try
    {
      std::size_t user_thr_num = 5;
      std::size_t my_actor_size = 10;
      context ctx;
      threaded_actor base = spawn(ctx);

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
        base->recv(1);
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}

