///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_ASIO_HPP
#define GCE_ASIO_ASIO_HPP

#include <gce/asio/config.hpp>
#ifdef GCE_LUA
# include <gce/asio/detail/lua_wrap.hpp>
# include <sstream>
#endif

#ifdef GCE_LUA
namespace gce
{
namespace asio
{
inline void make_libasio(lua_State* L)
{
  typedef detail::lua::timer<system_timer> lua_system_timer_t;
  
  /// register libasio
  gce::lualib::open(L)
    .begin("libasio")
      .add_function("make_system_timer", lua_system_timer_t::make)
      .begin_userdata(lua_system_timer_t::type().c_str())
        .add_function("async_wait", lua_system_timer_t::async_wait)
        .add_function("cancel", lua_system_timer_t::cancel)
        .add_function("__gc", lua_system_timer_t::gc)
      .end_userdata()
      .add_function("make_signal", detail::lua::signal::make)
      .begin_userdata("signal")
        .add_function("async_wait", detail::lua::signal::async_wait)
        .add_function("add", detail::lua::signal::add)
        .add_function("remove", detail::lua::signal::remove)
        .add_function("cancel", detail::lua::signal::cancel)
        .add_function("clear", detail::lua::signal::clear)
        .add_function("__gc", detail::lua::signal::gc)
      .end_userdata()
      .add_function("make_sptopt", detail::lua::spt_option::make)
      .add_function("make_serial_port", detail::lua::serial_port::make)
      .begin_userdata("serial_port")
        .add_function("async_read", detail::lua::serial_port::async_read)
        .add_function("async_read_some", detail::lua::serial_port::async_read_some)
        .add_function("async_write", detail::lua::serial_port::async_write)
        .add_function("async_write_some", detail::lua::serial_port::async_write_some)
        .add_function("open", detail::lua::serial_port::open)
        .add_function("is_open", detail::lua::serial_port::is_open)
        .add_function("send_break", detail::lua::serial_port::send_break)
        .add_function("cancel", detail::lua::serial_port::cancel)
        .add_function("close", detail::lua::serial_port::close)
        .add_function("__gc", detail::lua::serial_port::gc)
      .end_userdata()
      .add_function("make_tcpopt", detail::lua::tcp_option::make)
      .add_function("make_tcp_endpoint", detail::lua::tcp_endpoint::make)
      .begin_userdata("tcp_endpoint")
        .add_function("gcety", detail::lua::tcp_endpoint::gcety)
        .add_function("__gc", detail::lua::tcp_endpoint::gc)
      .end_userdata()
      .add_function("make_tcp_endpoint_itr", detail::lua::tcp_endpoint_itr::make)
      .begin_userdata("tcp_endpoint_itr")
        .add_function("gcety", detail::lua::tcp_endpoint_itr::gcety)
        .add_function("__gc", detail::lua::tcp_endpoint_itr::gc)
      .end_userdata()
      .add_function("make_tcp_socket_impl", detail::lua::tcp_socket_impl::make)
      .begin_userdata("tcp_socket_impl")
        .add_function("gcety", detail::lua::tcp_socket_impl::gcety)
        .add_function("__gc", detail::lua::tcp_socket_impl::gc)
      .end_userdata()
      .add_function("make_tcp_resolver", detail::lua::tcp_resolver::make)
      .begin_userdata("tcp_resolver")
        .add_function("async_resolve", detail::lua::tcp_resolver::async_resolve)
        .add_function("close", detail::lua::tcp_resolver::cancel)
        .add_function("__gc", detail::lua::tcp_resolver::gc)
      .end_userdata()
      .add_function("make_tcp_acceptor", detail::lua::tcp_acceptor::make)
      .begin_userdata("tcp_acceptor")
        .add_function("async_accept", detail::lua::tcp_acceptor::async_accept)
        .add_function("bind", detail::lua::tcp_acceptor::bind)
        .add_function("close", detail::lua::tcp_acceptor::close)
        .add_function("__gc", detail::lua::tcp_acceptor::gc)
      .end_userdata()
      .add_function("make_tcp_socket", detail::lua::tcp_socket::make)
      .begin_userdata("tcp_socket")
        .add_function("async_connect", detail::lua::tcp_socket::async_connect)
        .add_function("async_read", detail::lua::tcp_socket::async_read)
        .add_function("async_read_some", detail::lua::tcp_socket::async_read_some)
        .add_function("async_write", detail::lua::tcp_socket::async_write)
        .add_function("async_write_some", detail::lua::tcp_socket::async_write_some)
        .add_function("close", detail::lua::tcp_socket::close)
        .add_function("__gc", detail::lua::tcp_socket::gc)
      .end_userdata()
#ifdef GCE_OPENSSL
      .add_function("make_sslopt", detail::lua::ssl_option::make)
      .add_function("make_ssl_context", detail::lua::ssl_context::make)
      .begin_userdata("ssl_context")
        .add_function("set_verify_callback", detail::lua::ssl_context::set_verify_callback)
        .add_function("gcety", detail::lua::ssl_context::gcety)
        .add_function("__gc", detail::lua::ssl_context::gc)
      .end_userdata()
      .add_function("make_ssl_stream_impl", detail::lua::ssl_stream_impl::make)
      .begin_userdata("ssl_stream_impl")
        .add_function("gcety", detail::lua::ssl_stream_impl::gcety)
        .add_function("__gc", detail::lua::ssl_stream_impl::gc)
      .end_userdata()
      .add_function("make_ssl_stream", detail::lua::ssl_stream::make)
      .begin_userdata("ssl_stream")
        .add_function("set_verify_callback", detail::lua::ssl_stream::set_verify_callback)
        .add_function("async_connect", detail::lua::ssl_stream::async_connect)
        .add_function("async_handshake", detail::lua::ssl_stream::async_handshake)
        .add_function("async_shutdown", detail::lua::ssl_stream::async_shutdown)
        .add_function("async_read", detail::lua::ssl_stream::async_read)
        .add_function("async_read_some", detail::lua::ssl_stream::async_read_some)
        .add_function("async_write", detail::lua::ssl_stream::async_write)
        .add_function("async_write_some", detail::lua::ssl_stream::async_write_some)
        .add_function("close_lowest_layer", detail::lua::ssl_stream::close_lowest_layer)
        .add_function("__gc", detail::lua::ssl_stream::gc)
      .end_userdata()
#endif
    .end()
    ;

  /// init libasio
  std::ostringstream oss;
  oss << "local libasio = require('libasio')" << std::endl;

  /// types
  oss << "libasio.ty_ssl_context = " << gce::asio::lua::ty_ssl_context << std::endl;
  oss << "libasio.ty_ssl_stream_impl = " << gce::asio::lua::ty_ssl_stream_impl << std::endl;
  oss << "libasio.ty_tcp_endpoint = " << gce::asio::lua::ty_tcp_endpoint << std::endl;
  oss << "libasio.ty_tcp_endpoint_itr = " << gce::asio::lua::ty_tcp_endpoint_itr << std::endl;
  oss << "libasio.ty_tcp_socket_impl = " << gce::asio::lua::ty_tcp_socket_impl << std::endl;

  /// signals
  oss << "libasio.sigint = " << SIGINT << std::endl;
  oss << "libasio.sigterm = " << SIGTERM << std::endl;

  /// serial_port
  oss << "libasio.none = " << boost::asio::serial_port_base::flow_control::none << std::endl;
  oss << "libasio.software = " << boost::asio::serial_port_base::flow_control::software << std::endl;
  oss << "libasio.hardware = " << boost::asio::serial_port_base::flow_control::hardware << std::endl;
  oss << "libasio.odd = " << boost::asio::serial_port_base::parity::odd << std::endl;
  oss << "libasio.even = " << boost::asio::serial_port_base::parity::even << std::endl;
  oss << "libasio.one = " << boost::asio::serial_port_base::stop_bits::one << std::endl;
  oss << "libasio.onepointfive = " << boost::asio::serial_port_base::stop_bits::onepointfive << std::endl;
  oss << "libasio.two = " << boost::asio::serial_port_base::stop_bits::two << std::endl;

#ifdef GCE_OPENSSL
  /// method
  oss << "libasio.sslv2 = " << boost::asio::ssl::context::sslv2 << std::endl;
  oss << "libasio.sslv2_client = " << boost::asio::ssl::context::sslv2_client << std::endl;
  oss << "libasio.sslv2_server = " << boost::asio::ssl::context::sslv2_server << std::endl;
  oss << "libasio.sslv3 = " << boost::asio::ssl::context::sslv3 << std::endl;
  oss << "libasio.sslv3_client = " << boost::asio::ssl::context::sslv3_client << std::endl;
  oss << "libasio.sslv3_server = " << boost::asio::ssl::context::sslv3_server << std::endl;
  oss << "libasio.tlsv1 = " << boost::asio::ssl::context::tlsv1 << std::endl;
  oss << "libasio.tlsv1_client = " << boost::asio::ssl::context::tlsv1_client << std::endl;
  oss << "libasio.tlsv1_server = " << boost::asio::ssl::context::tlsv1_server << std::endl;
  oss << "libasio.sslv23 = " << boost::asio::ssl::context::sslv23 << std::endl;
  oss << "libasio.sslv23_client = " << boost::asio::ssl::context::sslv23_client << std::endl;
  oss << "libasio.sslv23_server = " << boost::asio::ssl::context::sslv23_server << std::endl;
  oss << "libasio.tlsv11 = " << boost::asio::ssl::context::tlsv11 << std::endl;
  oss << "libasio.tlsv11_client = " << boost::asio::ssl::context::tlsv11_client << std::endl;
  oss << "libasio.tlsv11_server = " << boost::asio::ssl::context::tlsv11_server << std::endl;
  oss << "libasio.tlsv12 = " << boost::asio::ssl::context::tlsv12 << std::endl;
  oss << "libasio.tlsv12_client = " << boost::asio::ssl::context::tlsv12_client << std::endl;
  oss << "libasio.tlsv12_server = " << boost::asio::ssl::context::tlsv12_server << std::endl;

  /// file_format
  oss << "libasio.asn1 = " << boost::asio::ssl::context::asn1 << std::endl;
  oss << "libasio.pem = " << boost::asio::ssl::context::pem << std::endl;

  /// password_purpose
  oss << "libasio.for_reading = " << boost::asio::ssl::context::for_reading << std::endl;
  oss << "libasio.for_writing = " << boost::asio::ssl::context::for_writing << std::endl;

  /// handshake_type
  oss << "libasio.client = " << boost::asio::ssl::stream_base::client << std::endl;
  oss << "libasio.server = " << boost::asio::ssl::stream_base::server << std::endl;
#endif

  std::string init_libasio_script = oss.str();
  if (luaL_dostring(L, init_libasio_script.c_str()) != 0)
  {
    std::string errmsg("gce::lua_exception: ");
    errmsg += lua_tostring(L, -1);
    GCE_VERIFY(false).msg(errmsg.c_str()).except<gce::lua_exception>();
  }
}
}
}
#endif

#endif /// GCE_ASIO_ASIO_HPP
