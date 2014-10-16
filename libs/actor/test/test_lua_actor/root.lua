--
-- Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

require("gce")

local test = function (name)
	local path = "test_lua_actor/" .. name .. ".lua"
	gce.print(name .. " test begin")
	gce.spawn(path, gce.linked)
	gce.recv(gce.exit)
	gce.print(name .. " test end")
end

gce.run_actor(
  function ()
		local sender, args = gce.recv("init", gce.aid())
		local base_aid = args[1]

		test("pingpong_ut")
		test("relay_ut")

		gce.send(base_aid)
  end)
