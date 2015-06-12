--
-- Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local libgce = require('libgce')
local gce = {}

-- gce packer type
gce.pkr_amsg = libgce.pkr_amsg
gce.pkr_adata = libgce.pkr_adata
gce.packer = libgce.packer
gce.openssl = libgce.openssl

local mt_adl = nil
local dur_adl = nil
local netopt_adl = nil
local aid_adl = nil
local svcid_adl = nil
if gce.packer == gce.pkr_adata then
  mt_adl = require('match_adl')
  aid_adl = require('actor_id_adl')
  svcid_adl = require('service_id_adl')
  dur_adl = require('duration_adl')
  netopt_adl = require('net_option_adl')
end

libgce.init_nil()

-- enum and constant def
gce.ty_pattern = libgce.ty_pattern
gce.ty_message = libgce.ty_message
gce.ty_response = libgce.ty_response
gce.ty_chunk = libgce.ty_chunk
gce.ty_errcode = libgce.ty_errcode
gce.ty_match = libgce.ty_match
gce.ty_actor_id = libgce.ty_actor_id
gce.ty_service_id = libgce.ty_service_id
gce.ty_duration = libgce.ty_duration
gce.ty_userdef = libgce.ty_userdef
gce.ty_lua = libgce.ty_lua
gce.ty_other = libgce.ty_other

gce.ec_ok = libgce.ec_ok
gce.ec_timeout = libgce.ec_timeout
gce.ec_guard = libgce.ec_guard

gce.dur_raw = libgce.dur_raw
gce.dur_microsec = libgce.dur_microsec
gce.dur_millisec = libgce.dur_millisec
gce.dur_second = libgce.dur_second
gce.dur_minute = libgce.dur_minute
gce.dur_hour = libgce.dur_hour

gce.no_link = libgce.no_link
gce.linked = libgce.linked
gce.monitored = libgce.monitored

gce.stackful = libgce.stackful
gce.stackless = libgce.stackless
gce.luaed = libgce.luaed

gce.luaver = libgce.luaver

gce.infin = libgce.infin
gce.zero = libgce.zero
gce.aid_nil = libgce.aid_nil
gce.svcid_nil = libgce.svcid_nil
gce.match_nil = libgce.match_nil
gce.err_nil = libgce.err_nil

-- functions
function gce.actor(f)
  local co = coroutine.create(f)
  libgce.self:init_coro(co, gce.resume)
  gce.resume(co)
end

function gce.resume(co)
  local rt, err = coroutine.resume(co)
  if not rt then
    error(err)
  end
end

function gce.send(target, cfg, ...)
  local oty = libgce.typeof(target)
  local m = gce.message(cfg, ...)
  if oty == gce.ty_actor_id then
    libgce.self:send(target, m)
  elseif oty == gce.ty_service_id then
    libgce.self:send2svc(target, m)
  else
    local name = gce.atom(target)
    libgce.self:send2svcs(name, m)
  end
end

function gce.relay(target, m)
  local oty = libgce.typeof(target)
  if oty == gce.ty_actor_id then
    libgce.self:relay(target, m)
  elseif oty == gce.ty_service_id then
    libgce.self:relay2svc(target, m)
  else
    local name = gce.atom(target)
    libgce.self:relay2svcs(name, m)
  end
end

function gce.request(target, cfg, ...)
  local oty = libgce.typeof(target)
  local m = gce.message(cfg, ...)
  if oty == gce.ty_actor_id then
    return libgce.self:request(target, m)
  elseif oty == gce.ty_service_id then
    return libgce.self:request2svc(target, m)
  else
    local name = gce.atom(target)
    return libgce.self:request2svcs(name, m)
  end
end

function gce.reply(target, cfg, ...)
  local m = gce.message(cfg, ...)
  assert (libgce.typeof(target) == gce.ty_actor_id)
  libgce.self:reply(target, m)
end

function gce.link(target)
  assert (libgce.typeof(target) == gce.ty_actor_id)
  libgce.self:link(target)
end

function gce.monitor(target)
  assert (libgce.typeof(target) == gce.ty_actor_id)
  libgce.self:monitor(target)
end

