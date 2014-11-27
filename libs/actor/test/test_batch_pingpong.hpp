///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

namespace gce
{
static std::size_t const msg_size = 100000;
class batch_pingpong_ut
{
public:
  static void run()
  {
    std::cout << "batch_pingpong_ut begin." << std::endl;
    test_common();
    std::cout << "batch_pingpong_ut end." << std::endl;
  }

private:
  class actor;
  typedef actor* aid_t;
  struct pack
  {
    pack()
      : sender_(0)
      , recver_(0)
    {
    }

    aid_t sender_;
    aid_t recver_;
    message msg_;
  };

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
      : primary_sending_(false)
      , secondary_sending_(false)
      , on_back_(false)
      , primary_(0)
      , secondary_(1)
      , standby_(2)
    {
    }

  public:
    pack* alloc_pack()
    {
      pack_list_t& pack_list = pack_list_arr_[standby_];
      pack_list.push_back(pack());
      return &pack_list.back();
    }

    send_pair try_forth()
    {
      send_pair ret;

      if (/*!on_back_ && */!pack_list_arr_[standby_].empty() && !primary_sending_ && !secondary_sending_)
      {
        if (primary_ret_.empty())
        {
          ret = send(primary_, primary_ret_, primary_sending_);
        }
        else if (secondary_ret_.empty())
        {
          ret = send(secondary_, secondary_ret_, secondary_sending_);
        }
      }
      return ret;
    }

    send_pair on_back(send_pair sp)
    {
      BOOST_ASSERT(sp);
      on_back_ = true;
      send_pair ret;
      sp.forth()->clear();

      if (sp.back() == &primary_ret_)
      {
        BOOST_ASSERT(primary_sending_);
        primary_sending_ = false;
        if (!pack_list_arr_[standby_].empty())
        {
          if (primary_ret_.empty())
          {
            ret = send(primary_, primary_ret_, primary_sending_);
          }
          else if (!secondary_sending_)
          {
            ret = send(secondary_, secondary_ret_, secondary_sending_);
          }
        }
      }
      else if (sp.back() == &secondary_ret_)
      {
        BOOST_ASSERT(secondary_sending_);
        secondary_sending_ = false;
        if (!pack_list_arr_[standby_].empty())
        {
          if (secondary_ret_.empty())
          {
            ret = send(secondary_, secondary_ret_, secondary_sending_);
          }
          else if (!primary_sending_)
          {
            ret = send(primary_, primary_ret_, primary_sending_);
          }
        }
      }
      return ret;
    }

    send_pair on_handle_back(send_pair sp)
    {
      BOOST_ASSERT(sp);
      on_back_ = false;
      sp.back()->clear();
      send_pair ret;
      if (!pack_list_arr_[standby_].empty())
      {
        if (!primary_sending_)
        {
          ret = send(primary_, primary_ret_, primary_sending_);
        }
        else if (!secondary_sending_)
        {
          ret = send(secondary_, secondary_ret_, secondary_sending_);
        }
      }
      return ret;
    }

  private:
    send_pair send(std::size_t& index, pack_list_t& ret, bool& sending)
    {
      std::swap(index, standby_);
      sending = true;
      return send_pair(pack_list_arr_[index], ret);
    }

  private:
    bool primary_sending_;
    bool secondary_sending_;
    bool on_back_;
    boost::array<pack_list_t, 3> pack_list_arr_;
    std::size_t primary_;
    std::size_t secondary_;
    std::size_t standby_;
    pack_list_t primary_ret_;
    pack_list_t secondary_ret_;
  };

  typedef boost::function<void (actor*, aid_t, message const&)> recv_handler_t;

  template <typename Actor>
  class service
  {
    typedef Actor actor_t;
    typedef boost::shared_ptr<actor_t> actor_ptr;
    typedef service<actor_t> service_t;
  public:
    service(io_service_t& ios, std::size_t index, std::size_t svc_size)
      : ios_(ios)
      , snd_(ios_)
      , index_(index)
      , svc_size_(svc_size)
      , msgr_list_(svc_size_)
      , back_list_(svc_size_, (pack_list_t*)0)
    {
    }

  public:
    std::size_t get_index() const
    {
      return index_;
    }
    
    strand_t& get_strand()
    {
      return snd_;
    }

    void send(aid_t sender, aid_t aid, message const& m)
    {
      service_t& tgt_svc = aid->get_service();
      std::size_t tgt_index = tgt_svc.get_index();
      pack_list_t* back_list = back_list_[tgt_index];
      if (back_list)
      {
        back_list->push_back(pack());
        pack* pk = &back_list->back();
        pk->sender_ = sender;
        pk->recver_ = aid;
        pk->msg_ = m;
      }
      else
      {
        messager& msgr = msgr_list_[tgt_index];
        pack* pk = msgr.alloc_pack();
        pk->sender_ = sender;
        pk->recver_ = aid;
        pk->msg_ = m;
        
        send_pair sp = msgr.try_forth();
        if (sp)
        {
          tgt_svc.on_recv_forth(this, sp);
        }
      }
    }

