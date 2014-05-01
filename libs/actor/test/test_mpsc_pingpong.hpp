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
class mpsc_pingpong_ut
{
static std::size_t const msg_size = 10000000;
static int const max_counter = 10000;
static std::size_t const cache_free_size = 1000;
struct data 
  : public detail::object_pool<data>::object
  , public detail::mpsc_queue<data>::node
{
  data()
    : i_(0)
  {
  }

  int i_;
};

public:
  static void run()
  {
    std::cout << "mpsc_pingpong_ut begin." << std::endl;
    test();
    std::cout << "mpsc_pingpong_ut end." << std::endl;
  }

private:
  static void handle_data(
    data* d, 
    detail::mpsc_queue<data>& que,
    detail::object_pool<data>& pool, 
    bool& quit
    )
  {
    ++d->i_;
    if (d->i_ >= msg_size)
    {
      quit = true;
    }
    data* o = pool.get();
    *o = *d;
    //free_que.push(d);
    que.push(o);
  }

  static void free_data(data* o, detail::object_pool<data>& pool)
  {
    while (o)
    {
      data* next = detail::node_access::get_next(o);
      detail::node_access::set_next(o, (data*)0);
      pool.free(o);
      o = next;
    }
  }

  static void pong(
    detail::mpsc_queue<data>& ping_que, detail::mpsc_queue<data>& pong_que,
    detail::object_pool<data>& pong_pool, detail::mpsc_queue<data>& ping_free_que, 
    detail::mpsc_queue<data>& pong_free_que, 
    io_service_t& ios
    )
  {
    bool quit = false;
    int counter = max_counter;
    while (!quit)
    {
      data* d = pong_que.pop_all();
      if (d)
      {
        detail::scope scp(
          boost::bind(
            &detail::mpsc_queue<data>::push,
            &ping_free_que, d
            )
          );
        /*ios.dispatch(
          boost::bind(
            &mpsc_pingpong_ut::handle_data,
            d, boost::ref(counter), boost::ref(ping_que), boost::ref(quit)
            )
          );*/
        handle_data(d, ping_que, pong_pool, quit);
        //counter = max_counter;
      }
      //else
      //{
      //  //boost::this_thread::yield();
      //  if (counter > 100)
      //  {
      //    --counter;
      //  }
      //  else if (counter > 0)
      //  {
      //    --counter;
      //    boost::this_thread::yield();
      //  }
      //  else
      //  {
      //    boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
      //  }
      //}
      free_data(pong_free_que.pop_all_reverse(), pong_pool);

      ios.poll();
    }
  }

  static void test()
  {
    try
    {
      detail::mpsc_queue<data> ping_que, ping_free_que;
      detail::mpsc_queue<data> pong_que, pong_free_que;
      detail::object_pool<data> ping_pool((detail::cache_pool*)0, size_nil);
      detail::object_pool<data> pong_pool((detail::cache_pool*)0, size_nil);
      io_service_t ping_ios(1);
      io_service_t pong_ios(1);

      boost::thread thr(
        boost::bind(
          &mpsc_pingpong_ut::pong,
          boost::ref(ping_que), boost::ref(pong_que), 
          boost::ref(pong_pool), 
          boost::ref(ping_free_que), boost::ref(pong_free_que), 
          boost::ref(pong_ios)
          )
        );

      {
        boost::timer::auto_cpu_timer t;
        data* o = ping_pool.get();
        pong_que.push(o);

        int counter = max_counter;
        bool quit = false;
        while (!quit)
        {
          data* d = ping_que.pop_all();
          if (d)
          {
            detail::scope scp(
              boost::bind(
                &detail::mpsc_queue<data>::push, 
                &pong_free_que, d
                )
              );
            /*ping_ios.dispatch(
              boost::bind(
                &mpsc_pingpong_ut::handle_data,
                d, boost::ref(counter), boost::ref(pong_que), boost::ref(quit)
                )
              );*/
            handle_data(d, pong_que, ping_pool, quit);
            //counter = max_counter;
          }
          //else
          //{
          //  //boost::this_thread::yield();
          //  if (counter > 100)
          //  {
          //    --counter;
          //  }
          //  else if (counter > 0)
          //  {
          //    --counter;
          //    boost::this_thread::yield();
          //  }
          //  else
          //  {
          //    boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
          //  }
          //}
          free_data(ping_free_que.pop_all_reverse(), ping_pool);

          ping_ios.poll();
        }
      }
      thr.join();
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}

