--
-- Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local gce = require('gce')
local asio = require('asio')

gce.actor(
  function ()
    local ec, sender, args, msg, err
    local ecount = 10
    local cln_count = 10
    local ptype = asio.plength

    ec, sender, args = gce.match('init').recv(ptype, asio.tcp_endpoint_itr)
    ptype = args[1]
    local eitr = args[2]

    local session_list = {}
    
    for i=1, cln_count do
      local parser
      if ptype == asio.plength then
        parser = asio.simple_length()
      else
        parser = asio.simple_regex('||')
      end
      local skt_impl = asio.tcp_socket_impl()
      local opt = asio.sn_option()
      opt.id = gce.atom(i)
      local sn = asio.session(parser, skt_impl, eitr, opt)
      local sn_data = {}
      sn_data.sn_ = sn
      sn_data.ecount_ = ecount
      session_list[gce.match2number(opt.id)] = sn_data
      sn:open()
    end
    
    local str = 'hello world!||'
    local m = gce.message()
    gce.pack(m, str)
    
    while true do
      ec, sender, args, msg = 
        gce.match(asio.sn_open, asio.sn_recv, asio.sn_close).recv(gce.atom)
      if msg:getty() == asio.sn_close then
        local snid = args[1]
        session_list[gce.match2number(snid)] = nil
        cln_count = cln_count - 1
        if cln_count == 0 then
          break
        end
      elseif msg:getty() == asio.sn_open then
        local snid = args[1]
        local sn_data = session_list[gce.match2number(snid)]
        assert (sn_data ~= nil)
        
        sn_data.sn_:send(m)
      elseif msg:getty() == asio.sn_recv then
        local snid = args[1]
        local sn_data = session_list[gce.match2number(snid)]
        assert (sn_data ~= nil)
        
        local sn = sn_data.sn_
        args = gce.unpack(msg, '')
        local echo_str = args[1]
        if echo_str == 'bye||' then
          sn:close()
        else
          assert (echo_str == str)
          sn_data.ecount_ = sn_data.ecount_ - 1
          if sn_data.ecount_ == 0 then
            local bye_msg = gce.message()
            gce.pack(bye_msg, 'bye||')
            sn:send(bye_msg)
          else
            sn:send(m)
          end
        end
      else
        error ('msg type error')
      end
    end
  end)
