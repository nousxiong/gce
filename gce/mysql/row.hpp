///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_ROW_HPP
#define GCE_MYSQL_ROW_HPP

#include <gce/mysql/config.hpp>
#include <gce/mysql/datetime.hpp>
#include <gce/mysql/error.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>
#include <string>
#include <vector>

namespace gce
{
namespace mysql
{
struct var_t 
{
  bool operator!() const
  {
    return false;
  }

  operator bool() const
  {
    return true;
  }
};

static var_t const var = var_t();

struct row
{
  row()
    : res_(NULL)
    , row_(NULL)
    , index_(size_nil)
    , fields_(NULL)
    , lengths_(NULL)
    , field_size_(0)
  {
  }

  explicit row(MYSQL_RES* res, size_t index)
    : res_(res)
    , row_(NULL)
    , index_(index)
    , fields_(NULL)
    , lengths_(NULL)
    , field_size_(0)
  {
    mysql_data_seek(res_, index);
    row_ = mysql_fetch_row(res_);
    GCE_ASSERT(row_ != NULL)(index);
    fields_ = mysql_fetch_fields(res_);
    lengths_ = mysql_fetch_lengths(res_);
    field_size_ = mysql_num_fields(res_);
  }

  ~row()
  {
  }

  size_t field_size() const
  {
    return field_size_;
  }

  boost::string_ref fetch_raw(int i, errcode_t& ec)
  {
    if (!check(i, ec))
    {
      return boost::string_ref();
    }
    return boost::string_ref(row_[i], lengths_[i]);
  }

  MYSQL_FIELD& get_field(int i)
  {
    GCE_ASSERT(i >= 0 && i < field_size_);
    return fields_[i];
  }

  ///--------------------------------------------------------------------------
  /// fetch field, return 0 if field is NULL, using error code to handle error
  ///--------------------------------------------------------------------------
  signed char* operator()(int i, signed char& t, errcode_t& ec)
  {
    return fetch_number(i, t, MYSQL_TYPE_TINY, ec);
  }

  signed char* operator()(char const* name, signed char& t, errcode_t& ec)
  {
    size_t i = find_field_index(name);
    if (i == size_nil)
    {
      ec = make_errcode(field_name_not_found);
      GCE_VERIFY(!ec).except(ec);
    }
    return fetch_number(i, t, MYSQL_TYPE_TINY, ec);
  }

  unsigned char* operator()(int i, unsigned char& t, errcode_t& ec)
  {
    return fetch_number(i, t, MYSQL_TYPE_TINY, ec);
  }

  unsigned char* operator()(char const* name, unsigned char& t, errcode_t& ec)
  {
    size_t i = find_field_index(name);
    if (i == size_nil)
    {
      ec = make_errcode(field_name_not_found);
      GCE_VERIFY(!ec).except(ec);
    }
    return fetch_number(i, t, MYSQL_TYPE_TINY, ec);
  }

  int16_t* operator()(int i, int16_t& t, errcode_t& ec)
  {
    return fetch_number(i, t, MYSQL_TYPE_SHORT, ec);
  }

  int16_t* operator()(char const* name, int16_t& t, errcode_t& ec)
  {
    size_t i = find_field_index(name);
    if (i == size_nil)
    {
      ec = make_errcode(field_name_not_found);
      GCE_VERIFY(!ec).except(ec);
    }
    return fetch_number(i, t, MYSQL_TYPE_SHORT, ec);
  }

  uint16_t* operator()(int i, uint16_t& t, errcode_t& ec)
  {
    return fetch_number(i, t, MYSQL_TYPE_SHORT, ec);
  }

  uint16_t* operator()(char const* name, uint16_t& t, errcode_t& ec)
  {
    size_t i = find_field_index(name);
    if (i == size_nil)
    {
      ec = make_errcode(field_name_not_found);
      GCE_VERIFY(!ec).except(ec);
    }
    return fetch_number(i, t, MYSQL_TYPE_SHORT, ec);
  }

  int32_t* operator()(int i, int32_t& t, errcode_t& ec)
  {
    return fetch_number(i, t, MYSQL_TYPE_LONG, ec);
  }

