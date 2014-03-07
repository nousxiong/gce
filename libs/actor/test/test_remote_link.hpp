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
class remote_link_ut
{
public:
  static void run()
  {
    std::cout << "remote_link_ut begin." << std::endl;
    test_base();
    std::cout << "remote_link_ut end." << std::endl;
  }

public:
  static void dummy(self_t)
  {
  }

  static void test_base()
  {
    try
    {
      std::size_t quiter_num = 100;

      attributes attrs;
      attrs.id_ = atom("ctx_one");
      context ctx1(attrs);
      attrs.id_ = atom("ctx_two");
      context ctx2(attrs);

      mixin_t base1 = spawn(ctx1);
      mixin_t base2 = spawn(ctx2);

      remote_func_list_t func_list;
      func_list.push_back(
        std::make_pair(
          atom("dummy"),
          boost::bind(&remote_link_ut::dummy, _1)
          )
        );
      gce::bind(base2, "tcp://127.0.0.1:14923", func_list);

      wait(base1, boost::chrono::milliseconds(100));
      net_option opt;
      opt.reconn_period_ = seconds_t(1);
      connect(base1, atom("ctx_two"), "tcp://127.0.0.1:14923", opt);

      std::vector<aid_t> quiter_list(quiter_num);
      for (std::size_t i=0; i<quiter_num; ++i)
      {
        quiter_list[i] =
          spawn(
            base2,
            boost::bind(
              &remote_link_ut::quiter, _1
              ),
            monitored
            );
        base1.link(quiter_list[i]);
      }

      for (std::size_t i=0; i<quiter_num; ++i)
      {
        send(base1, quiter_list[i]);
      }

      for (std::size_t i=0; i<quiter_num; ++i)
      {
        recv(base1);
        recv(base2);
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << "test_base except: " << ex.what() << std::endl;
    }
  }

  static void quiter(self_t self)
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
