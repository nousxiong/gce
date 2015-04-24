--
-- Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local gce = require('gce')

local test = function (name)
	local path = 'test_lua_actor/' .. name .. '.lua'
	gce.print(name .. ' test begin')
	gce.spawn(path, gce.monitored)
	local ec, sender, args = gce.recv(gce.exit, gce.atom, '')
	if args[1] ~= gce.exit_normal then
		gce.print(name, ' ', gce.deatom(args[1]), ': ', args[2])
	end
	gce.print(name .. ' test end')
end

gce.actor(
  function ()
		local ec, sender, args = gce.recv('init', gce.actor_id)
		local base_aid = args[1]

		test('match_ut')
		test('match_recver_ut')
		test('pingpong_ut')
		test('relay_ut')
  end)
