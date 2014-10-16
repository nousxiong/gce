///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_LINK_HPP
#define GCE_ACTOR_DETAIL_LINK_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor_id.hpp>

namespace gce
{
namespace detail
{
class link_t
{
public:
  link_t() : type_(no_link) {}
  link_t(link_type type, aid_t aid) : type_(type), aid_(aid) {}
  ~link_t() {}

public:
  link_type get_type() const { return type_; }
  aid_t get_aid() const { return aid_; }

private:
  link_type type_;
  aid_t aid_;
};

class fwd_link_t
{
public:
  fwd_link_t() : type_(no_link) {}
  fwd_link_t(link_type type, aid_t aid, aid_t skt)
    : type_(type)
    , aid_(aid)
    , skt_(skt)
  {
  }

  ~fwd_link_t()
  {
  }

public:
  link_type get_type() const { return type_; }
  aid_t get_aid() const { return aid_; }
  aid_t get_skt() const { return skt_; }

private:
  link_type type_;
  aid_t aid_;
  aid_t skt_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_LINK_HPP


