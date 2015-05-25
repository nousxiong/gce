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
if gce.packer == gce.pkr_adata then
  tcpopt_adl = require('tcp_option_adl')
  if gce.openssl ~= 0 then
    sslopt_adl = require('ssl_option_adl')
  end
end

-- enum and constant def
asio.ty_ssl_context = libasio.ty_ssl_context
asio.ty_ssl_stream_impl = libasio.ty_ssl_stream_impl
asio.ty_tcp_endpoint = libasio.ty_tcp_endpoint
asio.ty_tcp_endpoint_itr = libasio.ty_tcp_endpoint_itr
asio.ty_tcp_socket_impl = libasio.ty_tcp_socket_impl

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

  function asio.ssl_stream_impl(ssl_ctx)
    return libasio.make_ssl_stream_impl(libgce.self, ssl_ctx)
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


asio.as_timeout = gce.atom('as_timeout')
asio.as_signal = gce.atom('as_signal')
asio.as_resolve = gce.atom('as_resolve')
asio.as_accept = gce.atom('as_accept')
asio.as_conn = gce.atom('as_conn')
asio.as_handshake = gce.atom('as_handshake')
asio.as_shutdown = gce.atom('as_shutdown')
asio.as_recv = gce.atom('as_recv')
asio.as_recv_some = gce.atom('as_recv_some')
asio.as_send = gce.atom('as_send')
asio.as_send_some = gce.atom('as_send_some')

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
