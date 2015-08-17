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
  	gce.register_service("other_svc")
    local echo_num = 10
    local echo_svc = gce.make_svcid('two', 'echo_svc')

    gce.monitor(echo_svc)
    for i=1, echo_num do
      gce.send(echo_svc, 'echo')
      gce.match('echo').guard(echo_svc).recv()
    end

    gce.send(echo_svc, 'end')
    gce.recv(gce.exit)
    gce.deregister_service("other_svc")
  end)
