///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_CONFIG_HPP
#define GCE_ACTOR_CONFIG_HPP

#include <gce/config.hpp>
#include <gce_actor_user.hpp>
#include <gce/integer.hpp>
#include <gce/actor/atom.hpp>
#include <gce/amsg/amsg.hpp>
#include <boost/system/error_code.hpp>
#include <boost/chrono.hpp>
#include <boost/atomic.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/buffer.hpp>
#include <vector>
#include <string>

#define GCE_MAX_MSG_SIZE (GCE_SOCKET_RECV_CACHE_SIZE - GCE_SOCKET_RECV_MAX_SIZE)

namespace gce
{
typedef boost::system::error_code errcode_t;
typedef boost::chrono::seconds seconds_t;
static seconds_t zero(0);
static seconds_t infin(99999999);

typedef boost::uint32_t sid_t;
static boost::uint32_t const sid_nil = static_cast<sid_t>(-1);

typedef boost::uint64_t match_t;
static std::size_t const match_nil = static_cast<boost::int64_t>(-1);
typedef std::vector<match_t> match_list_t;

enum stack_scale_type
{
  stack_default = 1,
  stack_half = 2, /// half of default
  stack_third = 3,
  stack_fourth = 4,

  stack_min = 10000
};

enum link_type
{
  no_link = 0,
  linked,
  monitored
};

typedef boost::asio::io_service io_service_t;
typedef boost::asio::io_service::strand strand_t;
typedef boost::asio::system_timer timer_t;
typedef boost::asio::yield_context yield_t;

typedef match_t exit_code_t;
static exit_code_t const exit_normal = atom("gce_ex_normal");
static exit_code_t const exit_timeout = atom("gce_ex_timeout");
static exit_code_t const exit_except = atom("gce_ex_except");
static exit_code_t const exit_remote = atom("gce_ex_remote");
static exit_code_t const exit_already = atom("gce_ex_already");
static exit_code_t const exit_notify = atom("gce_ex_notify");
static exit_code_t const exit_neterr = atom("gce_ex_neterr");

class context;
namespace detail
{
typedef std::basic_string<byte_t, std::char_traits<byte_t>, std::allocator<byte_t> > bytes_t;
typedef boost::asio::coroutine coro_t;

enum actor_code
{
  actor_normal = 0,
  actor_timeout,
};

static match_t const msg_hb = atom("gce_msg_hb");

struct actor_attrs
{
  actor_attrs()
    : ctx_(0)
    , stack_scale_type_(stack_default)
    , cache_match_size_(16)
  {
  }

  actor_attrs(context* ctx, stack_scale_type sst, std::size_t cache_match_size)
    : ctx_(ctx)
    , stack_scale_type_(sst)
    , cache_match_size_(cache_match_size)
  {
  }

  context* ctx_;
  stack_scale_type stack_scale_type_;
  std::size_t cache_match_size_;
};

struct thin_attrs
{
  thin_attrs()
    : ctx_(0)
    , cache_match_size_(16)
  {
  }

  thin_attrs(context* ctx, std::size_t cache_match_size)
    : ctx_(ctx)
    , cache_match_size_(cache_match_size)
  {
  }

  context* ctx_;
  std::size_t cache_match_size_;
};

template <typename Bool>
struct scope_flag
{
  explicit scope_flag(Bool& flag, Bool val = true)
    : flag_(flag)
  {
    flag_ = val;
  }

  ~scope_flag()
  {
    flag_ = !flag_;
  }

  Bool& flag_;
};

template <>
struct scope_flag<boost::atomic_bool>
{
  explicit scope_flag(
    boost::atomic_bool& flag,
    bool val = true,
    boost::memory_order begin_order = boost::memory_order_release,
    boost::memory_order end_order = boost::memory_order_release
    )
    : flag_(flag)
    , val_(!val)
    , end_order_(end_order)
  {
    flag_.store(val, begin_order);
  }

  ~scope_flag()
  {
    flag_.store(val_, end_order_);
  }

  boost::atomic_bool& flag_;
  bool val_;
  boost::memory_order end_order_;
};
}

struct net_option
{
  net_option()
    : heartbeat_period_(30)
    , heartbeat_count_(3)
    , reconn_period_(5)
    , reconn_count_(3)
    , reconn_try_(3)
  {
  }

  seconds_t heartbeat_period_;
  std::size_t heartbeat_count_;
  seconds_t reconn_period_;
  std::size_t reconn_count_;
  std::size_t reconn_try_;
};
}

#ifndef GCE_REENTER
# define GCE_REENTER(t) BOOST_ASIO_CORO_REENTER(t.coro())
#endif

#ifndef GCE_YIELD
# define GCE_YIELD BOOST_ASIO_CORO_YIELD
#endif

#ifndef GCE_PACK
# define GCE_PACK AMSG
#endif

#endif /// GCE_ACTOR_CONFIG_HPP
