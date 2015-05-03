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

    local svr = gce.spawn('test_lua_asio/ssl_echo_server.lua', gce.monitored)
    gce.send(svr, 'init')
    local ec = gce.match('ready').guard(svr).recv()
    assert(ec == gce.ec_ok, ec)

    local rsv = asio.tcp_resolver()
    rsv:async_resolve('127.0.0.1', '23333')
    ec, sender, args = gce.match(asio.as_resolve).recv(gce.errcode, asio.tcp_endpoint_itr)
    err = args[1]
    assert(err == gce.err_nil, tostring(err))
    local eitr = args[2]

    local ssl_opt = asio.ssl_option()
    ssl_opt.verify_file = 'ssl_pem/ca.pem'
    local ssl_ctx = asio.ssl_context(asio.sslv23, ssl_opt)

    for i=1, cln_count do
      local cln = gce.spawn('test_lua_asio/ssl_echo_client.lua', gce.monitored)
      gce.send(cln, 'init', eitr, ssl_ctx)
    end

    for i=1, cln_count do
      gce.check_exit('client')
    end

    gce.send(svr, 'end')
    gce.check_exit('server')
  end)
