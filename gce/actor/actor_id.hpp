///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_ACTOR_ID_HPP
#define GCE_ACTOR_ACTOR_ID_HPP

#include <gce/actor/config.hpp>

namespace gce
{
class basic_actor;
class actor_id
{
public:
  actor_id() : uintptr_(0), sid_(sid_nil) {}
  actor_id(basic_actor* ptr, sid_t sid) : uintptr_((boost::uint64_t)ptr), sid_(sid) {}
  ~actor_id() {}

public:
  inline operator bool() const
  {
    return uintptr_ != 0;
  }

  bool operator!() const
  {
    return uintptr_ == 0;
  }

  inline bool operator==(actor_id const& rhs) const
  {
    return uintptr_ == rhs.uintptr_ && sid_ == rhs.sid_;
  }

  inline bool operator!=(actor_id const& rhs) const
  {
    return !(*this == rhs);
  }

  inline bool operator<(actor_id const& rhs) const
  {
    if (uintptr_ < rhs.uintptr_)
    {
      return true;
    }
    else if (uintptr_ > rhs.uintptr_)
    {
      return false;
    }

    if (sid_ < rhs.sid_)
    {
      return true;
    }
    else if (sid_ > rhs.sid_)
    {
      return false;
    }

    return false;
  }

  inline basic_actor* get_actor_ptr() const
  {
    return (basic_actor*)uintptr_;
  }

  inline sid_t get_sid() const
  {
    return sid_;
  }

  boost::uint64_t uintptr_;
  sid_t sid_;
};

typedef actor_id aid_t;
}

GCE_PACK(gce::aid_t, (uintptr_)(sid_));

#endif /// GCE_ACTOR_ACTOR_ID_HPP
