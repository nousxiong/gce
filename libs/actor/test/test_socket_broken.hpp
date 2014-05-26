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
    test_base();
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

      actor<threaded> a = spawn(ctx);
      gce::bind(ctx, "tcp://127.0.0.1:14923");

      std::vector<aid_t> quiter_list(quiter_num);
      boost::thread thr(
        boost::bind(
          &socket_broken_ut::two,
          boost::ref(quiter_list),
          ctx.get_aid(), boost::ref(a)
          )
        );

      recv(ctx);
      BOOST_FOREACH(aid_t const& aid, quiter_list)
      {
        ctx.link(aid);
      }

      thr.join();

      for (std::size_t i=0; i<quiter_num; ++i)
      {
        recv(ctx);
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << "test_base except: " << ex.what() << std::endl;
    }
  }

  static void two(
    std::vector<aid_t>& quiter_list,
    aid_t base_id, actor<threaded> a
    )
  {
    try
    {
      attributes attrs;
      attrs.id_ = atom("two");
      context ctx(attrs);

      wait(ctx, boost::chrono::milliseconds(100));
      net_option opt;
      opt.reconn_period_ = seconds_t(1);
      connect(ctx, atom("one"), "tcp://127.0.0.1:14923", false, opt);

      BOOST_FOREACH(aid_t& aid, quiter_list)
      {
        aid =
          spawn(
            ctx,
            boost::bind(
              &socket_broken_ut::quiter, _1
              ),
            monitored
            );
      }

      send(a, base_id, atom("notify"));
      BOOST_FOREACH(aid_t const& aid, quiter_list)
      {
        send(ctx, aid, atom("quit"));
      }

      for (std::size_t i=0; i<quiter_list.size(); ++i)
      {
        recv(ctx);
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << "two except: " << ex.what() << std::endl;
    }
  }

  static void quiter(actor<stackful>& self)
  {
    try
    {
      recv(self);
    }
    catch (std::exception& ex)
    {
      std::cerr << "quiter except: " << ex.what() << std::endl;
    }
  }
};
}
