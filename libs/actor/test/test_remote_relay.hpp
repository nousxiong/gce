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
    test_common();
    std::cout << "remote_relay_ut end." << std::endl;
  }

private:
  static void my_actor(actor<stacked>& self)
  {
    message msg;
    aid_t last_id;
    recv(self, atom("init"), last_id);

    aid_t sender = self.recv(msg);
    if (last_id)
    {
      self.relay(last_id, msg);
    }
    else
    {
      message m(atom("hello"));
      self.reply(sender, m);
    }
  }

  static void test_common()
  {
    try
    {
      std::size_t root_num = 10;
      attributes attrs;
      attrs.id_ = atom("router");
      attrs.thread_num_ = 1;
      context ctx(attrs);

      gce::bind(ctx, "tcp://127.0.0.1:14924", true);

      boost::thread_group thrs;
      for (std::size_t i=0; i<root_num; ++i)
      {
        thrs.create_thread(
          boost::bind(
            &remote_relay_ut::root, i, root_num,
            ctx.get_aid(), spawn(ctx)
            )
          );
      }

      std::vector<aid_t> root_list(root_num);
      for (std::size_t i=0; i<root_num; ++i)
      {
        root_list[i] = recv(ctx);
      }

      BOOST_FOREACH(aid_t aid, root_list)
      {
        send(ctx, aid);
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
    aid_t base_aid, actor<threaded> mix
    )
  {
    try
    {
      attributes attrs;
      attrs.id_ = id;
      attrs.thread_num_ = 1;
      context ctx(attrs);

      net_option opt;
      opt.reconn_period_ = seconds_t(1);
      remote_func_list_t func_list;
      func_list.push_back(
        std::make_pair(
          atom("my_actor"),
          actor_func_t(boost::bind(&remote_relay_ut::my_actor, _1))
          )
        );
      connect(ctx, atom("router"), "tcp://127.0.0.1:14924", true, opt, func_list);
      wait(ctx, boost::chrono::milliseconds(1000));

      aid_t last_id;
      aid_t first_id;
      for (std::size_t i=0; i<root_num; ++i)
      {
        aid_t aid = spawn(ctx, atom("my_actor"), i);
        send(ctx, aid, atom("init"), last_id);
        if (i == 0)
        {
          first_id = aid;
        }
        last_id = aid;
      }

      int i = 0;
      response_t res = request(ctx, last_id, atom("hi"), i);
      message msg;
      aid_t sender = ctx.recv(res, msg);
      BOOST_ASSERT(sender == first_id);
      BOOST_ASSERT(msg.get_type() == atom("hello"));

      send(mix, base_aid);
      recv(mix);
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}

