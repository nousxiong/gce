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

  struct ctxid_list
  {
    ::std::vector< ::gce::adl::match > list_;
    ctxid_list()
    {}
  };

  struct global_service_list
  {
    ::std::map< ::gce::adl::match,::gce::adl::detail::ctxid_list > list_;
    global_service_list()
    {}
  };

  struct svc_pair
  {
    ::gce::adl::match name_;
    ::gce::adl::match ctxid_;
    svc_pair()
    {}
  };

  struct add_svc
  {
    ::std::vector< ::gce::adl::detail::svc_pair > svcs_;
    add_svc()
    {}
  };

  struct rmv_svc
  {
    ::gce::adl::match ctxid_;
    ::std::vector< ::gce::adl::match > names_;
    rmv_svc()
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

  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::adl::detail::ctxid_list& value)
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
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::adl::detail::ctxid_list* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::adl::detail::ctxid_list& value)
  {
    int32_t size = 0;
    uint64_t tag = 0ULL;
    if(!value.list_.empty()){tag|=1ULL;}
    if(tag&1ULL)
    {
      int32_t len = (int32_t)(value.list_).size();
      size += size_of(len);
      for (::std::vector< ::gce::adl::match >::const_iterator i = value.list_.begin() ; i != value.list_.end() ; ++i)
      {
        size += size_of(*i);
      }
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::adl::detail::ctxid_list&value)
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
      for (::std::vector< ::gce::adl::match >::const_iterator i = value.list_.begin() ; i != value.list_.end() ; ++i, ++count)
      {
        {write(stream,*i);}
        {if(stream.error()){stream.trace_error("list_",count);return;}}
      }
    }
    return;
  }

  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::adl::detail::global_service_list& value)
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
      for (std::size_t i = 0 ; i < len ; ++i)
      {
        ::gce::adl::match first_element;
        ::gce::adl::detail::ctxid_list second_element;
        {read(stream,first_element);}
        {read(stream,second_element);}
        {if(stream.error()){stream.trace_error("list_",(int32_t)i);return;}}
        value.list_.insert(::std::make_pair(first_element,second_element));
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
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::adl::detail::global_service_list* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::adl::detail::global_service_list& value)
  {
    int32_t size = 0;
    uint64_t tag = 0ULL;
    if(!value.list_.empty()){tag|=1ULL;}
    if(tag&1ULL)
    {
      int32_t len = (int32_t)(value.list_).size();
      size += size_of(len);
      for (::std::map< ::gce::adl::match,::gce::adl::detail::ctxid_list >::const_iterator i = value.list_.begin() ; i != value.list_.end() ; ++i)
      {
        size += size_of(i->first);
        size += size_of(i->second);
      }
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::adl::detail::global_service_list&value)
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
      for (::std::map< ::gce::adl::match,::gce::adl::detail::ctxid_list >::const_iterator i = value.list_.begin() ; i != value.list_.end() ; ++i, ++count)
      {
        {write(stream,i->first);}
        {write(stream,i->second);}
        {if(stream.error()){stream.trace_error("list_",count);return;}}
      }
    }
    return;
  }

  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::adl::detail::svc_pair& value)
  {
    ::std::size_t offset = stream.read_length();
    uint64_t tag = 0;
    read(stream,tag);
    if(stream.error()){return;}
    int32_t len_tag = 0;
    read(stream,len_tag);
    if(stream.error()){return;}

    if(tag&1ULL)    {read(stream,value.name_);{if(stream.error()){stream.trace_error("name_",-1);return;}}}
    if(tag&2ULL)    {read(stream,value.ctxid_);{if(stream.error()){stream.trace_error("ctxid_",-1);return;}}}
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::adl::detail::svc_pair* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::adl::detail::svc_pair& value)
  {
    int32_t size = 0;
    uint64_t tag = 3ULL;
    {
      size += size_of(value.name_);
    }
    {
      size += size_of(value.ctxid_);
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::adl::detail::svc_pair&value)
  {
    uint64_t tag = 3ULL;
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    {write(stream,value.name_);{if(stream.error()){stream.trace_error("name_",-1);return;}}}
    {write(stream,value.ctxid_);{if(stream.error()){stream.trace_error("ctxid_",-1);return;}}}
    return;
  }

  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::adl::detail::add_svc& value)
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
      {if(stream.error()){stream.trace_error("svcs_",-1);return;}}
      value.svcs_.resize(len);
      for (std::size_t i = 0 ; i < len ; ++i)
      {
        {read(stream,value.svcs_[i]);}
        {if(stream.error()){stream.trace_error("svcs_",(int32_t)i);return;}}
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
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::adl::detail::add_svc* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::adl::detail::add_svc& value)
  {
    int32_t size = 0;
    uint64_t tag = 0ULL;
    if(!value.svcs_.empty()){tag|=1ULL;}
    if(tag&1ULL)
    {
      int32_t len = (int32_t)(value.svcs_).size();
      size += size_of(len);
      for (::std::vector< ::gce::adl::detail::svc_pair >::const_iterator i = value.svcs_.begin() ; i != value.svcs_.end() ; ++i)
      {
        size += size_of(*i);
      }
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::adl::detail::add_svc&value)
  {
    uint64_t tag = 0ULL;
    if(!value.svcs_.empty()){tag|=1ULL;}
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    if(tag&1ULL)
    {
      uint32_t len = (uint32_t)(value.svcs_).size();
      write(stream,len);
      int32_t count = 0;
      for (::std::vector< ::gce::adl::detail::svc_pair >::const_iterator i = value.svcs_.begin() ; i != value.svcs_.end() ; ++i, ++count)
      {
        {write(stream,*i);}
        {if(stream.error()){stream.trace_error("svcs_",count);return;}}
      }
    }
    return;
  }

  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::adl::detail::rmv_svc& value)
  {
    ::std::size_t offset = stream.read_length();
    uint64_t tag = 0;
    read(stream,tag);
    if(stream.error()){return;}
    int32_t len_tag = 0;
    read(stream,len_tag);
    if(stream.error()){return;}

    if(tag&1ULL)    {read(stream,value.ctxid_);{if(stream.error()){stream.trace_error("ctxid_",-1);return;}}}
    if(tag&2ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("names_",-1);return;}}
      value.names_.resize(len);
      for (std::size_t i = 0 ; i < len ; ++i)
      {
        {read(stream,value.names_[i]);}
        {if(stream.error()){stream.trace_error("names_",(int32_t)i);return;}}
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
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::adl::detail::rmv_svc* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::adl::detail::rmv_svc& value)
  {
    int32_t size = 0;
    uint64_t tag = 1ULL;
    if(!value.names_.empty()){tag|=2ULL;}
    {
      size += size_of(value.ctxid_);
    }
    if(tag&2ULL)
    {
      int32_t len = (int32_t)(value.names_).size();
      size += size_of(len);
      for (::std::vector< ::gce::adl::match >::const_iterator i = value.names_.begin() ; i != value.names_.end() ; ++i)
      {
        size += size_of(*i);
      }
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::adl::detail::rmv_svc&value)
  {
    uint64_t tag = 1ULL;
    if(!value.names_.empty()){tag|=2ULL;}
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    {write(stream,value.ctxid_);{if(stream.error()){stream.trace_error("ctxid_",-1);return;}}}
    if(tag&2ULL)
    {
      uint32_t len = (uint32_t)(value.names_).size();
      write(stream,len);
      int32_t count = 0;
      for (::std::vector< ::gce::adl::match >::const_iterator i = value.names_.begin() ; i != value.names_.end() ; ++i, ++count)
      {
        {write(stream,*i);}
        {if(stream.error()){stream.trace_error("names_",count);return;}}
      }
    }
    return;
  }

}

#endif
