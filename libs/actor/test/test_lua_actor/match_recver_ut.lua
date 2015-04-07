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
  	local base_id = gce.get_aid()
  	local name = 'test_lua_actor/match_recver_actor.lua'
  	local aid = gce.spawn(name, gce.monitored)
  	gce.send(aid, 'init', base_id)
  	local ignored = gce.spawn(name, gce.monitored)
  	gce.send(ignored, 'init', gce.aid_nil)

  	local ec, sender, args, msg

  	ec = gce.match('timeout').timeout(gce.millisecs(1)).recv()
  	assert (ec == gce.ec_timeout)

  	local i = -1
  	ec, sender, args = gce.match('catch').guard(aid).recv(i)
  	assert (args[1] == 0)
  	gce.print('catch ', args[1])

  	ec, sender, args = gce.match('catch').guard(aid).recv(i)
  	assert (args[1] == 1)
  	gce.print('catch ', args[1])

  	local res = gce.request(aid, 'resp', i)
  	ec, sender, args, msg = gce.match(res).respond(i)
  	assert (ec == gce.ec_guard)
  	assert (msg:getty() == gce.exit)

  	ec = gce.match('not_catch').guard(aid).recv(i)
  	assert (ec == gce.ec_guard)
  end)
