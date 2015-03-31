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
  typedef dynarray<messager> msgr_arr_t;
  typedef std::vector<pack_list_t*> pack_list_arr_t;
public:
  explicit batch_sender(size_t service_size)
    : msgr_list_((size_t)actor_num)
    , back_list_((size_t)actor_num)
  {
    for (size_t i=0, size=(size_t)actor_num; i<size; ++i)
    {
      msgr_list_.emplace_back(service_size);
      msgr_arr_t& msgr_arr = msgr_list_.back();
      for (size_t j=0; j<service_size; ++j)
      {
        msgr_arr.emplace_back();
      }
    }

    for (size_t i=0, size=back_list_.size(); i<size; ++i)
    {
      pack_list_arr_t& arr = back_list_[i];
      arr.resize(service_size, 0);
    }
  }

public:
  messager& get_messager(actor_type type, size_t index)
  {
    GCE_VERIFY(index < msgr_list_[type].size())((int)type)(index)(msgr_list_[type].size())
      .msg("get_messager index out of msgr_list");
    return msgr_list_[type][index];
  }

  pack_list_t* get_pack_list(actor_type type, size_t index)
  {
    GCE_VERIFY(index < back_list_[type].size())((int)type)(index)(back_list_[type].size())
      .msg("get_pack_list index out of back_list_");
    return back_list_[type][index];
  }

  void set_pack_list(actor_type type, size_t index, pack_list_t* back_list)
  {
    GCE_VERIFY(index < back_list_[type].size())((int)type)(index)(back_list_[type].size())
      .msg("get_pack_list index out of back_list_");
    back_list_[type][index] = back_list;
  }

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];
  
  GCE_CACHE_ALIGNED_VAR(dynarray<msgr_arr_t>, msgr_list_)
  GCE_CACHE_ALIGNED_VAR(std::vector<pack_list_arr_t>, back_list_)
};
}
}

#endif /// GCE_ACTOR_DETAIL_BATCH_SENDER_HPP
