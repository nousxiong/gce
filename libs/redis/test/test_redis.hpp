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
class redis_ut
{
typedef boost::asio::ip::tcp::resolver tcp_resolver_t;
static size_t const pubsize = 6;
public:
  static void run()
  {
    std::cout << "redis_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "redis_ut end." << std::endl;
  }

private:
  static int64_t get_uid(redis::snid_t snid, size_t index)
  {
    return snid.val_ * 10 + index;
  }

  static void standalone(stackful_actor self)
  {
    redis::ctxid_t ctxid;
    size_t index;
    boost::shared_ptr<tcp_resolver_t::iterator> eitr;
    aid_t base_aid = self->match("init").recv(index, ctxid, eitr);

    redis::errno_t errn = redis::errno_nil;
    std::string errmsg;
    redis::result_ptr res;
    std::string key_prefix(boost::lexical_cast<std::string>(index));
    key_prefix += "#";

    redis::conn_t c = redis::make_conn(ctxid, *eitr);
    redis::session redsn(self, c);

    redsn.open();
    self->match(redis::sn_open).recv(errn, errmsg);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);

    /// set key value
    redsn.query("SET", key_prefix+"key", "value");
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->string() == "OK");

    /// get key
    redsn.query("GET", key_prefix+"key");
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->bulkstr() == "value");
    
    /// hset key
    redsn.query("HSET", key_prefix+"hkey", "hfield", 1);
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->type() == resp::ty_integer);

    /// set binary data
    redsn.query("SET", key_prefix+"base_aid", base_aid);
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->string() == "OK");

    /// get binary data
    redsn.query("GET", key_prefix+"base_aid");
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    aid_t aid;
    GCE_VERIFY(res->get(aid) == base_aid);

    /// pipeline
    redsn.query("SET", key_prefix+"key", "value");
    redsn.query("GET", key_prefix+"key");
    /// SET
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->string() == "OK");
    /// GET
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->bulkstr() == "value");

    /// array
    redsn.query("DEL", key_prefix+"mylist");
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);

    redsn
      .cmd("RPUSH", key_prefix+"mylist", "one")
      .cmd("RPUSH", key_prefix+"mylist").args("two")
      .cmd("RPUSH").args(key_prefix+"mylist", "three")
      .cmd("LRANGE").args(key_prefix+"mylist", 0).args(2)
      .execute();
    /// RPUSH
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->integer() == 1);
    /// RPUSH
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->integer() == 2);
    /// RPUSH
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->integer() == 3);
    /// LRANGE
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    resp::unique_array<resp::unique_value> const& arr = res->array();
    GCE_VERIFY(arr[0].bulkstr() == "one");
    GCE_VERIFY(arr[1].bulkstr() == "two");
    GCE_VERIFY(arr[2].bulkstr() == "three");
  }

  static void shared(stackful_actor self)
  {
    size_t index;
    redis::conn_t c;
    aid_t base_aid = self->match("init").recv(index, c);

    redis::errno_t errn = redis::errno_nil;
    std::string errmsg;
    redis::result_ptr res;
    std::string key_prefix(boost::lexical_cast<std::string>(index));
    key_prefix += "#";

    redis::session redsn(self, c);

    redsn.open();
    self->match(redis::sn_open).recv(errn, errmsg);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);

    /// set key value
    redsn.query("SET", key_prefix+"key", "value");
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->string() == "OK");

    /// get key
    redsn.query("GET", key_prefix+"key");
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->bulkstr() == "value");
    
    /// hset key
    redsn.query("HSET", key_prefix+"hkey", "hfield", 1);
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->type() == resp::ty_integer);

    /// set binary data
    redsn.query("SET", key_prefix+"base_aid", base_aid);
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->string() == "OK");

    /// get binary data
    redsn.query("GET", key_prefix+"base_aid");
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    aid_t aid;
    GCE_VERIFY(res->get(aid) == base_aid);

    /// pipeline
    redsn.query("SET", key_prefix+"key", "value");
    redsn.query("GET", key_prefix+"key");
    /// SET
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->string() == "OK");
    /// GET
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->bulkstr() == "value");

    /// array
    redsn.query("DEL", key_prefix+"mylist");
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);

    redsn
      .cmd("RPUSH", key_prefix+"mylist", "one")
      .cmd("RPUSH", key_prefix+"mylist").args("two")
      .cmd("RPUSH").args(key_prefix+"mylist", "three")
      .cmd("LRANGE").args(key_prefix+"mylist", 0).args(2)
      .execute();
    /// RPUSH
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->integer() == 1);
    /// RPUSH
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->integer() == 2);
    /// RPUSH
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->integer() == 3);
    /// LRANGE
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    resp::unique_array<resp::unique_value> const& arr = res->array();
    GCE_VERIFY(arr[0].bulkstr() == "one");
    GCE_VERIFY(arr[1].bulkstr() == "two");
    GCE_VERIFY(arr[2].bulkstr() == "three");
  }

  static void pubsub(stackful_actor self)
  {
    redis::ctxid_t ctxid;
    size_t index;
    boost::shared_ptr<tcp_resolver_t::iterator> eitr;
    aid_t base_aid = self->match("init").recv(index, ctxid, eitr);

    redis::errno_t errn = redis::errno_nil;
    std::string errmsg;
    redis::result_ptr res;
    std::string key_prefix(boost::lexical_cast<std::string>(index));
    key_prefix += "#";

    redis::conn_t c = redis::make_conn(ctxid, *eitr);
    redis::session redsn(self, c);

    redsn.open();
    self->match(redis::sn_open).recv(errn, errmsg);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);

    redsn.query("SUBSCRIBE", "node", "other");
    /// channel node
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->array()[0].bulkstr() == "subscribe");
    GCE_VERIFY(res->array()[1].bulkstr() == "node");
    GCE_VERIFY(res->array()[2].type() == resp::ty_integer);
    /// channel other
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->array()[0].bulkstr() == "subscribe");
    GCE_VERIFY(res->array()[1].bulkstr() == "other");
    GCE_VERIFY(res->array()[2].type() == resp::ty_integer);

    self->send(base_aid, "sub");

    /// wait for N sn_pubmsg
    for (size_t i=0; i<pubsize; ++i)
    {
      self->match(redis::sn_pubmsg).recv(res);
      resp::buffer const& chan = res->array()[1].bulkstr();
      if (chan == "node")
      {
        GCE_VERIFY(res->array()[2].bulkstr() == "hello node!");
      }
      else
      {
        GCE_VERIFY(res->array()[2].bulkstr() == "23333!");
      }
    }

    /// unsubscribe all channels
    redsn.query("UNSUBSCRIBE");
    /// channel node/other
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->array()[0].bulkstr() == "unsubscribe");
    resp::buffer const& chan1 = res->array()[1].bulkstr();
    GCE_VERIFY(chan1 == "other" || chan1 == "node");
    GCE_VERIFY(res->array()[2].type() == resp::ty_integer);
    /// channel other/node
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->array()[0].bulkstr() == "unsubscribe");
    resp::buffer const& chan2 = res->array()[1].bulkstr();
    GCE_VERIFY(chan2 == "other" || chan2 == "node");
    GCE_VERIFY(res->array()[2].type() == resp::ty_integer);

    /// continue other cmds
    redsn
      .cmd("SET", key_prefix+"key", "value")
      .cmd("GET", key_prefix+"key")
      .execute();
    /// SET
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->string() == "OK");
    /// GET
    self->match(redis::sn_query).recv(errn, errmsg, res);
    GCE_VERIFY(errn == redis::errno_nil)(errmsg);
    GCE_VERIFY(res->bulkstr() == "value");
  }

  static void test_common()
  {
    log::asio_logger lgr;
    log::logger_t lg = boost::bind(&gce::log::asio_logger::output, &lgr, _arg1, "");

    try
    {
      attributes attr;
      attr.lg_ = lg;
      context ctx(attr);
      threaded_actor base = spawn(ctx);
      aid_t base_aid = base.get_aid();
      redis::errno_t errn = redis::errno_nil;
      std::string errmsg;
      errcode_t ec;

      redis::context rctx(ctx);
      redis::ctxid_t ctxid = rctx.get_ctxid();

      asio::tcp::resolver rsv(base);
      tcp_resolver_t::query qry("127.0.0.1", "6379");
      rsv.async_resolve(qry);
      boost::shared_ptr<tcp_resolver_t::iterator> eitr;
      base->match(asio::tcp::as_resolve).recv(ec, eitr);
      GCE_VERIFY(!ec).except(ec);

      redis::conn_t c = redis::make_conn(ctxid, *eitr);
      redis::session redsn(base, c);

      redsn.open();
      base->match(redis::sn_open).recv(errn, errmsg);
      GCE_VERIFY(errn == redis::errno_nil)(errmsg);

      /// for boost::timer::auto_cpu_timer to test time
      {
        boost::timer::auto_cpu_timer t;
        redis::result_ptr res;

        /// set key value
        redsn.query("SET", "key", "value");
        base->match(redis::sn_query).recv(errn, errmsg, res);
        GCE_VERIFY(errn == redis::errno_nil)(errmsg);
        GCE_VERIFY(res->string() == "OK");

        /// get key
        redsn.query("GET", "key");
        base->match(redis::sn_query).recv(errn, errmsg, res);
        GCE_VERIFY(errn == redis::errno_nil)(errmsg);
        GCE_VERIFY(res->bulkstr() == "value");

        /// hset key
        redsn.query("HSET", "hkey", "hfield", 1);
        base->match(redis::sn_query).recv(errn, errmsg, res);
        GCE_VERIFY(errn == redis::errno_nil)(errmsg);
        GCE_VERIFY(res->type() == resp::ty_integer);

        /// set binary data
        redsn.query("SET", "base_aid", base_aid);
        base->match(redis::sn_query).recv(errn, errmsg, res);
        GCE_VERIFY(errn == redis::errno_nil)(errmsg);
        GCE_VERIFY(res->string() == "OK");

        /// get binary data
        redsn.query("GET", "base_aid");
        base->match(redis::sn_query).recv(errn, errmsg, res);
        GCE_VERIFY(errn == redis::errno_nil)(errmsg);
        aid_t aid;
        GCE_VERIFY(res->get(aid) == base_aid);

        /// pipeline
        redsn
          .cmd("SET", "key", "value")
          .cmd("GET", "key")
          .execute();
        /// SET
        base->match(redis::sn_query).recv(errn, errmsg, res);
        GCE_VERIFY(errn == redis::errno_nil)(errmsg);
        GCE_VERIFY(res->string() == "OK");
        /// GET
        base->match(redis::sn_query).recv(errn, errmsg, res);
        GCE_VERIFY(errn == redis::errno_nil)(errmsg);
        GCE_VERIFY(res->bulkstr() == "value");

        /// array
        redsn.query("DEL", "mylist");
        base->match(redis::sn_query).recv(errn, errmsg, res);
        GCE_VERIFY(errn == redis::errno_nil)(errmsg);

        redsn
          .cmd("RPUSH", "mylist", "one")
          .cmd("RPUSH", "mylist").args("two")
          .cmd("RPUSH").args("mylist", "three")
          .cmd("LRANGE").args("mylist", 0).args(2)
          .execute();
        /// RPUSH
        base->match(redis::sn_query).recv(errn, errmsg, res);
        GCE_VERIFY(errn == redis::errno_nil)(errmsg);
        GCE_VERIFY(res->integer() == 1);
        /// RPUSH
        base->match(redis::sn_query).recv(errn, errmsg, res);
        GCE_VERIFY(errn == redis::errno_nil)(errmsg);
        GCE_VERIFY(res->integer() == 2);
        /// RPUSH
        base->match(redis::sn_query).recv(errn, errmsg, res);
        GCE_VERIFY(errn == redis::errno_nil)(errmsg);
        GCE_VERIFY(res->integer() == 3);
        /// LRANGE
        base->match(redis::sn_query).recv(errn, errmsg, res);
        GCE_VERIFY(errn == redis::errno_nil)(errmsg);
        resp::unique_array<resp::unique_value> const& arr = res->array();
        GCE_VERIFY(arr[0].bulkstr() == "one");
        GCE_VERIFY(arr[1].bulkstr() == "two");
        GCE_VERIFY(arr[2].bulkstr() == "three");

        size_t shared_num = 16;
        for (size_t i=0; i<shared_num; ++i)
        {
          aid_t aid = spawn(base, boost::bind(&redis_ut::shared, _arg1), monitored);
          base->send(aid, "init", i, c);
        }

        for (size_t i=0; i<shared_num; ++i)
        {
          base->recv(exit);
        }

        size_t simple_num = 16;
        for (size_t i=0; i<simple_num; ++i)
        {
          aid_t aid = spawn(base, boost::bind(&redis_ut::standalone, _arg1), monitored);
          base->send(aid, "init", i, ctxid, eitr);
        }

        for (size_t i=0; i<simple_num; ++i)
        {
          base->recv(exit);
        }

        size_t pubsub_num = 10;
        for (size_t i=0; i<pubsub_num; ++i)
        {
          aid_t aid = spawn(base, boost::bind(&redis_ut::pubsub, _arg1), monitored);
          base->send(aid, "init", i, ctxid, eitr);
        }

        for (size_t i=0; i<pubsub_num; ++i)
        {
          base->recv("sub");
        }

        for (size_t i=0; i<pubsize; ++i)
        {
          if (i % 2 == 0)
          {
            redsn.query("PUBLISH", "node", "hello node!");
            base->match(redis::sn_query).recv(errn, errmsg, res);
            GCE_VERIFY(errn == redis::errno_nil)(errmsg);
            GCE_VERIFY(res->integer() == pubsub_num);
          }
          else
          {
            redsn.query("PUBLISH", "other", "23333!");
            base->match(redis::sn_query).recv(errn, errmsg, res);
            GCE_VERIFY(errn == redis::errno_nil)(errmsg);
            GCE_VERIFY(res->integer() == pubsub_num);
          }
        }

        for (size_t i=0; i<pubsub_num; ++i)
        {
          base->recv(exit);
        }
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << "test_common except: " << ex.what();
    }
  }
};
}
