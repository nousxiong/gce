///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASSERT_ASSERTION_HPP
#define GCE_ASSERT_ASSERTION_HPP

#include <gce/assert/config.hpp>
#include <gce/log/all.hpp>
#include <gce/detail/cow_buffer.hpp>
#include <boost/chrono.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/array.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <exception>
#include <string>

namespace gce
{
class assert_except
  : public virtual std::exception
{
  typedef boost::chrono::system_clock::time_point time_point_t;
  typedef boost::array<char, 32> strbuf_t;

public:
  assert_except(time_point_t tp, char const* msg, boost::string_ref err, bool is_verify)
  {
    if (!err.empty())
    {
      what_.append(err.data(), err.size());
    }
    make_description(what_, is_verify ? "Verify exception, timestamp <" : "Assert exception, timestamp <", tp, msg);
  }

  
  virtual ~assert_except() throw() 
  {
  }

  char const* what() const throw()
  {
    return what_.c_str();
  }

  template <typename Buffer>
  static void make_description(Buffer& buf, char const* prefix, time_point_t tp, char const* msg)
  {
    buf.append(prefix);
    buf.append(boost::lexical_cast<strbuf_t>(tp.time_since_epoch().count()).cbegin());
    buf.append(">");
    if (msg)
    {
      buf.append(", \"");
      buf.append(msg);
      buf.append("\"");
    }
  }

private:
  std::string what_;
};

namespace detail
{
class assertion
{
  typedef boost::chrono::system_clock system_clock_t;
  typedef system_clock_t::time_point time_point_t;
  typedef gce::detail::cow_buffer<GCE_SMALL_ASSERT_SIZE, GCE_ASSERT_MIN_GROW_SIZE> errmsg_t;
  typedef boost::array<char, 32> strbuf_t;

  enum log_type
  {
    log_default,
    log_std,
    log_gce
  };

  enum handle_type
  {
    handle_default,
    handle_excepted,
    handle_aborted
  };

  struct enum_t {};
  struct ptr_t {};
  struct other_t {};

public:
  assertion(char const* expr, char const* file, int line, gce::log::level lv, bool is_verify = false)
    : GCE_ASSERT_A(*this)
    , GCE_ASSERT_B(*this)
    , lv_(lv)
    , ex_((time_point_t::min)())
    , msg_(0)
    , lg_type_(log_default)
    , hdl_type_(handle_default)
    , is_verify_(is_verify)
    , is_throw_(expr == "false" || expr == "0" || expr == "NULL")
  {
    if (is_throw_)
    {
      str_.append("Throw exception in ");
    }
    else
    {
      str_.append(is_verify_ ? "Verify failed in " : "Assert failed in ");
    }
    str_.append(file);
    str_.append(": ");
    str_.append(boost::lexical_cast<strbuf_t>(line).cbegin());
    if (is_throw_)
    {
      str_.append("\n");
    }
    else
    {
      str_.append("\nExpression: '");
      str_.append(expr);
      str_.append("'\n");
    }
  }

  assertion(assertion const& other)
    : GCE_ASSERT_A(*this)
    , GCE_ASSERT_B(*this)
    , str_(other.str_)
    , lg_(other.lg_)
    , lv_(other.lv_)
    , ex_(other.ex_)
    , msg_(other.msg_)
    , lg_type_(other.lg_type_)
    , hdl_type_(other.hdl_type_)
    , is_verify_(other.is_verify_)
    , is_throw_(other.is_throw_)
  {
  }

  ~assertion()
  {
    if (msg_)
    {
      str_.append(msg_);
      str_.append("\n");
    }

    if (ex_ != (time_point_t::min)())
    {
      gce::assert_except::make_description(str_, "Exception timestamp <", ex_, msg_);
    }

    bool throw_except = false;
    switch (hdl_type_)
    {
    case handle_default:
      {
        switch (lv_)
        {
        case gce::log::debug:
          pri_abort();
          break;
        case gce::log::info:
        case gce::log::warn:
          break;
        case gce::log::error:
          {
            throw_except = true;
            ex_ = system_clock_t::now();
          }break;
        case gce::log::fatal:
          pri_abort();
          break;
        default: 
          BOOST_ASSERT(false);
          break;
        }
      }break;
    case handle_aborted:
      pri_abort();
      break;
    default:
      break;
    }

    byte_t const* str = str_.data();
    boost::string_ref str_ref((char const*)str, str_.get_buffer_ref().write_size());
    switch (lg_type_)
    {
    case log_std: std::cerr << str_ref << std::endl; break;
    case log_gce: GCE_LOG(lg_, lv_) << str_ref; break;
    default: break;
    }

    if (throw_except)
    {
      if (lg_type_ != log_default)
      {
        str_ref = boost::string_ref();
      }
      throw gce::assert_except(ex_, msg_, str_ref, is_verify_);
    }
  }

