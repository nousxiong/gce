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
    if (base_id != aid_nil)
    {
      int i = 0;
      self->send(base_id, "catch", i);
      ++i;
      self->send(base_id, "catch", i);

      aid_t sender = self->match("resp").recv();
      GCE_VERIFY(sender == base_id);
      self->send(sender, exit);

      ++i;
      self->send(base_id, "catch", i);
    }
  }

  static void test_common()
  {
    try
    {
      context ctx;
      threaded_actor base = spawn(ctx);

      aid_t base_id = base.get_aid();
      aid_t aid1 = spawn(base, boost::bind(&match_recver_ut::my_actor, _arg1, base_id), monitored);
      spawn(base, boost::bind(&match_recver_ut::my_actor, _arg1, aid_t()), monitored);

      message msg;
      pattern patt;
      patt.add_match("timeout");
      patt.timeout_ = millisecs(1);
      aid_t sender = base.recv(msg, patt);
      GCE_VERIFY(sender == aid_nil);

      int i = -1;
      base->recv("catch", guard(aid1), i);
      GCE_VERIFY(i == 0);
      std::cout << "catch " << i << std::endl;
      base->match("catch").guard(aid1).recv(i);
      GCE_VERIFY(i == 1);
      std::cout << "catch " << i << std::endl;

      resp_t res = base->request(aid1, "resp", i);
      errcode_t ec;
      base->match(res).guard(ec).raw(msg).respond(i);
      GCE_VERIFY(ec);
      GCE_VERIFY(msg.get_type() == exit);

      base->match("not_catch").guard(aid1, ec).recv(i);
      GCE_VERIFY(ec);
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
