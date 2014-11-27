///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_LOG_RECORD_HPP
#define GCE_LOG_RECORD_HPP

#include <gce/log/config.hpp>
#include <gce/detail/cow_buffer.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/array.hpp>

namespace gce
{
namespace log
{
class record
{
  typedef boost::array<char, 32> strbuf_t;

public:
  record(logger_t& lg, level lv, time_point_t nw = time_point_t())
    : lg_(&lg)
    , lv_(lv)
    , flushed_(false)
    , ts_(nw)
    , rec_(0)
  {
  }

  template <typename Scope>
  record(Scope& scp, level lv)
    : lg_(&scp.get_logger())
    , lv_(lv)
    , flushed_(false)
    , rec_(&scp.add_record(lv))
  {
  }

  ~record()
  {
  }

public:
  level get_level() const
  {
    return lv_;
  }

  boost::string_ref get_log_string() const
  {
    if (rec_)
    {
      return rec_->get_log_string();
    }
    else
    {
      return boost::string_ref((char const*)str_.data(), str_.size());
    }
  }

  record& operator<<(char c)
  {
    if (rec_)
    {
      (*rec_) << c;
    }
    else
    {
      str_.append(&c, 1);
    }
    return *this;
  }

  record& operator<<(char const* p)
  {
    if (rec_)
    {
      (*rec_) << p;
    }
    else
    {
      str_.append(p);
    }
    return *this;
  }

  record& operator<<(std::string const& str)
  {
    if (rec_)
    {
      (*rec_) << str;
    }
    else
    {
      str_.append(str);
    }
    return *this;
  }

  record& operator<<(boost::string_ref str)
  {
    if (rec_)
    {
      (*rec_) << str;
    }
    else
    {
      str_.append(str.data(), str.size());
    }
    return *this;
  }

  record& operator<<(bool v)
  {
    if (rec_)
    {
      (*rec_) << v;
    }
    else
    {
      str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    }
    return *this;
  }

  record& operator<<(signed char v)
  {
    if (rec_)
    {
      (*rec_) << v;
    }
    else
    {
      str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    }
    return *this;
  }

  record& operator<<(unsigned char v)
  {
    if (rec_)
    {
      (*rec_) << v;
    }
    else
    {
      str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    }
    return *this;
  }

  record& operator<<(short v)
  {
    if (rec_)
    {
      (*rec_) << v;
    }
    else
    {
      str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    }
    return *this;
  }

  record& operator<<(unsigned short v)
  {
    if (rec_)
    {
      (*rec_) << v;
    }
    else
    {
      str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    }
    return *this;
  }

  record& operator<<(int v)
  {
    if (rec_)
    {
      (*rec_) << v;
    }
    else
    {
      str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    }
    return *this;
  }

  record& operator<<(unsigned int v)
  {
    if (rec_)
    {
      (*rec_) << v;
    }
    else
    {
      str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    }
    return *this;
  }

  record& operator<<(long v)
  {
    if (rec_)
    {
      (*rec_) << v;
    }
    else
    {
      str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    }
    return *this;
  }

  record& operator<<(unsigned long v)
  {
    if (rec_)
    {
      (*rec_) << v;
    }
    else
    {
      str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    }
    return *this;
  }

#if !defined(BOOST_NO_LONG_LONG)
  record& operator<<(long long v)
  {
    if (rec_)
    {
      (*rec_) << v;
    }
    else
    {
      str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    }
    return *this;
  }

  record& operator<<(unsigned long long v)
  {
    if (rec_)
    {
      (*rec_) << v;
    }
    else
    {
      str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    }
    return *this;
  }
#endif

  record& operator<<(float v)
  {
    if (rec_)
    {
      (*rec_) << v;
    }
    else
    {
      str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    }
    return *this;
  }

  record& operator<<(double v)
  {
    if (rec_)
    {
      (*rec_) << v;
    }
    else
    {
      str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    }
    return *this;
  }

  record& operator<<(long double v)
  {
    if (rec_)
    {
      (*rec_) << v;
    }
    else
    {
      str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    }
    return *this;
  }

  template <typename T>
  record& operator<<(T const* v)
  {
    if (rec_)
    {
      (*rec_) << v;
    }
    else
    {
      str_.append(boost::lexical_cast<strbuf_t>(v).cbegin());
    }
    return *this;
  }

