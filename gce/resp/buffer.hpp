///
/// config.hpp
///

#ifndef RESP_BUFFER_HPP
#define RESP_BUFFER_HPP

#include "config.hpp"
#include <string>
#include <cstring>
#include <cstdlib>

namespace resp
{
/// Can hold ref or deep copy data buffer.
class buffer
{
public:
  /// Default constructor.
  buffer()
    : capacity_(0)
    , data_(0)
    , size_(0)
  {
  }

  /// Hold an ref of char pointer.
  /**
   * @param ptr Pointer of char const.
   */
  buffer(char const* ptr)
    : capacity_(0)
    , data_(const_cast<char*>(ptr))
    , size_(std::strlen(ptr))
  {
  }

  /// Hold an ref of std::string.
  /**
   * @param str String.
   */
  buffer(std::string const& str)
    : capacity_(0)
    , data_(const_cast<char*>(str.data()))
    , size_(str.size())
  {
  }

  /// Hold an ref of char pointer and its size.
  /**
   * @param ptr Pointer of char const.
   * @param size Length of char const array.
   */
  buffer(char const* ptr, size_t size)
    : capacity_(0)
    , data_(const_cast<char*>(ptr))
    , size_(size)
  {
  }

  /// Copy constructor.
  buffer(buffer const& other)
    : capacity_(other.capacity_)
    , data_(0)
    , size_(other.size_)
  {
    if (other.is_ref())
    {
      data_ = other.data_;
    }
    else if (other.is_small())
    {
      data_ = small_;
    }
    else
    {
      /// large
      data_ = (char*)std::malloc(capacity_);
      if (data_ == 0)
      {
        throw std::bad_alloc();
      }
    }

    if (!is_ref())
    {
      std::memcpy(data_, other.data_, size_);
    }
  }

  /// Move src to des, clear src, like C++11 std::move.
  /**
   * @note des first will be reset to ref.
   */
  static void move(buffer& des, buffer& src)
  {
    des.reset();
    if (src.is_ref())
    {
      des.data_ = src.data_;
      des.size_ = src.size_;
    }
    else if (src.is_small())
    {
      des.capacity_ = RESP_SMALL_BUFFER_SIZE;
      des.data_ = des.small_;
      des.size_ = src.size_;
      std::memcpy(des.data_, src.data_, des.size_);
    }
    else
    {
      /// large
      des.capacity_ = src.capacity_;
      des.data_ = src.data_;
      des.size_ = src.size_;
    }

    /// clear src
    src.capacity_ = 0;
    src.data_ = 0;
    src.size_ = 0;
  }

  ~buffer()
  {
    dealloc();
  }

  /// Assignment.
  buffer& operator=(buffer const& rhs)
  {
    if (this != &rhs)
    {
      if (rhs.is_ref())
      {
        dealloc();
        capacity_ = 0;
        data_ = rhs.data_;
      }
      else if (rhs.is_small())
      {
        dealloc();
        capacity_ = RESP_SMALL_BUFFER_SIZE;
        data_ = small_;
      }
      else
      {
        /// rhs is large
        if (rhs.capacity_ > capacity_)
        {
          if (is_ref() || is_small())
          {
            data_ = 0;
          }
          data_ = (char*)std::realloc(data_, rhs.capacity_);
          if (data_ == 0)
          {
            throw std::bad_alloc();
          }
          capacity_ = rhs.capacity_;
        }
      }

      size_ = rhs.size_;
      if (!is_ref())
      {
        std::memcpy(data_, rhs.data_, size_);
      }
    }
    return *this;
  }

public:
  /// Get holding data.
  /**
   * @return Holding data pointer.
   */
  char const* data() const
  {
    return data_;
  }

  /// Get holding data.
  /**
   * @return Holding data pointer.
   */
  char* data()
  {
    return data_;
  }

  /// Get holding data size.
  /**
   * @return Holding data size.
   */
  size_t size() const
  {
    return size_;
  }

  /// Check if empty.
  bool empty() const
  {
    return size_ == 0;
  }

  /// Check if data is ref.
  bool is_ref() const
  {
    return capacity_ == 0;
  }

  /// Check if data is small(no dynamic alloc).
  bool is_small() const
  {
    return capacity_ > 0 && capacity_ <= RESP_SMALL_BUFFER_SIZE;
  }

  /// Check if data is large(dynamic alloc).
  bool is_large() const
  {
    return capacity_ > RESP_SMALL_BUFFER_SIZE;
  }

  bool operator==(char const* rhs) const
  {
    if (std::strlen(rhs) != size_)
    {
      return false;
    }
    return std::memcmp(data_, rhs, size_) == 0;
  }

  bool operator==(std::string const& rhs) const
  {
    if (rhs.size() != size_)
    {
      return false;
    }
    return std::memcmp(data_, rhs.data(), size_) == 0;
  }

  /// Clear data, no dealloc, for reusing.
  void clear()
  {
    size_ = 0;
  }

  /// Reset buffer to ref mode, if large will dealloc.
  void reset()
  {
    dealloc();
    capacity_ = 0;
    size_ = 0;
    data_ = 0;
  }

  /// Reserve memory, may change buffer type(ref, small or large).
  void reserve(size_t capacity)
  {
    if (capacity > capacity_)
    {
      /// Make capacity % RESP_SMALL_BUFFER_SIZE == 0.
      capacity =
        capacity % RESP_SMALL_BUFFER_SIZE == 0 ?
        capacity : RESP_SMALL_BUFFER_SIZE * (capacity / RESP_SMALL_BUFFER_SIZE + 1);

      if (is_ref() || is_small())
      {
        char const* ptr = data_;
        bool is_small_before = is_small();
        if (capacity > RESP_SMALL_BUFFER_SIZE)
        {
          capacity_ = capacity;
          data_ = (char*)std::malloc(capacity_);
          if (data_ == 0)
          {
            throw std::bad_alloc();
          }
        }
        else
        {
          capacity_ = RESP_SMALL_BUFFER_SIZE;
          data_ = small_;
        }

        if (!(is_small_before && is_small()))
        {
          std::memcpy(data_, ptr, size_);
        }
      }
      else
      {
        /// large
        capacity_ = capacity;
        data_ = (char*)std::realloc(data_, capacity_);
      }
    }
  }

  /// Resize size.
  void resize(size_t size)
  {
    reserve(size);
    size_ = size;
  }

  /// Append other buffer's data, if self is ref, then change to small or large.
  void append(buffer const& other)
  {
    reserve(size_ + other.size_);
    std::memcpy(data_ + size_, other.data_, other.size_);
    size_ += other.size_;
  }

  void append(char c)
  {
    reserve(size_ + 1);
    data_[size_] = c;
    size_ += 1;
  }

  void append(char const* str)
  {
    size_t len = std::strlen(str);
    append(str, len);
  }

  void append(char const* str, size_t size)
  {
    reserve(size_ + size);
    std::memcpy(data_ + size_, str, size);
    size_ += size;
  }

  void append(std::string const& str)
  {
    size_t len = str.size();
    reserve(size_ + len);
    std::memcpy(data_ + size_, str.data(), len);
    size_ += len;
  }

private:
  void dealloc()
  {
    if (is_large() && data_ != 0)
    {
      std::free(data_);
      data_ = 0;
    }
  }

private:
  size_t capacity_;
  char small_[RESP_SMALL_BUFFER_SIZE];
  char* data_;
  size_t size_;
};
}

#endif // RESP_BUFFER_HPP
