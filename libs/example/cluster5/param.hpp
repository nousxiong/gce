///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef CLUSTER5_PARAM_HPP
#define CLUSTER5_PARAM_HPP

#include "param.adl.h"
#include <gce/actor/all.hpp>

/// 如果使用了amsg，则这里会绑定数据结构；如果不是amsg则不会有影响
GCE_PACK(cluster5::param_list, (v.list_));

#endif /// CLUSTER5_PARAM_HPP
