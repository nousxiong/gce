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
    for (std::size_t i=0; i<300; ++i)
    {
      test_base();
    }
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

      message msg;
      match mat;
      mat.match_list_.push_back(atom("bye"));
      base.recv(msg, mat);

      std::cout << "test_base end.\n";
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
      bind(self, "tcp://127.0.0.1:10000");
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

//      message msg;
//      self.recv(msg);
      self.send(base_id, message(atom("bye")));
      std::cout << "echo_server end.\n";
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
      aid_t svr = connect(self, "tcp://127.0.0.1:10000");

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

        if (msg.get_type() == exit_neterr)
        {
          std::cout << "echo_client exit_neterr\n";
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

      self.send(svr, message(atom("end")));
      std::cout << "echo_client end\n";
    }
    catch (std::exception& ex)
    {
      std::cerr << "echo_client except: " << ex.what() << std::endl;
    }
  }
};
}