  assertion& GCE_ASSERT_A;
  assertion& GCE_ASSERT_B;

public:
  assertion& msg(char const* msg)
  {
    msg_ = msg;
    return *this;
  }

  assertion& log(char const* msg = 0)
  {
    lg_type_ = log_std;
    msg_ = msg;
    return *this;
  }

  assertion& log(gce::log::logger_t const& lg, char const* msg = 0)
  {
    return log(lg, lv_, msg);
  }

  assertion& log(gce::log::logger_t const& lg, gce::log::level lv, char const* msg = 0)
  {
    lg_ = lg;
    lg_type_ = log_gce;
    lv_ = lv;
    msg_ = msg;
    return *this;
  }

  assertion& debug(gce::log::logger_t const& lg, char const* msg = 0)
  {
    return log(lg, gce::log::debug, msg);
  }

  assertion& info(gce::log::logger_t const& lg, char const* msg = 0)
  {
    return log(lg, gce::log::info, msg);
  }

  assertion& warn(gce::log::logger_t const& lg, char const* msg = 0)
  {
    return log(lg, gce::log::warn, msg);
  }

  assertion& error(gce::log::logger_t const& lg, char const* msg = 0)
  {
    return log(lg, gce::log::error, msg);
  }

  assertion& fatal(gce::log::logger_t const& lg, char const* msg = 0)
  {
    return log(lg, gce::log::fatal, msg);
  }

  void except()
  {
    boost::string_ref str_ref = pri_except();
    throw gce::assert_except(ex_, msg_, str_ref, is_verify_);
  }

  void except(boost::system::error_code const& ec)
  {
    boost::string_ref str_ref = pri_except();
    gce::assert_except ex(ex_, msg_, str_ref, is_verify_);
    boost::throw_exception(boost::system::system_error(ec, ex.what()));
  }

  template <typename Except>
  void except()
  {
    boost::string_ref str_ref = pri_except();
    gce::assert_except ex(ex_, msg_, str_ref, is_verify_);
    throw Except(ex.what());
  }

  void abort()
  {
    hdl_type_ = handle_aborted;
  }

private:
  boost::string_ref pri_except()
  {
    ex_ = system_clock_t::now();
    hdl_type_ = handle_excepted;
    boost::string_ref str_ref;
    if (lg_type_ == log_default)
    {
      byte_t const* str = str_.data();
      str_ref = boost::string_ref((char const*)str, str_.get_buffer_ref().write_size());
    }
    return str_ref;
  }

public:
  /// internal use
  assertion& set_var(char v, char const* name)
  {
    return pri_set_var(v, name);
  }

  assertion& set_var(char const* v, char const* name)
  {
    str_.append("  ");
    str_.append(name);
    str_.append(" = ");
    str_.append(v);
    str_.append("\n");
    return *this;
  }

  assertion& set_var(std::string const& v, char const* name)
  {
    str_.append("  ");
    str_.append(name);
    str_.append(" = ");
    str_.append(v.c_str());
    str_.append("\n");
    return *this;
  }

  assertion& set_var(boost::string_ref v, char const* name)
  {
    str_.append("  ");
    str_.append(name);
    str_.append(" = ");
    str_.append(v.data(), v.size());
    str_.append("\n");
    return *this;
  }

  assertion& set_var(bool v, char const* name)
  {
    return pri_set_var(v, name);
  }

  assertion& set_var(signed char v, char const* name)
  {
    return pri_set_var(v, name);
  }

  assertion& set_var(unsigned char v, char const* name)
  {
    return pri_set_var(v, name);
  }

  assertion& set_var(short v, char const* name)
  {
    return pri_set_var(v, name);
  }

  assertion& set_var(unsigned short v, char const* name)
  {
    return pri_set_var(v, name);
  }

  assertion& set_var(int v, char const* name)
  {
    return pri_set_var(v, name);
  }

  assertion& set_var(unsigned int v, char const* name)
  {
    return pri_set_var(v, name);
  }

  assertion& set_var(long v, char const* name)
  {
    return pri_set_var(v, name);
  }

