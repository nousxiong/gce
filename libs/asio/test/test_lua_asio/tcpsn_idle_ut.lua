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

gce.actor(
  function ()
    local cln_count = 10
    local ec, sender, args, msg, err
    local len_port = '23333'
    local reg_port = '23334'

    -- spawn plength server
    local len_svr = gce.spawn('test_lua_asio/tcpsn_idle_echo_server.lua', gce.monitored)
    gce.send(len_svr, 'init', asio.plength, len_port)
    ec = gce.match('ready').guard(len_svr).recv()
    assert(ec == gce.ec_ok, ec)

    -- spawn pregex server
    local reg_svr = gce.spawn('test_lua_asio/tcpsn_idle_echo_server.lua', gce.monitored)
    gce.send(reg_svr, 'init', asio.pregex, reg_port)
    ec = gce.match('ready').guard(reg_svr).recv()
    assert(ec == gce.ec_ok, ec)

    -- make resolver
    local rsv = asio.tcp_resolver()

    -- resolve plength endpoint
    rsv:async_resolve('127.0.0.1', len_port)
    ec, sender, args = gce.match(asio.as_resolve).recv(gce.errcode, asio.tcp_endpoint_itr)
    err = args[1]
    assert(err == gce.err_nil, tostring(err))
    local len_eitr = args[2]

    -- resolve pregex endpoint
    rsv:async_resolve('127.0.0.1', reg_port)
    ec, sender, args = gce.match(asio.as_resolve).recv(gce.errcode, asio.tcp_endpoint_itr)
    err = args[1]
    assert(err == gce.err_nil, tostring(err))
    local reg_eitr = args[2]

    -- spawn clients
    for i=1, cln_count do
      if i % 2 == 0 then 
        local cln = gce.spawn('test_lua_asio/tcpsn_idle_echo_client.lua', gce.monitored)
        gce.send(cln, 'init', asio.plength, len_eitr)
      else
        local cln = gce.spawn('test_lua_asio/tcpsn_idle_echo_client.lua', gce.monitored)
        gce.send(cln, 'init', asio.pregex, reg_eitr)
      end
    end

    -- wait for clients exit
    for i=1, cln_count do
      gce.check_exit('client')
    end

    -- wait for servers exit
    gce.send(len_svr, 'end')
    gce.check_exit('server')
    gce.send(reg_svr, 'end')
    gce.check_exit('server')
  end)
