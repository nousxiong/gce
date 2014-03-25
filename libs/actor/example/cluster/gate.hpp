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
#include "basic_app.hpp"
#include <gce/actor/all.hpp>
#include <boost/asio.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>

class gate
  : public basic_app
{
  typedef boost::asio::ip::tcp::acceptor acceptor_t;

public:
  gate(std::string cln_ep, app_ctxid_list_t game_list);
  ~gate();

public:
  gce::aid_t start(gce::self_t sire);

private:
  void run(gce::self_t);
  void accept(gce::self_t, std::vector<gce::aid_t> conn_group_list);
  static void conn_group(gce::self_t, gce::aid_t ga_id);

private:
  std::string const cln_ep_;
  app_ctxid_list_t const game_list_;

  boost::scoped_ptr<acceptor_t> acpr_;
};

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_GATE_HPP
