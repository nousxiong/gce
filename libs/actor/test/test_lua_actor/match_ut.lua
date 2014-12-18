--
-- Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local gce = require("gce")

gce.run_actor(
  function ()
  	local size = 100
  	local base_id = gce.get_aid()
  	local name = "test_lua_actor/match_recver.lua"

  	local aid_list = {}
  	for i=1, size do
			local aid = gce.spawn(name, gce.monitored)
			aid_list[i] = aid
			gce.send(aid, "init", base_id);
		end

		for i=1, size do
			local aid = gce.spawn(name, gce.monitored)
			gce.send(aid, "init", gce.aid());
		end
		
		local sender = gce.recv{"timeout", gce.millisecs(1)}
		assert (sender:is_nil())

		for i=1, size do
			local aid, args, m = gce.recv{"not_catch", aid_list[i], gce.seconds(180)}
			assert (not aid:is_nil())
			assert (m:get_type() == gce.exit)
		end

		for i=1, size do
			gce.recv(gce.exit)
		end
  end)
