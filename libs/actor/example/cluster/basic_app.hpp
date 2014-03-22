///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_EXAMPLE_CLUSTER_BASIC_APP_HPP
#define GCE_ACTOR_EXAMPLE_CLUSTER_BASIC_APP_HPP

#include <gce/actor/all.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>

class basic_app
{
public:
  basic_app() {}
  virtual ~basic_app() {}

public:
  virtual gce::aid_t start(gce::self_t sire) = 0;
};

typedef boost::shared_ptr<basic_app> basic_app_ptr;

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_BASIC_APP_HPP
