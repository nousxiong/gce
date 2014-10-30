///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASSERT_CONFIG_HPP
#define GCE_ASSERT_CONFIG_HPP

#include <gce/config.hpp>
#include <gce/integer.hpp>

#ifndef GCE_SMALL_ASSERT_SIZE
# define GCE_SMALL_ASSERT_SIZE 512
#endif

#ifndef GCE_ASSERT_MIN_GROW_SIZE
# define GCE_ASSERT_MIN_GROW_SIZE 64
#endif

#ifndef GCE_ENABLE_ASSERT
# ifndef NDEBUG
#   define GCE_ENABLE_ASSERT
# endif
#endif

#endif /// GCE_ASSERT_CONFIG_HPP
