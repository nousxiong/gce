///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/detail/object_pool.hpp>
#include <gce/detail/unique_ptr.hpp>
#include <string>

namespace gce
{
class object_pool_ut
{
  struct my_base
  {
    my_base(std::size_t index, std::string const& name)
      : index_(index)
      , name_(name)
    {
    }

    virtual ~my_base()
    {
    }

    void clear()
    {
      index_ = 0;
      name_.clear();
    }

    std::size_t index_;
    std::string name_;
  };

  struct my_data
    : public detail::object_pool<my_data, std::string>::object
    , public my_base
  {
    explicit my_data(std::string const& str)
      : my_base(0, str)
      , str_(str)
      , i_(0)
      , l_(0)
    {
    }

    ~my_data()
    {
    }

    void on_free()
    {
      my_base::clear();
      str_.clear();
      i_ = 0;
      l_ = 0;
    }

    void print()
    {
      //std::cout << "my_base[ index_: " << index_ << ", name_: " << name_ << " ]" << std::endl;
      //std::cout << "basic_object[ ]" << std::endl;
      //std::cout << "my_data[ str_: " << str_ << ", i_: " << i_ << ", l_: " << l_ << std::endl;
    }

    std::string str_;
    int i_;
    long l_;
  };

public:
  static void run()
  {
    std::cout << "object_pool_ut begin." << std::endl;
    test_freelist();
    std::cout << "object_pool_ut end." << std::endl;
  }

private:
  static void test_freelist()
  {
    typedef detail::object_pool<my_data, std::string> op_t;
    detail::unique_ptr<op_t> op(
      GCE_CACHE_ALIGNED_NEW(op_t)(
        (detail::cache_pool*)0, std::string(), size_nil
        ),
      detail::cache_aligned_deleter()
      );

    //boost::timer::auto_cpu_timer t;
    run_test(op.get(), true);
  }

  template <typename Op>
  static void run_test(Op* op, bool st)
  {
    for (std::size_t i=0; i<100; ++i)
    {
      {
        my_data* md = op->get();
        md->str_ = "md";
        md->index_ = 1;
        //md->print();
        op->free(md);
        if (st)
        {
          BOOST_ASSERT(md->str_.empty());
          BOOST_ASSERT(md->name_.empty());
          BOOST_ASSERT(md->index_ == 0);
        }
      }

      for (std::size_t j=0; j<200; ++j)
      {
        std::vector<my_data*> my_data_list(1000, (my_data*)0);
        for (std::size_t i=0; i<my_data_list.size(); ++i)
        {
          my_data* md = op->get();
          md->str_ = "md_list";
          md->index_ = i;
          BOOST_ASSERT(md->str_ == "md_list");
          BOOST_ASSERT(md->name_.empty());
          BOOST_ASSERT(md->index_ == i);
          //md->print();
          my_data_list[i] = md;
        }

        for (std::size_t i=0; i<my_data_list.size(); ++i)
        {
          my_data* md = my_data_list[i];
          //md->print();
          op->free(md);
          BOOST_ASSERT(md->str_.empty());
          BOOST_ASSERT(md->name_.empty());
          BOOST_ASSERT(md->index_ == 0);
        }
        my_data_list.clear();
      }
    }
  }
};
}

