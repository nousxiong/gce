#ifndef ssl_option_adl_h_adata_header_define
#define ssl_option_adl_h_adata_header_define

#include <gce/adata/cpp/adata.hpp>

namespace gce {namespace asio {namespace adl {
  struct ssl_option
  {
    int8_t default_verify_paths;
    int8_t default_workarounds;
    int8_t single_dh_use;
    int8_t no_sslv2;
    int8_t no_sslv3;
    int8_t no_tlsv1;
    int8_t no_compression;
    int8_t verify_none;
    int8_t verify_peer;
    int8_t verify_fail_if_no_peer_cert;
    int8_t verify_client_once;
    int8_t certificate_format;
    int8_t private_key_format;
    int8_t rsa_private_key_format;
    int32_t verify_depth;
    ::std::vector< ::std::string > verify_paths;
    ::std::string certificate_authority;
    ::std::string verify_file;
    ::std::string certificate;
    ::std::string certificate_file;
    ::std::string certificate_chain;
    ::std::string certificate_chain_file;
    ::std::string private_key;
    ::std::string private_key_file;
    ::std::string rsa_private_key;
    ::std::string rsa_private_key_file;
    ::std::string tmp_dh;
    ::std::string tmp_dh_file;
    ssl_option()
    :    default_verify_paths(-1),
    default_workarounds(-1),
    single_dh_use(-1),
    no_sslv2(-1),
    no_sslv3(-1),
    no_tlsv1(-1),
    no_compression(-1),
    verify_none(-1),
    verify_peer(-1),
    verify_fail_if_no_peer_cert(-1),
    verify_client_once(-1),
    certificate_format(1),
    private_key_format(1),
    rsa_private_key_format(1),
    verify_depth(-1)
    {}
  };

}}}

