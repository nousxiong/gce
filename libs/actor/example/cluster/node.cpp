///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "node.hpp"
#include <boost/foreach.hpp>

///----------------------------------------------------------------------------
node::node(gce::attributes attrs)
  : ctx_(attrs)
  , base_(gce::spawn(ctx_))
  , sig_(ctx_.get_io_service())
{
  sig_.add(SIGINT);
  sig_.add(SIGTERM);
#if defined(SIGQUIT)
  sig_.add(SIGQUIT);
#endif /// defined(SIGQUIT)
}
///----------------------------------------------------------------------------
node::~node()
{
}
///----------------------------------------------------------------------------
void node::wait_end()
{
  gce::recv(base_);
}
///----------------------------------------------------------------------------
void node::add_app(basic_app_ptr app)
{
  app_list_.push_back(app);
}
///----------------------------------------------------------------------------
void node::start(gce::actor_func_t init_func)
{
  gce::spawn(base_, boost::bind(&node::run, this, _1, init_func), gce::monitored);
}
///----------------------------------------------------------------------------
void node::run(gce::self_t self, gce::actor_func_t init_func)
{
  std::vector<gce::aid_t> app_list;
  try
  {
    init_func(self);

    BOOST_FOREACH(basic_app_ptr app, app_list_)
    {
      app_list.push_back(app->start(self));
    }

    gce::aid_t sig =
      gce::spawn(
        self,
        boost::bind(
          &node::handle_signal, this, _1
          ),
        gce::monitored
        );

    bool running = true;
    while (running)
    {
      gce::message msg;
      gce::aid_t sender = self.recv(msg);
      gce::match_t type = msg.get_type();
      if (type == gce::exit)
      {
        if (sender == sig)
        {
          running = false;
        }
        else
        {
          gce::exit_code_t type;
          std::string errmsg;
          msg >> type >> errmsg;

          /// one of apps quit;
          /// here, just quit;
          /// for more, you can restart the error app
          throw std::runtime_error(errmsg);
        }
      }
      else
      {
        std::string errmsg("node::run unexpected message, type: ");
        errmsg += gce::atom(type);
        throw std::runtime_error(errmsg);
      }
    }
  }
  catch (std::exception& ex)
  {
    std::printf("node::run except: %s\n", ex.what());
  }

  std::vector<gce::response_t> response_list;
  BOOST_REVERSE_FOREACH(gce::aid_t app, app_list)
  {
    response_list.push_back(gce::request(self, app, gce::atom("stop")));
  }

  BOOST_FOREACH(gce::response_t res, response_list)
  {
    gce::message msg;
    self.recv(res, msg);
  }
}
///----------------------------------------------------------------------------
void node::handle_signal(gce::self_t self)
{
  gce::yield_t yield = self.get_yield();
  gce::errcode_t ec;
  sig_.async_wait(yield[ec]);
}
///----------------------------------------------------------------------------
