--
-- Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

-- 导入gce模块
local gce = require('gce')
local ep_adl = require('endpoint_adl')
local ni_adl = require('node_info_adl')
local libutil = require('libutil')

local node_stat = {}
node_stat.offline = 0
node_stat.login = 1
node_stat.online = 2

function stat2str(stat)
  if stat == node_stat.offline then
    return 'offline'
  elseif stat == node_stat.login then
    return 'login'
  else
    return 'online'
  end
end

function make_node_info(svcid, conn_ep)
  local ni = ni_adl.node_info()
  ni.svcid_ = svcid
  ni.ep_ = conn_ep
  return ni
end

function make_node_info_key(conn_ep)
  local ctxid = libutil.hash(conn_ep)
  local svcid = gce.make_svcid(ctxid, 'node')
  return tostring(svcid), svcid
end

function add_node_info(node_stat_list, conn_ep)
  local key, svcid = make_node_info_key(conn_ep)
  local ns = node_stat_list[key]
  if ns == nil then
    ns = {}
    ns.stat_ = node_stat.offline
    ns.ni_ = make_node_info(svcid, conn_ep)
    node_stat_list[key] = ns
    return nil
  else
    return ns.ni_.ep_
  end
end

function find_node_stat(node_stat_list, conn_ep)
  local key = make_node_info_key(conn_ep)
  return node_stat_list[key]
end

function get_node_stat(node_stat_list, node_id)
  local key = tostring(node_id)
  return node_stat_list[key]
end

function rmv_node_info(node_stat_list, conn_ep)
  local ns = find_node_stat(node_stat_list, conn_ep)
  if ns ~= nil then
    if ns.stat_ == node_stat.offline then
      local key = make_node_info_key(conn_ep)
      node_stat_list[key] = nil
      return 0
    end
    return 1
  end
  return 2
end

function update_node_stat(node_stat_list, node_id, stat)
  local key = tostring(node_id)
  local ns = node_stat_list[key]
  if ns ~= nil then
    ns.stat_ = stat
    return true
  else
    return false
  end
end

