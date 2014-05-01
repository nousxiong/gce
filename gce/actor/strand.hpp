///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_STRAND_HPP
#define GCE_ACTOR_STRAND_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/context.hpp>

namespace gce
{
template <typename Sire>
class strand
{
public:
  strand(Sire& sire)
   : sire_(sire)
   , thr_(sire_.get_context()->select_thread())
   , ios_(thr_.get_io_service())
 {
 }

  ~strand()
  {
  }

public:
  inline Sire& get_sire() { return sire_; }
  inline thread& get_thread() { return thr_; }
  inline io_service_t& get_io_service() { return ios_; }

private:
  Sire& sire_;
  thread& thr_;
  io_service_t& ios_;
};

template <typename Sire>
inline strand<Sire> make_strand(Sire& sire)
{
  return strand<Sire>(sire);
}
}

#endif /// GCE_ACTOR_STRAND_HPP
