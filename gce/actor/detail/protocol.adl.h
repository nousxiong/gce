#ifndef protocol_adl_h_adata_header_define
#define protocol_adl_h_adata_header_define

#include <gce/adata/cpp/adata.hpp>

namespace gce {namespace adl {
  struct msg_header
  {
    uint32_t size_;
    uint32_t tag_offset_;
    uint64_t type_;
    msg_header()
    :    size_(0),
    type_(0ULL),
    tag_offset_(0)
    {}
  };

}}

namespace adata
{
  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::adl::msg_header& value)
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
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::adl::msg_header* value)
  {
    ::std::size_t offset = stream.read_length();
    uint64_t tag = 0;
    read(stream,tag);
    if(stream.error()){return;}
    int32_t len_tag = 0;
    read(stream,len_tag);
    if(stream.error()){return;}

    if(tag&1ULL)    {uint32_t* dummy_value = 0;skip_read(stream,dummy_value);{if(stream.error()){stream.trace_error("size_",-1);return;}}}
    if(tag&2ULL)    {uint64_t* dummy_value = 0;skip_read(stream,dummy_value);{if(stream.error()){stream.trace_error("type_",-1);return;}}}
    if(tag&4ULL)    {uint32_t* dummy_value = 0;skip_read(stream,dummy_value);{if(stream.error()){stream.trace_error("tag_offset_",-1);return;}}}
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  ADATA_INLINE int32_t size_of(const ::gce::adl::msg_header& value)
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
  ADATA_INLINE void write(stream_ty& stream , const ::gce::adl::msg_header&value)
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

}

#endif
