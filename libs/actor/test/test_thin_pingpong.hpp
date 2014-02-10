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
class thin_pingpong_ut
{
static std::size_t const msg_size = 100000;

public:
  static void run()
  {
    std::cout << "thin_pingpong_ut begin." << std::endl;
    test();
    std::cout << "thin_pingpong_ut end." << std::endl;
  }

  struct stack
  {
    stack()
      : msg(1)
    {
    }

    gce::aid_t sender;
    message msg;
    std::size_t i;
    std::size_t size;
  };

  static void my_child(thin_t self, aid_t base_id, stack& s)
  {
    GCE_REENTER(self)
    {
      while (true)
      {
        GCE_YIELD self.recv(s.sender, s.msg);
        if (s.msg.get_type() == 2)
        {
          break;
        }
        else
        {
          self.send(s.sender, s.msg);
        }
      }
      self.send(base_id, s.msg);
    }
  }

  static void my_thin(thin_t self, aid_t base_id, stack& s)
  {
    GCE_REENTER(self)
    {
      s.sender = spawn<stackless>(
        self,
        boost::bind(
          &thin_pingpong_ut::my_child, _1,
          base_id, stack()
          )
        );

      for (s.i=0; s.i<msg_size; ++s.i)
      {
        s.msg = message(1);
        self.send(s.sender, s.msg);
        GCE_YIELD self.recv(s.sender, s.msg);
      }
      s.msg = message(2);
      self.send(s.sender, s.msg);
    }
  }

  static void test()
  {
    try
    {
      context ctx;

      mixin_t base = spawn(ctx);
      aid_t base_id = base.get_aid();
      aid_t aid =
        spawn<stackless>(
          base,
          boost::bind(
            &thin_pingpong_ut::my_thin, _1,
            base_id, stack()
            )
          );

      boost::timer::auto_cpu_timer t;
      recv(base);
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}

