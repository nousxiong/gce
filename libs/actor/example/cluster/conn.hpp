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
  : public boost::enable_shared_from_this<conn>
{
  enum status
  {
    conned = 0,
    online
  };
  typedef boost::shared_ptr<conn> ptr_t;

public:
  conn(socket_ptr skt, gce::aid_t group_aid, app_ctxid_list_t game_list);
  ~conn();

public:
  static gce::aid_t spawn(
    gce::actor<gce::stackful>& sire, socket_ptr skt, 
    gce::aid_t group_aid, app_ctxid_list_t game_list
    );

public:
  void run(gce::actor<gce::stackless>&);

private:
  class timeout
  {
  public:
    void run(gce::actor<gce::stackless>&);

  private:
    boost::chrono::seconds curr_tmo_;
    std::size_t max_count_;
    std::size_t curr_count_;

    gce::message msg_;
    bool running_;
    gce::aid_t sender_;
    gce::errcode_t ec_;
  };

  class recv
  {
  public:
    recv(socket_ptr skt, app_ctxid_list_t game_list);

  public:
    void run(
      gce::actor<gce::stackless>&,
      gce::aid_t tmo_aid, gce::aid_t conn_aid
      );

  private:
    socket_ptr skt_;
    app_ctxid_list_t game_list_;

    gce::aid_t tmo_aid_;
    gce::aid_t conn_aid_;
    gce::svcid_t game_svcid_;
    gce::aid_t usr_aid_;
    status stat_;

    gce::message msg_;
    gce::errcode_t ec_;
    std::string errmsg_;
    std::string username_;
    gce::match_t type_;
  };

private:
  socket_ptr skt_;
  gce::aid_t group_aid_;
  app_ctxid_list_t game_list_;

  timeout tmo_;
  recv rcv_;

  /// local stack
  gce::message msg_;
  gce::aid_t tmo_aid_;
  bool running_;
  gce::aid_t sender_;
  gce::errcode_t ec_;
  gce::match_t type_;
  gce::aid_t tmp_aid_;
};

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_CONN_HPP
