#ifndef GCE_DETAIL_CACHE_ALIGNED_PTR_HPP
#define GCE_DETAIL_CACHE_ALIGNED_PTR_HPP

#include <gce/config.hpp>
#include <gce/detail/unique_ptr.hpp>
#include <utility>

namespace gce
{
namespace detail
{
template <typename T, typename Ptr>
class cache_aligned_ptr
{
public:
  typedef T value_type;
  typedef value_type* pointer;
  typedef value_type& reference;
  typedef Ptr pointer_t;

public:
  cache_aligned_ptr()
  {
  }

  explicit cache_aligned_ptr(pointer p)
    : ptr_(p)
  {
  }

  template <class D>
  cache_aligned_ptr(pointer p, D d)
    : ptr_(p, d)
  {
  }

  ~cache_aligned_ptr()
  {
  }

  cache_aligned_ptr(cache_aligned_ptr const& other)
    : ptr_(other.ptr_)
  {
  }

  cache_aligned_ptr& operator=(cache_aligned_ptr const& rhs)
  {
    if (this != &rhs)
    {
      ptr_ = rhs.ptr_;
    }
    return *this;
  }

public:
  void reset()
  {
    ptr_.reset();
  }

  void reset(pointer p)
  {
    ptr_.reset(p);
  }

  template <class D>
  void reset(pointer p, D d)
  {
    ptr_.reset(p, d);
  }

  reference operator*() const
  {
    return *ptr_;
  }

  pointer operator->() const
  {
    return ptr_.get();
  }

  T* get() const
  {
    return ptr_.get();
  }

  operator bool() const
  {
    return ptr_.get() != 0;
  }

  bool operator!() const
  {
    return ptr_.get() == 0;
  }

  void swap(cache_aligned_ptr& b)
  {
    ptr_.swap(b.ptr_);
  }

private:
  pointer_t ptr_;
  byte_t pad_[GCE_CACHE_LINE_SIZE - sizeof(pointer_t)];
};

template <typename T>
class cache_aligned_ptr<T, T*>
{
public:
  typedef T value_type;
  typedef value_type* pointer;
  typedef value_type& reference;
  typedef pointer pointer_t;

public:
  cache_aligned_ptr()
    : ptr_(0)
  {
  }

  explicit cache_aligned_ptr(pointer p)
    : ptr_(p)
  {
  }

  ~cache_aligned_ptr()
  {
  }

  cache_aligned_ptr(cache_aligned_ptr const& other)
    : ptr_(other.ptr_)
  {
  }

  cache_aligned_ptr& operator=(cache_aligned_ptr const& rhs)
  {
    if (this != &rhs)
    {
      ptr_ = rhs.ptr_;
    }
    return *this;
  }

  cache_aligned_ptr& operator=(pointer rhs)
  {
    ptr_ = rhs;
    return *this;
  }

public:
  reference operator*() const
  {
    BOOST_ASSERT(ptr_ != 0);
    return *ptr_;
  }

  pointer operator->() const
  {
    BOOST_ASSERT(ptr_ != 0);
    return ptr_;
  }

  pointer get() const
  {
    return ptr_;
  }

  operator bool() const
  {
    return ptr_ != 0;
  }

  bool operator!() const
  {
    return ptr_ == 0;
  }

  void swap(cache_aligned_ptr& b)
  {
    std::swap(ptr_, b.ptr_);
  }

private:
  pointer_t ptr_;
  byte_t pad_[GCE_CACHE_LINE_SIZE - sizeof(pointer_t)];
};

template <typename T>
class cache_aligned_ptr<T, unique_ptr<T> >
{
  cache_aligned_ptr(cache_aligned_ptr const&);
  cache_aligned_ptr& operator=(cache_aligned_ptr const&);

  void operator==(cache_aligned_ptr const&) const;
  void operator!=(cache_aligned_ptr const&) const;

public:
  typedef T value_type;
  typedef value_type* pointer;
  typedef value_type& reference;
  typedef unique_ptr<T> pointer_t;

public:
  cache_aligned_ptr()
  {
  }

  explicit cache_aligned_ptr(pointer p)
    : ptr_(p)
  {
  }

  template <class D>
  cache_aligned_ptr(pointer p, D d)
    : ptr_(p, d)
  {
  }

  ~cache_aligned_ptr()
  {
  }

public:
  void reset()
  {
    ptr_.reset();
  }

  void reset(pointer p)
  {
    ptr_.reset(p);
  }

  template <class D>
  void reset(pointer p, D d)
  {
    ptr_.reset(p, d);
  }

  reference operator*() const
  {
    return *ptr_;
  }

  pointer operator->() const
  {
    return ptr_.get();
  }

  T* get() const
  {
    return ptr_.get();
  }

  operator bool() const
  {
    return ptr_.get() != 0;
  }

  bool operator!() const
  {
    return ptr_.get() == 0;
  }

  void swap(cache_aligned_ptr& b)
  {
    ptr_.swap(b.ptr_);
  }

private:
  pointer_t ptr_;
  byte_t pad_[GCE_CACHE_LINE_SIZE - sizeof(pointer_t)];
};
}
}

#endif /// GCE_DETAIL_CACHE_ALIGNED_PTR_HPP
