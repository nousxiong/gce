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
local queue = require('test_lua_asio/queue')

local header = gce.atom('header')
local body = gce.atom('body')

gce.actor(
  function ()
    local ec, sender, args, msg, err

    ec, sender, args = gce.match('init').recv(asio.ssl_stream_impl)
    local skt_impl = args[1]

    local skt = asio.ssl_stream(skt_impl)
    skt:async_handshake(asio.server)
    ec, sender, args = gce.match(asio.as_handshake).recv(gce.errcode)
    err = args[1]
    assert (err == gce.err_nil, tostring(err))

    local write_queue = queue.new()
    local write_in_progress = false
    local ty_int = 0
    local body_len = 0
    local header_len = 8 -- lua_Number default is double 

    asio.async_read(skt, header_len, header)
    while true do
      ec, sender, args, msg = 
        gce.match(header, body, asio.as_send).recv()

      if msg:getty() == header then
        args = gce.unpack(msg, gce.errcode)
        err = args[1]
        assert (err == gce.err_nil, tostring(err))

        args = gce.unpack(msg, ty_int)
        body_len = args[1]
        asio.async_read(skt, body_len, body)
      elseif msg:getty() == body then
        args = gce.unpack(msg, gce.errcode)
        err = args[1]
        assert (err == gce.err_nil, tostring(err))

        local ch = gce.chunk(body_len)
        args = gce.unpack(msg, ch)

        local echo = ch:to_string()
        if echo == 'bye' then
          break
        end

        -- gce.print('server recv echo: ', echo)

        local m = gce.message(asio.as_send, body_len, ch)
        if write_in_progress then
          queue.push(write_queue, m)
          queue.push(write_queue, ch)
        else
          queue.push(write_queue, ch)
          skt:async_write(m)
          write_in_progress = true
        end

        asio.async_read(skt, header_len, header)
      else
        local ch = queue.pop(write_queue)
        args = gce.unpack(msg, ty_int, ch, gce.errcode)
        err = args[3]
        assert (err == gce.err_nil, tostring(err))

        write_in_progress = false
        if not queue.empty(write_queue) then
          local m = queue.pop(write_queue)
          skt:async_write(m)
          write_in_progress = true
        end
      end
    end

    skt:async_shutdown()
    gce.match(asio.as_shutdown).recv()
  end)
