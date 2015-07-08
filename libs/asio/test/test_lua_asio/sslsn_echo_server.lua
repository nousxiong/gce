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

local function pwd_cb(max_len, purpose)
  return 'test'
end

gce.actor(
  function ()
    local ec, sender, args, msg, base_aid
    local ptype = asio.plength
    local port = ''
    ec, base_aid, args = gce.match('init').recv(ptype, port)
    ptype = args[1]
    port = args[2]

    local rsv = asio.tcp_resolver()
    rsv:async_resolve('0.0.0.0', port)
    ec, sender, args = gce.match(asio.as_resolve).recv(gce.errcode, asio.tcp_endpoint_itr)
    local err = args[1]
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

    -- ssl context
    local ssl_opt = asio.ssl_option()
    ssl_opt.default_workarounds = 1
    ssl_opt.no_sslv2 = 1
    ssl_opt.single_dh_use = 1
    ssl_opt.certificate_chain_file = 'test_ssl_asio/server.pem'
    ssl_opt.private_key_file = 'test_ssl_asio/server.pem'
    ssl_opt.tmp_dh_file = 'test_ssl_asio/dh512.pem'
    local ssl_ctx = asio.ssl_context(asio.sslv23, ssl_opt, pwd_cb)

    gce.send(base_aid, 'ready')

    while true do
      local skt_impl = asio.ssl_stream_impl(ssl_ctx)
      acpr:async_accept(skt_impl)
      ec, sender, args, msg = gce.match(asio.as_accept, 'end').recv()
      if msg:getty() == gce.atom('end') then
        break
      end

      args = gce.unpack(msg, gce.errcode)
      local err = args[1]
      if err == gce.err_nil then
        local cln = gce.spawn('test_lua_asio/sslsn_echo_session.lua', gce.monitored)
        gce.send(cln, 'init', ptype, skt_impl)
        scount = scount + 1
      end
    end

    for i=1, scount do
      gce.check_exit('session')
    end
  end)
