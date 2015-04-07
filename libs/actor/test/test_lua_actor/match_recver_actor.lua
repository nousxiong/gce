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
  	local ec, sender, args = gce.recv('init', gce.actor_id)
    local base_id = args[1]

    if base_id ~= gce.aid_nil then
      local i = 0
      gce.send(base_id, 'catch', i)
      i = i + 1
      gce.send(base_id, 'catch', i)

      ec, sender = gce.match('resp').recv()
      assert (sender == base_id)
      gce.send(sender, gce.exit)

      i = i + 1
      gce.send(base_id, 'catch', i)
    end
  end)
