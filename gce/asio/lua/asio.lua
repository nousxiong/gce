--
-- Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local gce = require('gce')
local libgce = require('libgce')
local libasio = require('libasio')
local asio = {}

local tcpopt_adl = nil
local sslopt_adl = nil
local snopt_adl = nil
if gce.packer == gce.pkr_adata then
  tcpopt_adl = require('tcp_option_adl')
  if gce.openssl ~= 0 then
    sslopt_adl = require('ssl_option_adl')
  end
  snopt_adl = require('sn_option_adl')
end

-- enum and constant def
asio.ty_ssl_context = libasio.ty_ssl_context
asio.ty_ssl_stream_impl = libasio.ty_ssl_stream_impl
asio.ty_tcp_endpoint = libasio.ty_tcp_endpoint
asio.ty_tcp_endpoint_itr = libasio.ty_tcp_endpoint_itr
asio.ty_tcp_socket_impl = libasio.ty_tcp_socket_impl

asio.plength = libasio.plength
asio.pregex = libasio.pregex

asio.sigint = libasio.sigint
asio.sigterm = libasio.sigterm

asio.none = libasio.none
asio.software = libasio.software
asio.hardware = libasio.hardware
asio.odd = libasio.odd
asio.even = libasio.even
asio.one = libasio.one
asio.onepointfive = libasio.onepointfive
asio.two = libasio.two

if gce.openssl ~= 0 then
  asio.sslv2 = libasio.sslv2
  asio.sslv2_client = libasio.sslv2_client
  asio.sslv2_server = libasio.sslv2_server
  asio.sslv3 = libasio.sslv3
  asio.sslv3_client = libasio.sslv3_client
  asio.sslv3_server = libasio.sslv3_server
  asio.tlsv1 = libasio.tlsv1
  asio.tlsv1_client = libasio.tlsv1_client
  asio.tlsv1_server = libasio.tlsv1_server
  asio.sslv23 = libasio.sslv23
  asio.sslv23_client = libasio.sslv23_client
  asio.sslv23_server = libasio.sslv23_server
  asio.tlsv11 = libasio.tlsv11
  asio.tlsv11_client = libasio.tlsv11_client
  asio.tlsv11_server = libasio.tlsv11_server
  asio.tlsv12 = libasio.tlsv12
  asio.tlsv12_client = libasio.tlsv12_client
  asio.tlsv12_server = libasio.tlsv12_server

  asio.asn1 = libasio.asn1
  asio.pem = libasio.pem

  asio.for_reading = libasio.for_reading
  asio.for_writing = libasio.for_writing

  asio.client = libasio.client
  asio.server = libasio.server
end


function asio.timer()
  return libasio.make_system_timer(libgce.self)
end

-- all sig could be nil/none
function asio.signal(sig1, sig2, sig3)
  return libasio.make_signal(libgce.self, sig1, sig2, sig3)
end

function asio.spt_option()
  return libasio.make_sptopt()
end

function asio.serial_port(device, opt)
  return libasio.make_serial_port(libgce.self, device, opt)
end

function asio.tcp_option()
  return libasio.make_tcpopt()
end

function asio.tcp_endpoint()
  return libasio.make_tcp_endpoint()
end

function asio.tcp_endpoint_itr()
  return libasio.make_tcp_endpoint_itr()
end

function asio.tcp_resolver()
  return libasio.make_tcp_resolver(libgce.self)
end

function asio.tcp_acceptor()
  return libasio.make_tcp_acceptor(libgce.self)
end

function asio.tcp_socket_impl()
  return libasio.make_tcp_socket_impl(libgce.self)
end

function asio.tcp_socket(impl)
  return libasio.make_tcp_socket(libgce.self, impl)
end

if gce.openssl ~= 0 then
  function asio.ssl_option()
    return libasio.make_sslopt()
  end

  -- ssl_opt: optional, could be nil or none
  function asio.ssl_stream_impl(ssl_ctx, ssl_opt)
    return libasio.make_ssl_stream_impl(libgce.self, ssl_ctx, ssl_opt)
  end

  function asio.ssl_context(method, opt, pwdcb)
    return libasio.make_ssl_context(method, opt, pwdcb)
  end

  function asio.ssl_stream(cfg, opt)
    -- cfg is ssl_context or ssl_stream_impl
    local is_ssl_ctx = cfg:gcety() ~= asio.ty_ssl_stream_impl
    return libasio.make_ssl_stream(libgce.self, is_ssl_ctx, cfg, opt)
  end
end

function asio.sn_option()
  return libasio.make_snopt()
end

function asio.simple_length()
  return libasio.make_simple_length()
end

-- expr: regex's string expr
function asio.simple_regex(expr)
  return libasio.make_simple_regex(expr)
end

-- parser: msg parser, e.g. parser_simple
-- skt_impl: tcp_socket_impl or ssl_socket_impl
-- eitr: tcp_endpoint_itr (optional, could be nil/none)
-- opt: sn_option (optional, could be nil/none)
function asio.session(parser, skt_impl, eitr, opt)
  return libasio.make_session(libgce.self, parser, skt_impl, eitr, opt)
end


asio.as_timeout = gce.atom('as_timeout')
asio.as_signal = gce.atom('as_signal')
asio.as_resolve = gce.atom('as_resolve')
asio.as_accept = gce.atom('as_accept')
asio.as_conn = gce.atom('as_conn')
asio.as_handshake = gce.atom('as_handshake')
asio.as_shutdown = gce.atom('as_shutdown')
asio.as_recv = gce.atom('as_recv')
asio.as_recv_some = gce.atom('as_recv_some')
asio.as_recv_until = gce.atom('as_recv_until')
asio.as_send = gce.atom('as_send')
asio.as_send_some = gce.atom('as_send_some')

asio.sn_open = gce.atom('sn_open')
asio.sn_recv = gce.atom('sn_recv')
asio.sn_idle = gce.atom('sn_idle')
asio.sn_close = gce.atom('sn_close')

function asio.async_read(s, length, ty)
  ty = ty or asio.as_recv
  local m = gce.message(ty)
  s:async_read(length, m)
end

function asio.async_write(s, ch, ty)
  ty = ty or asio.as_send
  local m = gce.message(ty, ch)
  s:async_write(m)
end

return asio
