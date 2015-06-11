///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "application.hpp"
#include <boost/lexical_cast.hpp>

namespace usr
{
///----------------------------------------------------------------------------
application::application(char const* ctxid, gce::log::logger_t& lg)
  : ctxid_(ctxid)
  , lg_(lg)
  , ctx_(make_init(ctxid, lg))
{
  GCE_INFO(lg_) << "application<" << ctxid_ << "> init done.";
}
///----------------------------------------------------------------------------
application::~application()
{
  GCE_INFO(lg_) << "application<" << ctxid_ << "> done.";
}
///----------------------------------------------------------------------------
void application::run(char const* script)
{
  GCE_INFO(lg_) << "application<" << ctxid_ << "> running...";

  gce::threaded_actor base = gce::spawn(ctx_);
  gce::aid_t root = gce::spawn(base, script, gce::monitored);
  base->send(root, "init", ctxid_);
  base->recv(gce::exit);
}
///----------------------------------------------------------------------------
gce::context::init_t application::make_init(char const* ctxid, gce::log::logger_t& lg)
{
  GCE_INFO(lg) << "application<" << ctxid << "> initing...";
  gce::context::init_t init;
  init.attrs_.lg_ = lg;

  uint64_t id = 0;
  if (boost::conversion::try_lexical_convert(ctxid, id))
  {
    init.attrs_.id_ = gce::to_match(id);
  }
  else
  {
    init.attrs_.id_ = gce::to_match(ctxid);
  }
  return init;
}
///----------------------------------------------------------------------------
}
