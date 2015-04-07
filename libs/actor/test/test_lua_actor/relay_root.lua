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
  	local free_actor_num = 10
  	local last_id = gce.actor_id()
  	local first_id = gce.actor_id()

  	for i=1, free_actor_num do
			local aid = gce.spawn('test_lua_actor/relay_actor.lua')
			gce.send(aid, 'init', last_id)
			if i == 1 then
				first_id = aid
			end
			last_id = aid
		end
		
		local i = 1
		local res = gce.request(last_id, 'hi', i)
		local ec, sender, args, msg = gce.respond(res)
		assert (sender == first_id)
		assert (msg:getty() == gce.atom('hello'))
  end)
