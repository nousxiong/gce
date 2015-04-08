#ifndef echo_adl_h_adata_header_define
#define echo_adl_h_adata_header_define

#include <gce/adata/cpp/adata.hpp>

namespace gce {namespace adl {
  struct echo_data
  {
    int32_t i_;
    ::std::string hi_;
    echo_data()
    :    i_(0)
    {}
  };

}}

namespace adata
{
  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::adl::echo_data& value)
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
      {if(stream.error()){stream.trace_error("hi_",-1);return;}}
      value.hi_.resize(len);
      stream.read((char *)value.hi_.data(),len);
      {if(stream.error()){stream.trace_error("hi_",-1);return;}}
    }
    if(tag&2ULL)    {read(stream,value.i_);{if(stream.error()){stream.trace_error("i_",-1);return;}}}
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::adl::echo_data* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::adl::echo_data& value)
  {
    int32_t size = 0;
    uint64_t tag = 2ULL;
    if(!value.hi_.empty()){tag|=1ULL;}
    if(tag&1ULL)
    {
      int32_t len = (int32_t)(value.hi_).size();
      size += size_of(len);
      size += len;
    }
    {
      size += size_of(value.i_);
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::adl::echo_data&value)
  {
    uint64_t tag = 2ULL;
    if(!value.hi_.empty()){tag|=1ULL;}
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    if(tag&1ULL)
    {
      uint32_t len = (uint32_t)(value.hi_).size();
      write(stream,len);
      stream.write((value.hi_).data(),len);
      {if(stream.error()){stream.trace_error("hi_",-1);return;}}
    }
    {write(stream,value.i_);{if(stream.error()){stream.trace_error("i_",-1);return;}}}
    return;
  }

}

#endif
