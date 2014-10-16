///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_EXAMPLE_CLUSTER_GATE_HPP
#define GCE_ACTOR_EXAMPLE_CLUSTER_GATE_HPP

#include "conn.hpp"
#include <gce/actor/all.hpp>
#include <boost/asio.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>

class gate
{
  typedef boost::asio::ip::tcp::acceptor acceptor_t;

public:
  static gce::aid_t start(
    gce::actor<gce::stackful>& sire, 
    gce::match_t svc_name,
    std::string cln_ep, 
    app_ctxid_list_t game_list
    );

private:
  static void run(
    gce::actor<gce::stackful>& self,
    gce::match_t svc_name,
    std::string cln_ep, 
    app_ctxid_list_t game_list
    );
  static void accept(
    gce::actor<gce::stackful>& self, 
    acceptor_t& acpr, 
    app_ctxid_list_t game_list,
    std::vector<gce::aid_t> conn_group_list
    );
  static void conn_group(
    gce::actor<gce::stackful>& self, 
    gce::aid_t ga_id
    );
};

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_GATE_HPP
