#ifndef net_option_adl_h_adata_header_define
#define net_option_adl_h_adata_header_define

#include <gce/adata/cpp/adata.hpp>
#include <gce/actor/duration.adl.h>

namespace gce {namespace adl {
  struct net_option
  {
    int8_t is_router;
    int32_t heartbeat_count;
    int32_t init_reconn_try;
    int32_t reconn_try;
    int32_t rebind_try;
    ::gce::adl::duration heartbeat_period;
    ::gce::adl::duration init_reconn_period;
    ::gce::adl::duration reconn_period;
    ::gce::adl::duration rebind_period;
    net_option()
    :    is_router(0),
    heartbeat_count(0),
    init_reconn_try(0),
    reconn_try(0),
    rebind_try(0)
    {}
  };

}}

namespace adata
{
  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::adl::net_option& value)
  {
    ::std::size_t offset = stream.read_length();
    uint64_t tag = 0;
    read(stream,tag);
    if(stream.error()){return;}
    int32_t len_tag = 0;
    read(stream,len_tag);
    if(stream.error()){return;}

    if(tag&1ULL)    {read(stream,value.is_router);{if(stream.error()){stream.trace_error("is_router",-1);return;}}}
    if(tag&2ULL)    {read(stream,value.heartbeat_period);{if(stream.error()){stream.trace_error("heartbeat_period",-1);return;}}}
    if(tag&4ULL)    {read(stream,value.heartbeat_count);{if(stream.error()){stream.trace_error("heartbeat_count",-1);return;}}}
    if(tag&8ULL)    {read(stream,value.init_reconn_period);{if(stream.error()){stream.trace_error("init_reconn_period",-1);return;}}}
    if(tag&16ULL)    {read(stream,value.init_reconn_try);{if(stream.error()){stream.trace_error("init_reconn_try",-1);return;}}}
    if(tag&32ULL)    {read(stream,value.reconn_period);{if(stream.error()){stream.trace_error("reconn_period",-1);return;}}}
    if(tag&64ULL)    {read(stream,value.reconn_try);{if(stream.error()){stream.trace_error("reconn_try",-1);return;}}}
    if(tag&128ULL)    {read(stream,value.rebind_period);{if(stream.error()){stream.trace_error("rebind_period",-1);return;}}}
    if(tag&256ULL)    {read(stream,value.rebind_try);{if(stream.error()){stream.trace_error("rebind_try",-1);return;}}}
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::adl::net_option* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::adl::net_option& value)
  {
    int32_t size = 0;
    uint64_t tag = 511ULL;
    {
      size += size_of(value.is_router);
    }
    {
      size += size_of(value.heartbeat_period);
    }
    {
      size += size_of(value.heartbeat_count);
    }
    {
      size += size_of(value.init_reconn_period);
    }
    {
      size += size_of(value.init_reconn_try);
    }
    {
      size += size_of(value.reconn_period);
    }
    {
      size += size_of(value.reconn_try);
    }
    {
      size += size_of(value.rebind_period);
    }
    {
      size += size_of(value.rebind_try);
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::adl::net_option&value)
  {
    uint64_t tag = 511ULL;
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    {write(stream,value.is_router);{if(stream.error()){stream.trace_error("is_router",-1);return;}}}
    {write(stream,value.heartbeat_period);{if(stream.error()){stream.trace_error("heartbeat_period",-1);return;}}}
    {write(stream,value.heartbeat_count);{if(stream.error()){stream.trace_error("heartbeat_count",-1);return;}}}
    {write(stream,value.init_reconn_period);{if(stream.error()){stream.trace_error("init_reconn_period",-1);return;}}}
    {write(stream,value.init_reconn_try);{if(stream.error()){stream.trace_error("init_reconn_try",-1);return;}}}
    {write(stream,value.reconn_period);{if(stream.error()){stream.trace_error("reconn_period",-1);return;}}}
    {write(stream,value.reconn_try);{if(stream.error()){stream.trace_error("reconn_try",-1);return;}}}
    {write(stream,value.rebind_period);{if(stream.error()){stream.trace_error("rebind_period",-1);return;}}}
    {write(stream,value.rebind_try);{if(stream.error()){stream.trace_error("rebind_try",-1);return;}}}
    return;
  }

}

#endif
