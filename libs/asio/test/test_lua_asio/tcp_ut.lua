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

    local svr = gce.spawn('test_lua_asio/tcp_echo_server.lua', gce.monitored)
    gce.send(svr, 'init')
    local ec = gce.match('ready').guard(svr).recv()
    assert(ec == gce.ec_ok, ec)

    for i=1, cln_count do
      gce.spawn('test_lua_asio/tcp_echo_client.lua', gce.monitored)
    end

    for i=1, cln_count do
      gce.check_exit('client')
    end

    gce.send(svr, 'end')
    gce.check_exit('server')
  end)
