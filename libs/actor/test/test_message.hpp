///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <boost/thread.hpp>
#include <boost/assign.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <map>

namespace gce
{
typedef adl::arg1_t arg1_t;
typedef adl::arg2_t arg2_t;
}
GCE_PACK(gce::arg1_t, (v.hi_&smax(100))(v.i_&sfix));
GCE_PACK(gce::arg2_t, (v.v_&smax(5))(v.i_));

namespace gce
{
static std::size_t const lv1_thr_num = 5;
static std::size_t const lv2_thr_num = 5;
class message_ut
{
public:
  static void run()
  {
    std::cout << "message_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "message_ut end." << std::endl;
  }

  static void print(std::string const& tag, int size, message& msg, int i = 0)
  {
    arg1_t arg1;
    arg2_t arg2;

    std::stringstream ss;
    ss << "begin [" << tag << "," << i << "]\n";
    {
      msg >> arg1 >> arg2;
      ss << "arg1: " << arg1.hi_ << ", " << arg1.i_ << ", arg2: "
        << arg2.v_[0] << ", " << arg2.v_[1] << ", " << arg2.v_[2]
        << ", " << arg2.v_[3] << ", " << arg2.v_[4] << ", "
        << arg2.i_ << "\n";
    }

    for (i=1; i<size; ++i)
    {
      message m;
      msg >> m;
      print(tag, 0, m, i);
    }
    ss << "end.\n";
    //std::cout << ss.str();
  }

  static void add_arg(message& msg)
  {
    arg1_t arg1;
    arg1.hi_ = "arg1.hi_, need more words";
    arg1.i_ = 1;
    arg2_t arg2;
    arg2.v_ = boost::assign::list_of(1)(2)(3)(4)(5).to_container(arg2.v_);
    arg2.i_ = 2;
    msg << arg1 << arg2;
  }

  static void lv2_thr(int size, message& msg)
  {
    print("lv2_thr", size, msg);
  }

  static void lv1_thr(message& msg)
  {
    print("lv1_thr", 2, msg);

    boost::thread_group thrs;
    for (std::size_t i=0; i<lv2_thr_num; ++i)
    {
      int size = 2;
      if (i == 1)
      {
        msg << msg;
        size = 3;
      }
      else if (i > 1)
      {
        size = 3;
      }
      thrs.create_thread(boost::bind(&message_ut::lv2_thr, size, msg));
    }

    thrs.join_all();
  }

  static void test_common()
  {
    try
    {
      message m(1);
      add_arg(m);
      m << m;

      boost::thread_group thrs;
      for (std::size_t i=0; i<lv1_thr_num; ++i)
      {
        thrs.create_thread(boost::bind(&message_ut::lv1_thr, m));
      }

      thrs.join_all();

      //std::cout << "\n///----------------------------------------------------------\n" << std::endl;
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
