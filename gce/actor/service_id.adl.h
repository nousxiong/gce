#ifndef service_id_adl_h_adata_header_define
#define service_id_adl_h_adata_header_define

#include <gce/adata/cpp/adata.hpp>
#include <gce/actor/match.adl.h>

namespace gce {namespace adl {
  struct service_id
  {
    int8_t valid_;
    ::gce::adl::match ctxid_;
    ::gce::adl::match name_;
    service_id()
    :    valid_(0)
    {}
  };

}}

namespace adata
{
  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::adl::service_id& value)
  {
    ::std::size_t offset = stream.read_length();
    uint64_t tag = 0;
    read(stream,tag);
    if(stream.error()){return;}
    int32_t len_tag = 0;
    read(stream,len_tag);
    if(stream.error()){return;}

    if(tag&1ULL)    {read(stream,value.valid_);{if(stream.error()){stream.trace_error("valid_",-1);return;}}}
    if(tag&2ULL)    {read(stream,value.ctxid_);{if(stream.error()){stream.trace_error("ctxid_",-1);return;}}}
    if(tag&4ULL)    {read(stream,value.name_);{if(stream.error()){stream.trace_error("name_",-1);return;}}}
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::adl::service_id* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::adl::service_id& value)
  {
    int32_t size = 0;
    uint64_t tag = 7ULL;
    {
      size += size_of(value.valid_);
    }
    {
      size += size_of(value.ctxid_);
    }
    {
      size += size_of(value.name_);
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::adl::service_id&value)
  {
    uint64_t tag = 7ULL;
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    {write(stream,value.valid_);{if(stream.error()){stream.trace_error("valid_",-1);return;}}}
    {write(stream,value.ctxid_);{if(stream.error()){stream.trace_error("ctxid_",-1);return;}}}
    {write(stream,value.name_);{if(stream.error()){stream.trace_error("name_",-1);return;}}}
    return;
  }

}

#endif
