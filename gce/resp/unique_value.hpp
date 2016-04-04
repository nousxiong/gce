///
/// unique_value.hpp
///

#ifndef RESP_UNIQUE_VALUE_HPP
#define RESP_UNIQUE_VALUE_HPP

#include "config.hpp"
#include "buffer.hpp"
#include "unique_array.hpp"
#ifdef _MSC_VER
# if _MSC_VER <= 1500
#   include "stdint.hpp"
# else
#   include <cstdint>
# endif
#elif __cplusplus < 201103L
# include <stdint.h>
#else
# include <cstdint>
#endif
#include <cassert>

namespace resp
{
enum value_type
{
  ty_null,
  ty_string,
  ty_error,
  ty_integer,
  ty_bulkstr,
  ty_array
};

/// Unique value, copy means move(no copy).
/**
 * @note Under C++98/03 there is no move, so just use copy(consturctor/assignment) to instead;
 *  after C++11 could use move directly.
 */
class unique_value
{
public:
  unique_value()
    : ty_(ty_null)
    , integer_(0)
  {
  }

  unique_value(unique_value const& other)
    : ty_(other.ty_)
    , integer_(other.integer_)
    , array_(other.array_)
  {
    unique_value* src = const_cast<unique_value*>(&other);
    buffer::move(str_, src->str_);

    src->ty_ = ty_null;
    src->integer_ = 0;
  }

  unique_value& operator=(unique_value const& rhs)
  {
    if (this != &rhs)
    {
      unique_value* src = const_cast<unique_value*>(&rhs);
      ty_ = src->ty_;
      integer_ = src->integer_;
      buffer::move(str_, src->str_);
      array_ = src->array_;

      src->ty_ = ty_null;
      src->integer_ = 0;
    }
    return *this;
  }

  unique_value(char const* str)
    : ty_(ty_string)
    , integer_(0)
  {
    str_.append(str);
  }

  unique_value(char const* str, value_type ty)
    : ty_(ty)
    , integer_(0)
  {
    assert(ty_ != ty_array);
    assert(ty_ != ty_integer);
    str_.append(str);
  }

  unique_value(char const* str, size_t size, value_type ty)
    : ty_(ty)
    , integer_(0)
  {
    assert(ty_ != ty_array);
    assert(ty_ != ty_integer);
    str_.append(str, size);
  }

  unique_value(int64_t integer)
    : ty_(ty_integer)
    , integer_(integer)
  {
  }

  unique_value(unique_array<unique_value> const& array)
    : ty_(ty_array)
    , integer_(0)
    , array_(array)
  {
  }

  /// For copy use.
  static void copy(unique_value& des, unique_value const& src)
  {
    des.ty_ = src.ty_;
    des.integer_ = src.integer_;
    des.str_ = src.str_;
    des.array_.clear();
    for (size_t i=0, size=src.array_.size(); i<size; ++i)
    {
      unique_value& uv = des.array_.emplace_back();
      copy(uv, src.array_[i]);
    }
  }

  ~unique_value()
  {
  }

public:
  /// Get unique_value type.
  value_type type() const
  {
    return ty_;
  }

  /// Check if null.
  operator bool() const
  {
    return ty_ != ty_null;
  }

  /// Check if not null.
  bool operator!() const
  {
    return ty_ == ty_null;
  }

  /// Cast to integer.
  int64_t integer() const
  {
    assert(ty_ == ty_integer);
    return integer_;
  }

  /// Cast to string.
  buffer const& string() const
  {
    assert(ty_ == ty_string);
    return str_;
  }

  /// Cast to error.
  buffer const& error() const
  {
    assert(ty_ == ty_error);
    return str_;
  }

  /// Cast to bulk string.
  buffer const& bulkstr() const
  {
    assert(ty_ == ty_bulkstr);
    return str_;
  }

  /// Cast to array.
  unique_array<unique_value> const& array() const
  {
    assert(ty_ == ty_array);
    return array_;
  }

private:
  value_type ty_;
  int64_t integer_;
  buffer str_;
  unique_array<unique_value> array_;
};
}

#endif /// RESP_VALUE_HPP
