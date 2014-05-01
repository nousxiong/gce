///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_THREAD_FWD_HPP
#define GCE_ACTOR_THREAD_FWD_HPP

#include <gce/actor/config.hpp>
#include <boost/function.hpp>

namespace gce
{
typedef std::size_t thrid_t;
typedef boost::function<void (thrid_t)> thread_callback_t;
}

#endif /// GCE_ACTOR_THREAD_FWD_HPP
