--
-- Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local gce = require('gce')
local asio = require('asio')
local redis = require('redis')

gce.actor(
  function ()
    local ec, sender, args, errn, errmsg
    local ty_int = 0
    
    ec, sender, args = 
        gce.match('init').recv(ty_int, redis.context_id, asio.tcp_endpoint_itr)
    local index = args[1]
    local redis_ctxid = args[2]
    local eitr = args[3]
    local key_prefix = index .. '#'
    local base_aid = sender

    local c = redis.conn(redis_ctxid, eitr)
    local redsn = redis.session(c)
    redsn:open()
    ec, sender, args = gce.match(redis.sn_open).recv(redis.errno, '')
    errn = args[1]
    errmsg = args[2]
    assert(errn == redis.errno_nil, errmsg)

    local res = redis.result()

    -- set key value
    redsn:query('SET', key_prefix .. 'key', 'value')
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    assert(res:string() == 'OK')
    
    -- get key
    redsn:query('GET', key_prefix .. 'key')
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    assert(res:bulkstr() == 'value')
    
    -- set binary data
    redsn:query('SET', key_prefix .. 'base_aid', base_aid)
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    assert(res:string() == 'OK')

    -- get binary data
    redsn:query('GET', key_prefix .. 'base_aid')
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    assert(redis.get(res, gce.actor_id) == base_aid)

    -- pipeline
    redsn
      :cmd('SET', key_prefix .. 'key', 'value')
      :cmd('GET', key_prefix .. 'key')
      :execute()
    -- SET
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    assert(res:string() == 'OK')
    -- GET
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    assert(res:bulkstr() == 'value')

    -- array
    redsn:query('DEL', key_prefix .. 'mylist')
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)

    redsn
      :cmd('RPUSH', key_prefix .. 'mylist', 'one')
      :cmd('RPUSH', key_prefix .. 'mylist'):args('two')
      :cmd('RPUSH'):args(key_prefix .. 'mylist', 'three')
      :cmd('LRANGE'):args(key_prefix .. 'mylist', 0):args(2)
      :execute()
    -- RPUSH
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    assert(res:integer() == 1)
    -- RPUSH
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    assert(res:integer() == 2)
    -- RPUSH
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    assert(res:integer() == 3)
    -- LRANGE
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    local arr = res:array()
    assert(arr:bulkstr(0) == 'one')
    assert(arr:bulkstr(1) == 'two')
    assert(arr:bulkstr(2) == 'three')

  end)
