///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <boost/asio.hpp>

namespace gce
{
class move_ptr_ut
{
  typedef gce::moved_ptr<void> void_ptr;
  typedef boost::asio::ip::tcp::socket tcp_socket;
  typedef gce::moved_ptr<tcp_socket> tcp_socket_ptr;

public:
  static void run()
  {
    std::cout << "move_ptr_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_base();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "move_ptr_ut end." << std::endl;
  }

private:
  static void test_base()
  {
    boost::asio::io_service ios;
    tcp_socket_ptr skt(new tcp_socket(ios));
    {
      tcp_socket_ptr skt_other(skt);
      BOOST_ASSERT(skt_other);
      BOOST_ASSERT(!skt);
      skt = skt_other;
    }
    {
      tcp_socket_ptr skt_other;
      skt_other = skt;
      BOOST_ASSERT(skt_other);
      BOOST_ASSERT(!skt);
      skt = skt_other;
    }
    {
      void_ptr ptr(skt);
      skt = gce::static_pointer_cast<tcp_socket>(ptr);
      BOOST_ASSERT(!ptr);
      BOOST_ASSERT(skt);
    }
    {
      void_ptr ptr(skt);
      BOOST_ASSERT(ptr);
      BOOST_ASSERT(!skt);
      skt = gce::static_pointer_cast<tcp_socket>(ptr);
    }
    {
      void_ptr ptr;
      ptr = skt;
      BOOST_ASSERT(ptr);
      BOOST_ASSERT(!skt);
      skt = gce::static_pointer_cast<tcp_socket>(ptr);
    }
    {
      std::vector<tcp_socket_ptr> socket_list;
      socket_list.push_back(skt);
      BOOST_ASSERT(socket_list.front());
      BOOST_ASSERT(!skt);
    
      std::vector<tcp_socket_ptr> socket_list_other(socket_list);
      BOOST_ASSERT(!socket_list.front());
      BOOST_ASSERT(socket_list_other.front());
      skt = socket_list_other.front();
    }
  }
};
}
