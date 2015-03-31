///
/// bytes.hpp
///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_BYTES_HPP
#define GCE_DETAIL_BYTES_HPP

#include <gce/config.hpp>
#include <gce/integer.hpp>
#include <string>

namespace gce
{
namespace detail
{
typedef std::basic_string<gce::byte_t, std::char_traits<gce::byte_t>, std::allocator<gce::byte_t> > bytes_t;
}
}

#endif /// GCE_DETAIL_BYTES_HPP
