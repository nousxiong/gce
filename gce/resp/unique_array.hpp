///
/// unique_array.hpp
///

#ifndef RESP_DEQUE_HPP
#define RESP_DEQUE_HPP

#include "config.hpp"
#include <vector>
#include <cassert>

namespace resp
{
/// Unique array, copy means move(no copy).
/**
 * @note Under C++98/03 there is no move, so just use copy(consturctor/assignment) to instead;
 *  after C++11 could use move directly.
 */
template <typename T>
class unique_array
{
  class node
  {
  public:
    node()
      : chunk_((T*)std::malloc(sizeof(T)*RESP_UNIQUE_ARRAY_NODE_CAPACITY))
      , size_(0)
    {
      if (chunk_ == 0)
      {
        throw std::bad_alloc();
      }
    }

    ~node()
    {
      for (size_t i=0; i<size_; ++i)
      {
        chunk_[i].~T();
      }

      if (chunk_ != 0)
      {
        std::free(chunk_);
        chunk_ = 0;
      }
      size_ = 0;
    }

    T& operator[](size_t i)
    {
      assert(i < size_);
      return chunk_[i];
    }

    void push_back(T const& t)
    {
      assert(!full());
      T& back = emplace_back();
      back = t;
    }

    void pop_back()
    {
      assert(size_ > 0);
      chunk_[--size_].~T();
    }

    T& emplace_back()
    {
      assert(!full());
      void* mem = &(chunk_[size_++]);
      return *(new (mem) T);
    }

    void clear()
    {
      for (size_t i=0; i<size_; ++i)
      {
        chunk_[i].~T();
      }
      size_ = 0;
    }

    size_t size() const
    {
      return size_;
    }

    bool full() const
    {
      return size_ == RESP_UNIQUE_ARRAY_NODE_CAPACITY;
    }

  private:
    T* chunk_;
    size_t size_;
  };

  typedef node* node_ptr;

public:
  unique_array()
    : node_size_(0)
    , size_(0)
  {
  }

  explicit unique_array(size_t capacity)
    : node_size_(0)
    , size_(0)
  {
    reserve(capacity);
  }

  unique_array(unique_array const& other)
    : index_(other.index_)
    , node_size_(other.node_size_)
    , size_(other.size_)
  {
    unique_array* src = const_cast<unique_array*>(&other);
    src->index_.clear();
    src->node_size_ = 0;
    src->size_ = 0;
  }

  unique_array& operator=(unique_array const& rhs)
  {
    if (this != &rhs)
    {
      unique_array* src = const_cast<unique_array*>(&rhs);
      destroy();

      index_ = src->index_;
      node_size_ = src->node_size_;
      size_ = src->size_;

      src->index_.clear();
      src->node_size_ = 0;
      src->size_ = 0;
    }
    return *this;
  }

  ~unique_array()
  {
    destroy();
  }

public:
  size_t size() const
  {
    return size_;
  }

  T& operator[](size_t i)
  {
    assert(i < size_);
    size_t node_index = i / RESP_UNIQUE_ARRAY_NODE_CAPACITY;
    assert(node_index < node_size_);
    size_t sub_index = i % RESP_UNIQUE_ARRAY_NODE_CAPACITY;
    return (*index_[node_index])[sub_index];
  }

  T const& operator[](size_t i) const
  {
    assert(i < size_);
    size_t node_index = i / RESP_UNIQUE_ARRAY_NODE_CAPACITY;
    assert(node_index < node_size_);
    size_t sub_index = i % RESP_UNIQUE_ARRAY_NODE_CAPACITY;
    return (*index_[node_index])[sub_index];
  }

  void reserve(size_t capacity)
  {
    size_t const node_capacity = RESP_UNIQUE_ARRAY_NODE_CAPACITY;
    size_t curr_capacity = node_size_ * node_capacity;
    if (capacity > curr_capacity)
    {
      size_t incr_size = capacity - curr_capacity;
      size_t base_size = incr_size / node_capacity;
      size_t incr_node_size = incr_size % node_capacity == 0 ? base_size : base_size + 1;
      grow(incr_node_size);
    }
  }

  void push_back(T const& t)
  {
    T& back = emplace_back();
    back = t;
  }

  T& emplace_back()
  {
    reserve(size_ + 1);
    size_t curr_node = node_size_ == 0 ? 0 : node_size_ - 1;
    node_ptr n = index_[curr_node];
    assert(!n->full());
    T& t = n->emplace_back();
    ++size_;
    node_size_ = size_ / RESP_UNIQUE_ARRAY_NODE_CAPACITY + 1;
    return t;
  }

  void clear()
  {
    for (size_t i=0; i<node_size_; ++i)
    {
      index_[i]->clear();
    }
  }

private:
  void destroy()
  {
    for (size_t i=0; i<index_.size(); ++i)
    {
      delete index_[i];
    }

    index_.clear();

    node_size_ = 0;
    size_ = 0;
  }

  node_ptr grow(size_t size)
  {
    size_t old_node_size = index_.size();
    index_.resize(old_node_size + size);

    node_ptr first = 0;
    for (size_t i=0; i<size; ++i)
    {
      node_ptr n = new node;
      index_[old_node_size + i] = n;
      if (i == 0)
      {
        first = n;
      }
    }
    return first;
  }

private:
  std::vector<node_ptr> index_;
  size_t node_size_;
  size_t size_;
};
}

#endif /// RESP_DEQUE_HPP
