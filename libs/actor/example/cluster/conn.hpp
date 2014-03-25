///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_EXAMPLE_CLUSTER_CONN_HPP
#define GCE_ACTOR_EXAMPLE_CLUSTER_CONN_HPP

#include "app.hpp"
#include "socket.hpp"
#include <gce/actor/all.hpp>

typedef boost::shared_ptr<tcp_socket> socket_ptr;
class conn
{
  enum status
  {
    conned = 0,
    online
  };

public:
  conn();
  ~conn();

public:
  static void run(
    gce::self_t, socket_ptr,
    gce::aid_t group_aid,
    app_ctxid_list_t game_list
    );
  static void timeout(gce::self_t);
  static void recv(
    gce::self_t, socket_ptr skt,
    gce::aid_t tmo_aid, gce::aid_t conn_aid,
    app_ctxid_list_t game_list
    );
};

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_CONN_HPP
