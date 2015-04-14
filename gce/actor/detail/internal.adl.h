#ifndef internal_adl_h_adata_header_define
#define internal_adl_h_adata_header_define

#include <gce/adata/cpp/adata.hpp>
#include <gce/actor/match.adl.h>

namespace gce {namespace adl {namespace detail {
  struct header
  {
    uint32_t size_;
    uint32_t tag_offset_;
    ::gce::adl::match type_;
    header()
    :    size_(0),
    tag_offset_(0)
    {}
  };

  struct errcode
  {
    uint32_t code_;
    uint64_t errcat_;
    errcode()
    :    code_(0),
    errcat_(0ULL)
    {}
  };

}}}

namespace adata
{
  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::adl::detail::header& value)
  {
    ::std::size_t offset = stream.read_length();
    uint64_t tag = 0;
    read(stream,tag);
    if(stream.error()){return;}
    int32_t len_tag = 0;
    read(stream,len_tag);
    if(stream.error()){return;}

    if(tag&1ULL)    {read(stream,value.size_);{if(stream.error()){stream.trace_error("size_",-1);return;}}}
    if(tag&2ULL)    {read(stream,value.type_);{if(stream.error()){stream.trace_error("type_",-1);return;}}}
    if(tag&4ULL)    {read(stream,value.tag_offset_);{if(stream.error()){stream.trace_error("tag_offset_",-1);return;}}}
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::adl::detail::header* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::adl::detail::header& value)
  {
    int32_t size = 0;
    uint64_t tag = 7ULL;
    {
      size += size_of(value.size_);
    }
    {
      size += size_of(value.type_);
    }
    {
      size += size_of(value.tag_offset_);
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::adl::detail::header&value)
  {
    uint64_t tag = 7ULL;
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    {write(stream,value.size_);{if(stream.error()){stream.trace_error("size_",-1);return;}}}
    {write(stream,value.type_);{if(stream.error()){stream.trace_error("type_",-1);return;}}}
    {write(stream,value.tag_offset_);{if(stream.error()){stream.trace_error("tag_offset_",-1);return;}}}
    return;
  }

  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::adl::detail::errcode& value)
  {
    ::std::size_t offset = stream.read_length();
    uint64_t tag = 0;
    read(stream,tag);
    if(stream.error()){return;}
    int32_t len_tag = 0;
    read(stream,len_tag);
    if(stream.error()){return;}

    if(tag&1ULL)    {fix_read(stream,value.code_);{if(stream.error()){stream.trace_error("code_",-1);return;}}}
    if(tag&2ULL)    {fix_read(stream,value.errcat_);{if(stream.error()){stream.trace_error("errcat_",-1);return;}}}
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::adl::detail::errcode* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::adl::detail::errcode& value)
  {
    int32_t size = 0;
    uint64_t tag = 3ULL;
    {
      size += fix_size_of(value.code_);
    }
    {
      size += fix_size_of(value.errcat_);
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::adl::detail::errcode&value)
  {
    uint64_t tag = 3ULL;
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    {fix_write(stream,value.code_);{if(stream.error()){stream.trace_error("code_",-1);return;}}}
    {fix_write(stream,value.errcat_);{if(stream.error()){stream.trace_error("errcat_",-1);return;}}}
    return;
  }

}

#endif
