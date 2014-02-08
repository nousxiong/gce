#include <gce/actor/message.hpp>
#include <boost/thread.hpp>
#include <boost/assign.hpp>
#include <gce/amsg/amsg.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <map>

namespace gce
{
struct arg1_t
{
  std::string hi_;
  int i_;
};

struct arg2_t
{
  std::vector<int> v_;
  int i_;
};
}
GCE_PACK(gce::arg1_t, (hi_&smax(100))(i_&sfix));
GCE_PACK(gce::arg2_t, (v_&smax(5))(i_));

namespace gce
{
static std::size_t const lv1_thr_num = 5;
static std::size_t const lv2_thr_num = 5;
static std::size_t const lv3_thr_num = 2;
class message_ut
{
public:
  static void run()
  {
    for (std::size_t i=0; i<100; ++i)
    {
      test_common();
      std::cout << "message_ut count: " << i << std::endl;
    }
  }

  static void print(int size, message& msg)
  {
    arg1_t arg1;
    arg2_t arg2;

    std::stringstream ss;
    ss << "begin: \n";
    for (int i=0; i<size; ++i)
    {
      msg >> arg1 >> arg2;
      ss << "arg1: " << arg1.hi_ << ", " << arg1.i_ << ", arg2: "
        << arg2.v_[0] << ", " << arg2.v_[1] << ", " << arg2.v_[2]
        << ", " << arg2.v_[3] << ", " << arg2.v_[4] << ", "
        << arg2.i_ << "\n";
    }
    ss << "end.\n";
    std::cout << ss.str();
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
    print(size, msg);
  }

  static void lv1_thr(message& msg)
  {
    print(2, msg);

    boost::thread_group thrs;
    for (std::size_t i=0; i<lv2_thr_num; ++i)
    {
      int size = 2;
      if (i == 1)
      {
        add_arg(msg);
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
      add_arg(m);

      boost::thread_group thrs;
      for (std::size_t i=0; i<lv1_thr_num; ++i)
      {
        thrs.create_thread(boost::bind(&message_ut::lv1_thr, m));
      }

      thrs.join_all();

      std::cout << "\n///----------------------------------------------------------\n" << std::endl;
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
