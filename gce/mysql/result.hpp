///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_RESULT_HPP
#define GCE_MYSQL_RESULT_HPP

#include <gce/mysql/config.hpp>
#include <boost/foreach.hpp>
#include <vector>

namespace gce
{
namespace mysql
{
struct result
{
  typedef std::pair<MYSQL_RES*, size_t> result_pair;
  result()
  {
  }

  ~result()
  {
    clear();
  }

  void clear()
  {
    BOOST_FOREACH(result_pair pr, res_list_)
    {
      mysql_free_result(pr.first);
    }
    res_list_.clear();
  }

  gce::packer pkr_;
  std::vector<result_pair> res_list_;
};
typedef boost::shared_ptr<result> result_ptr;
}
}

#endif /// GCE_MYSQL_RESULT_HPP
