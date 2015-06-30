///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_PARSER_SIMPLE_HPP
#define GCE_ASIO_PARSER_SIMPLE_HPP

#include <gce/asio/config.hpp>
#include <gce/asio/parser/basic.hpp>

namespace gce
{
namespace asio
{
namespace parser
{
struct simple
  : public length
  , public regex
{
  /// length parser
  header on_recv_header(boost::asio::const_buffer b, message& msg)
  {
    pkr_.clear();
    pkr_.set_read(boost::asio::buffer_cast<byte_t const*>(b), boost::asio::buffer_size(b));
    packer::error_code_t ec = packer::ok();

    uint64_t size;
    header hdr;
    hdr.finish_ = false;
    if (!read(size))
    {
      pkr_.clear();
      return hdr;
    }

    hdr.finish_ = true;
    hdr.header_size_ = pkr_.read_length();
    hdr.body_size_ = (size_t)size;
    return hdr;
  }

  void on_send(message& hdr, message& msg)
  {
    hdr << (uint64_t)msg.size();
  }

  size_t max_header_size() const
  {
    return sizeof(uint64_t) * 2;
  }

  /// regex parser
  void on_recv(message&)
  {
    /// do nothing
  }

  std::string get_expr() const
  {
    return r_;
  }

public:
  simple(std::string r = "\r\n")
    : r_(r)
  {
  }

  void set_expr(std::string const& r)
  {
    r_ = r;
  }

private:
  template <typename T>
  bool read(T& t)
  {
    packer::error_code_t ec = packer::ok();
    pkr_.read(t, ec);
    return ec == packer::ok();
  }

  packer pkr_;
  std::string r_;
};
}
}
}

#endif /// GCE_ASIO_PARSER_SIMPLE_HPP
