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
class match_recver_ut
{
public:
  static void run()
  {
    std::cout << "match_recver_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "match_recver_ut end." << std::endl;
  }

private:
  static void my_actor(stackful_actor self, aid_t base_id)
  {
    if (base_id)
    {
      self->send(base_id);
    }
  }

  static void test_common()
  {
    try
    {
      context ctx;
      threaded_actor base = spawn(ctx);

      aid_t base_id = base.get_aid();
      aid_t aid1 = spawn(base, boost::bind(&match_recver_ut::my_actor, _1, base_id), monitored);
      spawn(base, boost::bind(&match_recver_ut::my_actor, _1, aid_t()), monitored);

      message msg;
      aid_t sender = base.recv(msg, pattern("timeout", millisecs(1)));
      GCE_VERIFY(!sender);

      base->recv(match("not_catch", aid1));
      GCE_VERIFY(false);
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
