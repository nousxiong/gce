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

function get_uid(snid, index)
  return gce.match2number(snid) * 10 + index
end

gce.actor(
  function ()
    local ec, sender, args, msg, errn, errmsg
    local ty_int = 0
    local ecount = 5
    local cln_count = 5
    
    ec, sender, args = gce.match('init').recv(mysql.context_id, mysql.conn, ty_int)
    local sql_ctxid = args[1]
    local c = args[2]
    local index = args[3]
    local base_aid = sender

    local session_list = {}
    for i=1, cln_count do
      local opt = mysql.conn_option()
      opt.reconnect = 1
      opt.set_charset_name = 'utf8'
      opt.client_multi_statements = 1
      c = mysql.conn(sql_ctxid, '127.0.0.1', 3306, 'xhzs', '132', 'test', opt)

      local snid = mysql.session_id(i)
      local sn = mysql.session(c, snid)
      local sn_data = {}
      sn_data.sn_ = sn
      sn_data.ecount_ = ecount
      sn_data.res_ = mysql.result()
      session_list[gce.match2number(snid)] = sn_data
      sn:open()
    end

    while true do
      ec, sender, args, msg = 
        gce.match(mysql.sn_open, mysql.sn_query).recv(mysql.session_id, mysql.errno, '')
      local snid = args[1]
      errn = args[2]
      errmsg = args[3]
      local sn_data = session_list[gce.match2number(snid)]

      if errn ~= mysql.errno_nil then
        gce.error(snid, ': ', errmsg)
        session_list[gce.match2number(snid)] = nil
        cln_count = cln_count - 1
        if cln_count == 0 then
          break
        end
      else
        local uid = get_uid(snid, index)
        assert (sn_data ~= nil)
        if msg:getty() == mysql.sn_open then
          sn_data.sn_:query([[SELECT * FROM sample1 where uid='{}']], uid)
        else
          args = gce.unpack(msg, sn_data.res_)
          local res = args[1]
          sn_data.ecount_ = sn_data.ecount_ - 1
          if sn_data.ecount_ == 0 then
            session_list[gce.match2number(snid)] = nil
            cln_count = cln_count - 1
            if cln_count == 0 then
              break
            end
          else
            if sn_data.ecount_ % 3 == 0 then
              sn_data.sn_:sql([[insert into sample1 (`uid`, `m1`, `dt_reg`) values]])
                :sql([[('{}','{}','{}'),]], uid, base_aid, '2042-09-25 11:00:42')
                :sql([[('{}','{}','{}')]], uid*100, base_aid, '2043-09-25 11:00:43') 
                :sql([[ ON DUPLICATE KEY UPDATE `m1`=VALUES(`m1`),`dt_reg`=VALUES(`dt_reg`)]])
                :execute(sn_data.res_)
            elseif sn_data.ecount_ % 5 == 0 then
              sn_data.sn_:query([[
                start transaction; 
                insert into sample1 (`uid`, `energy`) values('{0}','{1}) ON DUPLICATE KEY UPDATE `energy`=VALUES(`energy`); 
                update sample1 set quid='{2}' where uid='{0}'; 
                commit;
                ]], uid, 42, base_aid, uid
                )
            else
              sn_data.sn_:query([[SELECT * FROM sample1 where uid='{}']], uid)
            end
          end
        end
      end
    end
  end)
