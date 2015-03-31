///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_MESSAGER_HPP
#define GCE_ACTOR_DETAIL_MESSAGER_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/pack.hpp>
#include <boost/array.hpp>
#include <deque>
#include <vector>

namespace gce
{
namespace detail
{
typedef std::deque<pack> pack_list_t;
class send_pair
{
public:
  send_pair()
    : forth_(0)
    , back_(0)
  {
  }

  send_pair(pack_list_t& forth, pack_list_t& back)
    : forth_(&forth)
    , back_(&back)
  {
  }

  operator bool() const
  {
    return forth_ != 0 && back_ != 0;
  }

  pack_list_t* forth() const
  {
    return forth_;
  }

  pack_list_t* back() const
  {
    return back_;
  }

private:
  pack_list_t* forth_;
  pack_list_t* back_;
};

class messager
{
public:
  messager()
    : on_back_(false)
    , primary_sending_(false)
    , primary_(0)
    , standby_(1)
  {
    /*pack_list_arr_[0].reserve(1000);
    pack_list_arr_[1].reserve(1000);
    primary_ret_.reserve(1000);*/
  }

public:
  pack& alloc_pack()
  {
    pack_list_t& pack_list = pack_list_arr_[standby_];
    pack_list.push_back(nil_pk_);
    return pack_list.back();
  }

  send_pair try_forth()
  {
    send_pair ret;

    if (!on_back_ && !pack_list_arr_[standby_].empty() && !primary_sending_)
    {
      if (primary_ret_.empty())
      {
        ret = send(primary_, primary_ret_, primary_sending_);
      }
    }
    return ret;
  }

  send_pair on_back(send_pair sp)
  {
    GCE_ASSERT(sp);
    send_pair ret;
    on_back_ = true;
    sp.forth()->clear();

    if (sp.back() == &primary_ret_)
    {
      GCE_ASSERT(primary_sending_);
      primary_sending_ = false;
    }
    return ret;
  }

  send_pair on_handle_back(send_pair sp)
  {
    GCE_ASSERT(sp);
    on_back_ = false;
    sp.back()->clear();
    send_pair ret;
    if (!pack_list_arr_[standby_].empty())
    {
      if (!primary_sending_)
      {
        ret = send(primary_, primary_ret_, primary_sending_);
      }
    }
    return ret;
  }

private:
  send_pair send(size_t& index, pack_list_t& ret, bool& sending)
  {
    std::swap(index, standby_);
    sending = true;
    return send_pair(pack_list_arr_[index], ret);
  }

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  bool on_back_;
  bool primary_sending_;
  boost::array<pack_list_t, 2> pack_list_arr_;
  size_t primary_;
  size_t standby_;
  pack_list_t primary_ret_;
  pack nil_pk_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_MESSAGER_HPP
