///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_BATCH_SENDER_HPP
#define GCE_ACTOR_DETAIL_BATCH_SENDER_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/messager.hpp>
#include <gce/detail/dynarray.hpp>
#include <vector>

namespace gce
{
namespace detail
{
class batch_sender
{
public:
  explicit batch_sender(std::size_t service_size)
    : msgr_list_(service_size)
    , back_list_(service_size, 0)
  {
    for (std::size_t i=0; i<service_size; ++i)
    {
      msgr_list_.emplace_back();
    }
  }

public:
  messager& get_messager(std::size_t index)
  {
    GCE_VERIFY(index < msgr_list_.size())(index)(msgr_list_.size())
      .msg("get_messager index out of msgr_list");
    return msgr_list_[index];
  }

  pack_list_t* get_pack_list(std::size_t index)
  {
    GCE_VERIFY(index < back_list_.size())(index)(back_list_.size())
      .msg("get_pack_list index out of back_list_");
    return back_list_[index];
  }

  void set_pack_list(std::size_t index, pack_list_t* back_list)
  {
    GCE_VERIFY(index < back_list_.size())(index)(back_list_.size())
      .msg("get_pack_list index out of back_list_");
    back_list_[index] = back_list;
  }

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];
  
  GCE_CACHE_ALIGNED_VAR(detail::dynarray<messager>, msgr_list_)
  GCE_CACHE_ALIGNED_VAR(std::vector<pack_list_t*>, back_list_)
};
}
}

#endif /// GCE_ACTOR_DETAIL_BATCH_SENDER_HPP
