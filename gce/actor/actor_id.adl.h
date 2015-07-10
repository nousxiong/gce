#ifndef actor_id_adl_h_adata_header_define
#define actor_id_adl_h_adata_header_define

#include <gce/adata/cpp/adata.hpp>
#include <gce/actor/match.adl.h>
#include <gce/actor/service_id.adl.h>

namespace gce {namespace adl {
  struct actor_id
  {
    uint8_t type_;
    uint8_t in_pool_;
    uint16_t svc_id_;
    uint32_t sid_;
    uint64_t timestamp_;
    uint64_t uintptr_;
    ::gce::adl::match ctxid_;
    ::gce::adl::service_id svc_;
    actor_id()
    :    type_(0),
    in_pool_(0),
    svc_id_(0),
    sid_(0),
    timestamp_(0ULL),
    uintptr_(0ULL)
    {}
  };

}}

namespace adata
{
  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::adl::actor_id& value)
  {
    ::std::size_t offset = stream.read_length();
    uint64_t tag = 0;
    read(stream,tag);
    if(stream.error()){return;}
    int32_t len_tag = 0;
    read(stream,len_tag);
    if(stream.error()){return;}

    if(tag&1ULL)    {read(stream,value.ctxid_);{if(stream.error()){stream.trace_error("ctxid_",-1);return;}}}
    if(tag&2ULL)    {read(stream,value.timestamp_);{if(stream.error()){stream.trace_error("timestamp_",-1);return;}}}
    if(tag&4ULL)    {read(stream,value.uintptr_);{if(stream.error()){stream.trace_error("uintptr_",-1);return;}}}
    if(tag&8ULL)    {read(stream,value.svc_id_);{if(stream.error()){stream.trace_error("svc_id_",-1);return;}}}
    if(tag&16ULL)    {read(stream,value.type_);{if(stream.error()){stream.trace_error("type_",-1);return;}}}
    if(tag&32ULL)    {read(stream,value.in_pool_);{if(stream.error()){stream.trace_error("in_pool_",-1);return;}}}
    if(tag&64ULL)    {read(stream,value.sid_);{if(stream.error()){stream.trace_error("sid_",-1);return;}}}
    if(tag&128ULL)    {read(stream,value.svc_);{if(stream.error()){stream.trace_error("svc_",-1);return;}}}
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::adl::actor_id* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::adl::actor_id& value)
  {
    int32_t size = 0;
    uint64_t tag = 255ULL;
    {
      size += size_of(value.ctxid_);
    }
    {
      size += size_of(value.timestamp_);
    }
    {
      size += size_of(value.uintptr_);
    }
    {
      size += size_of(value.svc_id_);
    }
    {
      size += size_of(value.type_);
    }
    {
      size += size_of(value.in_pool_);
    }
    {
      size += size_of(value.sid_);
    }
    {
      size += size_of(value.svc_);
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::adl::actor_id&value)
  {
    uint64_t tag = 255ULL;
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    {write(stream,value.ctxid_);{if(stream.error()){stream.trace_error("ctxid_",-1);return;}}}
    {write(stream,value.timestamp_);{if(stream.error()){stream.trace_error("timestamp_",-1);return;}}}
    {write(stream,value.uintptr_);{if(stream.error()){stream.trace_error("uintptr_",-1);return;}}}
    {write(stream,value.svc_id_);{if(stream.error()){stream.trace_error("svc_id_",-1);return;}}}
    {write(stream,value.type_);{if(stream.error()){stream.trace_error("type_",-1);return;}}}
    {write(stream,value.in_pool_);{if(stream.error()){stream.trace_error("in_pool_",-1);return;}}}
    {write(stream,value.sid_);{if(stream.error()){stream.trace_error("sid_",-1);return;}}}
    {write(stream,value.svc_);{if(stream.error()){stream.trace_error("svc_",-1);return;}}}
    return;
  }

}

#endif