    aid_t spawn(recv_handler_t const& h)
    {
      actor_ptr a = boost::make_shared<actor_t>(boost::ref(*this), h);
      snd_.dispatch(boost::bind(&service_t::pri_spawn, this, a));
      return a.get();
    }

  private:
    void on_recv_forth(service_t* svc, send_pair sp)
    {
      snd_.dispatch(boost::bind(&service_t::handle_recv_forth, this, svc, sp));
    }

    void on_recv_back(service_t* svc, send_pair sp)
    {
      snd_.post(boost::bind(&service_t::handle_recv_back, this, svc, sp));
    }

    void end_handle_recv_forth(service_t* svc, send_pair sp)
    {
      back_list_[svc->get_index()] = 0;
      svc->on_recv_back(this, sp);
    }

    void handle_recv_forth(service_t* svc, send_pair sp)
    {
      std::size_t svc_index = svc->get_index();
      BOOST_ASSERT(sp);
      BOOST_ASSERT(back_list_[svc_index] == 0);

      detail::scope scp(
        boost::bind(
          &service_t::end_handle_recv_forth, this, svc, sp
          )
        );
      pack_list_t* forth_list = sp.forth();
      back_list_[svc_index] = sp.back();
      BOOST_FOREACH(pack& pk, *forth_list)
      {
        try
        {
          actor_t* a = (actor_t*)pk.recver_;
          BOOST_ASSERT(&a->get_service() == this);
          a->on_recv(&pk);
        }
        catch (std::exception& ex)
        {
          std::cerr << ex.what() << std::endl;
        }
      }
    }
    
    void end_handle_recv_back(service_t* svc, send_pair ret)
    {
      messager& msgr = msgr_list_[svc->get_index()];
      send_pair sp = msgr.on_handle_back(ret);
      if (sp)
      {
        svc->on_recv_forth(this, sp);
      }
    }

    void handle_recv_back(service_t* svc, send_pair ret)
    {
      BOOST_ASSERT(ret);
      detail::scope scp(
        boost::bind(
          &service_t::end_handle_recv_back, this, svc, ret
          )
        );

      messager& msgr = msgr_list_[svc->get_index()];
      send_pair sp = msgr.on_back(ret);
      if (sp)
      {
        svc->on_recv_forth(this, sp);
      }

      pack_list_t* back_list = ret.back();
      BOOST_FOREACH(pack& pk, *back_list)
      {
        try
        {
          actor_t* a = (actor_t*)pk.recver_;
          BOOST_ASSERT(&a->get_service() == this);
          a->on_recv(&pk);
        }
        catch (std::exception& ex)
        {
          std::cerr << ex.what() << std::endl;
        }
      }
    }

    void pri_spawn(actor_ptr a)
    {
      actor_list_.push_back(a);
    }

  private:
    io_service_t& ios_;
    strand_t snd_;
    std::size_t index_;
    std::size_t svc_size_;
    std::vector<actor_ptr> actor_list_;
    std::vector<messager> msgr_list_;
    std::vector<pack_list_t*> back_list_;
  };

  class actor
  {
    typedef service<actor> service_t;
  public:
    actor(service_t& svc, recv_handler_t h)
      : svc_(svc)
      , h_(h)
    {
    }

    service_t& get_service()
    {
      return svc_;
    }

  public:
    void send(aid_t aid, message const& m)
    {
      svc_.send(this, aid, m);
    }

    void on_recv(pack* pk)
    {
      h_(this, pk->sender_, pk->msg_);
    }

  private:
    service_t& svc_;
    recv_handler_t h_;
  };

  static void ping(actor* self, aid_t sender, message const& msg, std::size_t& i)
  {
    if (--i < msg_size)
    {
      self->send(sender, msg);
    }
  }

  static void pong(actor* self, aid_t sender, message const& msg)
  {
    self->send(sender, msg);
  }

  static void start(aid_t ping_aid, aid_t pong_aid)
  {
    for (std::size_t i=0; i<10; ++i)
    {
      ping_aid->send(pong_aid, message());
    }
  }

  static void test_common()
  {
    try
    {
      io_service_t ios;
      typedef service<actor> service_t;
      std::size_t svc_size = 3;
      boost::container::deque<service_t> svc_list;
      for (std::size_t i=0; i<svc_size; ++i)
      {
        svc_list.emplace_back(boost::ref(ios), i, svc_size);
      }

      aid_t ping_aid = 
        svc_list[0].spawn(
          boost::bind(
            &batch_pingpong_ut::ping, 
            _1, _2, _3, msg_size
            )
          );
      aid_t pong_aid = 
        svc_list[1].spawn(
          boost::bind(
            &batch_pingpong_ut::pong, _1, _2, _3
            )
          );

      svc_list[0].get_strand().post(
        boost::bind(
          &batch_pingpong_ut::start, ping_aid, pong_aid
          )
        );

      boost::timer::auto_cpu_timer t;
      boost::thread thr(boost::bind(&io_service_t::run, &ios));
      ios.run();
      thr.join();
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
