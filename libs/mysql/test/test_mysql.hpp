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
class mysql_ut
{
public:
  static void run()
  {
    std::cout << "mysql_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "mysql_ut end." << std::endl;
  }

private:
  static int64_t get_uid(mysql::snid_t snid, size_t index)
  {
    return snid.val_ * 10 + index;
  }

  static void sample(stackful_actor self)
  {
    context& ctx = self.get_context();
    log::logger_t& lg = ctx.get_logger();
    try
    {
      size_t ecount = 5;
      size_t cln_count = 5;
      mysql::errno_t errn = mysql::errno_nil;
      std::string errmsg;

      mysql::ctxid_t sql_ctxid;
      mysql::conn_t c;
      bool shared = false;
      size_t index;
      self->match("init").recv(sql_ctxid, c, shared, index);

      typedef boost::shared_ptr<mysql::session> session_ptr;
      typedef std::pair<session_ptr, size_t> session_data;
      typedef std::map<mysql::snid_t, session_data> session_list_t;
      session_list_t session_list;

      for (size_t i=0; i<cln_count; ++i)
      {
        mysql::snid_t snid = to_match(i);
        if (!shared)
        {
          mysql::connopt_t opt = mysql::make_connopt();
          opt.reconnect = 1;
          opt.set_charset_name = "utf8";
          opt.client_multi_statements = 1;
          c = mysql::make_conn(sql_ctxid, "127.0.0.1", 3306, "xhzs", "132", "test", opt);
        }
        session_ptr sn = boost::make_shared<mysql::session>(self, c, snid);
        session_list.insert(std::make_pair(snid, std::make_pair(sn, ecount)));
        sn->open();
      }

      while (true)
      {
        mysql::errno_t errn = mysql::errno_nil;
        std::string errmsg;
        message msg;
        match_t type;
        mysql::snid_t snid;

        self->match(mysql::sn_open, mysql::sn_query, type).raw(msg).recv(snid, errn, errmsg);
        session_list_t::iterator itr = session_list.find(snid);
        GCE_ASSERT(itr != session_list.end())(snid);
        if (errn != mysql::errno_nil)
        {
          GCE_ERROR(lg) << snid << ": " << errmsg;
          session_list.erase(itr);
          if (session_list.empty())
          {
            break;
          }
          continue;
        }

        int64_t uid = get_uid(snid, index);
        if (type == mysql::sn_open)
        {
          std::string& qry_buf = itr->second.first->get_query_buffer();
          itr->second.first->execute(
            fmt::sformat(qry_buf, 
              "SELECT * FROM sample1 where uid='{}'",
              uid
              )
            );
        }
        else
        {
          mysql::result_ptr res;
          msg >> res;
          if (--itr->second.second == 0)
          {
            session_list.erase(itr);
            if (session_list.empty())
            {
              break;
            }
          }
          else
          {
            std::string& qry_buf = itr->second.first->get_query_buffer();
            if (itr->second.second % 3 == 0)
            {
              itr->second.first->execute(
                fmt::sformat(qry_buf, 
                  "insert into sample1 (`uid`, `m1`, `dt_reg`) values('{}','{}','{}') \
                   ON DUPLICATE KEY UPDATE `m1`=VALUES(`m1`),`dt_reg`=VALUES(`dt_reg`)", 
                  uid, itr->second.second*10000, "2042-09-25 11:00:42")
                );
            }
            else if (itr->second.second % 5 == 0)
            {
              itr->second.first->execute(
                fmt::sformat(qry_buf, 
                  "start transaction; \
                  insert into sample1 (`uid`, `energy`) values('{0}','{1}') ON DUPLICATE KEY UPDATE `energy`=VALUES(`energy`); \
                  update sample1 set quid='{2}' where uid='{0}'; \
                  commit;", 
                  uid, 42, "NewQuid")
                );
            }
            else
            {
              std::string& qry_buf = itr->second.first->get_query_buffer();
              itr->second.first->execute(
                fmt::sformat(qry_buf, 
                  "SELECT * FROM sample1 where uid='{}'",
                  uid
                  )
                );
            }
          }
        }
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << "sample except: " << ex.what();
    }
  }

  static void test_common()
  {
    log::asio_logger lgr;
    log::logger_t lg = boost::bind(&gce::log::asio_logger::output, &lgr, _arg1, "");

    try
    {
      mysql::attributes sql_attr;
      sql_attr.lg_ = lg;
      sql_attr.thread_num_ = 5;
      mysql::context sql_ctx(sql_attr);

      attributes attr;
      attr.lg_ = lg;
      //attr.thread_num_ = 5;
      context ctx(attr);
      threaded_actor base = spawn(ctx);
      mysql::errno_t errn = mysql::errno_nil;
      std::string errmsg;
      mysql::ctxid_t sql_ctxid = sql_ctx.get_ctxid();

      mysql::connopt_t opt = mysql::make_connopt();
      opt.reconnect = 1;
      opt.set_charset_name = "utf8";
      opt.client_multi_statements = 1;
      mysql::conn_t c = mysql::make_conn(sql_ctxid, "127.0.0.1", 3306, "xhzs", "132", "test", opt);

      mysql::session sql_sn(base, c);

      sql_sn.open();
      base->match(mysql::sn_open).recv(errn, errmsg);
      GCE_VERIFY(errn == mysql::errno_nil)(errmsg);

      /// for boost::timer::auto_cpu_timer to test time
      {
        boost::timer::auto_cpu_timer t;

        sql_sn.execute("DROP TABLE IF EXISTS `sample1`");
        base->match(mysql::sn_query).recv(errn, errmsg);
        GCE_VERIFY(errn == mysql::errno_nil)(errmsg);

        sql_sn.execute(
          "CREATE TABLE `sample1` (\
           `uid` bigint(20) NOT NULL, \
           `quid` varchar(128) NOT NULL DEFAULT '', \
           `sid` char(10) NOT NULL DEFAULT '', \
           `energy` smallint(6) NOT NULL DEFAULT '1', \
           `m1` int(11) NOT NULL DEFAULT '0', \
           `dt_reg` datetime NOT NULL DEFAULT '2015-09-25 13:02:00', \
           PRIMARY KEY (`uid`) \
          ) ENGINE=InnoDB DEFAULT CHARSET=utf8;"
          );
        base->match(mysql::sn_query).recv(errn, errmsg);
        GCE_VERIFY(errn == mysql::errno_nil)(errmsg);

        std::string& qry_buf = sql_sn.get_query_buffer();
        sql_sn.execute(
          fmt::sformat(qry_buf, 
            "INSERT INTO `sample1` VALUES('{}','{}','{}','{}','{}','{}')", 
            int64_t(42), "Global", "42-sid", int16_t(100), int32_t(9000), "2015-09-25 12:02:55")
          );
        base->match(mysql::sn_query).recv(errn, errmsg);
        GCE_VERIFY(errn == mysql::errno_nil)(errmsg);

        for (size_t i=0; i<attr.thread_num_; ++i)
        {
          aid_t aid = spawn(base, boost::bind(&mysql_ut::sample, _arg1), monitored);
          base->send(aid, "init", sql_ctxid, c, /*i == 0*/false, i);
        }

        for (size_t i=0; i<attr.thread_num_; ++i)
        {
          base->recv(exit);
        }

        sql_sn.execute("SELECT * FROM sample1");
        mysql::result_ptr res; 
        base->match(mysql::sn_query).recv(errn, errmsg, res);
        GCE_VERIFY(errn == mysql::errno_nil)(errmsg);

        /// fetch result
        mysql::fetcher fch(res);
        size_t tabsize = fch.table_size();
        GCE_VERIFY(tabsize == 1)(tabsize);

        size_t rowsize = fch.row_size(0);
        int64_t uid;
        std::string quid, sid;
        int16_t energy;
        int32_t m1;
        mysql::datetime dt_reg;
        //GCE_INFO(lg) << "table sample1: ";
        for (size_t i=0; i<rowsize; ++i)
        {
          mysql::row row = fch.get_row(0, i);
          GCE_VERIFY(row != mysql::row_nil);
          size_t field_size = row.field_size();
          GCE_VERIFY(field_size == 6)(field_size);

          /// fetch fields using ref (will throw exception if error)
          GCE_VERIFY(row(0, uid) == &uid);
          GCE_VERIFY(row(1, quid, mysql::var) == &quid);
          GCE_VERIFY(row(2, sid, !mysql::var) == &sid);
          GCE_VERIFY(row(3, energy) == &energy);
          GCE_VERIFY(row(4, m1) == &m1);
          GCE_VERIFY(row(5, dt_reg) == &dt_reg);
          GCE_VERIFY(row("uid", uid) == &uid);
          GCE_VERIFY(row("quid", quid, mysql::var) == &quid);
          GCE_VERIFY(row("sid", sid, !mysql::var) == &sid);
          GCE_VERIFY(row("energy", energy) == &energy);
          GCE_VERIFY(row("m1", m1) == &m1);
          GCE_VERIFY(row("dt_reg", dt_reg) == &dt_reg);

          /// fetch fields using ref with errcode (set ec if error)
          errcode_t ec;
          GCE_VERIFY(row(0, uid, ec) == &uid);
          GCE_VERIFY(row(1, quid, mysql::var, ec) == &quid);
          GCE_VERIFY(row(2, sid, !mysql::var, ec) == &sid);
          GCE_VERIFY(row(3, energy, ec) == &energy);
          GCE_VERIFY(row(4, m1, ec) == &m1);
          GCE_VERIFY(row(5, dt_reg, ec) == &dt_reg);
          GCE_VERIFY(row("uid", uid, ec) == &uid);
          GCE_VERIFY(row("quid", quid, mysql::var, ec) == &quid);
          GCE_VERIFY(row("sid", sid, !mysql::var, ec) == &sid);
          GCE_VERIFY(row("energy", energy, ec) == &energy);
          GCE_VERIFY(row("m1", m1, ec) == &m1);
          GCE_VERIFY(row("dt_reg", dt_reg, ec) == &dt_reg);
          GCE_VERIFY(!ec)(ec);

          //GCE_INFO(lg) << fmt::format("  uid:{}, quid:{}, sid:{}, energy:{}, m1:{}, dt_reg:{}", uid, quid, sid, energy, m1, to_string(dt_reg));
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
