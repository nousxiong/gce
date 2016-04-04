--
-- Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local gce = require('gce')
local mysql = require('mysql')

gce.actor(
  function ()
    local ec, sender, args, msg, errn, errmsg, res
    local ty_int = 0
    ec, sender, args = gce.match('init').recv(mysql.context_id)
    local sql_ctxid = args[1]
    local base_aid = gce.get_aid()

    local opt = mysql.conn_option()
    opt.reconnect = 1
    opt.set_charset_name = 'utf8'
    opt.client_multi_statements = 1
    local c = mysql.conn(sql_ctxid, '127.0.0.1', 3306, 'xhzs', '132', 'test', opt)
    local sql_sn = mysql.session(c)
    
    sql_sn:open()
    ec, sender, args = gce.match(mysql.sn_open).recv(mysql.errno, '')
    errn = args[1]
    errmsg = args[2]
    assert(errn == mysql.errno_nil, errmsg)

    sql_sn:query([[DROP TABLE IF EXISTS `sample1`]])
    ec, sender, args = gce.match(mysql.sn_query).recv(mysql.errno, '')
    errn = args[1]
    errmsg = args[2]
    assert(errn == mysql.errno_nil, errmsg)
    
    sql_sn:query([[
      CREATE TABLE `sample1` (
        `uid` bigint(20) NOT NULL, 
        `quid` blob(128), 
        `sid` blob(128), 
        `energy` smallint(6) NOT NULL DEFAULT '1', 
        `m1` blob(128), 
        `dt_reg` datetime NOT NULL DEFAULT '2015-09-25 13:02:00', 
        PRIMARY KEY (`uid`) 
      ) ENGINE=InnoDB DEFAULT CHARSET=utf8;
      ]])
    ec, sender, args = gce.match(mysql.sn_query).recv(mysql.errno, '')
    errn = args[1]
    errmsg = args[2]
    assert(errn == mysql.errno_nil, errmsg)
    
    sql_sn:sql([[
      INSERT INTO `sample1` VALUES('{}','{}','{}','{}','{}','{}')
      ]], 42, base_aid, base_aid, 100, base_aid, '2015-09-25 12:02:55'
      ):execute()
    ec, sender, args = gce.match(mysql.sn_query).recv(mysql.errno, '')
    errn = args[1]
    errmsg = args[2]
    assert(errn == mysql.errno_nil, errmsg)
    
    local thrn = 8
    for i=1,thrn do
      local aid = gce.spawn('test_lua_mysql/sample.lua', gce.monitored)
      gce.send(aid, 'init', sql_ctxid, c, i)
    end

    local has_err = false
    for i=1,thrn do
      local exc = gce.check_exit('sample', gce.error)
      if exc ~= gce.exit_normal then
        has_err = true
      end
    end
    assert(has_err == false)
    
    sql_sn:query([[SELECT * FROM sample1]])
    ec, sender, args = gce.match(mysql.sn_query).recv(mysql.errno, '', mysql.result)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == mysql.errno_nil, errmsg)
    assert(res ~= nil)
    
    local tabsize = res:table_size()
    assert(tabsize == 1, tabsize)
    assert(res:field_size(1) == 6)
    
    local rowsize = res:row_size(1)
    local row = {}
    local uid, quid, sid, energy, m1, dt_reg
    for i=1,rowsize do
      uid, quid, sid, energy, m1, dt_reg = 
        mysql.blob(gce.actor_id).blob(gce.actor_id).blob(gce.actor_id).fetch(res, 1, i)
      if quid ~= nil then
        assert(quid == base_aid)
      end
      if sid ~= nil then
        assert(sid == base_aid)
      end
      if m1 ~= nil then
        assert(m1 == base_aid)
      end
      --gce.info(uid, quid, sid, energy, m1, dt_reg)
      --gce.info(uid, energy, m1, dt_reg)

      row = mysql.blob(gce.actor_id).blob(gce.actor_id).blob(gce.actor_id).fetch(res, 1, i, row, 'a')
      assert(row.uid == uid)
      assert(row.quid == quid)
      assert(row.sid == sid)
      assert(row.energy == energy)
      assert(row.m1 == m1)
      assert(row.dt_reg == dt_reg)
      if quid ~= nil then
        assert(quid == base_aid)
      end
      if sid ~= nil then
        assert(sid == base_aid)
      end
      if m1 ~= nil then
        assert(m1 == base_aid)
      end
      
      row = mysql.blob(gce.actor_id).blob(gce.actor_id).blob(gce.actor_id).fetch(res, 1, i, row, 'n')
      assert(row[1] == uid)
      assert(row[2] == quid)
      assert(row[3] == sid)
      assert(row[4] == energy)
      assert(row[5] == m1)
      assert(row[6] == dt_reg)
      if quid ~= nil then
        assert(quid == base_aid)
      end
      if sid ~= nil then
        assert(sid == base_aid)
      end
      if m1 ~= nil then
        assert(m1 == base_aid)
      end
    end
  end)
