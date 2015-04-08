///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

namespace gce
{
class remote_relay_ut
{
public:
  static void run()
  {
    std::cout << "remote_relay_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "remote_relay_ut end." << std::endl;
  }

private:
  static void my_actor(stackful_actor self)
  {
    message msg;
    aid_t last_id;
    self->recv("init", last_id);

    aid_t sender = self.recv(msg);
    if (last_id != aid_nil)
    {
      self.relay(last_id, msg);
    }
    else
    {
      message m("hello");
      self.reply(sender, m);
    }
  }

  static void test_common()
  {
    try
    {
      std::size_t root_num = 1;
      attributes attrs;
      attrs.id_ = atom("router");
      attrs.thread_num_ = 1;
      context ctx(attrs);
      threaded_actor base = spawn(ctx);

      netopt_t opt = make_netopt();
      opt.is_router = 1;
      gce::bind(base, "tcp://127.0.0.1:14924", remote_func_list_t(), opt);
      base.sleep_for(millisecs(100));

      boost::thread_group thrs;
      for (std::size_t i=0; i<root_num; ++i)
      {
        thrs.create_thread(
          boost::bind(
            &remote_relay_ut::root, i, root_num,
            base.get_aid(), spawn(ctx)
            )
          );
      }

      std::vector<aid_t> root_list(root_num);
      for (std::size_t i=0; i<root_num; ++i)
      {
        root_list[i] = base->recv();
      }

      BOOST_FOREACH(aid_t aid, root_list)
      {
        base->send(aid);
      }
      thrs.join_all();
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }

  static void root(
    std::size_t id, std::size_t root_num,
    aid_t base_aid, threaded_actor mix
    )
  {
    try
    {
      gce::log::asio_logger lg;
      attributes attrs;
      attrs.lg_ = boost::bind(&gce::log::asio_logger::output, &lg, _arg1, "");
      attrs.id_ = to_match(id);
      attrs.thread_num_ = 1;
      context ctx(attrs);
      threaded_actor base = spawn(ctx);

      netopt_t opt = make_netopt();
      opt.is_router = 1;
      opt.reconn_period = seconds(1);
      remote_func_list_t func_list;
      func_list.push_back(
        make_remote_func<stackful>(
          "my_actor", 
          boost::bind(&remote_relay_ut::my_actor, _arg1)
          )
        );
      connect(base, "router", "tcp://127.0.0.1:14924", opt, func_list);
      base.sleep_for(millisecs(1000));

      aid_t last_id;
      aid_t first_id;
      for (std::size_t i=0; i<root_num; ++i)
      {
        aid_t aid = spawn_remote(base, "my_actor", i);
        base->send(aid, "init", last_id);
        if (i == 0)
        {
          first_id = aid;
        }
        last_id = aid;
      }

      int i = 0;
      resp_t res = base->request(last_id, "hi", i);
      message msg;
      aid_t sender = base.respond(res, msg);
      BOOST_ASSERT(sender == first_id);
      BOOST_ASSERT(msg.get_type() == atom("hello"));

      mix->send(base_aid);
      mix->recv();
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}

