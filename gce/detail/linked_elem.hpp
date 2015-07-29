///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_LINKED_ELEM_HPP
#define GCE_DETAIL_LINKED_ELEM_HPP

#include <gce/config.hpp>

namespace gce
{
namespace detail
{
template <typename T>
class linked_elem
{
public:
  linked_elem()
    : next_(0)
  {
  }

  linked_elem(linked_elem const&)
    : next_(0)
  {
  }

  virtual ~linked_elem()
  {
  }

  void operator=(linked_elem const&)
  {
  }

public:
  T* next_;
};
}
}

#endif /// GCE_DETAIL_LINKED_ELEM_HPP
