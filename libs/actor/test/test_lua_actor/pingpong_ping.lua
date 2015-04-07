--
-- Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local gce = require('gce')

gce.actor(
  function ()
  	local ec, base_aid, args = gce.recv('init')
		local pong_aid = gce.spawn('test_lua_actor/pingpong_pong.lua')
		gce.send(pong_aid, 'init', base_aid)

		local msg_size = 10000
		local batch_size = 100
		for i=1, msg_size/batch_size do
			for n=1, batch_size do
				gce.send(pong_aid, 1)
			end

			for n=1, batch_size do
				gce.recv()
			end
		end
		gce.send(pong_aid, 2)
  end)
