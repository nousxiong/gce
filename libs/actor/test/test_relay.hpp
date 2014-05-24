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
class relay_ut
{
public:
  static void run()
  {
    std::cout << "relay_ut begin." << std::endl;
    test_common();
    std::cout << "relay_ut end." << std::endl;
  }

private:
  static void my_actor(actor<stacked>& self, aid_t last_id)
  {
    message msg;
    aid_t sender = self.recv(msg);
    if (last_id)
    {
      self.relay(last_id, msg);
    }
    else
    {
      message m(atom("hello"));
      self.reply(sender, m);
    }
  }

  static void root(actor<stacked>& self)
  {
    std::size_t free_actor_num = 20;

    aid_t last_id;
    aid_t first_id;
    for (std::size_t i=0; i<free_actor_num; ++i)
    {
      aid_t aid =
        spawn(
          self,
          boost::bind(
            &relay_ut::my_actor, _1, last_id
            )
          );
      if (i == 0)
      {
        first_id = aid;
      }
      last_id = aid;
    }

    int i = 0;
    response_t res = request(self, last_id, atom("hi"), i);
    message msg;
    aid_t sender = self.recv(res, msg);
    BOOST_ASSERT(sender == first_id);
    BOOST_ASSERT(msg.get_type() == atom("hello"));
  }

  static void test_common()
  {
    try
    {
      std::size_t root_num = 100;
      context ctx;

      for (std::size_t i=0; i<root_num; ++i)
      {
        spawn(
          ctx,
          boost::bind(
            &relay_ut::root, _1
            ),
          monitored
          );
      }

      for (std::size_t i=0; i<root_num; ++i)
      {
        recv(ctx);
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
