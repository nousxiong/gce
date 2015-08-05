///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/detail/linked_queue.hpp>
#include <gce/detail/linked_pool.hpp>
#include <gce/detail/object_pool.hpp>

namespace gce
{
class linked_ut
{
public:
  static void run()
  {
    std::cout << "linked_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_queue();
      test_pool();
      test_pool_attach();
      test_object_pool();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "linked_ut end." << std::endl;
  }

private:
  struct elem
    : public detail::linked_elem<elem>
  {
    void on_free() {}

    aid_t aid_;
  };

  static void test_queue()
  {
    try
    {
      std::vector<elem> elem_list(100);
      detail::linked_queue<elem> que;

      for (size_t i=0, size=elem_list.size(); i<size; ++i)
      {
        que.push(&elem_list[i]);
      }

      for (size_t i=0, size=elem_list.size(); i<size; ++i)
      {
        elem* e = que.pop();
        GCE_VERIFY(e == &elem_list[i])(i); 
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl; 
    }
  }

  static void test_pool()
  {
    try
    {
      std::vector<elem*> elem_list;
      elem_list.reserve(100);
      detail::linked_pool<elem> pool;

      for (size_t i=0; i<100; ++i)
      {
        elem_list.push_back(pool.get());
      }

      GCE_VERIFY(pool.size() == 0)(pool.size());

      for (size_t i=0; i<100; ++i)
      {
        pool.free(elem_list[i]);
      }

      GCE_VERIFY(pool.size() == 100)(pool.size());
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl; 
    }
  }

  static void test_pool_attach()
  {
    try
    {
      typedef detail::linked_pool<elem>::line line_t;
      detail::linked_pool<elem> pool1(50);
      detail::linked_pool<elem> pool2(50, 50);

      line_t l1 = pool2.detach(20);
      GCE_VERIFY(pool2.size() == 50 - 20)(pool2.size());
      pool1.attach(l1);
      GCE_VERIFY(pool1.size() == 50 + 20)(pool1.size());

      line_t l2 = pool2.detach(40);
      GCE_VERIFY(pool2.size() == 0)(pool2.size());
      pool1.attach(l2);
      GCE_VERIFY(pool1.size() == 50 + 20 + 40)(pool1.size());

      line_t l3 = pool1.detach(60);
      GCE_VERIFY(pool1.size() == 50 + 20 + 40 - 60)(pool1.size());
      pool2.attach(l3);
      GCE_VERIFY(pool2.size() == 50)(pool2.size());
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl; 
    }
  }

  static void test_object_pool()
  {
    try
    {
      std::vector<elem*> elem_list;
      elem_list.reserve(100);
      detail::object_pool<elem> pool;

      for (size_t i=0; i<100; ++i)
      {
        elem_list.push_back(pool.get());
      }

      GCE_VERIFY(pool.size() == 28)(pool.size());

      for (size_t i=0; i<100; ++i)
      {
        pool.free(elem_list[i]);
      }

      GCE_VERIFY(pool.size() == 128)(pool.size());
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl; 
    }
  }
};
}

