///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_RESPONSE_HPP
#define GCE_ACTOR_RESPONSE_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor_id.hpp>
#include <boost/array.hpp>

namespace gce
{
class response
{
public:
  response() : id_(sid_nil) {}
  response(sid_t id, aid_t aid) : id_(id), aid_(aid){}
  response(sid_t id, aid_t aid, aid_t recver) : id_(id), aid_(aid), recver_(recver) {}
  response(sid_t id, aid_t aid, svcid_t svc) : id_(id), aid_(aid), svc_(svc) {}
  ~response() {}

public:
  bool valid() const { return id_ != sid_nil; }
  sid_t get_id() const { return id_; }
  aid_t get_aid() const { return aid_; }

  /// for local use
  aid_t get_recver() const { return recver_; }
  svcid_t get_svcid() const { return svc_; }

  std::string to_string() const
  {
    typedef boost::array<char, 32> strbuf_t;
    std::string rt;
    rt += "<";
    rt += boost::lexical_cast<strbuf_t>(id_).cbegin();
    rt += ".";
    rt += aid_.to_string();
    rt += ">";
    return rt;
  }

private:
  sid_t id_;
  aid_t aid_;

  /// for local use
  aid_t recver_;
  svcid_t svc_;
};

typedef response resp_t;
}

#endif /// GCE_ACTOR_RESPONSE_HPP
