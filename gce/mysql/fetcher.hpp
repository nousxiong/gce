///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_FETCHER_HPP
#define GCE_MYSQL_FETCHER_HPP

#include <gce/mysql/config.hpp>
#include <gce/mysql/result.hpp>
#include <gce/mysql/row.hpp>
#include <vector>

namespace gce
{
namespace mysql
{
class fetcher
{
public:
  explicit fetcher(result_ptr res)
    : res_(res)
    , table_size_(res->res_list_.size())
  {
  }

  ~fetcher()
  {
  }

public:
  size_t table_size() const
  {
    return table_size_;
  }

  size_t row_size(size_t i) const
  {
    GCE_ASSERT(i < table_size_)(i)(table_size_);
    return res_->res_list_[i].second;
  }

  size_t field_size(size_t i) const
  {
    GCE_ASSERT(i < table_size_)(i)(table_size_);
    if (res_->res_list_[i].first == NULL)
    {
      return 0;
    }
    else
    {
      return (size_t)mysql_num_fields(res_->res_list_[i].first);
    }
  }

  row get_row(size_t tabidx, size_t rowidx)
  {
    GCE_ASSERT(tabidx < table_size_)(tabidx)(table_size_);
    size_t rowsize = res_->res_list_[tabidx].second;
    GCE_ASSERT(rowidx < rowsize)(rowidx)(rowsize);

    MYSQL_RES* my_res = res_->res_list_[tabidx].first;
    if (my_res != NULL)
    {
      return row(my_res, rowidx);
    }
    else
    {
      return row_nil;
    }
  }

private:
  result_ptr res_;
  size_t table_size_;
};
}
}

#endif /// GCE_MYSQL_FETCHER_HPP
