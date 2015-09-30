///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DURATION_HPP
#define GCE_ACTOR_DURATION_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/duration.adl.h>
#include <gce/actor/chrono.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

namespace gce
{
enum dur_type
{
  dur_raw = 0,
  dur_microsec,
  dur_millisec,
  dur_second,
  dur_minute,
  dur_hour,
};

namespace detail
{
inline gce::adl::duration make_dur(int64_t val, dur_type ty)
{
  gce::adl::duration o;
  o.val_ = val;
  o.ty_ = ty;
  return o;
}

inline gce::adl::duration make_zero()
{
  gce::adl::duration o;
  o.val_ = sysclock_t::duration::zero().count();
  o.ty_ = dur_raw;
  return o;
}

inline gce::adl::duration make_infin()
{
  gce::adl::duration o;
  o.val_ = (sysclock_t::duration::max)().count();
  o.ty_ = dur_raw;
  return o;
}
}

/// duration zero value
static adl::duration const zero = detail::make_zero();

/// duration infin value
static adl::duration const infin = detail::make_infin();

inline adl::duration duration(int64_t v)
{
  return detail::make_dur(v, dur_raw);
}

inline adl::duration microsecs(int64_t v)
{
  return detail::make_dur(v, dur_microsec);
}

inline adl::duration millisecs(int64_t v)
{
  return detail::make_dur(v, dur_millisec);
}

inline adl::duration seconds(int64_t v)
{
  return detail::make_dur(v, dur_second);
}

inline adl::duration minutes(int64_t v)
{
  return detail::make_dur(v, dur_minute);
}

inline adl::duration hours(int64_t v)
{
  return detail::make_dur(v, dur_hour);
}

inline adl::duration from_chrono(sysclock_t::duration const& dur)
{
  return duration(dur.count());
}

inline adl::duration from_chrono(boost::chrono::milliseconds const& dur)
{
  return millisecs(dur.count());
}

inline adl::duration from_chrono(boost::chrono::microseconds const& dur)
{
  return microsecs(dur.count());
}

inline adl::duration from_chrono(boost::chrono::seconds const& dur)
{
  return seconds(dur.count());
}

inline adl::duration from_chrono(boost::chrono::minutes const& dur)
{
  return minutes(dur.count());
}

inline adl::duration from_chrono(boost::chrono::hours const& dur)
{
  return hours(dur.count());
}

inline sysclock_t::duration to_chrono(adl::duration const& dur)
{
  switch(dur.ty_)
  {
  case dur_microsec: return boost::chrono::microseconds(dur.val_);
  case dur_millisec: return boost::chrono::milliseconds(dur.val_);
  case dur_second: return boost::chrono::seconds(dur.val_);
  case dur_minute: return boost::chrono::minutes(dur.val_);
  case dur_hour: return boost::chrono::hours(dur.val_);
  default: return sysclock_t::duration(dur.val_);
  }
}

namespace detail
{
inline int64_t to_raw_val(adl::duration const& dur)
{
  static int64_t microsec_den = 
    sysclock_t::duration::period::den / BOOST_RATIO_INTMAX_C(1000000);
  static int64_t millisec_den = 
    sysclock_t::duration::period::den / BOOST_RATIO_INTMAX_C(1000);
  static int64_t second_den = 
    sysclock_t::duration::period::den;
  static int64_t minute_den = 
    sysclock_t::duration::period::den * BOOST_RATIO_INTMAX_C(60);
  static int64_t hour_den = 
    sysclock_t::duration::period::den * BOOST_RATIO_INTMAX_C(3600);
  switch(dur.ty_)
  {
  case dur_microsec: return dur.val_ * microsec_den;
  case dur_millisec: return dur.val_ * millisec_den;
  case dur_second: return dur.val_ * second_den;
  case dur_minute: return dur.val_ * minute_den;
  case dur_hour: return dur.val_ * hour_den;
  default: return dur.val_;
  }
}
}

inline adl::duration to_raw(adl::duration dur)
{
  return duration(detail::to_raw_val(dur));
}

namespace detail
{
inline int64_t from_raw_val(int64_t dur, dur_type ty)
{
  static int64_t microsec_den = 
    sysclock_t::duration::period::den / BOOST_RATIO_INTMAX_C(1000000);
  static int64_t millisec_den = 
    sysclock_t::duration::period::den / BOOST_RATIO_INTMAX_C(1000);
  static int64_t second_den = 
    sysclock_t::duration::period::den;
  static int64_t minute_den = 
    sysclock_t::duration::period::den * BOOST_RATIO_INTMAX_C(60);
  static int64_t hour_den = 
    sysclock_t::duration::period::den * BOOST_RATIO_INTMAX_C(3600);

  switch(ty)
  {
  case dur_microsec: return dur / microsec_den;
  case dur_millisec: return dur / millisec_den;
  case dur_second: return dur / second_den;
  case dur_minute: return dur / minute_den;
  case dur_hour: return dur / hour_den;
  default: return dur;
  }
}

inline int64_t from_raw(adl::duration const& dur, dur_type ty)
{
  GCE_ASSERT(dur.ty_ == dur_raw)(dur);
  return from_raw_val(dur.val_, ty);
}
}

inline adl::duration from_raw(adl::duration const& dur, dur_type ty)
{
  adl::duration rt;
  rt.ty_ = ty;
  rt.val_ = detail::from_raw(dur, ty);
  return rt;
}

inline adl::duration from_raw(int64_t dur, dur_type ty)
{
  adl::duration rt;
  rt.ty_ = ty;
  rt.val_ = detail::from_raw_val(dur, ty);
  return rt;
}

namespace adl
{
inline bool operator==(duration const& lhs, duration const& rhs)
{
  if (lhs.ty_ == rhs.ty_)
  {
    return lhs.val_ == rhs.val_;
  }
  return gce::detail::to_raw_val(lhs) == gce::detail::to_raw_val(rhs);
}

inline bool operator!=(duration const& lhs, duration const& rhs)
{
  return !(lhs == rhs);
}

inline bool operator<(duration const& lhs, duration const& rhs)
{
  return gce::detail::to_raw_val(lhs) < gce::detail::to_raw_val(rhs);
}

inline bool operator>(duration const& lhs, duration const& rhs)
{
  return gce::detail::to_raw_val(lhs) > gce::detail::to_raw_val(rhs);
}

inline bool operator>=(duration const& lhs, duration const& rhs)
{
  return !(lhs < rhs);
}

inline bool operator<=(duration const& lhs, duration const& rhs)
{
  return !(lhs > rhs);
}

inline duration operator+(duration const& lhs, duration const& rhs)
{
  dur_type ty = (dur_type)((std::min)(lhs.ty_, rhs.ty_));
  return from_raw(gce::detail::to_raw_val(lhs) + gce::detail::to_raw_val(rhs), ty);
}

inline duration operator-(duration const& lhs, duration const& rhs)
{
  dur_type ty = (dur_type)((std::min)(lhs.ty_, rhs.ty_));
  return from_raw(gce::detail::to_raw_val(lhs) - gce::detail::to_raw_val(rhs), ty);
}

inline duration operator*(duration const& lhs, duration const& rhs)
{
  dur_type ty = (dur_type)((std::min)(lhs.ty_, rhs.ty_));
  return from_raw(gce::detail::to_raw_val(lhs) * gce::detail::to_raw_val(rhs), ty);
}

inline duration operator/(duration const& lhs, duration const& rhs)
{
  dur_type ty = (dur_type)((std::min)(lhs.ty_, rhs.ty_));
  return from_raw(gce::detail::to_raw_val(lhs) / gce::detail::to_raw_val(rhs), ty);
}
}

inline std::string to_string(gce::adl::duration const& o)
{
  std::string str;
  str += "dur<";
  str += boost::lexical_cast<intbuf_t>(o.val_).cbegin();
  str += ".";
  str += boost::lexical_cast<intbuf_t>((int)o.ty_).cbegin();
  str += ">";
  return str;
}

template <>
struct tostring<adl::duration>
{
  static std::string convert(adl::duration const& o)
  {
    return to_string(o);
  }
};

typedef adl::duration duration_t;
} /// namespace gce

inline std::ostream& operator<<(std::ostream& strm, gce::duration_t const& dur)
{
  strm << gce::to_string(dur);
  return strm;
}

GCE_PACK(gce::duration_t, (v.val_)(v.ty_));

#endif /// GCE_ACTOR_DURATION_HPP
