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
      aid_t base_aid = self->match("init").recv(sql_ctxid, c, shared, index);

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

        session_data& sndata = itr->second;
        int64_t uid = get_uid(snid, index);
        if (type == mysql::sn_open)
        {
          sndata.first->sql("SELECT * FROM sample1 where uid='{}'", uid).execute();
        }
        else
        {
          mysql::result_ptr res;
          msg >> res;
          if (--sndata.second == 0)
          {
            session_list.erase(itr);
            if (session_list.empty())
            {
              break;
            }
          }
          else
          {
            if (sndata.second % 3 == 0)
            {
              /*sndata.first->execute(
                "insert into sample1 (`uid`, `m1`, `dt_reg`) values('{}','{}','{}') \
                 ON DUPLICATE KEY UPDATE `m1`=VALUES(`m1`),`dt_reg`=VALUES(`dt_reg`)",
                uid, sndata.second*10000, "2042-09-25 11:00:42"
                );*/
              sndata.first->sql("insert into sample1 (`uid`, `m1`, `dt_reg`) values")
                .sql("('{}','{}','{}'),", uid, itr->second.second*10000, "2042-09-25 11:00:42")
                .sql("('{}','{}','{}')", uid*100, itr->second.second*10001, "2043-09-25 11:00:43")
                .sql(" ON DUPLICATE KEY UPDATE `m1`=VALUES(`m1`),`dt_reg`=VALUES(`dt_reg`)")
                .execute();
            }
            else if (sndata.second % 5 == 0)
            {
              /*sndata.first->execute(
                "start transaction; \
                 insert into sample1 (`uid`, `energy`) values('{0}','{1}') ON DUPLICATE KEY UPDATE `energy`=VALUES(`energy`); \
                 update sample1 set quid='{2}' where uid='{0}'; \
                 commit;", 
                 uid, 42, "NewQuid"
                );*/
              sndata.first->sql(
                "start transaction; \
                  insert into sample1 (`uid`, `energy`) values('{0}','{1}') ON DUPLICATE KEY UPDATE `energy`=VALUES(`energy`); \
                  update sample1 set quid='{2}' where uid='{0}'; \
                  commit;",
                  uid, 42, base_aid
                ).execute();
            }
            else
            {
              sndata.first->sql("SELECT * FROM sample1 where uid='{}'", uid).execute();
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
      aid_t base_aid = base.get_aid();
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
           `quid` blob(128), \
           `sid` blob(128), \
           `energy` smallint(6) NOT NULL DEFAULT '1', \
           `m1` int(11) NOT NULL DEFAULT '0', \
           `dt_reg` datetime NOT NULL DEFAULT '2015-09-25 13:02:00', \
           PRIMARY KEY (`uid`) \
          ) ENGINE=InnoDB DEFAULT CHARSET=utf8;"
          );
        base->match(mysql::sn_query).recv(errn, errmsg);
        GCE_VERIFY(errn == mysql::errno_nil)(errmsg);

        /*sql_sn.execute(
          fmt::format(
            "INSERT INTO `sample1` VALUES('{}','{}','{}','{}','{}','{}')", 
            int64_t(42), base_aid, "42-sid", int16_t(100), int32_t(9000), "2015-09-25 12:02:55"
            )
          );
        base->match(mysql::sn_query).recv(errn, errmsg);
        GCE_VERIFY(errn == mysql::errno_nil)(errmsg);*/

        /// syntactic sugar
        sql_sn.sql(
          "INSERT INTO `sample1` VALUES('{}','{}','{}','{}','{}','{}')", 
          42, base_aid, base_aid, 101, 9001, "2015-09-26 12:02:56"
          ).execute();
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
        aid_t sid;
        aid_t quid;
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
          if (row(1, quid) != 0)
          {
            GCE_VERIFY(row(1, quid) == &quid);
            GCE_VERIFY(quid == base_aid);
          }
          if (row(1, sid) != 0)
          {
            GCE_VERIFY(row(2, sid) == &sid);
            GCE_VERIFY(sid == base_aid);
          }
          GCE_VERIFY(row(3, energy) == &energy);
          GCE_VERIFY(row(4, m1) == &m1);
          GCE_VERIFY(row(5, dt_reg) == &dt_reg);
          GCE_VERIFY(row("uid", uid) == &uid);
          if (row("quid", quid) != 0)
          {
            GCE_VERIFY(row("quid", quid) == &quid);
            GCE_VERIFY(quid == base_aid);
          }
          if (row("sid", sid) != 0)
          {
            GCE_VERIFY(row("sid", sid) == &sid);
            GCE_VERIFY(sid == base_aid);
          }
          GCE_VERIFY(row("energy", energy) == &energy);
          GCE_VERIFY(row("m1", m1) == &m1);
          GCE_VERIFY(row("dt_reg", dt_reg) == &dt_reg);

          /// fetch fields using ref with errcode (set ec if error)
          errcode_t ec;
          GCE_VERIFY(row(0, uid, ec) == &uid);
          if (row(1, quid, ec) != 0)
          {
            GCE_VERIFY(row(1, quid, ec) == &quid);
            GCE_VERIFY(quid == base_aid);
          }
          if (row(1, sid, ec) != 0)
          {
            GCE_VERIFY(row(2, sid, ec) == &sid);
            GCE_VERIFY(sid == base_aid);
          }
          GCE_VERIFY(row(3, energy, ec) == &energy);
          GCE_VERIFY(row(4, m1, ec) == &m1);
          GCE_VERIFY(row(5, dt_reg, ec) == &dt_reg);
          GCE_VERIFY(row("uid", uid, ec) == &uid);
          if (row("quid", quid, ec) != 0)
          {
            GCE_VERIFY(row("quid", quid, ec) == &quid);
            GCE_VERIFY(quid == base_aid);
          }
          if (row("sid", sid, ec) != 0)
          {
            GCE_VERIFY(row("sid", sid, ec) == &sid);
            GCE_VERIFY(sid == base_aid);
          }
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
