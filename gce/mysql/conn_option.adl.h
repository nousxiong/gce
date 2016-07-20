#ifndef gce_mysql_adl_conn_option_adl_h_adata_header_define
#define gce_mysql_adl_conn_option_adl_h_adata_header_define

#include <gce/adata/cpp/adata.hpp>

namespace gce {namespace mysql {namespace adl {
  struct conn_option
  {
    int8_t compress;
    int8_t reconnect;
    int8_t client_compress;
    int8_t client_found_rows;
    int8_t client_ignore_sigpipe;
    int8_t client_ignore_space;
    int8_t client_multi_results;
    int8_t client_multi_statements;
    int32_t connect_timeout;
    int32_t read_timeout;
    int32_t write_timeout;
    ::std::string init_command;
    ::std::string read_default_file;
    ::std::string read_default_group;
    ::std::string set_charset_name;
    conn_option()
    :    compress(-1),
    reconnect(-1),
    client_compress(-1),
    client_found_rows(-1),
    client_ignore_sigpipe(-1),
    client_ignore_space(-1),
    client_multi_results(-1),
    client_multi_statements(-1),
    connect_timeout(-1),
    read_timeout(-1),
    write_timeout(-1)
    {}
  };

}}}

namespace adata
{
template<>
struct is_adata<gce::mysql::adl::conn_option>
{
  static const bool value = true;
};

}
namespace adata
{
  template<typename stream_ty>
  inline void read( stream_ty& stream, ::gce::mysql::adl::conn_option& value)
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
      {if(stream.error()){stream.trace_error("init_command",-1);return;}}
      value.init_command.resize(len);
      stream.read((char *)value.init_command.data(),len);
      {if(stream.error()){stream.trace_error("init_command",-1);return;}}
    }
    if(tag&2ULL)    {read(stream,value.compress);{if(stream.error()){stream.trace_error("compress",-1);return;}}}
    if(tag&4ULL)    {read(stream,value.connect_timeout);{if(stream.error()){stream.trace_error("connect_timeout",-1);return;}}}
    if(tag&8ULL)    {read(stream,value.read_timeout);{if(stream.error()){stream.trace_error("read_timeout",-1);return;}}}
    if(tag&16ULL)    {read(stream,value.reconnect);{if(stream.error()){stream.trace_error("reconnect",-1);return;}}}
    if(tag&32ULL)    {read(stream,value.write_timeout);{if(stream.error()){stream.trace_error("write_timeout",-1);return;}}}
    if(tag&64ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("read_default_file",-1);return;}}
      value.read_default_file.resize(len);
      stream.read((char *)value.read_default_file.data(),len);
      {if(stream.error()){stream.trace_error("read_default_file",-1);return;}}
    }
    if(tag&128ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("read_default_group",-1);return;}}
      value.read_default_group.resize(len);
      stream.read((char *)value.read_default_group.data(),len);
      {if(stream.error()){stream.trace_error("read_default_group",-1);return;}}
    }
    if(tag&256ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("set_charset_name",-1);return;}}
      value.set_charset_name.resize(len);
      stream.read((char *)value.set_charset_name.data(),len);
      {if(stream.error()){stream.trace_error("set_charset_name",-1);return;}}
    }
    if(tag&512ULL)    {read(stream,value.client_compress);{if(stream.error()){stream.trace_error("client_compress",-1);return;}}}
    if(tag&1024ULL)    {read(stream,value.client_found_rows);{if(stream.error()){stream.trace_error("client_found_rows",-1);return;}}}
    if(tag&2048ULL)    {read(stream,value.client_ignore_sigpipe);{if(stream.error()){stream.trace_error("client_ignore_sigpipe",-1);return;}}}
    if(tag&4096ULL)    {read(stream,value.client_ignore_space);{if(stream.error()){stream.trace_error("client_ignore_space",-1);return;}}}
    if(tag&8192ULL)    {read(stream,value.client_multi_results);{if(stream.error()){stream.trace_error("client_multi_results",-1);return;}}}
    if(tag&16384ULL)    {read(stream,value.client_multi_statements);{if(stream.error()){stream.trace_error("client_multi_statements",-1);return;}}}
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  inline void skip_read(stream_ty& stream, ::gce::mysql::adl::conn_option*)
  {
    skip_read_compatible(stream);
  }

  inline int32_t size_of(const ::gce::mysql::adl::conn_option& value)
  {
    int32_t size = 0;
    uint64_t tag = 32318ULL;
    if(!value.init_command.empty()){tag|=1ULL;}
    if(!value.read_default_file.empty()){tag|=64ULL;}
    if(!value.read_default_group.empty()){tag|=128ULL;}
    if(!value.set_charset_name.empty()){tag|=256ULL;}
    if(tag&1ULL)
    {
      {
        uint32_t len = (uint32_t)(value.init_command).size();
        size += size_of(len);
        size += len;
        }
    }
    {
      size += size_of(value.compress);
    }
    {
      size += size_of(value.connect_timeout);
    }
    {
      size += size_of(value.read_timeout);
    }
    {
      size += size_of(value.reconnect);
    }
    {
      size += size_of(value.write_timeout);
    }
    if(tag&64ULL)
    {
      {
        uint32_t len = (uint32_t)(value.read_default_file).size();
        size += size_of(len);
        size += len;
        }
    }
    if(tag&128ULL)
    {
      {
        uint32_t len = (uint32_t)(value.read_default_group).size();
        size += size_of(len);
        size += len;
        }
    }
    if(tag&256ULL)
    {
      {
        uint32_t len = (uint32_t)(value.set_charset_name).size();
        size += size_of(len);
        size += len;
        }
    }
    {
      size += size_of(value.client_compress);
    }
    {
      size += size_of(value.client_found_rows);
    }
    {
      size += size_of(value.client_ignore_sigpipe);
    }
    {
      size += size_of(value.client_ignore_space);
    }
    {
      size += size_of(value.client_multi_results);
    }
    {
      size += size_of(value.client_multi_statements);
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  inline void write(stream_ty& stream , const ::gce::mysql::adl::conn_option&value)
  {
    uint64_t tag = 32318ULL;
    if(!value.init_command.empty()){tag|=1ULL;}
    if(!value.read_default_file.empty()){tag|=64ULL;}
    if(!value.read_default_group.empty()){tag|=128ULL;}
    if(!value.set_charset_name.empty()){tag|=256ULL;}
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    if(tag&1ULL)
    {
      uint32_t len = (uint32_t)(value.init_command).size();
      write(stream,len);
      stream.write((value.init_command).data(),len);
      {if(stream.error()){stream.trace_error("init_command",-1);return;}}
    }
    {write(stream,value.compress);{if(stream.error()){stream.trace_error("compress",-1);return;}}}
    {write(stream,value.connect_timeout);{if(stream.error()){stream.trace_error("connect_timeout",-1);return;}}}
    {write(stream,value.read_timeout);{if(stream.error()){stream.trace_error("read_timeout",-1);return;}}}
    {write(stream,value.reconnect);{if(stream.error()){stream.trace_error("reconnect",-1);return;}}}
    {write(stream,value.write_timeout);{if(stream.error()){stream.trace_error("write_timeout",-1);return;}}}
    if(tag&64ULL)
    {
      uint32_t len = (uint32_t)(value.read_default_file).size();
      write(stream,len);
      stream.write((value.read_default_file).data(),len);
      {if(stream.error()){stream.trace_error("read_default_file",-1);return;}}
    }
    if(tag&128ULL)
    {
      uint32_t len = (uint32_t)(value.read_default_group).size();
      write(stream,len);
      stream.write((value.read_default_group).data(),len);
      {if(stream.error()){stream.trace_error("read_default_group",-1);return;}}
    }
    if(tag&256ULL)
    {
      uint32_t len = (uint32_t)(value.set_charset_name).size();
      write(stream,len);
      stream.write((value.set_charset_name).data(),len);
      {if(stream.error()){stream.trace_error("set_charset_name",-1);return;}}
    }
    {write(stream,value.client_compress);{if(stream.error()){stream.trace_error("client_compress",-1);return;}}}
    {write(stream,value.client_found_rows);{if(stream.error()){stream.trace_error("client_found_rows",-1);return;}}}
    {write(stream,value.client_ignore_sigpipe);{if(stream.error()){stream.trace_error("client_ignore_sigpipe",-1);return;}}}
    {write(stream,value.client_ignore_space);{if(stream.error()){stream.trace_error("client_ignore_space",-1);return;}}}
    {write(stream,value.client_multi_results);{if(stream.error()){stream.trace_error("client_multi_results",-1);return;}}}
    {write(stream,value.client_multi_statements);{if(stream.error()){stream.trace_error("client_multi_statements",-1);return;}}}
    return;
  }

}

#endif
