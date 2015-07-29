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
#include <gce/detail/dynarray.hpp>
#include <boost/container/deque.hpp>
#include <boost/array.hpp>
#include <vector>

namespace gce
{
namespace detail
{
///----------------------------------------------------------------------------
/// pack_list_t
///----------------------------------------------------------------------------
class pack_list_t
{
public:
  pack_list_t(size_t reserve_size, size_t max_size)
    : que_(reserve_size)
    , size_(0)
    , max_size_(max_size)
  {
  }

  ~pack_list_t()
  {
  }

public:
  bool empty() const
  {
    return size_ == 0;
  }

  size_t size() const
  {
    return size_;
  }

  pack& alloc_pack()
  {
    if (size_ < que_.size())
    {
      return que_[size_++];
    }
    else
    {
      que_.emplace_back();
      ++size_;
      return que_.back();
    }
  }

  void clear()
  {
    if (size_ > max_size_)
    {
      que_.resize(max_size_);
      size_ = max_size_;
    }
    for (size_t i=size_; i>0; --i)
    {
      que_[i - 1].on_free();
    }
    size_ = 0;
  }

  pack& operator[](size_t i)
  {
    return que_[i];
  }

  pack const& operator[](size_t i) const
  {
    return que_[i];
  }

private:
  boost::container::deque<pack> que_;
  size_t size_;
  size_t const max_size_;
};
///----------------------------------------------------------------------------
/// send_pair
///----------------------------------------------------------------------------
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
///----------------------------------------------------------------------------
/// messager
///----------------------------------------------------------------------------
class messager
{
public:
  messager(size_t pack_list_reserve_size, size_t pack_list_max_size)
    : on_back_(false)
    , primary_sending_(false)
    , pack_list_arr_(2)
    , primary_(0)
    , standby_(1)
    , primary_ret_(pack_list_reserve_size, pack_list_max_size)
  {
    for (size_t i=0; i<2; ++i)
    {
      pack_list_arr_.emplace_back(pack_list_reserve_size, pack_list_max_size);
    }
  }

public:
  pack& alloc_pack()
  {
    return pack_list_arr_[standby_].alloc_pack();
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
  dynarray<pack_list_t> pack_list_arr_;
  size_t primary_;
  size_t standby_;
  pack_list_t primary_ret_;
  pack pk_nil_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_MESSAGER_HPP
