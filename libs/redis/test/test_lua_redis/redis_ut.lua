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
    ec, sender, args = gce.match('init').recv(redis.context_id)
    local redis_ctxid = args[1]
    local base_aid = sender

    local rsv = asio.tcp_resolver()
    rsv:async_resolve('192.168.0.135', '10641')
    ec, sender, args = 
      gce.match(asio.as_resolve).recv(gce.errcode, asio.tcp_endpoint_itr)
    err = args[1]
    assert(err == gce.err_nil, tostring(err))
    local eitr = args[2]

    local c = redis.conn(redis_ctxid, eitr)
    local redsn = redis.session(c)
    
    redsn:open()
    ec, sender, args = gce.match(redis.sn_open).recv(redis.errno, '')
    errn = args[1]
    errmsg = args[2]
    assert(errn == redis.errno_nil, errmsg)

    res = redis.result()

    -- set key value
    redsn:query('SET', 'key', 'value')
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    assert(res:string() == 'OK')
    
    -- get key
    redsn:query('GET', 'key')
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    assert(res:bulkstr() == 'value')
    
    -- set binary data
    redsn:query('SET', 'base_aid', base_aid)
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    assert(res:string() == 'OK')

    -- get binary data
    redsn:query('GET', 'base_aid')
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    assert(redis.get(res, gce.actor_id) == base_aid)

    -- pipeline
    redsn
      :cmd('SET', 'key', 'value')
      :cmd('GET', 'key')
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
    redsn:query('DEL', 'mylist')
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)

    redsn
      :cmd('RPUSH', 'mylist', 'one')
      :cmd('RPUSH', 'mylist'):args('two')
      :cmd('RPUSH'):args('mylist', 'three')
      :cmd('LRANGE'):args('mylist', 0):args(2)
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

    local shared_num = 16
    for i=1, shared_num do
      local aid = gce.spawn('test_lua_redis/shared.lua', gce.monitored)
      gce.send(aid, 'init', i, c)
    end

    local has_err = false
    for i=1, shared_num do
      local exc = gce.check_exit('shared', gce.error)
      if exc ~= gce.exit_normal then
        has_err = true
      end
    end
    assert(has_err == false)

    local standalone_num = 16
    for i=1, standalone_num do
      local aid = gce.spawn('test_lua_redis/standalone.lua', gce.monitored)
      gce.send(aid, 'init', i, redis_ctxid, eitr)
    end

    has_err = false
    for i=1, standalone_num do
      local exc = gce.check_exit('standalone', gce.error)
      if exc ~= gce.exit_normal then
        has_err = true
      end
    end
    assert(has_err == false)

    local pubsub_num = 10
    for i=1, pubsub_num do
      local aid = gce.spawn('test_lua_redis/pubsub.lua', gce.monitored)
      gce.send(aid, 'init', i, redis_ctxid, eitr)
    end

    for i=1, pubsub_num do
      gce.match('sub').recv()
    end

    for i=1, pubsub_num do
      if i % 2 == 0 then
        redsn:query('PUBLISH', 'node', 'hello node!')
        ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
        errn = args[1]
        errmsg = args[2]
        res = args[3]
        assert(errn == redis.errno_nil, errmsg)
        assert(res:integer() == pubsub_num)
      else
        redsn:query('PUBLISH', 'other', '23333!')
        ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
        errn = args[1]
        errmsg = args[2]
        res = args[3]
        assert(errn == redis.errno_nil, errmsg)
        assert(res:integer() == pubsub_num)
      end
    end

    has_err = false
    for i=1, pubsub_num do
      local exc = gce.check_exit('pubsub', gce.error)
      if exc ~= gce.exit_normal then
        has_err = true
      end
    end
    assert(has_err == false)

  end)
