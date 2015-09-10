--
-- Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local gce = require('gce')
local ep_adl = require('endpoint_adl')
local ni_adl = require('node_info_adl')
local libutil = require('libutil')

-- 返回是否登录成功+需要连接的其它node的信息
function login()
  local ctxid = gce.get_ctxid()
  local master = gce.make_svcid('master', 'master')
  local ec, sender, args, msg

  while true do
    gce.info('node<', ctxid, '> login master...')
    gce.link(master)
    gce.send(master, 'login')

    local ret = false
    ec, sender, args, msg = gce.match('login_ret').guard(master).recv(ret)
    if ec == gce.ec_guard then
      gce.info('node<', ctxid, '> login master error: ', ec)
    else
      ret = args[1]
      if ret then
        gce.info('node<', ctxid, '> login master success')
        args = gce.unpack(msg, '')
        local bind_ep = args[1]

        gce.bind(bind_ep)

        -- 告诉master已经准备好连接其它node，等待master命令
        gce.send(master, 'ready')
        ec, sender, args, msg = gce.match('ready_ret').guard(master).recv(ret)
        if ec == gce.ec_guard then
          gce.info('node<', ctxid, '> wait conn error: ', ec)
        else
          ret = args[1]
          if ret then
            gce.info('node<', ctxid, '> ready success, will connect to other node(s)')

            args = gce.unpack(msg, ni_adl.node_info_list)
            local ni_list = args[1]

            -- 连接指定的其它node，来构建全联通
            for _, ni in ipairs(ni_list.list_) do
              gce.connect(ni.svcid_.ctxid_, ni.ep_)
            end
            return true
          else
            -- 这个node在master端找不到，不再继续，直接退出，让相关人员处理
            args = gce.unpack(msg, '')
            gce.info('node<', ctxid, '> ready failed: ', args[1])
            return false
          end
        end
      else
        -- 这个node在master端找不到，说明出了配置错误，很有可能是人为错误，不再继续，直接退出，让相关人员处理
        args = gce.unpack(msg, '')
        gce.info('node<', ctxid, '> login master failed: ', args[1])
        return false
      end
    end
  end
end

gce.actor(
  function ()
		-- 连接master
    gce.connect('master', 'tcp://127.0.0.1:23333')

    -- 注册服务
    gce.register_service('node')
    local need_login = true
    local ctxid = gce.get_ctxid()
    local master = gce.make_svcid('master', 'master')
    local node = gce.make_svcid(ctxid, 'node')

    while true do
      if need_login and login() == false then
        break
      end
      need_login = false

      local ec, sender, args, msg = 
        gce.match('conn', 'quit').guard(master).recv()
      local ty = msg:getty()
      if ec ~= gce.ec_guard then
        if ty == gce.atom('conn') then
          -- 连接指定的node
          args = gce.unpack(msg, ni_adl.node_info)
          local ni = args[1]
          gce.connect(ni.svcid_.ctxid_, ni.ep_)
        elseif ty == gce.atom('quit') then
          gce.send(sender, 'quit_ret')
          -- 为了确保退出消息尽可能的发送成功，这里等待1秒
          gce.sleep_for(gce.seconds(1))
          break
        else
          gce.send(master, 'error', 'not available command: ' .. gce.deatom(ty))
        end
      else
        need_login = true
      end
    end

    gce.deregister_service('node')
    gce.info('node<', ctxid, '> quit')
  end)
