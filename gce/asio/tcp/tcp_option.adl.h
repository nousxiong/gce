#ifndef tcp_option_adl_h_adata_header_define
#define tcp_option_adl_h_adata_header_define

#include <gce/adata/cpp/adata.hpp>

namespace gce {namespace asio {namespace adl {
  struct tcp_option
  {
    int8_t reuse_address;
    int8_t no_delay;
    int8_t keep_alive;
    int8_t enable_connection_aborted;
    int32_t backlog;
    int32_t receive_buffer_size;
    int32_t send_buffer_size;
    ::std::string address;
    ::std::string port;
    tcp_option()
    :    backlog(-1),
    reuse_address(-1),
    receive_buffer_size(-1),
    send_buffer_size(-1),
    no_delay(-1),
    keep_alive(-1),
    enable_connection_aborted(-1)
    {}
  };

}}}

namespace adata
{
  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::asio::adl::tcp_option& value)
  {
    ::std::size_t offset = stream.read_length();
    uint64_t tag = 0;
    read(stream,tag);
    if(stream.error()){return;}
    int32_t len_tag = 0;
    read(stream,len_tag);
    if(stream.error()){return;}

    if(tag&1ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("address",-1);return;}}
      value.address.resize(len);
      stream.read((char *)value.address.data(),len);
      {if(stream.error()){stream.trace_error("address",-1);return;}}
    }
    if(tag&2ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("port",-1);return;}}
      value.port.resize(len);
      stream.read((char *)value.port.data(),len);
      {if(stream.error()){stream.trace_error("port",-1);return;}}
    }
    if(tag&4ULL)    {read(stream,value.backlog);{if(stream.error()){stream.trace_error("backlog",-1);return;}}}
    if(tag&8ULL)    {read(stream,value.reuse_address);{if(stream.error()){stream.trace_error("reuse_address",-1);return;}}}
    if(tag&16ULL)    {read(stream,value.receive_buffer_size);{if(stream.error()){stream.trace_error("receive_buffer_size",-1);return;}}}
    if(tag&32ULL)    {read(stream,value.send_buffer_size);{if(stream.error()){stream.trace_error("send_buffer_size",-1);return;}}}
    if(tag&64ULL)    {read(stream,value.no_delay);{if(stream.error()){stream.trace_error("no_delay",-1);return;}}}
    if(tag&128ULL)    {read(stream,value.keep_alive);{if(stream.error()){stream.trace_error("keep_alive",-1);return;}}}
    if(tag&256ULL)    {read(stream,value.enable_connection_aborted);{if(stream.error()){stream.trace_error("enable_connection_aborted",-1);return;}}}
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::asio::adl::tcp_option* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::asio::adl::tcp_option& value)
  {
    int32_t size = 0;
    uint64_t tag = 508ULL;
    if(!value.address.empty()){tag|=1ULL;}
    if(!value.port.empty()){tag|=2ULL;}
    if(tag&1ULL)
    {
      int32_t len = (int32_t)(value.address).size();
      size += size_of(len);
      size += len;
    }
    if(tag&2ULL)
    {
      int32_t len = (int32_t)(value.port).size();
      size += size_of(len);
      size += len;
    }
    {
      size += size_of(value.backlog);
    }
    {
      size += size_of(value.reuse_address);
    }
    {
      size += size_of(value.receive_buffer_size);
    }
    {
      size += size_of(value.send_buffer_size);
    }
    {
      size += size_of(value.no_delay);
    }
    {
      size += size_of(value.keep_alive);
    }
    {
      size += size_of(value.enable_connection_aborted);
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::asio::adl::tcp_option&value)
  {
    uint64_t tag = 508ULL;
    if(!value.address.empty()){tag|=1ULL;}
    if(!value.port.empty()){tag|=2ULL;}
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    if(tag&1ULL)
    {
      uint32_t len = (uint32_t)(value.address).size();
      write(stream,len);
      stream.write((value.address).data(),len);
      {if(stream.error()){stream.trace_error("address",-1);return;}}
    }
    if(tag&2ULL)
    {
      uint32_t len = (uint32_t)(value.port).size();
      write(stream,len);
      stream.write((value.port).data(),len);
      {if(stream.error()){stream.trace_error("port",-1);return;}}
    }
    {write(stream,value.backlog);{if(stream.error()){stream.trace_error("backlog",-1);return;}}}
    {write(stream,value.reuse_address);{if(stream.error()){stream.trace_error("reuse_address",-1);return;}}}
    {write(stream,value.receive_buffer_size);{if(stream.error()){stream.trace_error("receive_buffer_size",-1);return;}}}
    {write(stream,value.send_buffer_size);{if(stream.error()){stream.trace_error("send_buffer_size",-1);return;}}}
    {write(stream,value.no_delay);{if(stream.error()){stream.trace_error("no_delay",-1);return;}}}
    {write(stream,value.keep_alive);{if(stream.error()){stream.trace_error("keep_alive",-1);return;}}}
    {write(stream,value.enable_connection_aborted);{if(stream.error()){stream.trace_error("enable_connection_aborted",-1);return;}}}
    return;
  }

}

#endif