-- 使用gce.actor创建master
gce.actor(
  function ()
    -- bind监听地址
    gce.bind('tcp://0.0.0.0:23333')

    local ec, sender, args, msg
    gce.register_service('master')

    -- 接收从主线程传来的节点总数
    local _num_ = 0
    ec, sender, args = gce.match('init').recv(_num_)
    local node_num = args[1]

    local node_stat_list = {}
    local node_ep = 'tcp://127.0.0.1:'
    for i=1, node_num do
      local port = 23333 + i
      local conn_ep = node_ep .. port
      local ret = add_node_info(node_stat_list, conn_ep)
      if ret ~= nil then
        gce.error('node conn_ep hash collision! they are: \n  ', conn_ep, '\n  ', ret)
        gce.deregister_service('master')
        return
      end
    end

    local node_quit_id = gce.aid_nil
    local admin_id = gce.aid_nil

    gce.info('master start')

    while true do
      ec, sender, args, msg = gce.recv()
      local ty = msg:getty()
      if ty == gce.atom('login') then
        local node_id = gce.get_svcid(sender)
        assert (node_id ~= gce.svcid_nil)

        if update_node_stat(node_stat_list, node_id, node_stat.login) then
          local ns = get_node_stat(node_stat_list, node_id)
          assert (ns ~= nil)

          -- 构建这个node的bind的网络地址，使用0.0.0.0 + 其conn_ep中的端口
          local ep = ns.ni_.ep_
          local i = string.find(ep, ':', -6)
          local bind_ep = 'tcp://0.0.0.0:' .. string.sub(ep, i + 1)
          gce.send(sender, 'login_ret', true, bind_ep)
        else
          gce.send(sender, 'login_ret', false, 'node not found')
        end
      elseif ty == gce.atom('ready') then
        local node_id = gce.get_svcid(sender)
        assert (node_id ~= gce.svcid_nil)

        if update_node_stat(node_stat_list, node_id, node_stat.online) then
          local ready_ni = nil
          -- 让这个node连接其它online状态的node
          local ni_list = ni_adl.node_info_list()
          for k,ns in pairs(node_stat_list) do
            local ni = ns.ni_
            if ns.stat_ == node_stat.online then
              if ni.svcid_ ~= node_id then
                table.insert(ni_list.list_, ni)
              else
                ready_ni = ni
              end
            end
          end
          gce.send(sender, 'ready_ret', true, ni_list)

          -- 让其它online状态的node连接这个node
          assert (ready_ni ~= nil)
          for i,ni in ipairs(ni_list.list_) do
            gce.send(ni.svcid_, 'conn', ready_ni)
          end
        else
          gce.send(sender, 'ready_ret', false, 'node not found')
        end
      elseif ty == gce.atom('status') then
        admin_id = sender
        local total_desc = ''
        for svcid, ns in pairs(node_stat_list) do
          total_desc = total_desc .. '\n  node<' .. tostring(ns.ni_.svcid_.ctxid_) .. '>: ' .. stat2str(ns.stat_)
        end
        gce.send(sender, 'status_ret', total_desc)
      elseif ty == gce.atom('add') then
        admin_id = sender
        -- 添加新的node(s)进入集群
        args = gce.unpack(msg, ep_adl.endpoint_list)
        local ep_list = args[1]
        local desc = 'result: '
        for _, conn_ep in ipairs(ep_list.list_) do
          desc = desc .. '\n  '.. conn_ep
          local ret = add_node_info(node_stat_list, conn_ep)
          if ret ~= nil then
            desc = desc .. ' add failed, conn_ep hash collision! collision: ' .. ret
          else
            desc = desc .. ' add success'
          end
        end

        gce.send(admin_id, 'add_ret', desc)
      elseif ty == gce.atom('rmv') then
        admin_id = sender
        -- 将指定node(s)从集群移除
        args = gce.unpack(msg, ep_adl.endpoint_list)
        local ep_list = args[1]
        local desc = 'result: '
        for _, conn_ep in ipairs(ep_list.list_) do
          desc = desc .. '\n  '.. conn_ep
          local r = rmv_node_info(node_stat_list, conn_ep)
          if r == 1 then
            desc = desc .. ' rmv failed, node status is not offline, plz quit it before rmv'
          elseif r == 2 then
            desc = desc .. ' rmv failed, node not found'
          else
            desc = desc .. ' rmv success'
          end
        end

        gce.send(admin_id, 'rmv_ret', desc)
      elseif ty == gce.atom('quit') then
        admin_id = sender
        if node_quit_id ~= gce.aid_nil then
          -- 说明上一次quit还在进行中，返回错误
          gce.send(admin_id, 'error', 'last node_quit not end yet')
        else
          -- 每次处理quit命令，单独使用一个actor来进行
          args = gce.unpack(msg, ep_adl.endpoint_list)
          local ep_list = args[1]
          node_quit_id = gce.spawn('node_quit.lua')

          local ni_list = ni_adl.node_info_list()
          if table.maxn(ep_list.list_) == 0 then
            for _, ns in pairs(node_stat_list) do
              if ns.stat_ == node_stat.online then
                table.insert(ni_list.list_, ns.ni_)
              end
            end
          else
            -- 找出指定的node(s)
            for _, ep in ipairs(ep_list.list_) do
              local ns = find_node_stat(node_stat_list, ep)
              if ns ~= nil and ns.stat_ == node_stat.online then
                table.insert(ni_list.list_, ns.ni_)
              end
            end
          end
          gce.send(node_quit_id, 'init', ni_list)
        end
      elseif ty == gce.atom('quit_ret') then
        -- node_quit结束，返回结果给admin
        gce.send(admin_id, msg)
        node_quit_id = gce.aid_nil
      elseif ty == gce.atom('shutdown') then
        admin_id = sender
        gce.send(admin_id, 'shutdown_ret')
        -- 为了确保回应消息尽可能的成功，这里等待1秒
        gce.sleep_for(gce.seconds(1))
        break
      elseif ty == gce.atom('error') then
        local node_id = gce.get_svcid(sender)
        assert (node_id ~= gce.svcid_nil)

        gce.send(admin_id, msg)
      elseif ty == gce.exit then
        -- 当前除了node之外，没有其他actor和master链接（admin使用的是单向的），所以肯定是node
        local node_id = gce.get_svcid(sender)
        assert (node_id ~= gce.svcid_nil)
        --gce.info('node exit: ', tostring(node_id))
        update_node_stat(node_stat_list, node_id, node_stat.offline)

        if node_quit_id ~= gce.aid_nil then
          gce.send(node_quit_id, 'node_exit', node_id)
        end
      end
    end

    gce.deregister_service('master')
    gce.info('master quit')
  end)
