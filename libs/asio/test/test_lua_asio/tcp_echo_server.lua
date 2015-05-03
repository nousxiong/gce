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

    local rsv = asio.tcp_resolver()
    rsv:async_resolve('0.0.0.0', '23333')
    ec, sender, args = gce.match(asio.as_resolve).recv(gce.errcode, asio.tcp_endpoint_itr)
    err = args[1]
    assert(err == gce.err_nil, tostring(err))
    local eitr = args[2]

    local scount = 0
    local acpr = asio.tcp_acceptor()

    local opt = asio.tcp_option()
    opt.reuse_address = 1
    opt.receive_buffer_size = 640000
    opt.send_buffer_size = 640000
    opt.backlog = 1024
    opt.no_delay = 1
    opt.keep_alive = 1
    opt.enable_connection_aborted = 1
    acpr:bind(eitr, opt)

    gce.send(base_aid, 'ready')

    while true do
      local skt_impl = asio.tcp_socket_impl()
      acpr:async_accept(skt_impl)
      ec, sender, args, msg = gce.match(asio.as_accept, 'end').recv()
      if msg:getty() == gce.atom('end') then
        break
      end

      args = gce.unpack(msg, gce.errcode)
      local err = args[1]
      if err == gce.err_nil then
        local cln = gce.spawn('test_lua_asio/tcp_echo_session.lua', gce.monitored)
        gce.send(cln, 'init', skt_impl)
        scount = scount + 1
      end
    end

    for i=1, scount do
      gce.check_exit('session')
    end
  end)
