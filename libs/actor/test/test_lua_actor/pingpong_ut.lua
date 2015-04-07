--
-- Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local gce = require('gce')

gce.actor(
  function ()
		local aid = gce.spawn('test_lua_actor/pingpong_ping.lua')
		gce.send(aid, 'init')
		gce.recv()
  end)
