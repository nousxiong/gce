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
local libredis = require('libredis')
local redis = {}

local conn_adl = nil
local ctxid_adl = nil
if gce.packer == gce.pkr_adata then
  conn_adl = require('conn_adl')
  ctxid_adl = require('context_id_adl')
end

libredis.init_nil()

-- enum and constant def
redis.ty_result = libredis.ty_result
redis.snid_nil = libredis.snid_nil
redis.errno_nil = libredis.errno_nil

-- resp types
redis.resp_null = libredis.resp_null
redis.resp_string = libredis.resp_string
redis.resp_error = libredis.resp_error
redis.resp_integer = libredis.resp_integer
redis.resp_bulkstr = libredis.resp_bulkstr
redis.resp_array = libredis.resp_array

function redis.errno()
  return gce.atom()
end

function redis.context_id()
  return libredis.make_ctxid()
end

-- ep: {host, port} or gce.tcp_endpoin_itr
-- timeout: gce.duration (optional, could be nil/none)
function redis.conn(redis_ctxid, ep, timeout)
  return libredis.make_conn(redis_ctxid, ep, timeout)
end

function redis.session_id(v)
  return gce.atom(v)
end

-- c: redis.conn
-- snid: redis.session_id (optional, could be nil/none)
function redis.session(c, snid)
  return libredis.make_session(libgce.self, c, snid)
end

function redis.result()
  return libredis.make_result()
end

-- get binary data
-- res: redis.result or redis.array
-- o: obj type or obj itself, like gce.deserialize
-- return o
function redis.get(res, o)
  local ty = type(o)
  if ty == 'function' then
    o = o()
    if (type(o) == 'function') then
      o = o()
    end
  end
  return res:get(o)
end

redis.sn_open = gce.atom('redis_open')
redis.sn_ping = gce.atom('redis_ping')
redis.sn_query = gce.atom('redis_query')
redis.sn_pubmsg = gce.atom('redis_pubmsg')


return redis
