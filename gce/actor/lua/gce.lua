--
-- Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

gce = {}

gce.overloading_0 = 0
gce.overloading_1 = 1

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
	if not gce_curr_co then
	  gce_curr_co = coroutine.create(
	  	function ()
	  		self:set_coro(gce_curr_co);
	  		f()
	  	end
	  	)
	end
  local rt, err = coroutine.resume(gce_curr_co)
  if not rt then
  	gce.print("actor error: %s", err)
  end
end

function gce.send(target, m, ...)
	m = m or nil
	local ms = gce.make_msg(m, ...)
	if target:get_overloading_type() == gce.overloading_0 then
		self:send(target, ms)
	else
		self:send2svc(target, ms)
	end
end

function gce.relay(target, ms)
	if target:get_overloading_type() == gce.overloading_0 then
		self:relay(target, ms)
	else
		self:relay2svc(target, ms)
	end
end

function gce.request(target, m, ...)
	m = m or nil
	local ms = gce.make_msg(m, ...)
	if target:get_overloading_type() == gce.overloading_0 then
		return self:request(target, ms)
	else
		return self:request2svc(target, ms)
	end
end

function gce.reply(target, m, ...)
	m = m or nil
	local ms = gce.make_msg(m, ...)
	assert (target:get_overloading_type() == gce.overloading_0)
	self:reply(target, ms)
end

function gce.link(target)
	assert (target:get_overloading_type() == gce.overloading_0)
	self:link(target)
end

function gce.monitor(target)
	assert (target:get_overloading_type() == gce.overloading_0)
	self:monitor(target)
end

function gce.recv(arg0, arg1, ...)
	arg0 = arg0 or nil
	arg1 = arg1 or nil
	if arg0 == nil then
  	self:recv()
	else
		if arg1 == nil then
			if arg0:get_overloading_type() == gce.overloading_0 then
				self:recv_match(arg0)
			else
				self:recv_response(arg0)
			end
		else
			self:recv_response_timeout(arg0, arg1)
		end
	end
	local ms = gce.unpack(gce_recv_msg, ...)
  return gce_recv_sender, ms
end

function gce.wait(dur)
	self:wait(dur)
end

function gce.spawn(script, sync_sire, link_type)
	sync_sire = sync_sire or false;
	link_type = link_type or gce.no_link;
	self:spawn(script, sync_sire, link_type)
	return gce_spawn_aid
end

function gce.spawn_remote(spw_type, func, ctxid, link_type, stack_size, tmo)
	link_type = link_type or gce.no_link;
	stack_size = stack_size or detail.default_stacksize()
	tmo = tmo or seconds(180)
	self:spawn_remote(spw_type, func, ctxid, link_type, stack_size, tmo)
	return gce_spawn_aid
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

function gce.atom(s)
	return detail.atom(s)
end

function gce.deatom(i)
	return detail.deatom(i)
end

function gce.default_stacksize()
	return detail.default_stacksize()
end

function gce.print(fmt, ...)
	local tab = {}
	for i,v in ipairs{...} do
		if type(v) == "userdata" then
			tab[i] = v:to_string()
		else
			tab[i] = v
		end
	end
	detail.print(string.format(fmt, table.unpack(tab)))
end

-------------------internal use-------------------
function gce.make_msg(m, ...)
	if m == nil then
		m = detail.msg()
	end
	local ms
	if m:get_overloading_type() == gce.overloading_0 then
		ms = detail.msg()
		ms:set_type(m)
		for i,v in ipairs{...} do
			gce.pack(ms, v)
		end
	else
		ms = m
	end
	return ms
end

function gce.serialize(ms, o)
	ty = type(o)
	if ty == "number" then
		detail.serialize_number(ms, o)
	elseif ty == "string" then
		detail.serialize_string(ms, o)
	elseif ty == "boolean" then
		detail.serialize_boolean(ms, o)
	elseif ty == "table" then
		local l = rawlen(o)
		gce.serialize(ms, l)
		for k,v in pairs(o) do
			gce.serialize(ms, k)
			gce.serialize(ms, v)
		end
	elseif ty == "userdata" then
		o:serialize(ms)
	else
		assert(false)
	end
end

function gce.deserialize(ms, o, need_make)
	need_make = need_make or false
	ty = type(o)
	if ty == "number" then
		o = detail.deserialize_number(ms)
	elseif ty == "string" then
		o = detail.deserialize_string(ms)
	elseif ty == "boolean" then
		o = detail.deserialize_boolean(ms)
	elseif ty == "table" then
		local tab = {}
		local ol = rawlen(o)
		local l = detail.deserialize_number(ms)
		assert(l >= ol)
		if l > ol then
			assert(ol == 1)
			for k,v in pairs(o) do
				for i=1,l do
					local tk = gce.deserialize(ms, k, true)
					local tv = gce.deserialize(ms, v, true)
					tab[tk] = tv
				end
			end
		else
			for k,v in pairs(o) do
				local tk = gce.deserialize(ms, k, true)
				local tv = gce.deserialize(ms, v, true)
				tab[tk] = tv
			end
		end
		o = tab
	elseif ty == "userdata" then
		if need_make then
			o = o:make()
		end
		o:deserialize(ms)
	else
		assert(false)
	end
	return o
end

function gce.pack(ms, ...)
	for i,v in ipairs{...} do
		gce.serialize(ms, v)
	end
end

function gce.unpack(ms, ...)
	local tab = {}
	for i,v in ipairs{...} do
		tab[i] = gce.deserialize(ms, v)
	end
	if rawlen(tab) == 0 then
		return ms
	else
		return tab
	end
end
