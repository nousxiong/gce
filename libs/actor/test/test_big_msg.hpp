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
class big_msg_ut
{
public:
  static void run()
  {
    std::cout << "big_msg_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_base();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "big_msg_ut end." << std::endl;
  }

public:
  static void test_base()
  {
    try
    {
      gce::log::asio_logger lg;
      std::size_t echo_num = 100;

      attributes attrs;
      attrs.lg_ = boost::bind(&gce::log::asio_logger::output, &lg, _arg1, "");
      attrs.id_ = atom("one");
      context ctx1(attrs);
      attrs.id_ = atom("two");
      context ctx2(attrs);
      
      threaded_actor base1 = spawn(ctx1);
      threaded_actor base2 = spawn(ctx2);

      gce::bind(base2, "tcp://127.0.0.1:14923");

      aid_t echo_aid =
        spawn(
          base2,
          boost::bind(
            &big_msg_ut::echo, _arg1
            ),
          monitored
          );

      netopt_t opt = make_netopt();
      opt.reconn_period = seconds(1);
      connect(base1, "two", "tcp://127.0.0.1:14923", opt);

      std::string big_msg(GCE_SOCKET_RECV_BUFFER_MIN_SIZE, 'c');
      uint64_t u64 = 3233332211;

      boost::timer::auto_cpu_timer t;
      size_t const max_times = 5;
      for (std::size_t i=0, n=0; i<echo_num; ++i, ++n)
      {
        if (n == max_times)
        {
          n = 0;
        }
        big_msg.resize(GCE_SOCKET_RECV_BUFFER_MIN_SIZE * (n+2));
        base1->send(echo_aid, "echo", u64, big_msg);
        base1->recv("echo");
      }
      base1->send(echo_aid, "end");

      base2->recv();
    }
    catch (std::exception& ex)
    {
      std::cerr << "test_base except: " << ex.what() << std::endl;
    }
  }

  static void echo(stackful_actor self)
  {
    try
    {
      std::string big_msg;
      uint64_t u64 = 0;
      while (true)
      {
        message msg;
        aid_t sender = self.recv(msg);
        match_t type = msg.get_type();
        if (type == atom("echo"))
        {
          msg >> u64;
          GCE_VERIFY(u64 == 3233332211)(u64);
          self.send(sender, msg);
        }
        else
        {
          break;
        }
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << "echo except: " << ex.what() << std::endl;
    }
  }
};
}