  int32_t* operator()(char const* name, int32_t& t, errcode_t& ec)
  {
    size_t i = find_field_index(name);
    if (i == size_nil)
    {
      ec = make_errcode(field_name_not_found);
      GCE_VERIFY(!ec).except(ec);
    }
    return fetch_number(i, t, MYSQL_TYPE_LONG, ec);
  }

  uint32_t* operator()(int i, uint32_t& t, errcode_t& ec)
  {
    return fetch_number(i, t, MYSQL_TYPE_LONG, ec);
  }

  uint32_t* operator()(char const* name, uint32_t& t, errcode_t& ec)
  {
    size_t i = find_field_index(name);
    if (i == size_nil)
    {
      ec = make_errcode(field_name_not_found);
      GCE_VERIFY(!ec).except(ec);
    }
    return fetch_number(i, t, MYSQL_TYPE_LONG, ec);
  }

  int64_t* operator()(int i, int64_t& t, errcode_t& ec)
  {
    return fetch_number(i, t, MYSQL_TYPE_LONGLONG, ec);
  }

  int64_t* operator()(char const* name, int64_t& t, errcode_t& ec)
  {
    size_t i = find_field_index(name);
    if (i == size_nil)
    {
      ec = make_errcode(field_name_not_found);
      GCE_VERIFY(!ec).except(ec);
    }
    return fetch_number(i, t, MYSQL_TYPE_LONGLONG, ec);
  }

  uint64_t* operator()(int i, uint64_t& t, errcode_t& ec)
  {
    return fetch_number(i, t, MYSQL_TYPE_LONGLONG, ec);
  }

  uint64_t* operator()(char const* name, uint64_t& t, errcode_t& ec)
  {
    size_t i = find_field_index(name);
    if (i == size_nil)
    {
      ec = make_errcode(field_name_not_found);
      GCE_VERIFY(!ec).except(ec);
    }
    return fetch_number(i, t, MYSQL_TYPE_LONGLONG, ec);
  }

  float* operator()(int i, float& t, errcode_t& ec)
  {
    return fetch_number(i, t, MYSQL_TYPE_FLOAT, ec);
  }

  float* operator()(char const* name, float& t, errcode_t& ec)
  {
    size_t i = find_field_index(name);
    if (i == size_nil)
    {
      ec = make_errcode(field_name_not_found);
      GCE_VERIFY(!ec).except(ec);
    }
    return fetch_number(i, t, MYSQL_TYPE_FLOAT, ec);
  }

  double* operator()(int i, double& t, errcode_t& ec)
  {
    return fetch_number(i, t, MYSQL_TYPE_DOUBLE, ec);
  }

  double* operator()(char const* name, double& t, errcode_t& ec)
  {
    size_t i = find_field_index(name);
    if (i == size_nil)
    {
      ec = make_errcode(field_name_not_found);
      GCE_VERIFY(!ec).except(ec);
    }
    return fetch_number(i, t, MYSQL_TYPE_DOUBLE, ec);
  }

  std::string* operator()(int i, std::string& t, bool is_var, errcode_t& ec)
  {
    return fetch_string(i, t, is_var ? MYSQL_TYPE_VAR_STRING : MYSQL_TYPE_STRING, ec);
  }

  std::string* operator()(char const* name, std::string& t, bool is_var, errcode_t& ec)
  {
    size_t i = find_field_index(name);
    if (i == size_nil)
    {
      ec = make_errcode(field_name_not_found);
      GCE_VERIFY(!ec).except(ec);
    }
    return fetch_string(i, t, is_var ? MYSQL_TYPE_VAR_STRING : MYSQL_TYPE_STRING, ec);
  }

  boost::string_ref* operator()(int i, boost::string_ref& t, bool is_var, errcode_t& ec)
  {
    return fetch_string(i, t, is_var ? MYSQL_TYPE_VAR_STRING : MYSQL_TYPE_STRING, ec);
  }

