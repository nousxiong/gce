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
  	local ec, base_aid, args = gce.recv('init', gce.actor_id)
		local last_id = args[1]

		local msg
		ec, sender, args, msg = gce.recv()
		if (last_id ~= gce.aid_nil) then
			gce.relay(last_id, msg)
		else
			gce.reply(sender, 'hello')
		end
  end)
