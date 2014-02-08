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
  actor_id() : ptr_(0), sid_(sid_nil) {}
  actor_id(basic_actor* ptr, sid_t sid) : ptr_(ptr), sid_(sid) {}
  ~actor_id() {}

public:
  inline operator bool() const
  {
    return ptr_ != 0;
  }

  bool operator!() const
  {
    return ptr_ == 0;
  }

  inline bool operator==(actor_id const& rhs) const
  {
    return ptr_ == rhs.ptr_ && sid_ == rhs.sid_;
  }

  inline bool operator!=(actor_id const& rhs) const
  {
    return !(*this == rhs);
  }

  inline bool operator<(actor_id const& rhs) const
  {
    if (ptr_ < rhs.ptr_)
    {
      return true;
    }
    else if (ptr_ > rhs.ptr_)
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
    return ptr_;
  }

  inline sid_t get_sid() const
  {
    return sid_;
  }

private:
  basic_actor* ptr_;
  sid_t sid_;
};

typedef actor_id aid_t;
}

#endif /// GCE_ACTOR_ACTOR_ID_HPP