  boost::string_ref* operator()(char const* name, boost::string_ref& t, bool is_var, errcode_t& ec)
  {
    size_t i = find_field_index(name);
    if (i == size_nil)
    {
      ec = make_errcode(field_name_not_found);
      GCE_VERIFY(!ec).except(ec);
    }
    return fetch_string(i, t, is_var ? MYSQL_TYPE_VAR_STRING : MYSQL_TYPE_STRING, ec);
  }

  datetime* operator()(int i, datetime& t, errcode_t& ec)
  {
    std::string* str = fetch_string(i, t.val_, MYSQL_TYPE_DATETIME, ec);
    if (str == 0)
    {
      return 0;
    }
    else
    {
      return &t;
    }
  }

  datetime* operator()(char const* name, datetime& t, errcode_t& ec)
  {
    size_t i = find_field_index(name);
    if (i == size_nil)
    {
      ec = make_errcode(field_name_not_found);
      GCE_VERIFY(!ec).except(ec);
    }
    return (*this)(i, t, ec);
  }

  sysclock_t::time_point* operator()(int i, sysclock_t::time_point& t, errcode_t& ec)
  {
    int64_t val;
    int64_t* str = fetch_number(i, val, MYSQL_TYPE_LONGLONG, ec);
    if (str == 0)
    {
      return 0;
    }
    else
    {
      std::time_t tm = (std::time_t)val;
      t = sysclock_t::from_time_t(tm);
      return &t;
    }
  }

  sysclock_t::time_point* operator()(char const* name, sysclock_t::time_point& t, errcode_t& ec)
  {
    size_t i = find_field_index(name);
    if (i == size_nil)
    {
      ec = make_errcode(field_name_not_found);
      GCE_VERIFY(!ec).except(ec);
    }
    return (*this)(i, t, ec);
  }

  ///--------------------------------------------------------------------------
  /// fetch field, return 0 if field is NULL, using exception code to handle error
  ///--------------------------------------------------------------------------
  signed char* operator()(int i, signed char& t)
  {
    errcode_t ec;
    signed char* rt = (*this)(i, t, ec);
    GCE_VERIFY(!ec)(i).except(ec);
    return rt;
  }

  signed char* operator()(char const* name, signed char& t)
  {
    errcode_t ec;
    signed char* rt = (*this)(name, t, ec);
    GCE_VERIFY(!ec)(name).except(ec);
    return rt;
  }

  unsigned char* operator()(int i, unsigned char& t)
  {
    errcode_t ec;
    unsigned char* rt = (*this)(i, t, ec);
    GCE_VERIFY(!ec)(i).except(ec);
    return rt;
  }

  unsigned char* operator()(char const* name, unsigned char& t)
  {
    errcode_t ec;
    unsigned char* rt = (*this)(name, t, ec);
    GCE_VERIFY(!ec)(name).except(ec);
    return rt;
  }

  int16_t* operator()(int i, int16_t& t)
  {
    errcode_t ec;
    int16_t* rt = (*this)(i, t, ec);
    GCE_VERIFY(!ec)(i).except(ec);
    return rt;
  }

  int16_t* operator()(char const* name, int16_t& t)
  {
    errcode_t ec;
    int16_t* rt = (*this)(name, t, ec);
    GCE_VERIFY(!ec)(name).except(ec);
    return rt;
  }

  uint16_t* operator()(int i, uint16_t& t)
  {
    errcode_t ec;
    uint16_t* rt = (*this)(i, t, ec);
    GCE_VERIFY(!ec)(i).except(ec);
    return rt;
  }

  uint16_t* operator()(char const* name, uint16_t& t)
  {
    errcode_t ec;
    uint16_t* rt = (*this)(name, t, ec);
    GCE_VERIFY(!ec)(name).except(ec);
    return rt;
  }

  int32_t* operator()(int i, int32_t& t)
  {
    errcode_t ec;
    int32_t* rt = (*this)(i, t, ec);
    GCE_VERIFY(!ec)(i).except(ec);
    return rt;
  }

  int32_t* operator()(char const* name, int32_t& t)
  {
    errcode_t ec;
    int32_t* rt = (*this)(name, t, ec);
    GCE_VERIFY(!ec)(name).except(ec);
    return rt;
  }

