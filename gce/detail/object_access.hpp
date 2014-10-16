///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_OBJECT_ACCESS_HPP
#define GCE_DETAIL_OBJECT_ACCESS_HPP

#include <gce/config.hpp>

namespace gce
{
namespace detail
{
class object_access
{
public:
  template <typename T>
  inline static T* get_next(T* obj)
  {
    return obj->next_obj_;
  }

  template <typename T>
  inline static void set_next(T* obj, T* next)
  {
    obj->next_obj_ = next;
  }
};
}
}

#endif /// GCE_DETAIL_OBJECT_ACCESS_HPP
