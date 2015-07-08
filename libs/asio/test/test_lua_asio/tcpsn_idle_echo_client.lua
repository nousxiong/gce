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
    local ec, sender, args, msg, err
    local ptype = asio.plength

    ec, sender, args = gce.match('init').recv(ptype, asio.tcp_endpoint_itr)
    ptype = args[1]
    local eitr = args[2]

    local parser
    if ptype == asio.plength then
      parser = asio.simple_length()
    else
      parser = asio.simple_regex('||')
    end
    local skt_impl = asio.tcp_socket_impl()
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
