///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_BASIC_ADDON_HPP
#define GCE_ACTOR_DETAIL_BASIC_ADDON_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/actor/detail/basic_service.hpp>
#include <gce/actor/asio.hpp>

namespace gce
{
namespace detail
{
template <typename Context>
class basic_addon
{
  typedef Context context_t;
  typedef basic_service<context_t> service_t;

public:
  template <typename Actor>
  explicit basic_addon(Actor& a)
    : aid_(make_aid(a))
    , target_(a.get_aid())
    , a_(a.get_listener())
    , svc_(a.get_service())
    , snd_(svc_.get_strand())
  {
  }
  
  virtual ~basic_addon()
  {
  }

public:
  aid_t const& get_aid() const
  {
    return aid_;
  }
  
  void send(message const& m)
  {
    pack pk;
    pk.tag_ = get_aid();
    pk.recver_ = target_;
    pk.skt_ = target_;
    pk.msg_ = m;
    svc_.send(a_, pk);
  }
  
  strand_t& get_strand()
  {
    return snd_;
  }
  
private:
  template <typename Actor>
  aid_t make_aid(Actor& a)
  {
    aid_t aid;
    aid_t target = a.get_aid();
    aid.ctxid_ = target.ctxid_;
    aid.timestamp_ = target.timestamp_;
    aid.uintptr_ = (uint64_t)this;
    aid.svc_id_ = 0;
    aid.type_ = (byte_t)actor_addon;
    aid.in_pool_ = 0;
    aid.sid_ = 0;
    return aid;
  }
  
private:
  aid_t aid_;
  aid_t target_;
  listener* a_;
  service_t& svc_;
  strand_t& snd_;
};
}
}

#endif // GCE_ACTOR_DETAIL_BASIC_ADDON_HPP
