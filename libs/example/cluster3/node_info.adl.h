#ifndef node_info_adl_h_adata_header_define
#define node_info_adl_h_adata_header_define

#include <gce/adata/cpp/adata.hpp>
#include <gce/actor/match.adl.h>
#include <gce/actor/service_id.adl.h>

namespace cluster3 {
  struct node_info
  {
    ::gce::adl::service_id svcid_;
    ::std::string ep_;
    node_info()
    {}
  };

  struct node_info_list
  {
    ::std::vector< ::cluster3::node_info > list_;
    node_info_list()
    {}
  };

}

namespace adata
{
  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::cluster3::node_info& value)
  {
    ::std::size_t offset = stream.read_length();
    uint64_t tag = 0;
    read(stream,tag);
    if(stream.error()){return;}
    int32_t len_tag = 0;
    read(stream,len_tag);
    if(stream.error()){return;}

    if(tag&1ULL)    {read(stream,value.svcid_);{if(stream.error()){stream.trace_error("svcid_",-1);return;}}}
    if(tag&2ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("ep_",-1);return;}}
      value.ep_.resize(len);
      stream.read((char *)value.ep_.data(),len);
      {if(stream.error()){stream.trace_error("ep_",-1);return;}}
    }
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  ADATA_INLINE void skip_read(stream_ty& stream, ::cluster3::node_info* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::cluster3::node_info& value)
  {
    int32_t size = 0;
    uint64_t tag = 1ULL;
    if(!value.ep_.empty()){tag|=2ULL;}
    {
      size += size_of(value.svcid_);
    }
    if(tag&2ULL)
    {
      int32_t len = (int32_t)(value.ep_).size();
      size += size_of(len);
      size += len;
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::cluster3::node_info&value)
  {
    uint64_t tag = 1ULL;
    if(!value.ep_.empty()){tag|=2ULL;}
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    {write(stream,value.svcid_);{if(stream.error()){stream.trace_error("svcid_",-1);return;}}}
    if(tag&2ULL)
    {
      uint32_t len = (uint32_t)(value.ep_).size();
      write(stream,len);
      stream.write((value.ep_).data(),len);
      {if(stream.error()){stream.trace_error("ep_",-1);return;}}
    }
    return;
  }

  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::cluster3::node_info_list& value)
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
      {if(stream.error()){stream.trace_error("list_",-1);return;}}
      value.list_.resize(len);
      for (std::size_t i = 0 ; i < len ; ++i)
      {
        {read(stream,value.list_[i]);}
        {if(stream.error()){stream.trace_error("list_",(int32_t)i);return;}}
      }
    }
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  ADATA_INLINE void skip_read(stream_ty& stream, ::cluster3::node_info_list* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::cluster3::node_info_list& value)
  {
    int32_t size = 0;
    uint64_t tag = 0ULL;
    if(!value.list_.empty()){tag|=1ULL;}
    if(tag&1ULL)
    {
      int32_t len = (int32_t)(value.list_).size();
      size += size_of(len);
      for (::std::vector< ::cluster3::node_info >::const_iterator i = value.list_.begin() ; i != value.list_.end() ; ++i)
      {
        size += size_of(*i);
      }
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::cluster3::node_info_list&value)
  {
    uint64_t tag = 0ULL;
    if(!value.list_.empty()){tag|=1ULL;}
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    if(tag&1ULL)
    {
      uint32_t len = (uint32_t)(value.list_).size();
      write(stream,len);
      int32_t count = 0;
      for (::std::vector< ::cluster3::node_info >::const_iterator i = value.list_.begin() ; i != value.list_.end() ; ++i, ++count)
      {
        {write(stream,*i);}
        {if(stream.error()){stream.trace_error("list_",count);return;}}
      }
    }
    return;
  }

}

#endif
