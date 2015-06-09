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
		local size = 50
		local res_list = {}
		local ec, sender, args = gce.recv('init', gce.actor_id)
		local base_id = args[1]

		for i=1, size do
			local sender
			if i % 2 == 0 then
				sender = gce.spawn('test_lua_actor/my_child.lua')
			else
				sender = gce.spawn_native(gce.stackful, 'my_native')
			end
			res_list[i] = gce.request(sender)
		end

		for i=1, size do
			repeat
				local ec, sender, args = gce.respond{res_list[i], gce.seconds(1)}
			until sender ~= gce.aid_nil
		end

		gce.send(base_id)
  end)
