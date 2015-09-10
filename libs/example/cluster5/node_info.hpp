///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef CLUSTER5_NODE_INFO_HPP
#define CLUSTER5_NODE_INFO_HPP

#include "node_info.adl.h"
#include <gce/actor/all.hpp>

namespace cluster5
{
node_info make_node_info(gce::svcid_t svcid, std::string const& ep)
{
  node_info ni;
  ni.svcid_ = svcid;
  ni.ep_ = ep;
  return ni;
}

static bool valid(node_info const& ni)
{
  return ni.svcid_ != gce::svcid_nil;
}
}

/// 如果使用了amsg，则这里会绑定数据结构；如果不是amsg则不会有影响
GCE_PACK(cluster5::node_info, (v.svcid_)(v.ep_));
GCE_PACK(cluster5::node_info_list, (v.list_));

#endif /// CLUSTER5_NODE_INFO_HPP
