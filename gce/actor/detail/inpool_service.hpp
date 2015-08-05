///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_INPOOL_SERVICE_HPP
#define GCE_ACTOR_DETAIL_INPOOL_SERVICE_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/basic_service.hpp>
#include <gce/actor/detail/batch_sender.hpp>
#include <gce/detail/scope.hpp>
#include <boost/foreach.hpp>
#include <boost/ref.hpp>
#include <set>

namespace gce
{
namespace detail
{
template <typename Actor>
class inpool_service
  : public basic_service<typename Actor::context_t>
{
  typedef Actor actor_t;
  typedef typename actor_t::context_t context_t;
  typedef basic_service<context_t> base_t;
  typedef inpool_service<actor_t> self_t;

public:
  inpool_service(context_t& ctx, strand_t& snd, size_t index)
    : base_t(ctx, snd, index, actor_t::get_type())
    , bs_(
        ctx.get_service_size(), 
        ctx.get_attributes().pack_list_reserve_size_, 
        ctx.get_attributes().pack_list_max_size_
        )
    , msg_pool_(ctx.get_msg_pool(index))
    , lg_(ctx.get_logger())
  {
  }

public:
  void on_recv_forth(base_t& sender_svc, send_pair& sp, msg_pool_t::line& ml)
  {
    base_t::snd_.post(handle_recv_way_binder<forth>(*this, sender_svc, sp, ml));
  }

  void on_recv_back(base_t& sender_svc, send_pair& sp, msg_pool_t::line& ml)
  {
    base_t::snd_.dispatch(handle_recv_way_binder<back>(*this, sender_svc, sp, ml));
  }

  void on_recv(pack& pk)
  {
    base_t::snd_.dispatch(handle_recv_binder(*this, pk));
  }

  void on_attach(msg_pool_t::line& ml)
  {
    base_t::snd_.dispatch(handle_attach_binder(*this, ml));
  }

  void handle_tick(pack& pk)
  {
    handle_recv(pk);
  }

  pack& alloc_pack(aid_t const& target)
  {
    if (in_pool(target))
    {
      actor_index ai = get_actor_index(target, base_t::ctxid_, base_t::timestamp_);
      if (ai)
      {
        pack* pk = 0;
        if (is_inpool() && ai.svc_id_ == base_t::index_)
        {
          pk = &base_t::ctx_.alloc_tick_pack(base_t::index_);
        }
        else
        {
          pack_list_t* back_list = bs_.get_pack_list(ai.type_, ai.svc_id_);
          if (back_list != 0)
          {
            //back_list->push_back(pk_nil_);
            pk = &back_list->alloc_pack();
          }
          else
          {
            messager& msgr = bs_.get_messager(ai.type_, ai.svc_id_);
            pk = &msgr.alloc_pack();
          }
        }
        pk->ai_ = ai;
        pk->pmsg_ = msg_pool_.get();
        return *pk;
      }
      else
      {
        //pk_ = pk_nil_;
        pk_.on_free();
        pk_.expiry_ = true;
        return pk_;
      }
    }
    //pk_ = pk_nil_;
    pk_.on_free();
    return pk_;
  }

  virtual void stop()
  {
    base_t::stop();
  }

protected:
  void pri_send(actor_index const& ai, pack&)
  {
    base_t& svc = base_t::ctx_.get_service(ai);
    pack_list_t* back_list = bs_.get_pack_list(ai.type_, ai.svc_id_);
    if (!back_list)
    {
      messager& msgr = bs_.get_messager(ai.type_, ai.svc_id_);
      send_pair sp = msgr.try_forth();
      if (sp)
      {
        msg_pool_t::line ml;
        svc.on_recv_forth(*this, sp, ml);
      }
    }
  }

  bool is_inpool() const
  {
    return true;
  }

  virtual actor_t* find_actor(actor_index const& ai, sid_t sid) = 0;

private:
  struct forth {};
  struct back {};

  template <typename Tag>
  struct handle_recv_way_binder
  {
    handle_recv_way_binder(self_t& self, base_t& sender_svc, send_pair const& sp, msg_pool_t::line& ml)
      : self_(self)
      , sender_svc_(sender_svc)
      , sp_(sp)
      , ml_(ml)
    {
    }

    void operator()()
    {
      invoke(Tag());
    }

  private:
    void invoke(forth)
    {
      self_.handle_recv_forth(sender_svc_, sp_, ml_);
    }

    void invoke(back)
    {
      self_.handle_recv_back(sender_svc_, sp_, ml_);
    }

    self_t& self_;
    base_t& sender_svc_;
    send_pair sp_;
    msg_pool_t::line ml_;
  };

  struct handle_recv_binder
  {
    handle_recv_binder(self_t& self, pack const& pk)
      : self_(self)
      , pk_(pk)
    {
    }

    void operator()()
    {
      self_.handle_recv(pk_);
    }

    self_t& self_;
    pack pk_;
  };

  struct handle_attach_binder
  {
    handle_attach_binder(self_t& self, msg_pool_t::line& ml)
      : self_(self)
      , ml_(ml)
    {
    }
    
    void operator()()
    {
      self_.handle_attach(ml_);
    }

    self_t& self_;
    msg_pool_t::line ml_;
  };

  struct end_handle_recv_forth
  {
    end_handle_recv_forth(
      self_t& self, 
      batch_sender& bs, 
      base_t& sender_svc, 
      send_pair& sp, 
      msg_pool_t::line& ml
      )
      : self_(self)
      , bs_(bs)
      , sender_svc_(sender_svc)
      , sp_(sp)
      , ml_(ml)
    {
    }

    void operator()() const
    {
      bs_.set_pack_list(sender_svc_.get_type(), sender_svc_.get_index(), 0);
      sender_svc_.on_recv_back(self_, sp_, ml_);
    }

    self_t& self_;
    batch_sender& bs_;
    base_t& sender_svc_;
    send_pair& sp_;
    msg_pool_t::line& ml_;
  };

  void handle_recv_forth(base_t& sender_svc, send_pair& sp, msg_pool_t::line& ml)
  {
    /// attach ml first
    msg_pool_.attach(ml);

    /// check svc type and index
    actor_type svc_type = sender_svc.get_type();
    size_t svc_index = sender_svc.get_index();
    GCE_ASSERT(sp)(svc_index);
    GCE_ASSERT(bs_.get_pack_list(svc_type, svc_index) == 0)((int)svc_type)(svc_index);

    /// give back equals num msgs
    pack_list_t* forth_list = sp.forth();
    ml = msg_pool_.detach(forth_list->size());

    scope_handler<end_handle_recv_forth> scp(end_handle_recv_forth(*this, bs_, sender_svc, sp, ml));
    bs_.set_pack_list(svc_type, svc_index, sp.back());
    //BOOST_FOREACH(pack& pk, *forth_list)
    for (size_t i=0, size=forth_list->size(); i<size; ++i)
    {
      try
      {
        pack& pk = (*forth_list)[i];
        recv_pack(pk);
      }
      catch (std::exception& ex)
      {
        GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
      }
    }
  }

  struct end_handle_recv_back
  {
    end_handle_recv_back(
      self_t& self, 
      batch_sender& bs, 
      base_t& sender_svc, 
      send_pair& ret, 
      msg_pool_t::line& ml
      )
      : self_(self)
      , bs_(bs)
      , sender_svc_(sender_svc)
      , ret_(ret)
      , ml_(ml)
    {
    }

    void operator()() const
    {
      messager& msgr = bs_.get_messager(sender_svc_.get_type(), sender_svc_.get_index());
      send_pair sp = msgr.on_handle_back(ret_);
      if (sp)
      {
        sender_svc_.on_recv_forth(self_, sp, ml_);
      }
      else if (ml_)
      {
        sender_svc_.on_attach(ml_);
      }
    }

    self_t& self_;
    batch_sender& bs_;
    base_t& sender_svc_;
    send_pair& ret_;
    msg_pool_t::line& ml_;
  };

  void handle_recv_back(base_t& sender_svc, send_pair& ret, msg_pool_t::line& ml)
  {
    /// attach ml first
    msg_pool_.attach(ml);

    /// check svc type and index
    actor_type svc_type = sender_svc.get_type();
    size_t svc_index = sender_svc.get_index();
    GCE_ASSERT(ret)(svc_index);

    pack_list_t* back_list = ret.back();
    ml = msg_pool_.detach(back_list->size());

    messager& msgr = bs_.get_messager(svc_type, svc_index);
    send_pair sp = msgr.on_back(ret);
    bool sp_valid = sp ? true : false;
    msg_pool_t::line ml_nil;
    scope_handler<end_handle_recv_back> scp(
      end_handle_recv_back(*this, bs_, sender_svc, ret, sp_valid ? ml_nil : ml)
      );
    if (sp_valid)
    {
      sender_svc.on_recv_forth(*this, sp, ml);
    }

    //BOOST_FOREACH(pack& pk, *back_list)
    for (size_t i=0, size=back_list->size(); i<size; ++i)
    {
      try
      {
        pack& pk = (*back_list)[i];
        recv_pack(pk);
      }
      catch (std::exception& ex)
      {
        GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
      }
    }
  }

  void handle_recv(pack& pk)
  {
    try
    {
      recv_pack(pk);
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
    }
  }

  void handle_attach(msg_pool_t::line& ml)
  {
    msg_pool_.attach(ml);
  }

  void recv_pack(pack& pk)
  {
    if (!pk.expiry_)
    {
      actor_t* a = find_actor(pk.ai_, pk.sid_);
      if (a)
      {
        GCE_ASSERT(&a->get_basic_service() == this)
          (pk.ai_.svc_id_)(pk.ai_.type_)(pk.sid_);
        a->on_recv(pk);
      }
      else
      {
        base_t::send_already_exit(pk);
        if (pk.pmsg_ != 0)
        {
          msg_pool_.free(pk.pmsg_);
        }
      }
    }
  }

private:
  GCE_CACHE_ALIGNED_VAR(batch_sender, bs_)

  /// coro local vars
  msg_pool_t& msg_pool_;
  pack pk_;
  pack const pk_nil_;
  log::logger_t& lg_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_INPOOL_SERVICE_HPP