namespace adata
{
  template<typename stream_ty>
  ADATA_INLINE void read( stream_ty& stream, ::gce::asio::adl::ssl_option& value)
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
      {if(stream.error()){stream.trace_error("verify_paths",-1);return;}}
      value.verify_paths.resize(len);
      for (std::size_t i = 0 ; i < len ; ++i)
      {
        {
          uint32_t len = check_read_size(stream);
          value.verify_paths[i].resize(len);
          stream.read((char *)value.verify_paths[i].data(),len);
        }
        {if(stream.error()){stream.trace_error("verify_paths",(int32_t)i);return;}}
      }
    }
    if(tag&2ULL)    {read(stream,value.default_verify_paths);{if(stream.error()){stream.trace_error("default_verify_paths",-1);return;}}}
    if(tag&4ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("certificate_authority",-1);return;}}
      value.certificate_authority.resize(len);
      stream.read((char *)value.certificate_authority.data(),len);
      {if(stream.error()){stream.trace_error("certificate_authority",-1);return;}}
    }
    if(tag&8ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("verify_file",-1);return;}}
      value.verify_file.resize(len);
      stream.read((char *)value.verify_file.data(),len);
      {if(stream.error()){stream.trace_error("verify_file",-1);return;}}
    }
    if(tag&16ULL)    {read(stream,value.default_workarounds);{if(stream.error()){stream.trace_error("default_workarounds",-1);return;}}}
    if(tag&32ULL)    {read(stream,value.single_dh_use);{if(stream.error()){stream.trace_error("single_dh_use",-1);return;}}}
    if(tag&64ULL)    {read(stream,value.no_sslv2);{if(stream.error()){stream.trace_error("no_sslv2",-1);return;}}}
    if(tag&128ULL)    {read(stream,value.no_sslv3);{if(stream.error()){stream.trace_error("no_sslv3",-1);return;}}}
    if(tag&256ULL)    {read(stream,value.no_tlsv1);{if(stream.error()){stream.trace_error("no_tlsv1",-1);return;}}}
    if(tag&512ULL)    {read(stream,value.no_compression);{if(stream.error()){stream.trace_error("no_compression",-1);return;}}}
    if(tag&1024ULL)    {read(stream,value.verify_depth);{if(stream.error()){stream.trace_error("verify_depth",-1);return;}}}
    if(tag&2048ULL)    {read(stream,value.verify_none);{if(stream.error()){stream.trace_error("verify_none",-1);return;}}}
    if(tag&4096ULL)    {read(stream,value.verify_peer);{if(stream.error()){stream.trace_error("verify_peer",-1);return;}}}
    if(tag&8192ULL)    {read(stream,value.verify_fail_if_no_peer_cert);{if(stream.error()){stream.trace_error("verify_fail_if_no_peer_cert",-1);return;}}}
    if(tag&16384ULL)    {read(stream,value.verify_client_once);{if(stream.error()){stream.trace_error("verify_client_once",-1);return;}}}
    if(tag&32768ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("certificate",-1);return;}}
      value.certificate.resize(len);
      stream.read((char *)value.certificate.data(),len);
      {if(stream.error()){stream.trace_error("certificate",-1);return;}}
    }
    if(tag&65536ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("certificate_file",-1);return;}}
      value.certificate_file.resize(len);
      stream.read((char *)value.certificate_file.data(),len);
      {if(stream.error()){stream.trace_error("certificate_file",-1);return;}}
    }
    if(tag&131072ULL)    {read(stream,value.certificate_format);{if(stream.error()){stream.trace_error("certificate_format",-1);return;}}}
    if(tag&262144ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("certificate_chain",-1);return;}}
      value.certificate_chain.resize(len);
      stream.read((char *)value.certificate_chain.data(),len);
      {if(stream.error()){stream.trace_error("certificate_chain",-1);return;}}
    }
    if(tag&524288ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("certificate_chain_file",-1);return;}}
      value.certificate_chain_file.resize(len);
      stream.read((char *)value.certificate_chain_file.data(),len);
      {if(stream.error()){stream.trace_error("certificate_chain_file",-1);return;}}
    }
    if(tag&1048576ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("private_key",-1);return;}}
      value.private_key.resize(len);
      stream.read((char *)value.private_key.data(),len);
      {if(stream.error()){stream.trace_error("private_key",-1);return;}}
    }
    if(tag&2097152ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("private_key_file",-1);return;}}
      value.private_key_file.resize(len);
      stream.read((char *)value.private_key_file.data(),len);
      {if(stream.error()){stream.trace_error("private_key_file",-1);return;}}
    }
    if(tag&4194304ULL)    {read(stream,value.private_key_format);{if(stream.error()){stream.trace_error("private_key_format",-1);return;}}}
    if(tag&8388608ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("rsa_private_key",-1);return;}}
      value.rsa_private_key.resize(len);
      stream.read((char *)value.rsa_private_key.data(),len);
      {if(stream.error()){stream.trace_error("rsa_private_key",-1);return;}}
    }
    if(tag&16777216ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("rsa_private_key_file",-1);return;}}
      value.rsa_private_key_file.resize(len);
      stream.read((char *)value.rsa_private_key_file.data(),len);
      {if(stream.error()){stream.trace_error("rsa_private_key_file",-1);return;}}
    }
    if(tag&33554432ULL)    {read(stream,value.rsa_private_key_format);{if(stream.error()){stream.trace_error("rsa_private_key_format",-1);return;}}}
    if(tag&67108864ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("tmp_dh",-1);return;}}
      value.tmp_dh.resize(len);
      stream.read((char *)value.tmp_dh.data(),len);
      {if(stream.error()){stream.trace_error("tmp_dh",-1);return;}}
    }
    if(tag&134217728ULL)
    {
      uint32_t len = check_read_size(stream);
      {if(stream.error()){stream.trace_error("tmp_dh_file",-1);return;}}
      value.tmp_dh_file.resize(len);
      stream.read((char *)value.tmp_dh_file.data(),len);
      {if(stream.error()){stream.trace_error("tmp_dh_file",-1);return;}}
    }
    if(len_tag >= 0)
    {
      ::std::size_t read_len = stream.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if(len > read_len) stream.skip_read(len - read_len);
    }
  }

  template <typename stream_ty>
  ADATA_INLINE void skip_read(stream_ty& stream, ::gce::asio::adl::ssl_option* value)
  {
    (value);
    skip_read_compatible(stream);
  }

  ADATA_INLINE int32_t size_of(const ::gce::asio::adl::ssl_option& value)
  {
    int32_t size = 0;
    uint64_t tag = 37912562ULL;
    if(!value.verify_paths.empty()){tag|=1ULL;}
    if(!value.certificate_authority.empty()){tag|=4ULL;}
    if(!value.verify_file.empty()){tag|=8ULL;}
    if(!value.certificate.empty()){tag|=32768ULL;}
    if(!value.certificate_file.empty()){tag|=65536ULL;}
    if(!value.certificate_chain.empty()){tag|=262144ULL;}
    if(!value.certificate_chain_file.empty()){tag|=524288ULL;}
    if(!value.private_key.empty()){tag|=1048576ULL;}
    if(!value.private_key_file.empty()){tag|=2097152ULL;}
    if(!value.rsa_private_key.empty()){tag|=8388608ULL;}
    if(!value.rsa_private_key_file.empty()){tag|=16777216ULL;}
    if(!value.tmp_dh.empty()){tag|=67108864ULL;}
    if(!value.tmp_dh_file.empty()){tag|=134217728ULL;}
    if(tag&1ULL)
    {
      int32_t len = (int32_t)(value.verify_paths).size();
      size += size_of(len);
      for (::std::vector< ::std::string >::const_iterator i = value.verify_paths.begin() ; i != value.verify_paths.end() ; ++i)
      {
        int32_t len = (int32_t)(*i).size();
        size += size_of(len);
        size += len;
      }
    }
    {
      size += size_of(value.default_verify_paths);
    }
    if(tag&4ULL)
    {
      int32_t len = (int32_t)(value.certificate_authority).size();
      size += size_of(len);
      size += len;
    }
    if(tag&8ULL)
    {
      int32_t len = (int32_t)(value.verify_file).size();
      size += size_of(len);
      size += len;
    }
    {
      size += size_of(value.default_workarounds);
    }
    {
      size += size_of(value.single_dh_use);
    }
    {
      size += size_of(value.no_sslv2);
    }
    {
      size += size_of(value.no_sslv3);
    }
    {
      size += size_of(value.no_tlsv1);
    }
    {
      size += size_of(value.no_compression);
    }
    {
      size += size_of(value.verify_depth);
    }
    {
      size += size_of(value.verify_none);
    }
    {
      size += size_of(value.verify_peer);
    }
    {
      size += size_of(value.verify_fail_if_no_peer_cert);
    }
    {
      size += size_of(value.verify_client_once);
    }
    if(tag&32768ULL)
    {
      int32_t len = (int32_t)(value.certificate).size();
      size += size_of(len);
      size += len;
    }
    if(tag&65536ULL)
    {
      int32_t len = (int32_t)(value.certificate_file).size();
      size += size_of(len);
      size += len;
    }
    {
      size += size_of(value.certificate_format);
    }
    if(tag&262144ULL)
    {
      int32_t len = (int32_t)(value.certificate_chain).size();
      size += size_of(len);
      size += len;
    }
    if(tag&524288ULL)
    {
      int32_t len = (int32_t)(value.certificate_chain_file).size();
      size += size_of(len);
      size += len;
    }
    if(tag&1048576ULL)
    {
      int32_t len = (int32_t)(value.private_key).size();
      size += size_of(len);
      size += len;
    }
    if(tag&2097152ULL)
    {
      int32_t len = (int32_t)(value.private_key_file).size();
      size += size_of(len);
      size += len;
    }
    {
      size += size_of(value.private_key_format);
    }
    if(tag&8388608ULL)
    {
      int32_t len = (int32_t)(value.rsa_private_key).size();
      size += size_of(len);
      size += len;
    }
    if(tag&16777216ULL)
    {
      int32_t len = (int32_t)(value.rsa_private_key_file).size();
      size += size_of(len);
      size += len;
    }
    {
      size += size_of(value.rsa_private_key_format);
    }
    if(tag&67108864ULL)
    {
      int32_t len = (int32_t)(value.tmp_dh).size();
      size += size_of(len);
      size += len;
    }
    if(tag&134217728ULL)
    {
      int32_t len = (int32_t)(value.tmp_dh_file).size();
      size += size_of(len);
      size += len;
    }
    size += size_of(tag);
    size += size_of(size + size_of(size));
    return size;
  }

  template<typename stream_ty>
  ADATA_INLINE void write(stream_ty& stream , const ::gce::asio::adl::ssl_option&value)
  {
    uint64_t tag = 37912562ULL;
    if(!value.verify_paths.empty()){tag|=1ULL;}
    if(!value.certificate_authority.empty()){tag|=4ULL;}
    if(!value.verify_file.empty()){tag|=8ULL;}
    if(!value.certificate.empty()){tag|=32768ULL;}
    if(!value.certificate_file.empty()){tag|=65536ULL;}
    if(!value.certificate_chain.empty()){tag|=262144ULL;}
    if(!value.certificate_chain_file.empty()){tag|=524288ULL;}
    if(!value.private_key.empty()){tag|=1048576ULL;}
    if(!value.private_key_file.empty()){tag|=2097152ULL;}
    if(!value.rsa_private_key.empty()){tag|=8388608ULL;}
    if(!value.rsa_private_key_file.empty()){tag|=16777216ULL;}
    if(!value.tmp_dh.empty()){tag|=67108864ULL;}
    if(!value.tmp_dh_file.empty()){tag|=134217728ULL;}
    write(stream,tag);
    if(stream.error()){return;}
    write(stream,size_of(value));
    if(stream.error()){return;}
    if(tag&1ULL)
    {
      uint32_t len = (uint32_t)(value.verify_paths).size();
      write(stream,len);
      int32_t count = 0;
      for (::std::vector< ::std::string >::const_iterator i = value.verify_paths.begin() ; i != value.verify_paths.end() ; ++i, ++count)
      {
        {
          uint32_t len = (uint32_t)(*i).size();
          write(stream,len);
          stream.write((*i).data(),len);
        }
        {if(stream.error()){stream.trace_error("verify_paths",count);return;}}
      }
    }
    {write(stream,value.default_verify_paths);{if(stream.error()){stream.trace_error("default_verify_paths",-1);return;}}}
    if(tag&4ULL)
    {
      uint32_t len = (uint32_t)(value.certificate_authority).size();
      write(stream,len);
      stream.write((value.certificate_authority).data(),len);
      {if(stream.error()){stream.trace_error("certificate_authority",-1);return;}}
    }
    if(tag&8ULL)
    {
      uint32_t len = (uint32_t)(value.verify_file).size();
      write(stream,len);
      stream.write((value.verify_file).data(),len);
      {if(stream.error()){stream.trace_error("verify_file",-1);return;}}
    }
    {write(stream,value.default_workarounds);{if(stream.error()){stream.trace_error("default_workarounds",-1);return;}}}
    {write(stream,value.single_dh_use);{if(stream.error()){stream.trace_error("single_dh_use",-1);return;}}}
    {write(stream,value.no_sslv2);{if(stream.error()){stream.trace_error("no_sslv2",-1);return;}}}
    {write(stream,value.no_sslv3);{if(stream.error()){stream.trace_error("no_sslv3",-1);return;}}}
    {write(stream,value.no_tlsv1);{if(stream.error()){stream.trace_error("no_tlsv1",-1);return;}}}
    {write(stream,value.no_compression);{if(stream.error()){stream.trace_error("no_compression",-1);return;}}}
    {write(stream,value.verify_depth);{if(stream.error()){stream.trace_error("verify_depth",-1);return;}}}
    {write(stream,value.verify_none);{if(stream.error()){stream.trace_error("verify_none",-1);return;}}}
    {write(stream,value.verify_peer);{if(stream.error()){stream.trace_error("verify_peer",-1);return;}}}
    {write(stream,value.verify_fail_if_no_peer_cert);{if(stream.error()){stream.trace_error("verify_fail_if_no_peer_cert",-1);return;}}}
    {write(stream,value.verify_client_once);{if(stream.error()){stream.trace_error("verify_client_once",-1);return;}}}
    if(tag&32768ULL)
    {
      uint32_t len = (uint32_t)(value.certificate).size();
      write(stream,len);
      stream.write((value.certificate).data(),len);
      {if(stream.error()){stream.trace_error("certificate",-1);return;}}
    }
    if(tag&65536ULL)
    {
      uint32_t len = (uint32_t)(value.certificate_file).size();
      write(stream,len);
      stream.write((value.certificate_file).data(),len);
      {if(stream.error()){stream.trace_error("certificate_file",-1);return;}}
    }
    {write(stream,value.certificate_format);{if(stream.error()){stream.trace_error("certificate_format",-1);return;}}}
    if(tag&262144ULL)
    {
      uint32_t len = (uint32_t)(value.certificate_chain).size();
      write(stream,len);
      stream.write((value.certificate_chain).data(),len);
      {if(stream.error()){stream.trace_error("certificate_chain",-1);return;}}
    }
    if(tag&524288ULL)
    {
      uint32_t len = (uint32_t)(value.certificate_chain_file).size();
      write(stream,len);
      stream.write((value.certificate_chain_file).data(),len);
      {if(stream.error()){stream.trace_error("certificate_chain_file",-1);return;}}
    }
    if(tag&1048576ULL)
    {
      uint32_t len = (uint32_t)(value.private_key).size();
      write(stream,len);
      stream.write((value.private_key).data(),len);
      {if(stream.error()){stream.trace_error("private_key",-1);return;}}
    }
    if(tag&2097152ULL)
    {
      uint32_t len = (uint32_t)(value.private_key_file).size();
      write(stream,len);
      stream.write((value.private_key_file).data(),len);
      {if(stream.error()){stream.trace_error("private_key_file",-1);return;}}
    }
    {write(stream,value.private_key_format);{if(stream.error()){stream.trace_error("private_key_format",-1);return;}}}
    if(tag&8388608ULL)
    {
      uint32_t len = (uint32_t)(value.rsa_private_key).size();
      write(stream,len);
      stream.write((value.rsa_private_key).data(),len);
      {if(stream.error()){stream.trace_error("rsa_private_key",-1);return;}}
    }
    if(tag&16777216ULL)
    {
      uint32_t len = (uint32_t)(value.rsa_private_key_file).size();
      write(stream,len);
      stream.write((value.rsa_private_key_file).data(),len);
      {if(stream.error()){stream.trace_error("rsa_private_key_file",-1);return;}}
    }
    {write(stream,value.rsa_private_key_format);{if(stream.error()){stream.trace_error("rsa_private_key_format",-1);return;}}}
    if(tag&67108864ULL)
    {
      uint32_t len = (uint32_t)(value.tmp_dh).size();
      write(stream,len);
      stream.write((value.tmp_dh).data(),len);
      {if(stream.error()){stream.trace_error("tmp_dh",-1);return;}}
    }
    if(tag&134217728ULL)
    {
      uint32_t len = (uint32_t)(value.tmp_dh_file).size();
      write(stream,len);
      stream.write((value.tmp_dh_file).data(),len);
      {if(stream.error()){stream.trace_error("tmp_dh_file",-1);return;}}
    }
    return;
  }

}

#endif
