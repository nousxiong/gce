#ifndef context_id_adl_h_adata_header_define
#define context_id_adl_h_adata_header_define

#include <gce/adata/cpp/adata.hpp>

namespace gce {namespace redis {namespace adl {
  struct context_id
  {
    uint64_t ptr_;
    context_id()
    :    ptr_(0ULL)
    {}
  };

}}}

namespace adata
{
  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::redis::adl::context_id& value)
  {
    ::std::size_t offset = stream.read_length();
    uint64_t tag = 0;
    read(stream,tag);
    if(stream.error()){return;}
    int32_t len_tag = 0;
    read(stream,len_tag);
    if(stream.error()){return;}

    if(tag&1ULL)    {read(stream,value.ptr_);{if(stream.error()){stream.trace_error("ptr_",-1);return;}}}
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::redis::adl::context_id* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::redis::adl::context_id& value)
  {
    int32_t size = 0;
    uint64_t tag = 1ULL;
    {
      size += size_of(value.ptr_);
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::redis::adl::context_id&value)
  {
    uint64_t tag = 1ULL;
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    {write(stream,value.ptr_);{if(stream.error()){stream.trace_error("ptr_",-1);return;}}}
    return;
  }

}

#endif
