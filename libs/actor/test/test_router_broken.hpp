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
class router_broken_ut
{
public:
  static void run()
  {
    std::cout << "router_broken_ut begin." << std::endl;
    test_base();
    std::cout << "router_broken_ut end." << std::endl;
  }

public:
  static void test_base()
  {
    try
    {
      std::size_t quiter_num = 100;

      attributes attrs;
      attrs.id_ = atom("one");
      context ctx1(attrs);
      attrs.id_ = atom("two");
      context ctx2(attrs);

      actor<threaded> a = spawn(ctx1);

      boost::thread thr(
        boost::bind(
          &router_broken_ut::router,
          ctx1.get_aid(), boost::ref(a)
          )
        );
      aid_t mix_id = recv(ctx1);

      net_option opt;
      opt.reconn_period_ = seconds_t(1);
      connect(ctx1, atom("router"), "tcp://127.0.0.1:14923", true, opt);
      connect(ctx2, atom("router"), "tcp://127.0.0.1:14923", true, opt);
      wait(ctx2, boost::chrono::milliseconds(100));

      std::vector<aid_t> quiter_list(quiter_num);
      for (std::size_t i=0; i<quiter_num; ++i)
      {
        quiter_list[i] =
          spawn(
            ctx2,
            boost::bind(
              &router_broken_ut::quiter, _1
              ),
            monitored
            );
        ctx1.link(quiter_list[i]);
      }
      send(ctx1, mix_id);

      thr.join();

      for (std::size_t i=0; i<quiter_num; ++i)
      {
        send(ctx2, quiter_list[i]);
      }

      for (std::size_t i=0; i<quiter_num; ++i)
      {
        recv(ctx1);
        recv(ctx2);
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << "test_base except: " << ex.what() << std::endl;
    }
  }

  static void router(aid_t base_id, actor<threaded> mix)
  {
    try
    {
      attributes attrs;
      attrs.id_ = atom("router");
      context ctx(attrs);

      gce::bind(ctx, "tcp://127.0.0.1:14923", true);
      send(mix, base_id);
      recv(mix);
    }
    catch (std::exception& ex)
    {
      std::cerr << "router except: " << ex.what() << std::endl;
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
