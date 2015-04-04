///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_TO_MATCH_HPP
#define GCE_ACTOR_TO_MATCH_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/atom.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>
#include <string>

namespace gce
{
namespace detail
{
struct match_enum_t {};
struct match_integer_t {};
struct match_other_t {};

template <typename Match>
inline match_t to_match_impl(match_enum_t, Match match)
{
  match_t rt;
  rt.val_ = match;
  return rt;
}

template <typename Match>
inline match_t to_match_impl(match_integer_t, Match match)
{
  match_t rt;
  rt.val_ = match;
  return rt;
}

inline match_t to_match_impl(char const* match)
{
  return atom(match);
}

inline match_t to_match_impl(std::string const& match)
{
  return atom(match.c_str());
}

inline match_t to_match_impl(match_t match)
{
  return match;
}

template <typename Match>
inline match_t to_match_impl(match_other_t, Match match)
{
  return to_match_impl(match);
}
} /// namespace detail

template <typename Match>
inline match_t to_match(Match match)
{
  typedef typename boost::remove_const<
    typename boost::remove_reference<
      typename boost::remove_volatile<Match>::type
      >::type
    >::type param_t;
  typedef typename boost::mpl::if_<
    typename boost::is_enum<param_t>::type, detail::match_enum_t, detail::match_other_t
    >::type select_enum_t;
  typedef typename boost::mpl::if_<
    typename boost::is_integral<param_t>::type, detail::match_integer_t, select_enum_t
    >::type select_t;
  return detail::to_match_impl(select_t(), match);
}
}

#endif /// GCE_ACTOR_TO_MATCH_HPP
