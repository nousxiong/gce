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

local function verify_cb(preverified, ctx)
  return true
end

gce.actor(
  function ()
    local ec, sender, args, msg, err
    local ptype = asio.plength

    ec, sender, args = gce.match('init').recv(ptype, asio.tcp_endpoint_itr, asio.ssl_context)
    ptype = args[1]
    local eitr = args[2]
    local ssl_ctx = args[3]

    local parser
    if ptype == asio.plength then
      parser = asio.simple_length()
    else
      parser = asio.simple_regex('||')
    end
    local ssl_opt = asio.ssl_option()
    ssl_opt.verify_peer = 1
    local skt_impl = asio.ssl_stream_impl(ssl_ctx, ssl_opt)
    skt_impl:set_verify_callback(verify_cb)
    local sn = asio.session(parser, skt_impl, eitr)

    sn:open()
    ec, sender, args, msg = 
      gce.match(asio.sn_open, asio.sn_close).recv()
    if msg:getty() == asio.sn_close then
      args = gce.unpack(msg, gce.errcode)
      err = args[1]
      error (tostring(err))
    end

    gce.match(asio.sn_close).recv()
  end)
