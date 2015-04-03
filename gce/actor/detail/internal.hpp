///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_INTERNAL_HPP
#define GCE_ACTOR_DETAIL_INTERNAL_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/match.hpp>
#include <gce/actor/detail/internal.adl.h>

namespace gce
{
namespace detail
{
typedef gce::adl::detail::header header_t;
header_t make_header(
  uint32_t size = 0, 
  gce::match_t type = gce::match_nil, 
  uint32_t tag_offset = gce::u32_nil
  )
{
  header_t o;
  o.size_ = size;
  o.type_ = type;
  o.tag_offset_ = tag_offset;
  return o;
}
}
}

GCE_PACK(gce::detail::header_t, (v.size_)(v.type_)(v.tag_offset_));

#endif /// GCE_ACTOR_DETAIL_INTERNAL_HPP
