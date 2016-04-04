///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_REDIS_RESULT_HPP
#define GCE_REDIS_RESULT_HPP

#include <gce/redis/config.hpp>
#include <gce/resp/result.hpp>
#include <vector>

namespace gce
{
namespace redis
{
struct result
{
  result()
  {
  }

  ~result()
  {
  }

  void clear()
  {
    res_ = resp::result();
  }

  void set(resp::result const& res)
  {
    res_ = res;
  }

  resp::value_type type() const
  {
    return res_.value().type();
  }

  int64_t integer() const
  {
    return res_.value().integer();
  }

  resp::buffer const& string() const
  {
    return res_.value().string();
  }

  resp::buffer const& error() const
  {
    return res_.value().error();
  }

  resp::buffer const& bulkstr() const
  {
    return res_.value().bulkstr();
  }

  resp::unique_array<resp::unique_value> const& array() const
  {
    return res_.value().array();
  }

  template <typename T>
  T& get(T& t)
  {
    resp::buffer const& buff = res_.value().bulkstr();
    pkr_.set_read((byte_t const*)buff.data(), buff.size());
    pkr_.read(t);
    return t;
  }

  template <typename T>
  T& get(resp::unique_array<resp::unique_value> const& arr, size_t arridx, T& t)
  {
    GCE_ASSERT(arr.size() > arridx)(arridx)(arr.size());
    resp::buffer const& buff = arr[arridx].bulkstr();
    pkr_.set_read((gce::byte_t const*)buff.data(), buff.size());
    pkr_.read(t);
    return t;
  }

  gce::packer pkr_;
  resp::result res_;
};
typedef boost::shared_ptr<result> result_ptr;
}
}

#endif /// GCE_REDIS_RESULT_HPP
