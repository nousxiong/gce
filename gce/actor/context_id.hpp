///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_CONTEXT_ID_HPP
#define GCE_ACTOR_CONTEXT_ID_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/match.hpp>

namespace gce
{
typedef match_t ctxid_t;
static match_t const ctxid_nil = make_match(u64_nil);
}

#endif /// GCE_ACTOR_CONTEXT_ID_HPP
