///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_ACCEPTOR_ACTOR_HPP
#define GCE_ACTOR_DETAIL_ACCEPTOR_ACTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/pattern.hpp>
#include <gce/actor/net_option.hpp>
#include <gce/actor/exception.hpp>
#include <gce/actor/detail/socket_actor.hpp>
#include <gce/actor/detail/network_service.hpp>
#include <gce/actor/detail/basic_actor.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/actor/detail/actor_function.hpp>
#include <gce/actor/detail/send.hpp>
#include <gce/actor/detail/tcp/acceptor.hpp>
#include <boost/optional.hpp>
#include <vector>

namespace gce
{
namespace detail
{
template <typename Context>
class acceptor_actor
  : public basic_actor<Context>
{
public:
  typedef Context context_t;

private:
  typedef basic_actor<context_t> base_t;
  typedef acceptor_actor<context_t> self_t;
  typedef typename context_t::socket_actor_t socket_actor_t;
  typedef network_service<self_t> service_t;
  typedef typename context_t::socket_service_t socket_service_t;
  typedef std::pair<match_t, remote_func<context_t> > remote_func_t;
  typedef std::map<match_t, remote_func<context_t> > remote_func_list_t;

  enum status
  {
    ready = 0,
    on,
    off,
  };

public:
  acceptor_actor(aid_t aid, service_t& svc)
    : base_t(svc.get_context(), svc, actor_acceptor, aid)
    , stat_(ready)
    , host_(this)
    , svc_(svc)
    , lg_(base_t::ctx_.get_logger())
  {
  }

  ~acceptor_actor()
  {
  }

public:
  void init(netopt_t opt)
  {
    GCE_ASSERT(stat_ == ready)(stat_).log(lg_, "acceptor_actor status error");
    opt_ = opt;
  }

  void bind(
    aid_t const& sire, 
    std::vector<remote_func_t> const& remote_func_list, 
    std::string const& ep
    )
  {
    BOOST_FOREACH(remote_func_t const& f, remote_func_list)
    {
      remote_func_list_.insert(std::make_pair(f.first, f.second));
    }

    boost::asio::spawn(
      base_t::snd_,
      run_binder(*this, sire, ep), 
      /*boost::bind(
        &self_t::run, this, sire, ep, _arg1
        ),*/
      boost::coroutines::attributes(default_stacksize())
      );

    if (stat_ != off)
    {
      base_t::ctx_.on_tick(svc_.get_index());
    }
  }

public:
  typedef gce::acceptor type;

  static actor_type get_type()
  {
    return actor_acceptor;
  }

  static size_t get_pool_reserve_size(attributes const& attr)
  {
    return attr.acceptor_pool_reserve_size_;
  }

  static size_t get_pool_max_size(attributes const& attr)
  {
    return attr.acceptor_pool_max_size_;
  }

  void stop()
  {
    close();
  }

  void on_recv(pack& pk)
  {
    handle_recv(pk);
  }

  void on_addon_recv(pack& pk)
  {
    base_t::snd_.dispatch(handle_recv_binder(*this, pk));
  }

  void send(aid_t const& recver, message const& m)
  {
    base_t::pri_send(recver, m);
  }

  void link(aid_t const&) {}
  void monitor(aid_t const&) {}

private:
  struct run_binder
  {
    run_binder(self_t& self, aid_t const& sire, std::string const& ep)
      : self_(self)
      , sire_(sire)
      , ep_(ep)
    {
    }

    void operator()(yield_t yld) const
    {
      self_.run(sire_, ep_, yld);
    }

    self_t& self_;
    aid_t const sire_;
    std::string const ep_;
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

  struct resume_binder
  {
    explicit resume_binder(self_t& self)
      : self_(self)
    {
    }

    void operator()() const
    {
      self_.resume();
    }

    self_t& self_;
  };

  struct host
    : public yielder::basic_host
  {
    explicit host(self_t* self)
      : self_(self)
    {
    }

    void yield()
    {
      self_->yield();
    }

    void resume()
    {
      self_->snd_.dispatch(resume_binder(*self_));
    }

    self_t* self_;
  };

  typedef boost::asio::detail::async_result_init<yield_t, void ()> async_result_init_t;

  void resume()
  {
    GCE_ASSERT(yld_cb_);
    yld_cb_();

    base_t::ctx_.on_tick(svc_.get_index());
  }

  void yield()
  {
    GCE_ASSERT(yld_);
    async_result_init_t init(BOOST_ASIO_MOVE_CAST(yield_t)(*yld_));

    yld_cb_ = init.handler;
    return init.result.get();
  }

private:
  void send_ret(aid_t const& sire)
  {
    gce::detail::send(*this, sire, msg_new_bind);
  }

  struct send_ret_binder
  {
    send_ret_binder(self_t& self, aid_t const& sire)
      : self_(self)
      , sire_(sire)
    {
    }

    void operator()() const
    {
      self_.send_ret(sire_);
    }

    self_t& self_;
    aid_t const& sire_;
  };

  void bind(std::string const& ep)
  {
    make_acceptor(ep);
    acpr_->bind();
  }

  struct spawn_socket_binder
  {
    spawn_socket_binder(
      self_t& self, 
      socket_service_t& svc, 
      remote_func_list_t const& remote_func_list, 
      socket_ptr prot
      )
      : self_(self)
      , svc_(svc)
      , remote_func_list_(remote_func_list)
      , prot_(prot)
    {
    }

    void operator()() const
    {
      self_.spawn_socket(svc_, remote_func_list_, prot_);
    }

    self_t& self_;
    socket_service_t& svc_;
    remote_func_list_t const remote_func_list_;
    socket_ptr prot_;
  };

  void run(aid_t const& sire, std::string const& ep, yield_t yld)
  {
    yld_ = boost::in_place(yld);
    exit_code_t exc = exit_normal;
    std::string exit_msg("exit normal");

    if (!svc_.stopped())
    {
      svc_.add_actor(this);

      try
      {
        stat_ = on;
        {
          scope scp(send_ret_binder(*this, sire));
          bind(ep);
        }

        typedef boost::chrono::system_clock system_clock_t;
        typedef system_clock_t::time_point time_point_t;
        boost::asio::system_timer tmr(base_t::get_context().get_io_service());
        time_point_t const min_tp = (time_point_t::min)();
        time_point_t last_tp = min_tp;
        int curr_rebind_num = 0;

        yielder ylder(host_);
        while (stat_ == on)
        {
          errcode_t ec;
          ylder[ec];
          socket_ptr prot = acpr_->accept(ylder);
          if (ec)
          {
            if (stat_ == off)
            {
              close();
              break;
            }
            else
            {
              GCE_ASSERT(stat_ == on)(stat_);
              ++curr_rebind_num;
              if (curr_rebind_num <= opt_.rebind_try)
              {
                if (last_tp != min_tp)
                {
                  duration_t diff = from_chrono(system_clock_t::now() - last_tp);
                  if (diff < opt_.rebind_period)
                  {
                    errcode_t ignored_ec;
                    tmr.expires_from_now(to_chrono(opt_.rebind_period - diff));
                    ylder[ignored_ec];
                    tmr.async_wait(ylder);
                    ylder.yield();
                  }
                }
                bind(ep);
                last_tp = system_clock_t::now();
                continue;
              }
              else
              {
                close();
                break;
              }
            }
          }
          else if (curr_rebind_num > 0)
          {
            curr_rebind_num = 0;
            last_tp = min_tp;
          }

          socket_service_t& svc = 
            base_t::ctx_.select_service<socket_service_t>();
          svc.get_strand().post(spawn_socket_binder(*this, svc, remote_func_list_, prot));
        }
      }
      catch (std::exception& ex)
      {
        exc = exit_except;
        exit_msg = ex.what();
        GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
        close();
      }
      catch (...)
      {
        exc = exit_except;
        exit_msg = boost::current_exception_diagnostic_information();
        GCE_ERROR(lg_)(__FILE__)(__LINE__) << exit_msg;
        close();
      }
    }
    else
    {
      gce::detail::send(*this, sire, msg_new_bind);
    }
    quit(exc, exit_msg);
  }

  void spawn_socket(
    socket_service_t& svc, 
    remote_func_list_t const& remote_func_list, 
    socket_ptr prot
    )
  {
    socket_actor_t* s = svc.make_actor();
    s->init(opt_);
    s->start(remote_func_list, prot, opt_.is_router != 0);
  }

  void make_acceptor(std::string const& ep)
  {
    /// Find protocol name
    size_t pos = ep.find("://");
    GCE_VERIFY(pos != std::string::npos)(ep)
      .log(lg_, "protocol name parse failed");

    std::string prot_name = ep.substr(0, pos);
    if (prot_name == "tcp")
    {
      /// Parse address
      size_t begin = pos + 3;
      pos = ep.find(':', begin);
      GCE_VERIFY(pos != std::string::npos)(ep)
        .log(lg_, "tcp address parse failed");

      std::string address = ep.substr(begin, pos - begin);

      /// Parse port
      begin = pos + 1;
      pos = ep.size();

      uint16_t port =
        boost::lexical_cast<uint16_t>(
          ep.substr(begin, pos - begin)
          );
      acpr_ = boost::in_place(boost::ref(base_t::snd_), address, port);
    }
    else
    {
      GCE_VERIFY(false)(prot_name)
        .log(lg_, "gce::unsupported_protocol_exception")
        .except<unsupported_protocol_exception>();
      // just suppress vc's warning
      throw 1;
    }
  }

  void handle_recv(pack& pk)
  {
    if (exit_t* ex = boost::get<exit_t>(&pk.tag_))
    {
      base_t::remove_link(ex->get_aid());
    }

    if (pk.pmsg_ != 0)
    {
      base_t::free_msg(pk.pmsg_);
    }
  }

private:
  void close()
  {
    stat_ = off;
    if (acpr_)
    {
      acpr_->close();
    }
  }

  void quit(exit_code_t exc, std::string const& exit_msg)
  {
    acpr_.reset();

    svc_.remove_actor(this);
    aid_t self_aid = base_t::get_aid();
    base_t::send_exit(self_aid, exc, exit_msg);
    base_t::snd_.post(free_self_binder(*this));
  }

  struct free_self_binder
  {
    explicit free_self_binder(self_t& self)
      : self_(self)
    {
    }

    void operator()() const
    {
      self_.free_self();
    }

    self_t& self_;
  };

  void free_self()
  {
    svc_.free_actor(this);
    base_t::ctx_.on_tick(svc_.get_index());
  }

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(status, stat_)
  GCE_CACHE_ALIGNED_VAR(netopt_t, opt_)

  /// coro local vars
  host host_;
  service_t& svc_;
  boost::optional<tcp::acceptor> acpr_;
  remote_func_list_t remote_func_list_;
  log::logger_t& lg_;

  typedef boost::function<void ()> yield_cb_t;
  boost::optional<yield_t> yld_;
  yield_cb_t yld_cb_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_ACCEPTOR_ACTOR_HPP
