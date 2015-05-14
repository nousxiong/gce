#ifndef arg_adl_h_adata_header_define
#define arg_adl_h_adata_header_define

#include <gce/adata/cpp/adata.hpp>

namespace gce {namespace adl {
  struct arg1_t
  {
    int32_t i_;
    ::std::string hi_;
    arg1_t()
    :    i_(0)
    {}
  };

  struct arg2_t
  {
    int32_t i_;
    ::std::vector< int32_t> v_;
    arg2_t()
    :    i_(0)
    {}
  };

}}

namespace adata
{
  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::adl::arg1_t& value)
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
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::adl::arg1_t* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::adl::arg1_t& value)
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
  ADATA_INLINE void write(stream_ty& stream , const ::gce::adl::arg1_t&value)
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

  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::adl::arg2_t& value)
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
      {if(stream.error()){stream.trace_error("v_",-1);return;}}
      value.v_.resize(len);
      for (std::size_t i = 0 ; i < len ; ++i)
      {
        {read(stream,value.v_[i]);}
        {if(stream.error()){stream.trace_error("v_",(int32_t)i);return;}}
      }
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
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::adl::arg2_t* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::adl::arg2_t& value)
  {
    int32_t size = 0;
    uint64_t tag = 2ULL;
    if(!value.v_.empty()){tag|=1ULL;}
    if(tag&1ULL)
    {
      int32_t len = (int32_t)(value.v_).size();
      size += size_of(len);
      for (::std::vector< int32_t>::const_iterator i = value.v_.begin() ; i != value.v_.end() ; ++i)
      {
        size += size_of(*i);
      }
    }
    {
      size += size_of(value.i_);
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::adl::arg2_t&value)
  {
    uint64_t tag = 2ULL;
    if(!value.v_.empty()){tag|=1ULL;}
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    if(tag&1ULL)
    {
      uint32_t len = (uint32_t)(value.v_).size();
      write(stream,len);
      int32_t count = 0;
      for (::std::vector< int32_t>::const_iterator i = value.v_.begin() ; i != value.v_.end() ; ++i, ++count)
      {
        {write(stream,*i);}
        {if(stream.error()){stream.trace_error("v_",count);return;}}
      }
    }
    {write(stream,value.i_);{if(stream.error()){stream.trace_error("i_",-1);return;}}}
    return;
  }

}

#endif
