///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_ACTOR_FUNCTION_HPP
#define GCE_ACTOR_DETAIL_ACTOR_FUNCTION_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/actor_ref.hpp>
#include <boost/function.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/logical.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>

namespace gce
{
namespace detail
{
template <typename Tag, typename Context>
struct actor_func
{
  BOOST_MPL_ASSERT((boost::mpl::or_<boost::is_same<Tag, stackful>, boost::is_same<Tag, stackless> >));

  actor_func()
  {
  }

  template <typename F>
  actor_func(F f)
    : f_(f)
  {
  }

  template <typename F, typename A>
  actor_func(F f, A a)
    : f_(f, a)
  {
  }

  boost::function<void (actor_ref<Tag, Context>)> f_;
};

template <typename Tag, typename Context, typename F>
inline actor_func<Tag, Context> make_actor_func(F f)
{
  return actor_func<Tag, Context>(f);
}

template <typename Tag, typename Context, typename F, typename A>
inline actor_func<Tag, Context> make_actor_func(F f, A a)
{
  return actor_func<Tag, Context>(f, a);
}

template <typename Context>
struct remote_func
{
  remote_func(actor_func<stackful, Context> const& f)
    : af_(f.f_)
  {
  }

  remote_func(actor_func<stackless, Context> const& f)
    : ef_(f.f_)
  {
  }

  boost::function<void (actor_ref<stackful, Context>)> af_;
  boost::function<void (actor_ref<stackless, Context>)> ef_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_ACTOR_FUNCTION_HPP
