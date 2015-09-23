--
-- Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local gce = require('gce')
local ni_adl = require('node_info_adl')

function add_quited_node(quited_node_list, node_id)
  quited_node_list[tostring(node_id)] = node_id
end

function find_quited_node(quited_node_list, node_id)
  return quited_node_list[tostring(node_id)]
end

gce.actor(
  function ()
    local ctxid = gce.get_ctxid()
    local master = gce.make_svcid('master', 'master')
    gce.monitor(master)

    local ready_quit_num = 0
    local quited_node_list = {}
    local ec, sender, args, msg

    ec, sender, args, msg = 
      gce.match('init').guard(master).recv(ni_adl.node_info_list)
    if ec == gce.ec_guard then
      gce.error('master quited, no quit will handle')
      return
    end

    -- 根据要quit的node信息，发送quit给node(s)
    local ni_list = args[1]
    for _, ni in ipairs(ni_list.list_) do
      ready_quit_num = ready_quit_num + 1
      gce.send(ni.svcid_, 'quit')
    end

    -- 如果已经全部非online状态，则立刻返回
    if ready_quit_num == 0 then
      gce.send(master, 'quit_ret', '0 node quited')
    end

    -- 依次接收所有node的quit_ret消息
    local i = 1
    local size = 0
    while i<=ready_quit_num do
      ec, sender, args, msg = 
        gce.match('quit_ret', 'node_exit').guard(master).recv()
      if ec == gce.ec_guard then
        gce.error('master quited, cannot continue, quit asap')
        return
      else
        local ty = msg:getty()
        if ty == gce.atom('quit_ret') then
          local node_id = gce.get_svcid(sender)
          assert (node_id ~= gce.svcid_nil)

          -- 将退出的node插入列表
          add_quited_node(quited_node_list, node_id)
          i = i + 1
          size = size + 1
        else
          args = gce.unpack(msg, gce.service_id)
          local node_id = args[1]
          assert (node_id ~= gce.svcid_nil)
          if find_quited_node(node_id) == nil then
            -- 这种情况表明node非正常退出（master转发的node的exit消息），不加入quited列表
            i = i + 1
          end
        end
      end
    end

    -- 全部结束，返回结果给master
    local total_desc = size .. ' node(s) quited: '
    for _, svcid in pairs(quited_node_list) do
      total_desc = total_desc .. '\n  node<'
      total_desc = total_desc .. tostring(svcid.ctxid_)
      total_desc = total_desc .. '>'
    end

    gce.send(master, 'quit_ret', total_desc)
  end)