  /// set log meta data
  template <typename T>
  record& operator()(T const& t)
  {
    if (rec_)
    {
      (*rec_)(t);
    }
    else
    {
      boost::amsg::error_code_t ec = boost::amsg::success;
      std::size_t size = boost::amsg::size_of(t, ec);
      if (ec != boost::amsg::success)
      {
        boost::amsg::base_store bs;
        bs.set_error_code(ec);
        throw std::runtime_error(bs.message());
      }

      meta_.reserve(size);
      gce::detail::buffer_ref& buf = meta_.get_buffer_ref();

      boost::amsg::zero_copy_buffer writer(
        buf.get_write_data(), buf.remain_write_size()
        );

      boost::amsg::write(writer, t);
      BOOST_ASSERT(!writer.bad());

      buf.write(writer.write_length());
    }
    return *this;
  }

  template <typename T>
  bool get_meta(T& t)
  {
    gce::detail::buffer_ref& buf = meta_.get_buffer_ref();
    boost::amsg::zero_copy_buffer reader(
      buf.get_read_data(), buf.remain_read_size()
      );

    boost::amsg::read(reader, t);
    if (reader.bad())
    {
      return false;
    }

    buf.read(reader.read_length());
    return true;
  }

  record& operator()(char const* str)
  {
    if (rec_)
    {
      (*rec_)(str);
    }
    else
    {
      boost::uint32_t size = (boost::uint32_t)std::char_traits<char>::length(str);
      (*this)(size);
      meta_.append(str, size);
    }
    return *this;
  }

  bool get_meta(boost::string_ref& str)
  {
    boost::uint32_t size;
    if (get_meta(size))
    {
      gce::detail::buffer_ref& buf = meta_.get_buffer_ref();
      str = boost::string_ref((char const*)buf.get_read_data(), size);
      buf.read(size);
      return true;
    }
    else
    {
      return false;
    }
  }

  record& operator()(bool flag)
  {
    if (rec_)
    {
      (*rec_)(flag);
    }
    else
    {
      boost::uint16_t f = flag ? 1 : 0;
      (*this)(f);
    }
    return *this;
  }

  bool get_meta(bool& flag)
  {
    boost::uint16_t f;
    if (get_meta(f))
    {
      flag = f != 0;
      return true;
    }
    else
    {
      return false;
    }
  }

public:
  /// internal use
  void flush()
  {
    flushed_ = true;
    if (!rec_)
    {
      std::vector<record> rec_list;
      rec_list.push_back(*this);
      (*lg_)(rec_list);
    }
  }

  time_point_t get_timestamp() const
  {
    return ts_;
  }

  bool operator!() const
  {
    if (*lg_)
    {
      return flushed_;
    }
    else
    {
      return !flushed_;
    }
  }

private:
  logger_t* lg_;
  level lv_;
  bool flushed_;

  /// meta data
  gce::detail::cow_buffer<GCE_SMALL_LOG_META_SIZE, GCE_LOG_META_MIN_GROW_SIZE> meta_;

  /// string buffer
  gce::detail::cow_buffer<GCE_SMALL_LOG_SIZE, GCE_LOG_MIN_GROW_SIZE> str_;

  /// timestamp
  time_point_t ts_;

  /// scope's record
  record* rec_;
};

namespace detail
{
class record_pump
{
public:
  explicit record_pump(record& rec)
    : rec_(rec)
  {
  }

  ~record_pump()
  {
    rec_.flush();
  }

public:
  record& get_record()
  {
    return rec_;
  }

private:
  record& rec_;
};
} /// namespace detail
} /// namespace log
} /// namespace gce

#define GCE_LOG(lg, lv) \
  for (gce::log::record rec(lg, lv); !!rec;) \
    gce::log::detail::record_pump(rec).get_record()

#define GCE_DEBUG(lg) GCE_LOG(lg, gce::log::debug)
#define GCE_INFO(lg) GCE_LOG(lg, gce::log::info)
#define GCE_WARN(lg) GCE_LOG(lg, gce::log::warn)
#define GCE_ERROR(lg) GCE_LOG(lg, gce::log::error)
#define GCE_FATAL(lg) GCE_LOG(lg, gce::log::fatal)

#endif /// GCE_LOG_RECORD_HPP
