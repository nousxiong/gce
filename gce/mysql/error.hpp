///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_ERROR_HPP
#define GCE_MYSQL_ERROR_HPP

#include <gce/mysql/config.hpp>

namespace gce
{
namespace mysql
{
enum sql_errors
{
  field_type_incorrect = 1,
  field_signed_incorrect,
  out_of_field_range,
  bad_str2num,
  field_name_not_found,
};

namespace detail
{
class sql_category : public boost::system::error_category
{
public:
  char const* name() const
  {
    return "gce.mysql.sql";
  }

  std::string message(int value) const
  {
    if (value == field_type_incorrect)
      return "field type incorrect";
    if (value == field_signed_incorrect)
      return "field signed incorrect";
    if (value == out_of_field_range)
      return "out of field range";
    if (value == bad_str2num)
      return "string convert to number failed";
    if (value == field_name_not_found)
      return "field name not found";
    return "gce.mysql.sql error";
  }
};
} // namespace detail

//static boost::system::error_category const& get_sql_category()
//{
//  detail::sql_category instance;
//  return instance;
//}

static detail::sql_category const sql_cat;
} /// namespace mysql
} /// namespace gce

namespace boost
{
namespace system
{
template<> struct is_error_code_enum<gce::mysql::sql_errors>
{
  static const bool value = true;
};
} // namespace system
} // namespace boost

namespace gce
{
namespace mysql
{
inline gce::errcode_t make_errcode(sql_errors e)
{
  return gce::errcode_t(static_cast<int>(e), sql_cat);
}
} // namespace mysql
} // namespace gce

#endif /// GCE_MYSQL_ERROR_HPP
