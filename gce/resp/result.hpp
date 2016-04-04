///
/// result.hpp
///

#ifndef RESP_RESULT_HPP
#define RESP_RESULT_HPP

#include "config.hpp"
#include "unique_value.hpp"

namespace resp
{
/// Result type of decoding.
enum result_type
{
  completed,
  incompleted,
  error
};

/// Result of decoding.
class result
{
public:
  result()
    : ty_(incompleted)
    , size_(0)
  {
  }

  result(result_type ty, size_t size)
    : ty_(ty)
    , size_(size)
  {
  }

  result(result_type ty, size_t size, unique_value const& val)
    : ty_(ty)
    , size_(size)
    , val_(val)
  {
  }

  /// For copy use.
  static void copy(result& des, result const& src)
  {
    des.ty_ = src.ty_;
    des.size_ = src.size_;
    unique_value::copy(des.val_, src.val_);
  }

public:
  /// Check result type.
  /**
   * @param ty Type of result.
   */
  bool operator==(result_type ty) const
  {
    return ty_ == ty;
  }

  bool operator!=(result_type ty) const
  {
    return ty_ != ty;
  }

  /// Get type.
  result_type type() const
  {
    return ty_;
  }

  /// Decoded size.
  size_t size() const
  {
    return size_;
  }

  unique_value const& value() const
  {
    return val_;
  }

public:
  void size(size_t size)
  {
    size_ = size;
  }

  void value(unique_value const& val)
  {
    val_ = val;
  }

private:
  result_type ty_;
  size_t size_;
  unique_value val_;
};
}

#endif /// RESP_RESULT_HPP

