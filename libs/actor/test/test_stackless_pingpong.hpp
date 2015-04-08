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
    for (std::size_t i=0; i<test_count; ++i)
    {
      test();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
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
    void run(stackless_actor self)
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
    void run(stackless_actor self)
    {
      GCE_REENTER (self)
      {
        GCE_YIELD spawn(
          self,
          boost::bind(
            &my_child::run, 
            boost::make_shared<my_child>(base_id_), 
            _arg1
            ),
          aid_
          );

        for (i_=0, size_=msg_size/100; i_<size_; ++i_)
        {
          for (j_=0; j_<100; ++j_)
          {
            self.send(aid_, msg_);
          }
          for (j_=0; j_<100; ++j_)
          {
            GCE_YIELD self.recv(aid_, msg_);
          }
        }

        self->send(aid_, 2);
      }
    }

  private:
    aid_t base_id_;
    aid_t aid_;

    message msg_;
    std::size_t i_;
    std::size_t j_;
    std::size_t size_;
  };

  static void test()
  {
    try
    {
      context ctx;
      threaded_actor base = spawn(ctx);

      aid_t base_id = base.get_aid();
      aid_t aid =
        spawn<stackless>(
          base,
          boost::bind(
            &my_actor::run, 
            boost::make_shared<my_actor>(base_id), 
            _arg1
            )
          );

      boost::timer::auto_cpu_timer t;
      base->recv();
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}

