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
  	local root_num = 1
  	for i=1, root_num do
			gce.spawn('test_lua_actor/relay_root.lua', gce.monitored)
		end
		
		for i=1, root_num do
			gce.recv(gce.exit)
		end
  end)
