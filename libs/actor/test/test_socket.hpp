///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/actor.hpp>
#include <gce/actor/detail/socket.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/spawn.hpp>
#include <gce/actor/remote.hpp>
#include <gce/actor/atom.hpp>
#include <gce/amsg/amsg.hpp>
#include <boost/atomic.hpp>

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
class socket_ut
{
public:
  static void run()
  {
    std::cout << "socket_ut begin." << std::endl;
    test_base();
    std::cout << "socket_ut end." << std::endl;
  }

public:
  static void test_base()
  {
    try
    {
      std::size_t client_num = 2;
      std::size_t echo_num = 10;
      context ctx;

      mixin& base = spawn(ctx);
      aid_t base_id = base.get_aid();
      spawn<stackful>(
        base,
        boost::bind(
          &socket_ut::echo_server, _1,
          base_id, client_num, echo_num
          )
        );

      recv(base, atom("bye"));
    }
    catch (std::exception& ex)
    {
      std::cerr << "test_base except: " << ex.what() << std::endl;
    }
  }

  static void echo_server(
    self_t self, aid_t base_id,
    std::size_t client_num, std::size_t echo_num
    )
  {
    try
    {
      bind(self, "tcp://127.0.0.1:14923");
      for (std::size_t i=0; i<client_num; ++i)
      {
        spawn<stackful>(
          self,
          boost::bind(
            &socket_ut::echo_client, _1, base_id, echo_num
            )
          );
      }

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

      send(self, base_id, atom("bye"));
    }
    catch (std::exception& ex)
    {
      std::cerr << "echo_server except: " << ex.what() << std::endl;
    }
  }

  static void echo_client(self_t self, aid_t base_id, std::size_t echo_num)
  {
    try
    {
      aid_t svr = connect(self, "tcp://127.0.0.1:14923");

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
