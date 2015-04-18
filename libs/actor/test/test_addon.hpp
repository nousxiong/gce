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
class addon_ut
{
public:
  static void run()
  {
    std::cout << "addon_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_base();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "addon_ut end." << std::endl;
  }

private:
  class echo_addon
    : public addon_t
  {
    typedef addon_t base_t;
  public:
    template <typename Actor>
    echo_addon(Actor& a)
      : addon_t(a)
    {
    }

    ~echo_addon()
    {
    }

  public:
    void echo(std::string const& str)
    {
      io_service_t& ios = base_t::get_strand().get_io_service();
      ios.post(boost::bind(&echo_addon::pri_echo, this, str));
    }

  private:
    void pri_echo(std::string const& str)
    {
      message m("echo");
      m << str;
      base_t::send2actor(m);
    }
  };

  static void my_actor(stackful_actor self)
  {
    log::logger_t& lg = self.get_context().get_logger();
    try
    {
      echo_addon ea(self);
      aid_t sender = self->match("init").recv();
      while (true)
      {
        match_t type;
        self->match("echo", "quit", type).recv();
        if (type == atom("quit"))
        {
          break;
        }

        ea.echo("hello world!");
        message msg;
        self->match("echo").raw(msg).recv();

        self.send(sender, msg);
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << ex.what();
    }
  }

  static void test_base()
  {
    log::asio_logger lgr;
    log::logger_t lg = boost::bind(&gce::log::asio_logger::output, &lgr, _arg1, "");

    try
    {
      size_t size = 10;
      attributes attrs;
      attrs.lg_ = lg;
      context ctx(attrs);
      threaded_actor base = spawn(ctx);
      echo_addon ea(base);
      std::vector<aid_t> echo_list(size);

      for (size_t i=0; i<size; ++i)
      {
        aid_t aid = spawn(base, boost::bind(&addon_ut::my_actor, _arg1), monitored);
        base->send(aid, "init");
        echo_list[i] = aid;
      }

      for (size_t n=0; n<10; ++n)
      {
        for (size_t i=0; i<size; ++i)
        {
          base->send(echo_list[i], "echo", "hi");
        }

        for (size_t i=0; i<size; ++i)
        {
          errcode_t ec;
          message msg;
          match_t type;
          std::string str;
          base->match("echo", type).guard(echo_list[i], ec).raw(msg).recv();
          GCE_VERIFY(!ec);
          GCE_VERIFY(type == atom("echo"));

          msg >> str;
          GCE_VERIFY(str == "hello world!");

          ea.echo(str);
          base->match("echo").raw(msg).recv();
        }
      }

      for (size_t i=0; i<size; ++i)
      {
        base->send(echo_list[i], "quit");
        base->recv(exit);
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << ex.what();
    }
  }
};
}
