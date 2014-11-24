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
  	local end_num = 0
  	while true do
  		local sender, args, msg = gce.recv()
  		local ty = msg:get_type()
  		if ty:equals(gce.atom("echo")) then
  			gce.send(sender, msg)
  		else
  			end_num = end_num + 1
  			if end_num == 2 then
  				break
  			end
  		end
  	end
  end)
