--
-- Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local gce = require('gce')
local libgce = require('libgce')
local libmysql = require('libmysql')
local mysql = {}

local conn_adl = nil
local connopt_adl = nil
local ctxid_adl = nil
if gce.packer == gce.pkr_adata then
  conn_adl = require('conn_adl')
  connopt_adl = require('conn_option_adl')
  ctxid_adl = require('context_id_adl')
end

libmysql.init_nil()

-- enum and constant def
mysql.ty_result = libmysql.ty_result
mysql.snid_nil = libmysql.snid_nil
mysql.errno_nil = libmysql.errno_nil

function mysql.errno()
  return gce.atom()
end

function mysql.conn_option()
  return libmysql.make_connopt()
end

function mysql.context_id()
  return libmysql.make_ctxid()
end

function mysql.conn(sql_ctxid, host, port, usr, pwd, db, opt)
  return libmysql.make_conn(sql_ctxid, host, port, usr, pwd, db, opt)
end

function mysql.session_id(v)
  return gce.atom(v)
end

function mysql.session(c, snid)
  return libmysql.make_session(libgce.self, c, snid)
end

function mysql.result()
  return libmysql.make_result()
end

mysql.sn_open = gce.atom('sn_open')
mysql.sn_query = gce.atom('sn_query')
mysql.sn_ping = gce.atom('sn_ping')

return mysql