  assertion& set_var(unsigned long v, char const* name)
  {
    return pri_set_var(v, name);
  }

#if !defined(BOOST_NO_LONG_LONG)
  assertion& set_var(long long v, char const* name)
  {
    return pri_set_var(v, name);
  }

  assertion& set_var(unsigned long long v, char const* name)
  {
    return pri_set_var(v, name);
  }
#endif

  assertion& set_var(float v, char const* name)
  {
    return pri_set_var(v, name);
  }

  assertion& set_var(double v, char const* name)
  {
    return pri_set_var(v, name);
  }

  assertion& set_var(long double v, char const* name)
  {
    return pri_set_var(v, name);
  }

  /*template <typename T>
  assertion& set_var(T const* const v, char const* name)
  {
    str_.append("  ");
    str_.append(name);
    str_.append(" = ");
    str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    str_.append("\n");
    return *this;
  }*/
  
  template <typename T>
  assertion& set_var(T const& v, char const* name)
  {
    typedef typename boost::remove_const<
      typename boost::remove_reference<
        typename boost::remove_volatile<T>::type
        >::type
      >::type param_t;

    typedef typename boost::mpl::if_<
      typename boost::is_enum<param_t>::type, enum_t, other_t
      >::type select_enum_t;

    typedef typename boost::mpl::if_<
      typename boost::is_pointer<param_t>::type, ptr_t, select_enum_t
    >::type select_ptr_t;

    pri_set_var(select_ptr_t(), v, name);
    return *this;
  }

private:
  template <typename T>
  assertion& pri_set_var(T const v, char const* name)
  {
    str_.append("  ");
    str_.append(name);
    str_.append(" = ");
    str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    str_.append("\n");
    return *this;
  }

  template <typename T>
  assertion& pri_set_var(enum_t, T const v, char const* name)
  {
    str_.append("  ");
    str_.append(name);
    str_.append(" = ");
    str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    str_.append("\n");
    return *this;
  }

  template <typename T>
  assertion& pri_set_var(other_t, T const& v, char const* name)
  {
    str_.append("  ");
    str_.append(name);
    str_.append(" = ");
    str_.append(tostring<T>::convert(v));
    str_.append("\n");
    return *this;
  }

  template <typename T>
  assertion& pri_set_var(ptr_t, T const& v, char const* name)
  {
    str_.append("  ");
    str_.append(name);
    str_.append(" = ");
    str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    str_.append("\n");
    return *this;
  }

  void pri_abort()
  {
    byte_t const* str = str_.data();
    boost::string_ref str_ref((char const*)str, str_.get_buffer_ref().write_size());
    switch (lg_type_)
    {
    case log_gce: 
      if (lg_)
      {
        GCE_LOG(lg_, lv_) << str_ref;
        break;
      }
    case log_std:
    default: 
      std::cerr << str_ref << std::endl; 
      break;
    }
    std::abort();
  }

private:
  errmsg_t str_;
  gce::log::logger_t lg_;
  gce::log::level lv_;
  time_point_t ex_;
  char const* msg_;
  log_type lg_type_;
  handle_type hdl_type_;
  bool is_verify_;
  bool is_throw_;
};

inline assertion make_assert(char const* expr, char const* file, int line, gce::log::level lv, bool is_verify = false)
{
  return assertion(expr, file, line, lv, is_verify);
}
} /// namespace detail
} /// namespace gce

#define GCE_ASSERT_A(x) GCE_ASSERT_OP(x, B)
#define GCE_ASSERT_B(x) GCE_ASSERT_OP(x, A)

#define GCE_ASSERT_OP(x, next) \
  GCE_ASSERT_A.set_var((x), #x).GCE_ASSERT_##next

#ifdef GCE_ENABLE_ASSERT
# define GCE_ASSERT(expr) \
  if ( (expr) ) ; \
  else gce::detail::make_assert(#expr, __FILE__, __LINE__, gce::log::debug).GCE_ASSERT_A

#else
# define GCE_ASSERT(expr) \
  if ( true ) ; \
  else gce::detail::make_assert(#expr, __FILE__, __LINE__, gce::log::debug).GCE_ASSERT_A

#endif

# define GCE_VERIFY(expr) \
  if ( (expr) ) ; \
  else gce::detail::make_assert(#expr, __FILE__, __LINE__, gce::log::error, true).GCE_ASSERT_A

#endif /// GCE_ASSERT_ASSERTION_HPP
