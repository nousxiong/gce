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
struct echo_data
{
  std::string hi_;
  int i_;
};
}
GCE_PACK(gce::echo_data, (hi_)(i_&sfix));

namespace gce
{
class remote_ut
{
public:
  static void run()
  {
    std::cout << "remote_ut begin." << std::endl;
    test_base();
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

      actor<threaded> base_cln = spawn(ctx_cln);
      actor<threaded> base_svr = spawn(ctx_svr);

      remote_func_list_t func_list;
      func_list.push_back(
        std::make_pair(
          atom("echo_client"),
          make_actor_func<stackful>(
            boost::bind(&remote_ut::echo_client, _1)
            )
          )
        );
      gce::bind(base_cln, "tcp://127.0.0.1:14923", false, func_list);
      net_option opt;
      opt.reconn_period_ = seconds_t(1);
      connect(base_svr, atom("client"), "tcp://127.0.0.1:14923", false, opt);

      aid_t svr =
        spawn(
          base_svr,
          boost::bind(
            &remote_ut::echo_server, _1, client_num
            ),
          monitored
          );

      for (std::size_t i=0; i<client_num; ++i)
      {
        aid_t cln =
          spawn(
            base_svr,
            "echo_client",
            atom("client")
            );
        send(base_svr, cln, atom("init"), svr, echo_num);
      }

      recv(base_svr);
    }
    catch (std::exception& ex)
    {
      std::cerr << "test_base except: " << ex.what() << std::endl;
    }
  }

  static void echo_server(actor<stackful>& self, std::size_t client_num)
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

  static void echo_client(actor<stackful>& self)
  {
    try
    {
      aid_t svr;
      std::size_t echo_num;
      recv(self, atom("init"), svr, echo_num);

      echo_data d;
      d.hi_ = "hello";
      d.i_ = 1;

      message m(atom("echo"));
      m << d << std::string("tag") << int(2);

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

      send(self, svr, atom("end"));
    }
    catch (std::exception& ex)
    {
      std::cerr << "echo_client except: " << ex.what() << std::endl;
    }
  }
};
}

