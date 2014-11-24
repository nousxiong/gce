--
-- Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local gce = {}

gce.overloading_0 = detail.overloading_0()
gce.overloading_1 = detail.overloading_1()
gce.overloading_2 = detail.overloading_2()

gce.overloading_aid = gce.overloading_0
gce.overloading_svcid = gce.overloading_1

gce.overloading_pattern = gce.overloading_0
gce.overloading_match_t = gce.overloading_1
gce.overloading_duration = gce.overloading_2

gce.overloading_msg = gce.overloading_0

gce.no_link = 0
gce.linked = 1
gce.monitored = 2

gce.spw_nil = 0
gce.spw_stacked = 1
gce.spw_evented = 2
gce.luaed = 3

gce.infin = detail.infin()
gce.zero = detail.zero()

function gce.run_actor(f)
  gce_curr_co = coroutine.create(
  	function ()
  		self:set_coro(gce_curr_co);
  		f()
  	end
  	)
	self:set_resume(gce.resume)
  gce.resume()
end

function gce.resume()
	local rt, err = coroutine.resume(gce_curr_co)
  if not rt then
  	gce.error("%s", err)
  end
end

function gce.send(target, arg, ...)
	arg = arg or nil
	local m = gce.make_msg(arg, ...)
	if target:get_overloading_type() == gce.overloading_aid then
		self:send(target, m)
	else
		assert (target:get_overloading_type() == gce.overloading_svcid)
		self:send2svc(target, m)
	end
end

function gce.relay(target, m)
	if target:get_overloading_type() == gce.overloading_aid then
		self:relay(target, m)
	else
		assert (target:get_overloading_type() == gce.overloading_svcid)
		self:relay2svc(target, m)
	end
end

function gce.request(target, arg, ...)
	arg = arg or nil
	local m = gce.make_msg(arg, ...)
	if target:get_overloading_type() == gce.overloading_aid then
		return self:request(target, m)
	else
		assert (target:get_overloading_type() == gce.overloading_svcid)
		return self:request2svc(target, m)
	end
end

function gce.reply(target, arg, ...)
	arg = arg or nil
	local m = gce.make_msg(arg, ...)
	assert (target:get_overloading_type() == gce.overloading_aid)
	self:reply(target, m)
end

function gce.link(target)
	assert (target:get_overloading_type() == gce.overloading_aid)
	self:link(target)
end

function gce.monitor(target)
	assert (target:get_overloading_type() == gce.overloading_aid)
	self:monitor(target)
end

