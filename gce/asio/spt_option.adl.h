#ifndef spt_option_adl_h_adata_header_define
#define spt_option_adl_h_adata_header_define

#include <gce/adata/cpp/adata.hpp>

namespace gce {namespace asio {namespace adl {
  struct spt_option
  {
    int32_t baud_rate;
    int32_t flow_control;
    int32_t parity;
    int32_t stop_bits;
    int32_t character_size;
    spt_option()
    :    baud_rate(-1),
    flow_control(-1),
    parity(-1),
    stop_bits(-1),
    character_size(-1)
    {}
  };

}}}

namespace adata
{
  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::asio::adl::spt_option& value)
  {
    ::std::size_t offset = stream.read_length();
    uint64_t tag = 0;
    read(stream,tag);
    if(stream.error()){return;}
    int32_t len_tag = 0;
    read(stream,len_tag);
    if(stream.error()){return;}

    if(tag&1ULL)    {read(stream,value.baud_rate);{if(stream.error()){stream.trace_error("baud_rate",-1);return;}}}
    if(tag&2ULL)    {read(stream,value.flow_control);{if(stream.error()){stream.trace_error("flow_control",-1);return;}}}
    if(tag&4ULL)    {read(stream,value.parity);{if(stream.error()){stream.trace_error("parity",-1);return;}}}
    if(tag&8ULL)    {read(stream,value.stop_bits);{if(stream.error()){stream.trace_error("stop_bits",-1);return;}}}
    if(tag&16ULL)    {read(stream,value.character_size);{if(stream.error()){stream.trace_error("character_size",-1);return;}}}
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::asio::adl::spt_option* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::asio::adl::spt_option& value)
  {
    int32_t size = 0;
    uint64_t tag = 31ULL;
    {
      size += size_of(value.baud_rate);
    }
    {
      size += size_of(value.flow_control);
    }
    {
      size += size_of(value.parity);
    }
    {
      size += size_of(value.stop_bits);
    }
    {
      size += size_of(value.character_size);
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::asio::adl::spt_option&value)
  {
    uint64_t tag = 31ULL;
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    {write(stream,value.baud_rate);{if(stream.error()){stream.trace_error("baud_rate",-1);return;}}}
    {write(stream,value.flow_control);{if(stream.error()){stream.trace_error("flow_control",-1);return;}}}
    {write(stream,value.parity);{if(stream.error()){stream.trace_error("parity",-1);return;}}}
    {write(stream,value.stop_bits);{if(stream.error()){stream.trace_error("stop_bits",-1);return;}}}
    {write(stream,value.character_size);{if(stream.error()){stream.trace_error("character_size",-1);return;}}}
    return;
  }

}

#endif
