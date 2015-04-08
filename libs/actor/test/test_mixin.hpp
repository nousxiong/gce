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
class mixin_ut
{
public:
  static void run()
  {
    std::cout << "mixin_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "mixin_ut end." << std::endl;
  }

private:
  static void my_child(stackful_actor self)
  {
    aid_t aid = self->recv();
    self->reply(aid);
  }

  static void my_thr(context& ctx)
  {
    threaded_actor a = spawn(ctx);
    std::size_t size = 10;
    std::vector<resp_t> res_list(size);
    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid =
        spawn(
          a,
          boost::bind(&mixin_ut::my_child, _arg1)
          );
      res_list[i] = a->request(aid);
    }

    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid;
      do
      {
        aid = a->respond(res_list[i], seconds(1));
      }
      while (aid == aid_nil);
    }
  }

  static void test_common()
  {
    try
    {
      std::size_t user_thr_num = 5;
      attributes attrs;
      context ctx(attrs);

      boost::thread_group thrs;
      for (std::size_t i=0; i<user_thr_num; ++i)
      {
        thrs.create_thread(
          boost::bind(
            &mixin_ut::my_thr,
            boost::ref(ctx)
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

