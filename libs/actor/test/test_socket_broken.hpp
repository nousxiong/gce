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
class socket_broken_ut
{
public:
  static void run()
  {
    std::cout << "socket_broken_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_base();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "socket_broken_ut end." << std::endl;
  }

public:
  static void test_base()
  {
    try
    {
      std::size_t quiter_num = 100;

      attributes attrs;
      attrs.id_ = atom("one");
      context ctx(attrs);
      threaded_actor base = spawn(ctx);

      threaded_actor a = spawn(ctx);
      gce::bind(base, "tcp://127.0.0.1:14923");

      std::vector<aid_t> quiter_list(quiter_num);
      boost::thread thr(
        boost::bind(
          &socket_broken_ut::two,
          boost::ref(quiter_list),
          base.get_aid(), boost::ref(a)
          )
        );

      base->recv();
      BOOST_FOREACH(aid_t const& aid, quiter_list)
      {
        base.link(aid);
      }

      thr.join();

      for (std::size_t i=0; i<quiter_num; ++i)
      {
        base->recv();
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << "test_base except: " << ex.what() << std::endl;
    }
  }

  static void two(
    std::vector<aid_t>& quiter_list,
    aid_t base_id, threaded_actor a
    )
  {
    try
    {
      attributes attrs;
      attrs.id_ = atom("two");
      context ctx(attrs);
      threaded_actor base = spawn(ctx);

      base.sleep_for(millisecs(100));
      netopt_t opt = make_netopt();
      opt.reconn_period = seconds(1);
      connect(base, "one", "tcp://127.0.0.1:14923", opt);

      BOOST_FOREACH(aid_t& aid, quiter_list)
      {
        aid =
          spawn(
            base,
            boost::bind(
              &socket_broken_ut::quiter, _arg1
              ),
            monitored
            );
      }

      a->send(base_id, "notify");
      BOOST_FOREACH(aid_t const& aid, quiter_list)
      {
        base->send(aid, "quit");
      }

      for (std::size_t i=0; i<quiter_list.size(); ++i)
      {
        base->recv();
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << "two except: " << ex.what() << std::endl;
    }
  }

  static void quiter(stackful_actor self)
  {
    try
    {
      self->recv();
    }
    catch (std::exception& ex)
    {
      std::cerr << "quiter except: " << ex.what() << std::endl;
    }
  }
};
}
