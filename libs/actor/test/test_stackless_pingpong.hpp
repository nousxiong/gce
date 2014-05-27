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
class stackless_pingpong_ut
{
static std::size_t const msg_size = 100000;
public:
  static void run()
  {
    std::cout << "stackless_pingpong_ut begin." << std::endl;
    test();
    std::cout << "stackless_pingpong_ut end." << std::endl;
  }

private:
  class my_child
    : public boost::enable_shared_from_this<my_child>
  {
  public:
    explicit my_child(aid_t base_id)
      : base_id_(base_id)
    {
    }

    ~my_child()
    {
    }

  public:
    void run(actor<stackless>& self)
    {
      GCE_REENTER (self)
      {
        while (true)
        {
          GCE_YIELD self.recv(sire_, msg_);
          if (msg_.get_type() == 2)
          {
            break;
          }
          else
          {
            self.send(sire_, msg_);
          }
        }
        self.send(base_id_, msg_);
      }
    }

  private:
    aid_t base_id_;
    aid_t sire_;
    message msg_;
  };

  class my_actor
    : public boost::enable_shared_from_this<my_actor>
  {
  public:
    explicit my_actor(aid_t base_id)
      : base_id_(base_id)
      , msg_(1)
    {
    }

    ~my_actor()
    {
    }

  public:
    void run(actor<stackless>& self)
    {
      GCE_REENTER (self)
      {
        GCE_YIELD spawn(
          self,
          boost::bind(
            &my_child::run, 
            boost::make_shared<my_child>(base_id_), 
            _1
            ),
          aid_
          );

        for (i_=0; i_<msg_size; ++i_)
        {
          self.send(aid_, msg_);
          GCE_YIELD self.recv(aid_, msg_);
        }

        send(self, aid_, 2);
      }
    }

  private:
    aid_t base_id_;
    aid_t aid_;

    message msg_;
    std::size_t i_;
  };

  static void test()
  {
    try
    {
      context ctx;

      aid_t base_id = ctx.get_aid();
      aid_t aid =
        spawn<stackless>(
          ctx,
          boost::bind(
            &my_actor::run, 
            boost::make_shared<my_actor>(base_id), 
            _1
            )
          );

      boost::timer::auto_cpu_timer t;
      recv(ctx);
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}

