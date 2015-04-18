///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_PACKER_HPP
#define GCE_ACTOR_DETAIL_PACKER_HPP

#include <gce/config.hpp>
#include <gce/assert/all.hpp>

#if GCE_PACKER == GCE_AMSG
# include <gce/amsg/all.hpp>
#elif GCE_PACKER == GCE_ADATA
# include <gce/adata/cpp/adata.hpp>
#endif

namespace gce
{
class packer
{
public:
#if GCE_PACKER == GCE_AMSG
  typedef amsg::error_code_t error_code_t;
#elif GCE_PACKER == GCE_ADATA
  typedef adata::error_code_t error_code_t;
#endif

  static error_code_t ok()
  {
#if GCE_PACKER == GCE_AMSG
    return amsg::success;
#elif GCE_PACKER == GCE_ADATA
    return adata::success;
#endif
  }

public:
  template <typename T>
  static size_t size_of(T const& t)
  {
#if GCE_PACKER == GCE_AMSG
    return amsg::size_of(t);
#elif GCE_PACKER == GCE_ADATA
    return adata::size_of(t);
#endif
  }

  void set_read(byte_t const* buffer, size_t len)
  {
    stream_.set_read(buffer, len);
  }

  void set_write(byte_t* buffer, size_t len)
  {
    stream_.set_write(buffer, len);
  }

  template <typename T>
  void read(T& t)
  {
#if GCE_PACKER == GCE_AMSG
    amsg::read(stream_, t);
#elif GCE_PACKER == GCE_ADATA
    adata::read(stream_, t);
#endif
    GCE_VERIFY(!stream_.bad()).msg(stream_.message());
  }

  template <typename T>
  void read(T& t, error_code_t& ec)
  {
#if GCE_PACKER == GCE_AMSG
    amsg::read(stream_, t);
#elif GCE_PACKER == GCE_ADATA
    adata::read(stream_, t);
#endif
    if (stream_.bad())
    {
      ec = stream_.error_code();
    }
  }

  template <typename T>
  void write(T const& t)
  {
#if GCE_PACKER == GCE_AMSG
    amsg::write(stream_, t);
#elif GCE_PACKER == GCE_ADATA
    adata::write(stream_, t);
#endif
    GCE_VERIFY(!stream_.bad()).msg(stream_.message());
  }

  template <typename T>
  void write(T const& t, error_code_t& ec)
  {
#if GCE_PACKER == GCE_AMSG
    amsg::write(stream_, t);
#elif GCE_PACKER == GCE_ADATA
    adata::write(stream_, t);
#endif
    if (stream_.bad())
    {
      ec = stream_.error_code();
    }
  }

  void write(byte_t const* buffer, size_t len)
  {
    stream_.write((char const*)buffer, len);
    GCE_VERIFY(!stream_.bad()).msg(stream_.message());
  }

  byte_t const* skip_read(size_t len)
  {
    byte_t const* ptr = stream_.skip_read(len);
    GCE_VERIFY(!stream_.bad()).msg(stream_.message());
    return ptr;
  }

  byte_t* skip_write(size_t len)
  {
    byte_t* ptr = stream_.append_write(len);
    GCE_VERIFY(!stream_.bad()).msg(stream_.message());
    return ptr;
  }

  void clear()
  {
    stream_.clear();
  }

  size_t read_length() const
  {
    return stream_.read_length();
  }

  size_t write_length() const
  {
    return stream_.write_length();
  }

#if GCE_PACKER == GCE_AMSG
  amsg::zero_copy_buffer& get_stream()
  {
    return stream_;
  }
#elif GCE_PACKER == GCE_ADATA
  adata::zero_copy_buffer& get_stream()
  {
    return stream_;
  }
#endif

private:
#if GCE_PACKER == GCE_AMSG
  amsg::zero_copy_buffer stream_;
#elif GCE_PACKER == GCE_ADATA
  adata::zero_copy_buffer stream_;
#endif
};
}

#endif /// GCE_ACTOR_DETAIL_PACKER_HPP
