--
-- Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local gce = require('gce')

local config = {}
config.nodes = {}

gce.actor(
  function ()
  	local ec, sender, args, msg

  	ec, sender, args = gce.match('init').recv('')
  	local ctxid = args[1]

  	gce.info('master<', ctxid, '> running...')
  	gce.info('master<', ctxid, '> end.')
  end)
