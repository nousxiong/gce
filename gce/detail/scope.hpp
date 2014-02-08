#ifndef GCE_DETAIL_SCOPE_HPP
#define GCE_DETAIL_SCOPE_HPP

#include <gce/config.hpp>
#include <boost/function.hpp>

namespace gce
{
namespace detail
{
class scope
{
  scope(scope const&);
  scope& operator=(scope const&);

  void operator==(scope const&) const;
  void operator!=(scope const&) const;

public:
  typedef boost::function<void ()> cb_t;

public:
  scope()
  {
  }

  template <typename F>
  explicit scope(F f)
    : cb_(f)
  {
  }

  template <typename F, typename A>
  explicit scope(F f, A a)
    : cb_(f, a)
  {
  }

  ~scope()
  {
    if (cb_)
    {
      cb_();
    }
  }

public:
  void reset()
  {
    cb_.clear();
  }

  void reset(cb_t const& cb)
  {
    cb_ = cb;
  }

  operator bool() const
  {
    return cb_ != 0;
  }

  bool operator!() const
  {
    return cb_ == 0;
  }

private:
  cb_t cb_;
};
}
}

#endif /// GCE_DETAIL_SCOPE_HPP
