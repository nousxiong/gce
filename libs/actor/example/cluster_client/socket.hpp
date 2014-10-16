///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_EXAMPLE_CLUSTER_SOCKET_HPP
#define GCE_ACTOR_EXAMPLE_CLUSTER_SOCKET_HPP

#include <gce/actor/all.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/predef.h>

struct msg_header
{
  boost::uint32_t size_;
  gce::match_t type_;
  boost::uint32_t tag_offset_;
};
GCE_PACK(msg_header, (size_&sfix)(type_&sfix)(tag_offset_&sfix));

#define MSG_HEADER_SIZE sizeof(boost::uint32_t) + sizeof(gce::match_t) + sizeof(boost::uint32_t)

template <typename Socket, std::size_t MaxMsgSize = 5 * 1024>
class basic_socket
{
public:
  typedef Socket socket_t;

public:
  explicit basic_socket(gce::io_service_t& ios)
    : sock_(ios)
#ifdef BOOST_OS_WINDOWS
    , max_batch_size_(64) /// no winnt below
#else
    , max_batch_size_((std::min)(64, IOV_MAX))
#endif
  {
  }

  ~basic_socket()
  {
    close();
  }

public:
  socket_t& get_socket()
  {
    return sock_;
  }

  std::size_t get_max_batch_size() const
  {
    return max_batch_size_;
  }

  gce::message recv(gce::yield_t yield)
  {
    BOOST_STATIC_ASSERT((MaxMsgSize > MSG_HEADER_SIZE));

    gce::byte_t buf[MaxMsgSize];
    std::size_t header_size = MSG_HEADER_SIZE;
    boost::asio::async_read(
      sock_,
      boost::asio::buffer(buf, header_size),
      yield
      );

    if (yield.ec_ && *yield.ec_)
    {
      return gce::message();
    }

    msg_header hdr;
    boost::amsg::zero_copy_buffer zbuf(buf, MaxMsgSize);
    boost::amsg::read(zbuf, hdr);
    if (zbuf.bad())
    {
      throw std::runtime_error("message header parse error");
    }

    std::size_t size = hdr.size_;
    std::size_t max_size = MaxMsgSize;
    if (size > max_size)
    {
      std::runtime_error("message out of length");
    }

    if (size > 0)
    {
      boost::asio::async_read(
        sock_,
        boost::asio::buffer(buf, size),
        yield
        );
    }
    return gce::message(hdr.type_, buf, hdr.size_, hdr.tag_offset_);
  }

  void send(gce::message const& msg, gce::yield_t yield)
  {
    msg_header hdr;
    hdr.size_ = msg.size();
    hdr.type_ = msg.get_type();
    hdr.tag_offset_ = msg.get_tag_offset();
    gce::byte_t buf[sizeof(msg_header)];
    boost::amsg::zero_copy_buffer zbuf(buf, sizeof(msg_header));
    boost::amsg::write(zbuf, hdr);

    boost::array<boost::asio::const_buffer, 2> bufs;
    bufs[0] = boost::asio::buffer(buf, zbuf.write_length());
    bufs[1] = boost::asio::buffer(msg.data(), msg.size());

    boost::asio::async_write(sock_, bufs, yield);
  }

  void close()
  {
    gce::errcode_t ignore_ec;
    sock_.close(ignore_ec);
  }

private:
  socket_t sock_;
  std::size_t const max_batch_size_;
};

typedef basic_socket<boost::asio::ip::tcp::socket> tcp_socket;

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_SOCKET_HPP
