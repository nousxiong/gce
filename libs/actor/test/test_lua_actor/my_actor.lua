--
-- Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

require("gce")

gce.run_actor(
  function ()
		local size = 50
		local res_list = {}
		local sender, args = gce.recv("init", gce.aid())
		local base_id = args[1]

		for i=1, size do
			local sender = gce.spawn("test_lua_actor/my_child.lua")
			res_list[i] = gce.request(sender)
		end

		for i=1, size do
			repeat
				local sender, args = gce.respond{res_list[i], gce.seconds(1)}
			until not sender:is_nil()
		end

		gce.send(base_id)
  end)
