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
    local ec, sender, args, msg, base_aid
    ec, base_aid = gce.recv('init')

    local scount = 0
    local opt = asio.tcp_option()
    opt.address = '0.0.0.0'
    opt.port = 23333
    opt.reuse_address = 1
    opt.receive_buffer_size = 640000
    opt.send_buffer_size = 640000
    opt.backlog = 1024
    opt.no_delay = 1
    opt.keep_alive = 1
    opt.enable_connection_aborted = 1
    local acpr = asio.tcp_acceptor(opt)

    gce.send(base_aid, 'ready')

    while true do
      acpr:async_accept()
      ec, sender, args, msg = 
        gce.match(asio.as_accept, 'end').recv()
      if msg:getty() == gce.atom('end') then
        break
      end

      args = gce.unpack(msg, gce.errcode)
      local err = args[1]
      if err == gce.err_nil then
        local cln = gce.spawn('test_lua_asio/tcp_echo_session.lua', gce.monitored)
        msg:setty('init')
        gce.send(cln, msg)
        scount = scount + 1
      end
    end

    for i=1, scount do
      gce.check_exit('session')
    end
  end)
