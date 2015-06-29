#ifndef sn_option_adl_h_adata_header_define
#define sn_option_adl_h_adata_header_define

#include <gce/adata/cpp/adata.hpp>
#include <gce/actor/duration.adl.h>

namespace gce {namespace asio {namespace adl {
  struct sn_option
  {
    int32_t bigmsg_size;
    ::gce::adl::duration idle_period;
    sn_option()
    :    bigmsg_size(0)
    {}
  };

}}}

namespace adata
{
  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::asio::adl::sn_option& value)
  {
    ::std::size_t offset = stream.read_length();
    uint64_t tag = 0;
    read(stream,tag);
    if(stream.error()){return;}
    int32_t len_tag = 0;
    read(stream,len_tag);
    if(stream.error()){return;}

    if(tag&1ULL)    {read(stream,value.idle_period);{if(stream.error()){stream.trace_error("idle_period",-1);return;}}}
    if(tag&2ULL)    {read(stream,value.bigmsg_size);{if(stream.error()){stream.trace_error("bigmsg_size",-1);return;}}}
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::asio::adl::sn_option* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::asio::adl::sn_option& value)
  {
    int32_t size = 0;
    uint64_t tag = 3ULL;
    {
      size += size_of(value.idle_period);
    }
    {
      size += size_of(value.bigmsg_size);
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::asio::adl::sn_option&value)
  {
    uint64_t tag = 3ULL;
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    {write(stream,value.idle_period);{if(stream.error()){stream.trace_error("idle_period",-1);return;}}}
    {write(stream,value.bigmsg_size);{if(stream.error()){stream.trace_error("bigmsg_size",-1);return;}}}
    return;
  }

}

#endif
