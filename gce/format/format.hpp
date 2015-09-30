/*
 Formatting library for C++ extension: format to given string, base on Victor Zverovich's cppformat(1.1.0)

 By Nous Xiong (348944179@qq.com)
 */

/*
 Formatting library for C++

 Copyright (c) 2012 - 2015, Victor Zverovich
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FMT_FORMAT_HPP
#define FMT_FORMAT_HPP

#ifndef FMT_HEADER_ONLY
# define FMT_HEADER_ONLY
#endif
#include "format.h"

namespace fmt
{
namespace internal
{
// A memory buffer for POD types
template <typename StringType>
class StringBuffer : public Buffer<typename StringType::value_type> {
private:
  typedef typename StringType::value_type char_t;
  StringType* str_;

  static char_t* init_string(StringType& str)
  {
    str.resize(str.capacity());
    return const_cast<char_t*>(str.data());
  }

protected:
  void grow(std::size_t size)
  {
    std::size_t new_capacity =
      (std::max)(size, this->capacity_ + this->capacity_ / 2);
    str_->resize(new_capacity);
    this->capacity_ = new_capacity;
    this->ptr_ = const_cast<char_t*>(str_->data());
  }

public:
  explicit StringBuffer(StringType& str)
    : Buffer<char_t>(init_string(str), str.capacity())
    , str_(&str)
  {
  }

  ~StringBuffer()
  {
  }

#if FMT_USE_RVALUE_REFERENCES
 private:
  // Move data from other to this buffer.
  void move(StringBuffer &other)
  {
    this->str_ = other.str_;
    this->size_ = other.size_;
    this->capacity_ = other.capacity_;
    this->ptr_ = other.ptr_;
  }

 public:
  StringBuffer(StringBuffer &&other)
  {
    move(other);
  }

  StringBuffer &operator=(StringBuffer &&other)
  {
    assert(this != &other);
    free();
    move(other);
    return *this;
  }
#endif
};
}

template <typename StringType>
class BasicStringWriter : public BasicWriter<typename StringType::value_type>
{
  typedef typename StringType::value_type char_t;
  internal::StringBuffer<StringType> buffer_;

public:
  explicit BasicStringWriter(StringType& str)
    : BasicWriter<char_t>(buffer_), buffer_(str) {}

#if FMT_USE_RVALUE_REFERENCES
  /**
    \rst
    Constructs a :class:`fmt::BasicMemoryWriter` object moving the content
    of the other object to it.
    \endrst
   */
  BasicStringWriter(BasicStringWriter &&other)
    : BasicWriter<char_t>(buffer_), buffer_(std::move(other.buffer_))
  {
  }

  /**
    \rst
    Moves the content of the other ``BasicMemoryWriter`` object to this one.
    \endrst
   */
  BasicStringWriter &operator=(BasicStringWriter &&other)
  {
    buffer_ = std::move(other.buffer_);
    return *this;
  }
#endif
};

typedef BasicStringWriter<std::string> StringWriter;
typedef BasicStringWriter<std::wstring> WStringWriter;

/**
  \rst
  Formats arguments and returns the result as a string.

  **Example**::

    std::string str;
    std::string const& message = format(str, "The answer is {}", 42);
    assert(&str == &message);
    assert(str == "The answer is 42");
    assert(str == message);
  \endrst
*/
inline std::string const& sformat(std::string& str, StringRef format_str, ArgList args)
{
  StringWriter w(str);
  w.write(format_str, args);
  str.resize(w.size());
  return str;
}

inline std::wstring const& sformat(std::wstring& str, WStringRef format_str, ArgList args)
{
  WStringWriter w(str);
  w.write(format_str, args);
  str.resize(w.size());
  return str;
}
}

namespace fmt 
{
FMT_VARIADIC(std::string const&, sformat, std::string&, StringRef)
FMT_VARIADIC(std::wstring const&, sformat, std::wstring&, WStringRef)
}

#endif /// FMT_FORMAT_HPP