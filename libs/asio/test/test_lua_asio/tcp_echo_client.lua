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
    local ecount = 10

    ec, sender, args = gce.match('init').recv(asio.tcp_endpoint_itr)
    local eitr = args[1]

    local skt = asio.tcp_socket()
    skt:async_connect(eitr)
    ec, sender, args = gce.match(asio.as_conn).recv(gce.errcode)
    err = args[1]
    assert (err == gce.err_nil, tostring(err))

    local str = 'hello!'
    local ty_int = 0
    local length = string.len(str)
    local ch = gce.chunk(str)

    for i=1, ecount do
      local m = gce.message(asio.as_send, length, ch)
      local ms = m:size()
      skt:async_write(m)
      ec, sender, args = gce.match(asio.as_send).recv(ty_int, ch, gce.errcode)
      err = args[3]
      assert (err == gce.err_nil, tostring(err))

      skt:async_read(ms)
      ec, sender, args, msg = gce.match(asio.as_recv).recv(gce.errcode)
      err = args[1]
      assert (err == gce.err_nil, tostring(err))

      args = gce.unpack(msg, ty_int, ch)

      local echo = args[2]
      assert (echo:to_string() == str)
    end

    str = 'bye'
    length = string.len(str)
    ch:from_string(str)
    local m = gce.message(asio.as_send, length, ch)
    skt:async_write(m)
    ec, sender, args = gce.match(asio.as_send).recv(ty_int, ch, gce.errcode)
    err = args[3]
    assert (err == gce.err_nil, tostring(err))
  end)