function gce.recv(cfg, ...)
	cfg = cfg or nil
	if cfg == nil then
  	self:recv()
	else
		local ty = type(cfg)
		local patt
		if ty == "table" then
			-- match_t*N + timeout (N >= 1)
			local tmo = cfg[#cfg]
			cfg[#cfg] = nil
		  patt = gce.pattern(cfg)
		  patt:set_timeout(tmo)
		elseif ty == "string" or ty == "number" then
			patt = gce.pattern(gce.atom(cfg))
		else
			assert (ty == "userdata")
			if cfg:get_overloading_type() == gce.overloading_pattern then
				patt = cfg
			elseif cfg:get_overloading_type() == gce.overloading_match_t then
				patt = gce.pattern(cfg)
			else -- timeout
				assert (cfg:get_overloading_type() == gce.overloading_duration)
				patt = detail.pattern()
				patt:set_timeout(cfg)
			end
		end
		self:recv_match(patt)
	end
	local args, m = gce.unpack(gce_recv_msg, ...)
  return gce_recv_sender, args, m
end

function gce.respond(cfg, ...)
	assert (cfg ~= nil)
	local ty = type(cfg)
	local res, tmo
	if ty == "table" then
		res = cfg[1]
		tmo = cfg[2]
	else
		assert (ty == "userdata")
		res = cfg
		tmo = nil
	end

	if tmo == nil then
		self:recv_response(res)
	else
		self:recv_response_timeout(res, tmo)
	end
	local args, m = gce.unpack(gce_recv_msg, ...)
  return gce_recv_sender, args, m
end

function gce.sleep_for(dur)
	self:sleep_for(dur)
end

function gce.bind(ep, opt)
	opt = opt or nil
	if opt == nil then
		opt = gce.net_option()
	end
	self:bind(ep, opt)
end

function gce.connect(target, ep, opt)
	opt = opt or nil
	if opt == nil then
		opt = gce.net_option()
	end
	local ty = type(target)

	if ty == "string" or ty == "number" then
		target = gce.atom(target)
	else
		assert (ty == "userdata")
		assert (target:get_overloading_type() == gce.overloading_match_t)
	end
	self:connect(target, ep, opt)
	return gce_conn_ret, gce_conn_errmsg
end

function gce.spawn(script, link_type, sync_sire)
	link_type = link_type or gce.no_link;
	sync_sire = sync_sire or false;
	self:spawn(script, sync_sire, link_type)
	return gce_spawn_aid
end

function gce.spawn_remote(spw_type, func, ctxid, link_type, stack_size, tmo)
	link_type = link_type or gce.no_link;
	stack_size = stack_size or detail.default_stacksize()
	tmo = tmo or gce.seconds(180)
	self:spawn_remote(spw_type, func, ctxid, link_type, stack_size, tmo)
	return gce_spawn_aid
end

function gce.register_service(name)
	assert (name ~= nil)
	local ty = type(name)
	if ty == "string" or ty == "number" then
		name = gce.atom(name)
	else
		assert (ty == "userdata")
		assert (name:get_overloading_type() == gce.overloading_match_t)
	end
	self:register_service(name)
end

function gce.deregister_service(name)
	assert (name ~= nil)
	local ty = type(name)
	if ty == "string" or ty == "number" then
		name = gce.atom(name)
	else
		assert (ty == "userdata")
		assert (name:get_overloading_type() == gce.overloading_match_t)
	end
	self:deregister_service(name)
end

function gce.get_aid()
	return self:get_aid()
end

function gce.millisecs(v)
	return detail.millisecs(v)
end

function gce.seconds(v)
	return detail.seconds(v)
end

function gce.minutes(v)
	return detail.minutes(v)
end

function gce.hours(v)
	return detail.hours(v)
end

function gce.msg()
	return detail.msg()
end

function gce.aid()
	return detail.aid()
end

function gce.svcid()
	return detail.svcid()
end

function gce.pattern(...)
	local rt = detail.pattern()
	for i,v in ipairs{...} do
		rt:add_match(v)
	end
	return rt
end

function gce.net_option()
	return detail.net_option()
end

function gce.atom(v)
	if type(v) == "string" then
		return detail.atom(v)
	else
		return detail.make_match(v)
	end
end

function gce.deatom(i)
	return detail.deatom(i)
end

function gce.default_stacksize()
	return detail.default_stacksize()
end

function gce.print(fmt, ...)
	detail.print(gce.unpack_fmt(fmt, ...))
end

function gce.debug(fmt, ...)
	self:debug(gce.unpack_fmt(fmt, ...))
end

function gce.info(fmt, ...)
	self:info(gce.unpack_fmt(fmt, ...))
end

function gce.warn(fmt, ...)
	self:warn(gce.unpack_fmt(fmt, ...))
end

function gce.error(fmt, ...)
	self:error(gce.unpack_fmt(fmt, ...))
end

function gce.fatal(fmt, ...)
	self:fatal(gce.unpack_fmt(fmt, ...))
end

gce.exit = gce.atom("gce_exit")


-------------------internal use-------------------
function gce.unpack_fmt(fmt, ...)
	local tab = {}
	for i,v in ipairs{...} do
		if type(v) == "userdata" then
			tab[i] = v:to_string()
		else
			tab[i] = v
		end
	end
	return string.format(fmt, table.unpack(tab))
end

function gce.make_msg(arg, ...)
	local m = detail.msg()
	if arg ~= nil then
		local ty = type(arg)
		if ty == "string" or ty == "number" then
			local match = gce.atom(arg)
			m:set_type(match)
		else
			assert (ty == "userdata")
			if arg:get_overloading_type() == gce.overloading_msg then
				m = arg
			else
				assert (arg:get_overloading_type() == gce.overloading_match_t)
				m:set_type(arg)
			end
		end
	end
	return gce.pack(m, ...)
end

function gce.serialize(m, o)
	ty = type(o)
	if ty == "number" then
		m = detail.serialize_number(m, o)
	elseif ty == "string" then
		m = detail.serialize_string(m, o)
	elseif ty == "boolean" then
		m = detail.serialize_boolean(m, o)
	elseif ty == "table" then
		local l = rawlen(o)
		m = gce.serialize(m, l)
		for k,v in pairs(o) do
			m = gce.serialize(m, k)
			m = gce.serialize(m, v)
		end
	elseif ty == "userdata" then
		m = o:serialize(m)
	else
		assert(false)
	end
	return m
end

function gce.deserialize(m, o, need_make)
	need_make = need_make or false
	local res = {}

	ty = type(o)
	if ty == "number" then
		local r = detail.deserialize_number(m)
		res[1] = r.rt
		res[2] = r.ms
	elseif ty == "string" then
		local r = detail.deserialize_string(m)
		res[1] = r.rt
		res[2] = r.ms
	elseif ty == "boolean" then
		local r = detail.deserialize_boolean(m)
		res[1] = r.rt
		res[2] = r.ms
	elseif ty == "table" then
		local tab = {}
		local ol = rawlen(o)
		local l = detail.deserialize_number(m)
		assert(l.rt >= ol)
		m = l.ms
		local n = 1
		if l.rt > ol then
			assert(ol == 1)
			for k,v in pairs(o) do
				for i=1,l.rt do
					local tk = gce.deserialize(m, k, true)
					m = tk[2]
					local tv = gce.deserialize(m, v, true)
					m = tv[2]
					tab[tk[1]] = tv[1]
				end
			end
		else
			for k,v in pairs(o) do
				local tk = gce.deserialize(m, k, true)
				m = tk[2]
				local tv = gce.deserialize(m, v, true)
				m = tv[2]
				tab[tk[1]] = tv[1]
			end
		end
		res[1] = tab
		res[2] = m
	elseif ty == "userdata" then
		if need_make then
			o = o:make()
		end
		m = o:deserialize(m)
		res[1] = o
		res[2] = m
	else
		assert(false)
	end
	return res
end

function gce.pack(m, ...)
	for i,v in ipairs{...} do
		m = gce.serialize(m, v)
	end
	return m
end

function gce.unpack(m, ...)
	local res = {}
	m:enable_copy_read_size()
	for i,v in ipairs{...} do
		local r = gce.deserialize(m, v)
		res[i] = r[1]
		m = r[2]
	end
	m:disable_copy_read_size()
	return res, m
end

return gce
