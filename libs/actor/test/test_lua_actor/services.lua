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
  	gce.register_service('echo_svc')

    while true do
      local ec, sender, args, msg = gce.recv()
      if msg:getty() == gce.atom('echo') then
        gce.send(sender, msg)
      else
        break
      end
    end

    gce.deregister_service('echo_svc')
  end)
