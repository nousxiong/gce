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
	local path = 'test_lua_asio/' .. name .. '.lua'
	gce.print(name .. ' test begin')
	gce.spawn(path, gce.monitored)
	gce.check_exit(name)
	gce.print(name .. ' test end')
end

gce.actor(
  function ()
		local ec, sender, args = gce.recv('init', gce.actor_id)
		local base_aid = args[1]

		test('timer_ut')
		test('signal_ut')
		test('tcp_ut')
		if gce.openssl ~= 0 then
			test('ssl_ut')
		end
		test('tcpsn_ut')
		test('tcpsn_idle_ut')
		if gce.openssl ~= 0 then
			test('sslsn_ut')
			test('sslsn_idle_ut')
		end
  end)