  uint32_t* operator()(int i, uint32_t& t)
  {
    errcode_t ec;
    uint32_t* rt = (*this)(i, t, ec);
    GCE_VERIFY(!ec)(i).except(ec);
    return rt;
  }

  uint32_t* operator()(char const* name, uint32_t& t)
  {
    errcode_t ec;
    uint32_t* rt = (*this)(name, t, ec);
    GCE_VERIFY(!ec)(name).except(ec);
    return rt;
  }

  int64_t* operator()(int i, int64_t& t)
  {
    errcode_t ec;
    int64_t* rt = (*this)(i, t, ec);
    GCE_VERIFY(!ec)(i).except(ec);
    return rt;
  }

  int64_t* operator()(char const* name, int64_t& t)
  {
    errcode_t ec;
    int64_t* rt = (*this)(name, t, ec);
    GCE_VERIFY(!ec)(name).except(ec);
    return rt;
  }

  uint64_t* operator()(int i, uint64_t& t)
  {
    errcode_t ec;
    uint64_t* rt = (*this)(i, t, ec);
    GCE_VERIFY(!ec)(i).except(ec);
    return rt;
  }

  uint64_t* operator()(char const* name, uint64_t& t)
  {
    errcode_t ec;
    uint64_t* rt = (*this)(name, t, ec);
    GCE_VERIFY(!ec)(name).except(ec);
    return rt;
  }

  float* operator()(int i, float& t)
  {
    errcode_t ec;
    float* rt = (*this)(i, t, ec);
    GCE_VERIFY(!ec)(i).except(ec);
    return rt;
  }

  float* operator()(char const* name, float& t)
  {
    errcode_t ec;
    float* rt = (*this)(name, t, ec);
    GCE_VERIFY(!ec)(name).except(ec);
    return rt;
  }

  double* operator()(int i, double& t)
  {
    errcode_t ec;
    double* rt = (*this)(i, t, ec);
    GCE_VERIFY(!ec)(i).except(ec);
    return rt;
  }

  double* operator()(char const* name, double& t)
  {
    errcode_t ec;
    double* rt = (*this)(name, t, ec);
    GCE_VERIFY(!ec)(name).except(ec);
    return rt;
  }

  std::string* operator()(int i, std::string& t, bool is_var)
  {
    errcode_t ec;
    std::string* rt = (*this)(i, t, is_var, ec);
    GCE_VERIFY(!ec)(i).except(ec);
    return rt;
  }

  std::string* operator()(char const* name, std::string& t, bool is_var)
  {
    errcode_t ec;
    std::string* rt = (*this)(name, t, is_var, ec);
    GCE_VERIFY(!ec)(name).except(ec);
    return rt;
  }

  boost::string_ref* operator()(int i, boost::string_ref& t, bool is_var)
  {
    errcode_t ec;
    boost::string_ref* rt = (*this)(i, t, is_var, ec);
    GCE_VERIFY(!ec)(i).except(ec);
    return rt;
  }

  boost::string_ref* operator()(char const* name, boost::string_ref& t, bool is_var)
  {
    errcode_t ec;
    boost::string_ref* rt = (*this)(name, t, is_var, ec);
    GCE_VERIFY(!ec)(name).except(ec);
    return rt;
  }

  datetime* operator()(int i, datetime& t)
  {
    errcode_t ec;
    datetime* rt = (*this)(i, t, ec);
    GCE_VERIFY(!ec)(i).except(ec);
    return rt;
  }

  datetime* operator()(char const* name, datetime& t)
  {
    errcode_t ec;
    datetime* rt = (*this)(name, t, ec);
    GCE_VERIFY(!ec)(name).except(ec);
    return rt;
  }

  sysclock_t::time_point* operator()(int i, sysclock_t::time_point& t)
  {
    errcode_t ec;
    sysclock_t::time_point* rt = (*this)(i, t, ec);
    GCE_VERIFY(!ec)(i).except(ec);
    return rt;
  }

