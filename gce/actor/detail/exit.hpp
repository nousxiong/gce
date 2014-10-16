///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_EXIT_HPP
#define GCE_ACTOR_DETAIL_EXIT_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor_id.hpp>

namespace gce
{
namespace detail
{
class exit_t
{
public:
  exit_t() : ec_(exit_normal) {}
  exit_t(exit_code_t ec, aid_t aid) : ec_(ec), aid_(aid) {}
  ~exit_t() {}

public:
  exit_code_t get_code() const { return ec_; }
  aid_t get_aid() const { return aid_; }

private:
  exit_code_t ec_;
  aid_t aid_;
};

class fwd_exit_t
{
public:
  fwd_exit_t() : ec_(exit_normal) {}
  fwd_exit_t(exit_code_t ec, aid_t aid, aid_t skt)
    : ec_(ec)
    , aid_(aid)
    , skt_(skt)
  {
  }

  ~fwd_exit_t()
  {
  }

public:
  exit_code_t get_code() const { return ec_; }
  aid_t get_aid() const { return aid_; }
  aid_t get_skt() const { return skt_; }

private:
  exit_code_t ec_;
  aid_t aid_;
  aid_t skt_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_EXIT_HPP
