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
  	local sig = asio.signal(asio.sigint, asio.sigterm)
    local ec, sender, args, msg

    local ec, base_aid = gce.match('init').recv()
    assert(ec == gce.ec_ok, ec)
    while true do
      ec, sender, args, msg = gce.match('echo', 'quit').recv()
      if msg:getty() == gce.atom('quit') then
        break
      end

      sig:async_wait()
      sig:cancel()
      ec, sender, args = gce.match(asio.as_signal).recv(gce.errcode)
      assert(ec == gce.ec_ok, ec)

      local err = args[1]
      assert(err ~= gce.err_nil, err)

      gce.send(base_aid, msg)
    end

    -- test quit
    sig:async_wait()
  end)
