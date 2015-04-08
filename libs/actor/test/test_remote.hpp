///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "echo.adl.h"

namespace gce
{
typedef adl::echo_data echo_data;
}
GCE_PACK(gce::echo_data, (v.hi_)(v.i_&sfix));

namespace gce
{
class remote_ut
{
public:
  static void run()
  {
    std::cout << "remote_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_base();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "remote_ut end." << std::endl;
  }

public:
  static void test_base()
  {
    try
    {
      std::size_t client_num = 10;
      std::size_t echo_num = 10;

      attributes attrs;
      attrs.id_ = atom("server");
      context ctx_svr(attrs);
      attrs.id_ = atom("client");
      context ctx_cln(attrs);

      threaded_actor base_cln = spawn(ctx_cln);
      threaded_actor base_svr = spawn(ctx_svr);

      remote_func_list_t func_list;
      func_list.push_back(
        make_remote_func<stackful>(
          "echo_client", 
          boost::bind(&remote_ut::echo_client, _arg1)
          )
        );
      gce::bind(base_cln, "tcp://127.0.0.1:14923", func_list);
      netopt_t opt = make_netopt();
      opt.reconn_period = seconds(1);
      connect(base_svr, "client", "tcp://127.0.0.1:14923", opt);

      aid_t svr =
        spawn(
          base_svr,
          boost::bind(
            &remote_ut::echo_server, _arg1, client_num
            ),
          monitored
          );

      for (std::size_t i=0; i<client_num; ++i)
      {
        aid_t cln = spawn_remote(base_svr, "echo_client", "client");
        base_svr->send(cln, "init", svr, echo_num);
      }

      base_svr->recv();
    }
    catch (std::exception& ex)
    {
      std::cerr << "test_base except: " << ex.what() << std::endl;
    }
  }

  static void echo_server(stackful_actor self, std::size_t client_num)
  {
    try
    {
      while (true)
      {
        message msg;
        aid_t cln = self.recv(msg);
        match_t type = msg.get_type();
        if (type == atom("echo"))
        {
          self.send(cln, msg);
        }
        else if (type == atom("end"))
        {
          if (--client_num == 0)
          {
            break;
          }
        }
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << "echo_server except: " << ex.what() << std::endl;
    }
  }

  static void echo_client(stackful_actor self)
  {
    try
    {
      aid_t svr;
      std::size_t echo_num;
      self->recv("init", svr, echo_num);

      echo_data d;
      d.hi_ = "hello";
      d.i_ = 1;

      message m("echo");
      m << d << "tag" << int(2);

      for (std::size_t i=0; i<echo_num; ++i)
      {
        self.send(svr, m);

        message msg;
        self.recv(msg);

        if (msg.get_type() == exit)
        {
          exit_code_t exc;
          std::string exit_msg;
          msg >> exc >> exit_msg;
          std::cout << exit_msg << std::endl;
          return;
        }

        echo_data ret;
        int it;
        std::string tag;
        BOOST_ASSERT(msg.get_type() == m.get_type());
        msg >> ret >> tag >> it;
        BOOST_ASSERT(d.hi_ == ret.hi_);
        BOOST_ASSERT(d.i_ == ret.i_);
        BOOST_ASSERT(tag == "tag");
        BOOST_ASSERT(it == 2);
      }

      self->send(svr, "end");
    }
    catch (std::exception& ex)
    {
      std::cerr << "echo_client except: " << ex.what() << std::endl;
    }
  }
};
}