function gce.match(...)
  local receiver = libgce.receiver
  assert (receiver ~= nil)
  if receiver.matches == nil then
    receiver.matches = {}
  end
  local matches = receiver.matches
  if receiver.resp == nil then
    receiver.resp = {}
  end

  local resp = receiver.resp
  resp[1] = nil
  resp[2] = nil
  receiver.recver = nil
  receiver.tmo = nil

  matches[#matches + 1] = nil
  for i,v in ipairs{...} do
    if i == 1 then
      local oty = libgce.typeof(v)
      if oty == gce.ty_response then
        resp[1] = v
        break
      end
    end
    matches[i] = gce.atom(v)
    matches[i+1] = nil
  end

  return receiver
end

function gce.recv(cfg, ...)
  local co = nil
  if cfg == nil then
    co = libgce.self:recv()
  else
    local ty = type(cfg)
    local patt
    if ty == 'table' and cfg.adtype == nil then
      -- match_t*N + [aid/svcid] + [timeout] (N >= 0)
      local tmo, recver
      local last_idx = #cfg
      assert (last_idx >= 1)

      local last = cfg[last_idx]
      local last_ty = libgce.typeof(last)
      if last_ty ~= gce.ty_match then
        if last_ty == gce.ty_duration then
          tmo = last
          cfg[last_idx] = nil
          if last_idx - 1 >= 1 then
            local last_sec = cfg[last_idx - 1]
            local last_sec_ty = libgce.typeof(last_sec)
            if last_sec_ty == gce.ty_actor_id or last_sec_ty == gce.ty_service_id then
              recver = last_sec
              cfg[last_idx - 1] = nil
            end
          end
        else
          recver = last
          cfg[last_idx] = nil
        end
      end

      patt = gce.pattern(libgce.unpack(cfg))
      if tmo ~= nil then
        patt:set_timeout(tmo)
      end
      if recver ~= nil then
        gce.set_match_recver(patt, recver)
      end
    elseif ty == 'string' or ty == 'number' then
      patt = gce.pattern(cfg)
    else
      local oty = libgce.typeof(cfg)
      if oty == gce.ty_pattern then
        patt = cfg
      elseif oty == gce.ty_match then
        patt = gce.pattern(cfg)
      else -- timeout
        assert (oty == gce.ty_duration)
        patt = libgce.make_patt()
        patt:set_timeout(cfg)
      end
    end
    co = libgce.self:recv_match(patt)
  end
  if co ~= nil then
    coroutine.yield(co)
  end
  local args = nil
  if libgce.recv_ec == gce.ec_ok then
    args = gce.unpack(libgce.recv_msg, ...)
  end
  return libgce.recv_ec, libgce.recv_sender, args, libgce.recv_msg
end

function gce.respond(cfg, ...)
  assert (cfg ~= nil)
  local ty = type(cfg)
  local res, tmo
  if ty == 'table' then
    res = cfg[1]
    tmo = cfg[2]
  else
    assert (ty == 'userdata')
    res = cfg
    tmo = nil
  end

  local co = nil
  if tmo == nil then
    co = libgce.self:recv_response(res)
  else
    co = libgce.self:recv_response_timeout(res, tmo)
  end
  if co ~= nil then
    coroutine.yield(co)
  end
  local args = nil
  if libgce.recv_ec == gce.ec_ok then
    args = gce.unpack(libgce.recv_msg, ...)
  end
  return libgce.recv_ec, libgce.recv_sender, args, libgce.recv_msg
end

function gce.sleep_for(dur)
  local co = libgce.self:sleep_for(dur)
  if co ~= nil then
    coroutine.yield(co)
  end
end

function gce.bind(ep, opt)
  opt = opt or gce.net_option()
  local co = libgce.self:bind(ep, opt)
  if co ~= nil then
    coroutine.yield(co)
  end
end

function gce.connect(target, ep, opt)
  opt = opt or gce.net_option()
  target = gce.atom(target)
  local co = libgce.self:connect(target, ep, opt)
  if co ~= nil then
    coroutine.yield(co)
  end
  return libgce.conn_ret, libgce.conn_errmsg
end

function gce.spawn(func, link_type, sync_sire)
  link_type = link_type or gce.no_link
  sync_sire = sync_sire or false
  local co = libgce.self:spawn(gce.luaed, func, sync_sire, link_type, gce.stacksize())
  if co ~= nil then
    coroutine.yield(co)
  end
  return libgce.spawn_aid
end

function gce.spawn_native(spw_type, func, link_type, sync_sire, stack_size)
  link_type = link_type or gce.no_link
  sync_sire = sync_sire or false
  stack_size = stack_size or gce.stacksize()
  local co = libgce.self:spawn(spw_type, func, sync_sire, link_type, stack_size)
  if co ~= nil then
    coroutine.yield(co)
  end
  return libgce.spawn_aid
end

function gce.spawn_remote(spw_type, func, ctxid, link_type, stack_size, tmo)
  link_type = link_type or gce.no_link
  stack_size = stack_size or gce.stacksize()
  tmo = tmo or gce.seconds(180)
  local co = libgce.self:spawn_remote(spw_type, func, ctxid, link_type, stack_size, tmo)
  if co ~= nil then
    coroutine.yield(co)
  end
  return libgce.spawn_aid
end

function gce.register_service(name)
  assert (name ~= nil)
  name = gce.atom(name)
  libgce.self:register_service(name)
end

function gce.deregister_service(name)
  assert (name ~= nil)
  name = gce.atom(name)
  libgce.self:deregister_service(name)
end

function gce.set_match_recver(patt, recver)
  assert (patt:gcety() == gce.ty_pattern)
  local oty = libgce.typeof(recver)
  assert (oty == gce.ty_actor_id or oty == gce.ty_service_id)
  if oty == gce.ty_actor_id then
    patt:set_match_aid(recver)
  else
    patt:set_match_svcid(recver)
  end
end

function gce.get_aid()
  return libgce.self:get_aid()
end

function gce.get_ctxid()
  return libgce.self:get_ctxid()
end

function gce.duration(v)
  v = v or 0
  if gce.packer == gce.pkr_amsg then
    return libgce.make_dur(v)
  elseif gce.packer == gce.pkr_adata then
    local dur = dur_adl.duration()
    dur.val_ = v
    dur.ty_ = gce.dur_raw
    return dur
  else
    error('gce.packer invalid')
  end
end

function gce.millisecs(v)
  v = v or 0
  if gce.packer == gce.pkr_amsg then
    return libgce.make_millisecs(v)
  elseif gce.packer == gce.pkr_adata then
    local dur = dur_adl.duration()
    dur.val_ = v
    dur.ty_ = gce.dur_millisec
    return dur
  else
    error('gce.packer invalid')
  end
end

function gce.seconds(v)
  v = v or 0
  if gce.packer == gce.pkr_amsg then
    return libgce.make_seconds(v)
  elseif gce.packer == gce.pkr_adata then
    local dur = dur_adl.duration()
    dur.val_ = v
    dur.ty_ = gce.dur_second
    return dur
  else
    error('gce.packer invalid')
  end
end

function gce.minutes(v)
  v = v or 0
  if gce.packer == gce.pkr_amsg then
    return libgce.make_minutes(v)
  elseif gce.packer == gce.pkr_adata then
    local dur = dur_adl.duration()
    dur.val_ = v
    dur.ty_ = gce.dur_minute
    return dur
  else
    error('gce.packer invalid')
  end
end

function gce.hours(v)
  v = v or 0
  if gce.packer == gce.pkr_amsg then
    return libgce.make_hours(v)
  elseif gce.packer == gce.pkr_adata then
    local dur = dur_adl.duration()
    dur.val_ = v
    dur.ty_ = gce.dur_hour
    return dur
  else
    error('gce.packer invalid')
  end
end

function gce.chunk(cfg)
  return libgce.make_chunk(cfg)
end

function gce.errcode()
  return libgce.make_errcode()
end

function gce.message(cfg, ...)
  local m = nil
  if cfg ~= nil then
    local oty = libgce.typeof(cfg)
    if oty == gce.ty_message then
      m = cfg
    end

    if m == nil then
      m = libgce.make_msg()
    end

    if oty == gce.ty_lua then
      local mt = gce.atom(cfg)
      m:setty(mt)
    elseif oty == gce.ty_match then
      m:setty(cfg)
    end
  end
  if m == nil then
    m = libgce.make_msg()
  end
  gce.pack(m, ...)
  return m
end

function gce.actor_id()
  if gce.packer == gce.pkr_amsg then
    return libgce.make_aid()
  elseif gce.packer == gce.pkr_adata then
    return aid_adl.actor_id()
  else
    error('gce.packer invalid')
  end
end

function gce.service_id()
  if gce.packer == gce.pkr_amsg then
    return libgce.make_svcid()
  elseif gce.packer == gce.pkr_adata then
    return svcid_adl.service_id()
  else
    error('gce.packer invalid')
  end
end

function gce.pattern(...)
  local rt = libgce.make_patt()
  for _,v in ipairs{...} do
    rt:add_match(v)
  end
  return rt
end

function gce.net_option()
  return libgce.make_netopt()
end

function gce.atom(v)
  if v == nil then
    return gce.make_match(gce.match_nil.val_)
  end

  local ty = type(v)
  if ty == 'string' then
    return libgce.atom(v)
  elseif ty == 'number' then
    return gce.make_match(v)
  else
    if gce.packer == gce.pkr_amsg then
      assert (ty == 'userdata')
      assert (v:gcety() == gce.ty_match)
    elseif gce.packer == gce.pkr_adata then
      assert (ty == 'table')
      assert (v.adtype ~= nil)
      assert (v:adtype() == mt_adl.match)
    else
      error('gce.packer invalid')
    end
    return v
  end
end

function gce.deatom(v)
  assert (libgce.typeof(v) == gce.ty_match)
  return libgce.deatom(v)
end

function gce.stacksize()
  return libgce.stacksize
end

function gce.print(...)
  libgce.print(gce.concat(...))
end

function gce.debug(...)
  libgce.self:log_debug(gce.concat(...))
end

function gce.info(...)
  libgce.self:log_info(gce.concat(...))
end

function gce.warn(...)
  libgce.self:log_warn(gce.concat(...))
end

function gce.error(...)
  libgce.self:log_error(gce.concat(...))
end

function gce.fatal(...)
  libgce.self:log_fatal(gce.concat(...))
end

function gce.pack(m, ...)
  for _,v in ipairs{...} do
    gce.serialize(m, v)
  end
end

function gce.unpack(m, ...)
  local res = {}
  for i,v in ipairs{...} do
    res[i] = gce.deserialize(m, v)
  end
  return res
end

function gce.check_exit(pre, printer)
  pre = pre or ''
  printer = printer or gce.print
  local ec, sender, args = gce.match(gce.exit).recv(gce.atom, '')
  local mt = args[1]
  if mt ~= gce.exit_normal then
    printer(pre, ' ', gce.deatom(mt), ': ', args[2])
  end
end

gce.exit = gce.atom('gce_exit')
gce.exit_normal = gce.atom('gce_ex_normal')

-------------------internal use-------------------
function gce.make_match(v)
  if gce.packer == gce.pkr_amsg then
    return libgce.make_match(v)
  elseif gce.packer == gce.pkr_adata then
    local mt = mt_adl.match()
    mt.val_ = v
    return mt
  else
    error('gce.packer invalid')
  end
end

function gce.concat(...)
  local t = {}
  for _,v in ipairs{...} do
    t[#t + 1] = tostring(v)
  end
  return table.concat(t)
end

function gce.serialize(m, o)
  ty = type(o)
  if ty == 'number' then
    libgce.pack_number(m, o)
  elseif ty == 'string' then
    libgce.pack_string(m, o)
  elseif ty == 'boolean' then
    libgce.pack_boolean(m, o)
  else
    libgce.pack_object(m, o)
  end
end

function gce.deserialize(m, o)
  ty = type(o)
  if ty == 'number' then
    return libgce.unpack_number(m)
  elseif ty == 'string' then
    return libgce.unpack_string(m)
  elseif ty == 'boolean' then
    return libgce.unpack_boolean(m)
  else
    if ty == 'function' then
      o = o()
      if (type(o) == 'function') then
        o = o()
      end
    end
    libgce.unpack_object(m, o)
    return o
  end
end

if gce.packer == gce.pkr_adata then
  -- set object's method (through metatable)
  local set_method = function (o, name, f)
    local mt = getmetatable(o)
    if rawequal(mt, nil) then error('obj must have a metatable!') end
    mt[name] = f
  end

  local mt_eq = function(lhs, rhs)
    if rawequal(lhs, nil) or rawequal(rhs, nil) then return false end
    if rawequal(lhs, rhs) then return true end

    return lhs.val_ == rhs.val_
  end

  local svcid_eq = function(lhs, rhs)
    if rawequal(lhs, nil) or rawequal(rhs, nil) then return false end
    if rawequal(lhs, rhs) then return true end

    return 
      lhs.nil_ == rhs.nil_ and
      lhs.ctxid_ == rhs.ctxid_ and
      lhs.name_ == rhs.name_
  end

  local aid_eq = function(lhs, rhs)
    if rawequal(lhs, nil) or rawequal(rhs, nil) then return false end
    if rawequal(lhs, rhs) then return true end

    return 
      lhs.ctxid_ == rhs.ctxid_ and
      lhs.timestamp_ == rhs.timestamp_ and
      lhs.uintptr_ == rhs.uintptr_ and
      lhs.svc_id_ == rhs.svc_id_ and
      lhs.type_ == rhs.type_ and
      lhs.in_pool_ == rhs.in_pool_ and
      lhs.sid_ == rhs.sid_ -- no svc_, bcz no need
  end

  local tmp_svcid = gce.service_id()
  local tmp_aid = gce.actor_id()
  local tmp_dur = gce.duration()
  local tmp_mt = gce.atom()

  set_method(tmp_mt, '__eq', mt_eq)
  set_method(tmp_mt, '__tostring', libgce.mt_tostring)
  set_method(tmp_aid, '__eq', aid_eq)
  set_method(tmp_aid, '__tostring', libgce.aid_tostring)
  set_method(tmp_svcid, '__eq', svcid_eq)
  set_method(tmp_svcid, '__tostring', libgce.svcid_tostring)
  set_method(tmp_dur, '__eq', libgce.dur_eq)
  set_method(tmp_dur, '__lt', libgce.dur_lt)
  set_method(tmp_dur, '__le', libgce.dur_le)
  set_method(tmp_dur, '__add', libgce.dur_add)
  set_method(tmp_dur, '__sub', libgce.dur_sub)
  set_method(tmp_dur, '__tostring', libgce.dur_tostring)
  set_method(tmp_dur, 'type', function (dur) return dur.ty_ end)
end

function libgce.typeof(o)
  local ty = type(o)
  if ty == 'table' then
    if o.adtype ~= nil then
      local adty = o:adtype()
      if adty == aid_adl.actor_id then
        return gce.ty_actor_id
      elseif adty == svcid_adl.service_id then
        return gce.ty_service_id
      elseif adty == dur_adl.duration then
        return gce.ty_duration
      elseif adty == mt_adl.match then
        return gce.ty_match
      else
        return gce.ty_userdef
      end
    else
      return gce.ty_lua
    end
  elseif ty == 'userdata' then
    if o.gcety ~= nil then
      return o:gcety()
    else
      return gce.ty_other
    end
  else
    return gce.ty_lua
  end
end

function libgce.unpack(t)
  if gce.luaver == '5.1' then
    return unpack(t)
  else
    return table.unpack(t)
  end
end

-- recv and respond syntactic sugar
libgce.receiver = {}

-- guard
function libgce.receiver.guard(aid)
  libgce.receiver.recver = aid
  return libgce.receiver
end

-- timeout
function libgce.receiver.timeout(dur)
  libgce.receiver.tmo = dur
  return libgce.receiver
end

-- recv
function libgce.receiver.recv(...)
  local receiver = libgce.receiver
  local matches = receiver.matches
  if #matches == 0 then
    return gce.recv(nil, ...)
  else
    if receiver.recver ~= nil then
      matches[#matches + 1] = receiver.recver
    end
    if libgce.receiver.tmo ~= nil then
      matches[#matches + 1] = receiver.tmo
    end
    return gce.recv(matches, ...)
  end
end

-- recv
function libgce.receiver.respond(...)
  local receiver = libgce.receiver
  local resp = receiver.resp
  local tmo = receiver.tmo
  assert (resp ~= nil)
  assert (resp[1] ~= nil)
  
  if tmo ~= nil then
    resp[2] = tmo
    return gce.respond(resp, ...)
  else
    return gce.respond(resp[1], ...)
  end
end

return gce
