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
    local ec, sender, args, errn, errmsg, arr
    local ty_int = 0
    local pubsize = 10
    
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

    redsn:query('SUBSCRIBE', 'node', 'other')
    -- channel node
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    arr = res:array()
    assert(arr:bulkstr(0) == 'subscribe')
    assert(arr:bulkstr(1) == 'node')
    assert(arr:type(2) == redis.resp_integer)
    -- channel other
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    arr = res:array()
    assert(arr:bulkstr(0) == 'subscribe')
    assert(arr:bulkstr(1) == 'other')
    assert(arr:type(2) == redis.resp_integer)

    gce.send(base_aid, 'sub')

    -- wait for N sn_pubmsg
    for i=1, pubsize do
        ec, sender, args = gce.match(redis.sn_pubmsg).recv(res)
        res = args[1]
        arr = res:array()
        local chan = arr:bulkstr(1)
        if chan == 'node' then
            assert(arr:bulkstr(2) == 'hello node!')
        else
            assert(arr:bulkstr(2) == '23333!')
        end
    end

    -- unsubscribe all channels
    redsn:query('UNSUBSCRIBE')
    -- channel node/other
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    arr = res:array()
    assert(arr:bulkstr(0) == 'unsubscribe')
    local chan1 = arr:bulkstr(1)
    assert(chan1 == 'other' or chan1 == 'node')
    assert(arr:type(2) == redis.resp_integer)
    -- channel other/node
    ec, sender, args = gce.match(redis.sn_query).recv(redis.errno, '', res)
    errn = args[1]
    errmsg = args[2]
    res = args[3]
    assert(errn == redis.errno_nil, errmsg)
    arr = res:array()
    assert(arr:bulkstr(0) == 'unsubscribe')
    local chan1 = arr:bulkstr(1)
    assert(chan1 == 'other' or chan1 == 'node')
    assert(arr:type(2) == redis.resp_integer)

    -- continue other cmds
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

  end)
