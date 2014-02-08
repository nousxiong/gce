#include <gce/actor/actor.hpp>
#include <gce/actor/thin.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/spawn.hpp>
#include <boost/atomic.hpp>

namespace gce
{
static boost::atomic_size_t actor_count(0);
class actor_ut
{
public:
  static void run()
  {
    for (std::size_t i=0; i<2; ++i)
    {
      test_common();
    }
  }

private:
  static void my_child(self_t self)
  {
    ++actor_count;
    message msg;
    aid_t aid = self.recv(msg);
    self.recv(msg, match(seconds_t(3)));
    self.reply(aid, msg);
  }

  static void my_actor(self_t self, aid_t base_id)
  {
    std::size_t size = 5;
    std::vector<response_t> res_list(size);
    message m;
    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid =
        spawn<stackful>(
          self,
          boost::bind(&actor_ut::my_child, _1)
          );
      res_list[i] = self.request(aid, m);
    }

    message msg;
    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid;
      do
      {
        aid = self.recv(res_list[i], msg, seconds_t(1));
      }
      while (!aid);
    }

    self.send(base_id, message(2));
  }

  static void my_thr(context& ctx, aid_t base_id)
  {
    mixin& mix = spawn(ctx);
    for (std::size_t i=0; i<2; ++i)
    {
      spawn<stackful>(mix, boost::bind(&actor_ut::my_actor, _1, base_id));
    }
  }

  static void test_common()
  {
    try
    {
      std::size_t free_actor_num = 5;
      std::size_t user_thr_num = 5;
      std::size_t my_actor_size = free_actor_num + user_thr_num * 2;
      attributes attrs;
      attrs.mixin_num_ = user_thr_num + 1;
      context ctx(attrs);

      mixin& base = spawn(ctx);
      aid_t base_id = base.get_aid();
      for (std::size_t i=0; i<free_actor_num; ++i)
      {
        spawn<stackful>(
          base,
          boost::bind(
            &actor_ut::my_actor, _1,
            base_id
            )
          );
      }

      boost::thread_group thrs;
      for (std::size_t i=0; i<user_thr_num; ++i)
      {
        thrs.create_thread(
          boost::bind(
            &actor_ut::my_thr,
            boost::ref(ctx), base_id
            )
          );
      }

      thrs.join_all();

      message m;
      for (std::size_t i=0; i<my_actor_size; ++i)
      {
        base.recv(m);
      }

      std::cout << std::endl;
      std::cout << "actor_count: " << actor_count << std::endl;
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }


};
}
