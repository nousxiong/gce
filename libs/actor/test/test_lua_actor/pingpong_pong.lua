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
  	local ec, sender, args = gce.recv('init', gce.actor_id)
		base_aid = args[1]

		while true do
			local ec, aid, args, m = gce.recv()
			if m:getty() == gce.atom(2) then
				break
			else
				gce.send(aid, 1)
			end
		end
		gce.send(base_aid)
  end)
