#ifndef duration_adl_h_adata_header_define
#define duration_adl_h_adata_header_define

#include <gce/adata/cpp/adata.hpp>

namespace gce {namespace adl {
  struct duration
  {
    int8_t ty_;
    int64_t val_;
    duration()
    :    ty_(0),
    val_(0LL)
    {}
  };

}}

namespace adata
{
  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::adl::duration& value)
  {
    ::std::size_t offset = stream.read_length();
    uint64_t tag = 0;
    read(stream,tag);
    if(stream.error()){return;}
    int32_t len_tag = 0;
    read(stream,len_tag);
    if(stream.error()){return;}

    if(tag&1ULL)    {read(stream,value.val_);{if(stream.error()){stream.trace_error("val_",-1);return;}}}
    if(tag&2ULL)    {read(stream,value.ty_);{if(stream.error()){stream.trace_error("ty_",-1);return;}}}
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::adl::duration* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::adl::duration& value)
  {
    int32_t size = 0;
    uint64_t tag = 3ULL;
    {
      size += size_of(value.val_);
    }
    {
      size += size_of(value.ty_);
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::adl::duration&value)
  {
    uint64_t tag = 3ULL;
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    {write(stream,value.val_);{if(stream.error()){stream.trace_error("val_",-1);return;}}}
    {write(stream,value.ty_);{if(stream.error()){stream.trace_error("ty_",-1);return;}}}
    return;
  }

}

#endif
