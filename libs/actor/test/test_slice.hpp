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
class slice_ut
{
public:
  class my_3d_engine
    : public boost::noncopyable
  {
  public:
    typedef boost::chrono::system_clock system_clock_t;
    typedef system_clock_t::time_point time_point_t;
    typedef boost::chrono::milliseconds milliseconds_t;
    typedef boost::chrono::milliseconds::rep fps_rep_t;

  public:
    my_3d_engine(context& ctx, int count_down, fps_rep_t fps = 60)
      : frame_(1000/fps)
      , base_(spawn(ctx))
      , cln_(spawn(base_))
      , stopped_(false)
    {
      aid_t counter = spawn(base_, boost::bind(&slice_ut::cd, _arg1));
      cln_->send(counter, "cd", count_down);
    }

    ~my_3d_engine()
    {
    }

  public:
    void run()
    {
      milliseconds_t last_dt = frame_;
      while (!stopped_)
      {
        time_point_t begin_tp = system_clock_t::now();
        update(last_dt);
        last_dt =
          boost::chrono::duration_cast<milliseconds_t>(
            system_clock_t::now() - begin_tp
            );
        if (last_dt < frame_)
        {
          boost::this_thread::sleep_for(frame_ - last_dt);
          last_dt = frame_;
        }
      }
    }

  private:
    void update(milliseconds_t /*dt*/)
    {
      int count_down;
      aid_t counter = cln_->recv("cd", count_down);
      if (counter != aid_nil)
      {
        if (count_down > 0)
        {
          cln_->send(counter, "cd", count_down);
        }
        else
        {
          stopped_ = true;
        }
      }
    }

  private:
    milliseconds_t const frame_;
    threaded_actor base_;
    nonblocked_actor cln_;
    bool stopped_;
  };

  static void cd(stackful_actor self)
  {
    try
    {
      int count_down;
      while (true)
      {
        aid_t cln = self->recv("cd", count_down);
        --count_down;
        self->send(cln, "cd", count_down);
        if (count_down <= 0)
        {
          break;
        }
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }

public:
  static void run()
  {
    std::cout << "slice_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_base();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "slice_ut end." << std::endl;
  }

public:
  static void test_base()
  {
    try
    {
      context ctx;
      my_3d_engine engine(ctx, 20);
      engine.run();
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
