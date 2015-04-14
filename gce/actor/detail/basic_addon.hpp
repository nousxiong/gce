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
#include <gce/detail/ref_count.hpp>
#include <boost/noncopyable.hpp>

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
  template <typename T>
  class guard
    : public ref_count
  {
  public:
    explicit guard(T* p)
      : ref_count(boost::bind(&guard<T>::free, this))
      , p_(p)
    {
      GCE_ASSERT(p_ != 0);
    }

  public:
    void notify()
    {
      p_ = 0;
    }

    T* get() const
    {
      return p_;
    }

    void free()
    {
      delete this;
    }

  private:
    T* p_;
  };

  template <typename T>
  class scope
    : private boost::noncopyable
  {
  public:
    typedef guard<T> guard_t;
    typedef boost::intrusive_ptr<guard_t> guard_ptr;

  public:
    explicit scope(T* t)
      : guard_(new guard_t(t))
    {
    }

    ~scope()
    {
      guard_->notify();
    }

  public:
    guard_ptr get()
    {
      return guard_;
    }

  private:
    guard_ptr guard_;
  };

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
  
  basic_addon(aid_t const& target, listener* a, service_t& svc)
    : aid_(make_aid(target))
    , target_(target)
    , a_(a)
    , svc_(svc)
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
  
protected:
  aid_t const& get_target()
  {
    return target_;
  }
  
  listener* get_listener()
  {
    return a_;
  }
  
  service_t& get_service()
  {
    return svc_;
  }
  
private:
  template <typename Actor>
  aid_t make_aid(Actor& a)
  {
    return make_aid(a.get_aid());
  }
  
  aid_t make_aid(aid_t const& target)
  {
    aid_t aid;
    aid_t target = target;
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