  sysclock_t::time_point* operator()(char const* name, sysclock_t::time_point& t)
  {
    errcode_t ec;
    sysclock_t::time_point* rt = (*this)(name, t, ec);
    GCE_VERIFY(!ec)(name).except(ec);
    return rt;
  }

public:
  template <typename T>
  T* fetch_number(int i, T& t, int type, errcode_t& ec)
  {
    if (!check(i, ec))
    {
      return 0;
    }

    MYSQL_FIELD& my_field = fields_[i];
    if (type >= 0 && my_field.type != type)
    {
      ec = make_errcode(field_type_incorrect);
      return 0;
    }

    if (type >= 0)
    {
      bool is_unsigned = boost::is_unsigned<T>::value;
      my_bool unsigned_flag = my_field.flags & UNSIGNED_FLAG;
      if (!is_unsigned && unsigned_flag || is_unsigned && !unsigned_flag)
      {
        ec = make_errcode(field_signed_incorrect);
        return 0;
      }
    }

    if (!str2num(i, t))
    {
      ec = make_errcode(bad_str2num);
      return 0;
    }
    return &t;
  }

  std::string* fetch_string(int i, std::string& t, int type, errcode_t& ec)
  {
    if (!check(i, ec))
    {
      return 0;
    }

    if (!check_strtype(i, type, ec))
    {
      return 0;
    }

    t.assign(row_[i], lengths_[i]);
    return &t;
  }

  boost::string_ref* fetch_string(int i, boost::string_ref& t, int type, errcode_t& ec)
  {
    if (!check(i, ec))
    {
      return 0;
    }

    if (!check_strtype(i, type, ec))
    {
      return 0;
    }

    t = boost::string_ref(row_[i], lengths_[i]);
    return &t;
  }

private:
  bool check(int i, errcode_t& ec)
  {
    if (i < 0 || i >= (int)field_size_)
    {
      ec = make_errcode(out_of_field_range);
      return false;
    }

    if (row_[i] == NULL)
    {
      return false;
    }
    return true;
  }

  bool check_strtype(int i, int type, errcode_t& ec)
  {
    MYSQL_FIELD& my_field = fields_[i];
    if (
      type >= 0 && my_field.type != type && 
      my_field.type != MYSQL_TYPE_TINY_BLOB &&
      my_field.type != MYSQL_TYPE_MEDIUM_BLOB &&
      my_field.type != MYSQL_TYPE_LONG_BLOB &&
      my_field.type != MYSQL_TYPE_BLOB
      )
    {
      ec = make_errcode(field_type_incorrect);
      return false;
    }
    return true;
  }

  template <typename T>
  bool str2num(int i, T& t)
  {
    return boost::conversion::try_lexical_convert<T>(row_[i], lengths_[i], t);
  }

  bool str2num(int i, signed char& t)
  {
    int r = 0;
    bool ok = boost::conversion::try_lexical_convert<int>(row_[i], lengths_[i], r);
    if (ok)
    {
      t = (signed char)r;
    }
    return ok;
  }

  bool str2num(int i, unsigned char& t)
  {
    unsigned int r = 0;
    bool ok = boost::conversion::try_lexical_convert<unsigned int>(row_[i], lengths_[i], r);
    if (ok)
    {
      t = (unsigned char)r;
    }
    return ok;
  }

  size_t find_field_index(char const* name)
  {
    for (size_t i=0; i<field_size_; ++i)
    {
      if (strcmp(name, fields_[i].name) == 0)
      {
        return i;
      }
    }
    return size_nil;
  }

public:
  MYSQL_RES* res_;
  size_t index_;
  MYSQL_ROW row_;
  MYSQL_FIELD* fields_;
  unsigned long* lengths_;
  size_t field_size_;
};

static bool operator==(row const& lhs, row const& rhs)
{
  return 
    lhs.res_ == rhs.res_ &&
    lhs.index_ == rhs.index_ &&
    lhs.row_ == rhs.row_ &&
    lhs.fields_ == rhs.fields_ &&
    lhs.lengths_ == rhs.lengths_ &&
    lhs.field_size_ == rhs.field_size_
    ;
}

static bool operator!=(row const& lhs, row const& rhs)
{
  return !(lhs == rhs);
}

static row const row_nil = row();
}
}

#endif /// GCE_MYSQL_ROW_HPP
