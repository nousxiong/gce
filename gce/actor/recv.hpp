///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_RECV_HPP
#define GCE_ACTOR_RECV_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor.hpp>
#include <gce/actor/response.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/match.hpp>
#include <boost/bind.hpp>
#include <algorithm>

namespace gce
{
namespace detail
{
inline bool find_exit(match_t type)
{
  return type == exit;
}

inline bool check_exit(std::vector<match_t>& match_list)
{
  if (match_list.empty())
  {
    return false;
  }
  else
  {
    std::vector<match_t>::iterator itr =
      std::find_if(
        match_list.begin(),
        match_list.end(),
        boost::bind(&find_exit, _1)
        );
    return itr == match_list.end();
  }
}

template <typename Recver>
inline aid_t recv(Recver& recver, message& msg, match& mach)
{
  bool add_exit = check_exit(mach.match_list_);
  if (add_exit)
  {
    mach.match_list_.push_back(exit);
  }

  aid_t sender = recver.recv(msg, mach);
  if (add_exit && msg.get_type() == exit)
  {
    exit_code_t exc;
    std::string errmsg;
    msg >> exc >> errmsg;
    throw std::runtime_error(errmsg);
  }

  if (!sender)
  {
    throw std::runtime_error("recv timeout");
  }
  return sender;
}

inline aid_t recv(actor<nonblocked>& recver, message& msg, match& mach)
{
  bool has_exit = check_exit(mach.match_list_);
  if (!has_exit)
  {
    mach.match_list_.push_back(exit);
  }

  aid_t sender = recver.recv(msg, mach.match_list_);
  if (!has_exit && msg.get_type() == exit)
  {
    exit_code_t exc;
    std::string errmsg;
    msg >> exc >> errmsg;
    throw std::runtime_error(errmsg);
  }
  return sender;
}

template <typename Recver>
inline aid_t recv(Recver& recver, response_t res, message& msg, duration_t tmo)
{
  aid_t sender = recver.recv(res, msg, tmo);
  if (msg.get_type() == exit)
  {
    exit_code_t exc;
    std::string errmsg;
    msg >> exc >> errmsg;
    throw std::runtime_error(errmsg);
  }

  if (!sender)
  {
    throw std::runtime_error("recv response timeout");
  }
  return sender;
}

inline aid_t recv(actor<nonblocked>& recver, response_t res, message& msg, duration_t)
{
  aid_t sender = recver.recv(res, msg);
  if (msg.get_type() == exit)
  {
    exit_code_t exc;
    std::string errmsg;
    msg >> exc >> errmsg;
    throw std::runtime_error(errmsg);
  }
  return sender;
}
///------------------------------------------------------------------------------
/// recv stackless
///------------------------------------------------------------------------------
inline bool begin_recv(match& mach)
{
  bool add_exit = check_exit(mach.match_list_);
  if (add_exit)
  {
    mach.match_list_.push_back(exit);
  }
  return add_exit;
}

inline bool end_recv(
  actor<stackless>& recver, aid_t sender, message msg, 
  aid_t& osender, bool has_exit
  )
{
  osender = sender;
  bool ret = false;
  if (msg.get_type() == exit && !has_exit)
  {
    exit_code_t exc;
    std::string errmsg;
    msg >> exc >> errmsg;
    recver.get_actor().quit(exc, errmsg);
  }
  else if (!sender)
  {
    recver.get_actor().quit(exit_except, "recv timeout");
  }
  else
  {
    ret = true;
  }
  return ret;
}

inline void handle_recv(
  actor<stackless>& recver, aid_t sender, message msg, 
  aid_t& osender, bool has_exit
  )
{
  if (end_recv(recver, sender, msg, osender, has_exit))
  {
    recver.resume();
  }
}

template <typename A1>
inline void handle_recv(
  actor<stackless>& recver, aid_t sender, message msg, 
  aid_t& osender, bool has_exit, A1& a1
  )
{
  if (end_recv(recver, sender, msg, osender, has_exit))
  {
    msg >> a1;
    recver.resume();
  }
}

template <typename A1, typename A2>
inline void handle_recv(
  actor<stackless>& recver, aid_t sender, message msg, 
  aid_t& osender, bool has_exit, A1& a1, A2& a2
  )
{
  if (end_recv(recver, sender, msg, osender, has_exit))
  {
    msg >> a1 >> a2;
    recver.resume();
  }
}

template <typename A1, typename A2, typename A3>
inline void handle_recv(
  actor<stackless>& recver, aid_t sender, message msg, 
  aid_t& osender, bool has_exit, A1& a1, A2& a2, A3& a3
  )
{
  if (end_recv(recver, sender, msg, osender, has_exit))
  {
    msg >> a1 >> a2 >> a3;
    recver.resume();
  }
}

template <typename A1, typename A2, typename A3, typename A4>
inline void handle_recv(
  actor<stackless>& recver, aid_t sender, message msg, 
  aid_t& osender, bool has_exit, A1& a1, A2& a2, A3& a3, A4& a4
  )
{
  if (end_recv(recver, sender, msg, osender, has_exit))
  {
    msg >> a1 >> a2 >> a3 >> a4;
    recver.resume();
  }
}

template <typename A1, typename A2, typename A3, typename A4, typename A5>
inline void handle_recv(
  actor<stackless>& recver, aid_t sender, message msg, 
  aid_t& osender, bool has_exit, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5
  )
{
  if (end_recv(recver, sender, msg, osender, has_exit))
  {
    msg >> a1 >> a2 >> a3 >> a4 >> a5;
    recver.resume();
  }
}
}
///----------------------------------------------------------------------------
/// Receive
///----------------------------------------------------------------------------
template <typename Recver>
inline aid_t recv(Recver& recver, duration_t tmo = infin)
{
  message msg;
  match mach(tmo);
  return detail::recv(recver, msg, mach);
}
///----------------------------------------------------------------------------
template <typename Recver>
inline aid_t recv(Recver& recver, match_t type, duration_t tmo = infin)
{
  message msg;
  match mach(tmo);
  mach.match_list_.push_back(type);
  return detail::recv(recver, msg, mach);
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1>
inline aid_t recv(Recver& recver, match_t type, A1& a1, duration_t tmo = infin)
{
  message msg;
  match mach(tmo);
  mach.match_list_.push_back(type);
  aid_t sender = detail::recv(recver, msg, mach);
  if (sender)
  {
    msg >> a1;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1, typename A2>
inline aid_t recv(Recver& recver, match_t type, A1& a1, A2& a2, duration_t tmo = infin)
{
  message msg;
  match mach(tmo);
  mach.match_list_.push_back(type);
  aid_t sender = detail::recv(recver, msg, mach);
  if (sender)
  {
    msg >> a1 >> a2;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1, typename A2, typename A3>
inline aid_t recv(
  Recver& recver, match_t type, A1& a1, A2& a2, A3& a3, duration_t tmo = infin
  )
{
  message msg;
  match mach(tmo);
  mach.match_list_.push_back(type);
  aid_t sender = detail::recv(recver, msg, mach);
  if (sender)
  {
    msg >> a1 >> a2 >> a3;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1, typename A2, typename A3, typename A4>
inline aid_t recv(
  Recver& recver, match_t type, A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin
  )
{
  message msg;
  match mach(tmo);
  mach.match_list_.push_back(type);
  aid_t sender = detail::recv(recver, msg, mach);
  if (sender)
  {
    msg >> a1 >> a2 >> a3 >> a4;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <
  typename Recver, typename A1, typename A2,
  typename A3, typename A4, typename A5
  >
inline aid_t recv(
  Recver& recver, match_t type,
  A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin
  )
{
  message msg;
  match mach(tmo);
  mach.match_list_.push_back(type);
  aid_t sender = detail::recv(recver, msg, mach);
  if (sender)
  {
    msg >> a1 >> a2 >> a3 >> a4 >> a5;
  }
  return sender;
}
///----------------------------------------------------------------------------
inline void recv(actor<stackless>& recver, aid_t& sender, duration_t tmo = infin)
{
  match mach(tmo);
  bool has_exit = detail::begin_recv(mach);
  recver.recv(
    boost::bind(
      &detail::handle_recv, _1, _2, _3, 
      boost::ref(sender), has_exit
      ),
    mach
    );
}
///----------------------------------------------------------------------------
template <typename A1>
inline void recv(
  actor<stackless>& recver, aid_t& sender, match_t type,
  A1& a1, duration_t tmo = infin
  )
{
  match mach(tmo);
  mach.match_list_.push_back(type);
  bool has_exit = detail::begin_recv(mach);
  recver.recv(
    boost::bind(
      &detail::handle_recv<A1>, _1, _2, _3, 
      boost::ref(sender), has_exit, boost::ref(a1)
      ),
    mach
    );
}
///----------------------------------------------------------------------------
template <
  typename A1, typename A2
  >
inline void recv(
  actor<stackless>& recver, aid_t& sender, match_t type,
  A1& a1, A2& a2, duration_t tmo = infin
  )
{
  match mach(tmo);
  mach.match_list_.push_back(type);
  bool has_exit = detail::begin_recv(mach);
  recver.recv(
    boost::bind(
      &detail::handle_recv<A1, A2>, _1, _2, _3, 
      boost::ref(sender), has_exit, boost::ref(a1), boost::ref(a2)
      ),
    mach
    );
}
///----------------------------------------------------------------------------
template <
  typename A1, typename A2, typename A3
  >
inline void recv(
  actor<stackless>& recver, aid_t& sender, match_t type,
  A1& a1, A2& a2, A3& a3, duration_t tmo = infin
  )
{
  match mach(tmo);
  mach.match_list_.push_back(type);
  bool has_exit = detail::begin_recv(mach);
  recver.recv(
    boost::bind(
      &detail::handle_recv<A1, A2, A3>, _1, _2, _3, 
      boost::ref(sender), has_exit, boost::ref(a1), boost::ref(a2), 
      boost::ref(a3)
      ),
    mach
    );
}
///----------------------------------------------------------------------------
template <
  typename A1, typename A2,
  typename A3, typename A4
  >
inline void recv(
  actor<stackless>& recver, aid_t& sender, match_t type,
  A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin
  )
{
  match mach(tmo);
  mach.match_list_.push_back(type);
  bool has_exit = detail::begin_recv(mach);
  recver.recv(
    boost::bind(
      &detail::handle_recv<A1, A2, A3, A4>, _1, _2, _3, 
      boost::ref(sender), has_exit, boost::ref(a1), boost::ref(a2), 
      boost::ref(a3), boost::ref(a4)
      ),
    mach
    );
}
///----------------------------------------------------------------------------
template <
  typename A1, typename A2,
  typename A3, typename A4, typename A5
  >
inline void recv(
  actor<stackless>& recver, aid_t& sender, match_t type,
  A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin
  )
{
  match mach(tmo);
  mach.match_list_.push_back(type);
  bool has_exit = detail::begin_recv(mach);
  recver.recv(
    boost::bind(
      &detail::handle_recv<A1, A2, A3, A4, A5>, _1, _2, _3, 
      boost::ref(sender), has_exit, boost::ref(a1), boost::ref(a2), 
      boost::ref(a3), boost::ref(a4), boost::ref(a5)
      ),
    mach
    );
}
///----------------------------------------------------------------------------
/// Receive response
///----------------------------------------------------------------------------
template <typename Recver>
inline aid_t recv(Recver& recver, response_t res, duration_t tmo = infin)
{
  message msg;
  return detail::recv(recver, res, msg, tmo);
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1>
inline aid_t recv(Recver& recver, response_t res, A1& a1, duration_t tmo = infin)
{
  message msg;
  aid_t sender = detail::recv(recver, res, msg, tmo);
  if (sender)
  {
    msg >> a1;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1, typename A2>
inline aid_t recv(Recver& recver, response_t res, A1& a1, A2& a2, duration_t tmo = infin)
{
  message msg;
  aid_t sender = detail::recv(recver, res, msg, tmo);
  if (sender)
  {
    msg >> a1 >> a2;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1, typename A2, typename A3>
inline aid_t recv(Recver& recver, response_t res, A1& a1, A2& a2, A3& a3, duration_t tmo = infin)
{
  message msg;
  aid_t sender = detail::recv(recver, res, msg, tmo);
  if (sender)
  {
    msg >> a1 >> a2 >> a3;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1, typename A2, typename A3, typename A4>
inline aid_t recv(
  Recver& recver, response_t res, A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin
  )
{
  message msg;
  aid_t sender = detail::recv(recver, res, msg, tmo);
  if (sender)
  {
    msg >> a1 >> a2 >> a3 >> a4;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <
  typename Recver, typename A1, typename A2,
  typename A3, typename A4, typename A5
  >
inline aid_t recv(
  Recver& recver, response_t res,
  A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin
  )
{
  message msg;
  aid_t sender = detail::recv(recver, res, msg, tmo);
  if (sender)
  {
    msg >> a1 >> a2 >> a3 >> a4 >> a5;
  }
  return sender;
}
///----------------------------------------------------------------------------
inline void recv(actor<stackless>& recver, response_t res, aid_t& sender, duration_t tmo = infin)
{
  match mach(tmo);
  recver.recv(
    boost::bind(
      &detail::handle_recv, _1, _2, _3, 
      boost::ref(sender), false
      ),
    res, tmo
    );
}
///----------------------------------------------------------------------------
template <typename A1>
inline void recv(
  actor<stackless>& recver, response_t res, aid_t& sender, 
  A1& a1, duration_t tmo = infin
  )
{
  recver.recv(
    boost::bind(
      &detail::handle_recv<A1>, _1, _2, _3, 
      boost::ref(sender), false, boost::ref(a1)
      ),
    res, tmo
    );
}
///----------------------------------------------------------------------------
template <
  typename A1, typename A2
  >
inline void recv(
  actor<stackless>& recver, response_t res, aid_t& sender, 
  A1& a1, A2& a2, duration_t tmo = infin
  )
{
  recver.recv(
    boost::bind(
      &detail::handle_recv<A1, A2>, _1, _2, _3, 
      boost::ref(sender), false, boost::ref(a1), boost::ref(a2)
      ),
    res, tmo
    );
}
///----------------------------------------------------------------------------
template <
  typename A1, typename A2, typename A3
  >
inline void recv(
  actor<stackless>& recver, response_t res, aid_t& sender, 
  A1& a1, A2& a2, A3& a3, duration_t tmo = infin
  )
{
  recver.recv(
    boost::bind(
      &detail::handle_recv<A1, A2, A3>, _1, _2, _3, 
      boost::ref(sender), false, boost::ref(a1), boost::ref(a2), 
      boost::ref(a3)
      ),
    res, tmo
    );
}
///----------------------------------------------------------------------------
template <
  typename A1, typename A2,
  typename A3, typename A4
  >
inline void recv(
  actor<stackless>& recver, response_t res, aid_t& sender, 
  A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin
  )
{
  recver.recv(
    boost::bind(
      &detail::handle_recv<A1, A2, A3, A4>, _1, _2, _3, 
      boost::ref(sender), false, boost::ref(a1), boost::ref(a2), 
      boost::ref(a3), boost::ref(a4)
      ),
    res, tmo
    );
}
///----------------------------------------------------------------------------
template <
  typename A1, typename A2,
  typename A3, typename A4, typename A5
  >
inline void recv(
  actor<stackless>& recver, response_t res, aid_t& sender, 
  A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin
  )
{
  recver.recv(
    boost::bind(
      &detail::handle_recv<A1, A2, A3, A4, A5>, _1, _2, _3, 
      boost::ref(sender), false, boost::ref(a1), boost::ref(a2), 
      boost::ref(a3), boost::ref(a4), boost::ref(a5)
      ),
    res, tmo
    );
}
///----------------------------------------------------------------------------
}

#endif /// GCE_ACTOR_RECV_HPP
