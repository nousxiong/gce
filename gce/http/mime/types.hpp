///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_HTTP_MIME_TYPES_HPP
#define GCE_HTTP_MIME_TYPES_HPP

#include <gce/http/config.hpp>

namespace gce
{
namespace http
{
namespace mime
{
namespace detail
{
struct mapping
{
  char const* extension_;
  char const* mime_type_;
};

static mapping mappings[] =
{
  { "gif", "image/gif" },
  { "htm", "text/html" },
  { "html", "text/html" },
  { "jpg", "image/jpeg" },
  { "png", "image/png" },
  { 0, 0 } // Marks end of list.
};
}

static std::string ext2type(std::string const& extension)
{
  for (detail::mapping* m = detail::mappings; m->extension_; ++m)
  {
    if (m->extension_ == extension)
    {
      return m->mime_type_;
    }
  }

  return "text/plain";
}
} /// namespace mime
} /// namespace http
} /// namespace gce

#endif /// GCE_HTTP_MIME_TYPES_HPP
