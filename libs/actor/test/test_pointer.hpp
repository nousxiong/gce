///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

namespace gce
{
class pointer_ut
{
public:
  static void run()
  {
    std::cout << "pointer_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_base();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "pointer_ut end." << std::endl;
  }

private:
  typedef boost::shared_ptr<boost::atomic_int> shared_integer;
  static void my_actor(stackful_actor self)
  {
    shared_integer shr_init;
    aid_t sender = self->match("init").recv(shr_init);
    ++*shr_init;
    self->send(sender, "ok", shr_init);
  }

  static void test_base()
  {
    log::asio_logger lgr;
    log::logger_t lg = boost::bind(&gce::log::asio_logger::output, &lgr, _arg1, "");

    try
    {
      int const shr_val = 11;
      size_t const size = 100;
      attributes attrs;
      attrs.lg_ = lg;
      context ctx(attrs);
      threaded_actor base = spawn(ctx);

      shared_integer shr_int = boost::make_shared<boost::atomic_int>(shr_val);
      for (size_t i=0; i<size; ++i)
      {
        aid_t aid = spawn(base, boost::bind(&pointer_ut::my_actor, _arg1), monitored);
        base->send(aid, "init", shr_int);
      }

      for (size_t i=0; i<size; ++i)
      {
        shared_integer shr_ok;
        base->match("ok").recv(shr_ok);
      }

      int const val = *shr_int;
      GCE_VERIFY(*shr_int == shr_val + size)(val);

      for (size_t i=0; i<size; ++i)
      {
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
