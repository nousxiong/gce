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
local param_adl = require('param_adl')


-- 字符串分割函数
-- 传入字符串和分隔符，返回分割后的table
function string.split(str, delimiter)
  if str==nil or str=='' or delimiter==nil then
    return nil
  end

  local result = {}
  for match in (str..delimiter):gmatch("(.-)"..delimiter) do
      table.insert(result, match)
  end
  return result
end

-- 从base actor取得输入的命令及其参数
function getcmd()
  local ec, sender, args, msg = 
    gce.match('cmd', 'end').recv('', param_adl.param_list)
  local ty = msg:getty()
  if ty == gce.atom('cmd') then
    local tok_list = args[2]
    return sender, args[1], tok_list.list_
  else
    return sender, nil
  end
end

gce.actor(
  function ()
    local ec, sender, args, msg
		-- 连接master
    gce.connect('master', 'tcp://127.0.0.1:23333')

    local master = gce.make_svcid('master', 'master')
    gce.monitor(master)

    local help_desc = '\nadmin usage: \n  status: get node(s) state\n  quit: stop all nodes\n  shutdown: stop master and admin\n  help: display this info'

    gce.info(help_desc)

    while true do
      local base_id, cmd, tok_list = getcmd()
      if cmd == nil then
        break
      end

      if cmd == 'status' then
        gce.send(master, 'status')
        ec, sender, args, msg = 
          gce.match('status_ret', 'error').guard(master).recv()
        if ec == gce.ec_guard then
          gce.error(cmd, ' error')
          break
        end

        if msg:getty() == gce.atom('status_ret') then
          args = gce.unpack(msg, '')
          local total_desc = args[1]
          gce.info('node(s) status: ', total_desc)
        end
      elseif cmd == 'add' then
        -- 根据用户输入添加新的node(s)到集群，此命令仅为告诉master新node(s)的身份，之后需要手动启动新node(s)来真正加入
        if table.maxn(tok_list) >= 1 then
          local ep_list = ep_adl.endpoint_list()
          -- 复制所有输入的ep到ep_list
          ep_list.list_ = tok_list
          gce.send(master, 'add', ep_list)

          ec, sender, args, msg = 
            gce.match('add_ret', 'error').guard(master).recv()
          if ec == gce.ec_guard then
            gce.error(cmd, ' error')
            break
          end

          if msg:getty() == gce.atom('add_ret') then
            args = gce.unpack(msg, '')
            local desc = args[1]
            gce.info(cmd, ' ', desc)
          end
        else
          gce.error('not enough params; Usage: add <conn_ep> [conn_ep] ... ')
        end
      elseif cmd == 'rmv' then
        -- 根据用户输入将指定的node(s)从集群中移除，此命令仅为告诉master要移除的node(s)的身份，在此之前需要用quit命令来让其退出
        if table.maxn(tok_list) >= 1 then
          local ep_list = ep_adl.endpoint_list()
          -- 复制所有输入的ep到ep_list
          ep_list.list_ = tok_list
          gce.send(master, 'rmv', ep_list)

          ec, sender, args, msg = 
            gce.match('rmv_ret', 'error').guard(master).recv()
          if ec == gce.ec_guard then
            gce.error(cmd, ' error')
            break
          end

          if msg:getty() == gce.atom('rmv_ret') then
            args = gce.unpack(msg, '')
            local desc = args[1]
            gce.info(cmd, ' ', desc)
          end
        else
          gce.error('not enough params; Usage: rmv <conn_ep> [conn_ep] ... ')
        end
      elseif cmd == 'quit' then
        local ep_list = ep_adl.endpoint_list()
        -- 复制所有输入的ep到ep_list
        ep_list.list_ = tok_list
        gce.send(master, 'quit', ep_list)
        ec, sender, args, msg = 
          gce.match('quit_ret', 'error').guard(master).recv()
        if ec == gce.ec_guard then
          gce.error(cmd, ' error')
          break
        end

        if msg:getty() == gce.atom('quit_ret') then
          args = gce.unpack(msg, '')
          local total_desc = args[1]
          gce.info(cmd, ' ok, ', total_desc)
        end
      elseif cmd == 'shutdown' then
        gce.send(master, 'shutdown')
        ec, sender, args, msg = 
          gce.match('shutdown_ret', 'error').guard(master).recv()
        if ec == gce.ec_guard then
          gce.error(cmd, ' error')
          break
        end

        if msg:getty() == gce.atom('shutdown_ret') then
          gce.info(cmd, ' ok')
          break
        end
      elseif cmd == 'help' then
        gce.info(help_desc)
      end

      if msg ~= nil and msg:getty() == gce.atom('error') then
        args = gce.unpack(msg, '')
        local errmsg = args[1]
        gce.error(errmsg)
      end

      -- 回复base actor命令执行完毕
      gce.send(base_id, 'cmd_ret')
    end
  end)
