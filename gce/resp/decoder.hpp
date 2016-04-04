///
/// decoder.hpp
///

#ifndef RESP_DECODER_HPP
#define RESP_DECODER_HPP

#include "config.hpp"
#include "result.hpp"
#include <stack>
#include <vector>
#include <utility>
#include <cassert>
#include <algorithm>

namespace resp
{
static const char reply_string = '+';
static const char reply_error = '-';
static const char reply_integer = ':';
static const char reply_bulk = '$';
static const char reply_array = '*';

/// Decoder of RESP.
class decoder
{
  enum state
  {
    st_start = 0,

    st_string = 1,
    st_string_lf = 2,

    st_error_string = 3,
    st_error_lf = 4,

    st_integer = 5,
    st_integer_lf = 6,

    st_bulk_size = 7,
    st_bulk_size_lf = 8,
    st_bulk = 9,
    st_bulk_cr = 10,
    st_bulk_lf = 11,

    st_array_size = 12,
    st_array_size_lf = 13,
  };

public:
  decoder()
    : stat_(st_start)
    , bulk_size_(0)
  {
  }

  ~decoder()
  {
  }

public:
  result decode(char const* ptr, size_t size)
  {
    if (!array_stack_.empty())
    {
      return decode_array(ptr, size);
    }
    else
    {
      return decode_chunk(ptr, size);
    }
  }

private:
  static bool ischar(int c)
  {
      return c >= 0 && c <= 127;
  }

  static bool isctrl(int c)
  {
      return (c >= 0 && c <= 31) || (c == 127);
  }

  result decode_array(char const* ptr, size_t size)
  {
    assert( !array_stack_.empty() );
    assert( !value_stack_.empty() );

    long int arraySize = array_stack_.top();
    unique_array<unique_value> arrayValue = value_stack_.top().array();

    array_stack_.pop();
    value_stack_.pop();

    size_t i = 0;

    if( array_stack_.empty() == false  )
    {
//        std::pair<size_t, RedisParser::ParseResult>  pair = parseArray(ptr, size);
        result res = decode_array(ptr, size);

        if( res != completed )
        {
            value_stack_.push(arrayValue);
            array_stack_.push(arraySize);

            return res;
        }
        else
        {
//            arrayValue.push_back( value_stack_.top() );
//            value_stack_.pop();
            arrayValue.push_back(res.value());
            --arraySize;
        }

        i += res.size();
    }

    if( i == size )
    {
        value_stack_.push(arrayValue);

        if( arraySize == 0 )
        {
//            return std::make_pair(i, completed);
          return result(completed, i, get_result());
        }
        else
        {
            array_stack_.push(arraySize);
//            return std::make_pair(i, incompleted);
          return result(incompleted, i);
        }
    }

    long int x = 0;

    for(; x < arraySize; ++x)
    {
//      std::pair<size_t, RedisParser::ParseResult>  pair = parse(ptr + i, size - i);
      result res = decode(ptr + i, size - i);

      i += res.size();

      if( res == error )
      {
//        return std::make_pair(i, error);
        return result(error, i);
      }
      else if( res == incompleted )
      {
        arraySize -= x;
        value_stack_.push(arrayValue);
        array_stack_.push(arraySize);

//        return std::make_pair(i, incompleted);
        return result(incompleted, i);
      }
      else
      {
//        assert( value_stack_.empty() == false );
//        arrayValue.push_back( value_stack_.top() );
//        value_stack_.pop();
        arrayValue.push_back(res.value());
      }
    }

    assert( x == arraySize );

    value_stack_.push(arrayValue);
//    return std::make_pair(i, completed);
    return result(completed, i, get_result());
  }


  result decode_chunk(char const* ptr, size_t size)
  {
    size_t i = 0;

    for(; i<size; ++i)
    {
      char c = ptr[i];

      switch(stat_)
      {
      case st_start:
        buf_.clear();
        switch(c)
        {
        case reply_string:
          stat_ = st_string;
          break;
        case reply_error:
          stat_ = st_error_string;
          break;
        case reply_integer:
          stat_ = st_integer;
          break;
        case reply_bulk:
          stat_ = st_bulk_size;
          bulk_size_ = 0;
          break;
        case reply_array:
          stat_ = st_array_size;
          break;
        default:
          stat_ = st_start;
//          return std::make_pair(i + 1, error);
          return result(error, i + 1);
        }
        break;
      case st_string:
        if( c == '\r' )
        {
          stat_ = st_string_lf;
        }
        else if( ischar(c) && !isctrl(c) )
        {
//          buf_.push_back(c);
          buf_.append(c);
        }
        else
        {
          stat_ = st_start;
//          return std::make_pair(i + 1, error);
          return result(error, i + 1);
        }
        break;
      case st_error_string:
        if( c == '\r' )
        {
          stat_ = st_error_lf;
        }
        else if( ischar(c) && !isctrl(c) )
        {
//          buf_.push_back(c);
          buf_.append(c);
        }
        else
        {
          stat_ = st_start;
//          return std::make_pair(i + 1, error);
          return result(error, i + 1);
        }
        break;
      case st_bulk_size:
        if( c == '\r' )
        {
          if( buf_.empty() )
          {
            stat_ = st_start;
//            return std::make_pair(i + 1, error);
            return result(error, i + 1);
          }
          else
          {
            stat_ = st_bulk_size_lf;
          }
        }
        else if( isdigit(c) || c == '-' )
        {
//          buf_.push_back(c);
          buf_.append(c);
        }
        else
        {
          stat_ = st_start;
//          return std::make_pair(i + 1, error);
          return result(error, i + 1);
        }
        break;
      case st_string_lf:
        if( c == '\n')
        {
          stat_ = st_start;
//          value_stack_.push(buf_);
          value_stack_.push(unique_value(buf_.data(), buf_.size(), ty_string));
//          return std::make_pair(i + 1, completed);
          return result(completed, i + 1, get_result());
        }
        else
        {
          stat_ = st_start;
//          return std::make_pair(i + 1, error);
          return result(error, i + 1);
        }
        break;
      case st_error_lf:
        if( c == '\n')
        {
          stat_ = st_start;
//          RedisValue::ErrorTag tag;
//          value_stack_.push(RedisValue(buf_, tag));
          value_stack_.push(unique_value(buf_.data(), buf_.size(), ty_error));
//          return std::make_pair(i + 1, completed);
          return result(completed, i + 1, get_result());
        }
        else
        {
          stat_ = st_start;
//          return std::make_pair(i + 1, error);
          return result(error, i + 1);
        }
        break;
      case st_bulk_size_lf:
        if( c == '\n' )
        {
          // optimize me
//          std::string tmp(buf_.begin(), buf_.end());
//          std::string tmp(buf_.data(), buf_.size());
          assert(buf_.size() < 32);
          char numstr[32];
          std::memcpy(numstr, buf_.data(), buf_.size());
          numstr[buf_.size()] = '\n';
          bulk_size_ = std::strtol(numstr, 0, 10);
          buf_.clear();

          if( bulk_size_ == -1 )
          {
            stat_ = st_start;
//            value_stack_.push(RedisValue());  // Nil
            value_stack_.push(unique_value()); // null
//            return std::make_pair(i + 1, completed);
            return result(completed, i + 1, get_result());
          }
          else if( bulk_size_ == 0 )
          {
            stat_ = st_bulk_cr;
          }
          else if( bulk_size_ < 0 )
          {
            stat_ = st_start;
//            return std::make_pair(i + 1, error);
            return result(error, i + 1);
          }
          else
          {
            buf_.reserve(bulk_size_);

            long int available = (long int)(size - i - 1);
            long int canRead = (std::min)(bulk_size_, available);

            if( canRead > 0 )
            {
//              buf_.assign(ptr + i + 1, ptr + i + canRead + 1);
              buf_.append(ptr + i + 1, canRead);
            }

            i += canRead;

            if( bulk_size_ > available )
            {
              bulk_size_ -= canRead;
              stat_ = st_bulk;
//              return std::make_pair(i + 1, incompleted);
              return result(incompleted, i + 1);
            }
            else
            {
              stat_ = st_bulk_cr;
            }
          }
        }
        else
        {
          stat_ = st_start;
//          return std::make_pair(i + 1, error);
          return result(error, i + 1);
        }
        break;
      case st_bulk:
      {
        assert( bulk_size_ > 0 );

        long int available = (long int)(size - i);
        long int canRead = (std::min)(available, bulk_size_);

//        buf_.insert(buf_.end(), ptr + i, ptr + canRead);
        buf_.append(ptr + i, canRead - i);
        bulk_size_ -= canRead;
        i += canRead - 1;

        if( bulk_size_ > 0 )
        {
//          return std::make_pair(i + 1, incompleted);
          return result(incompleted, i + 1);
        }
        else
        {
          stat_ = st_bulk_cr;

          if( size == i + 1 )
          {
//            return std::make_pair(i + 1, incompleted);
            return result(incompleted, i + 1);
          }
        }
        break;
      }
      case st_bulk_cr:
        if( c == '\r')
        {
          stat_ = st_bulk_lf;
        }
        else
        {
          stat_ = st_start;
//          return std::make_pair(i + 1, error);
          return result(error, i + 1);
        }
        break;
      case st_bulk_lf:
        if( c == '\n')
        {
          stat_ = st_start;
//          value_stack_.push(buf_);
          value_stack_.push(unique_value(buf_.data(), buf_.size(), ty_bulkstr));
//          return std::make_pair(i + 1, completed);
          return result(completed, i + 1, get_result());
        }
        else
        {
          stat_ = st_start;
//          return std::make_pair(i + 1, error);
          return result(error, i + 1);
        }
        break;
      case st_array_size:
        if( c == '\r' )
        {
          if( buf_.empty() )
          {
            stat_ = st_start;
//            return std::make_pair(i + 1, error);
            return result(error, i + 1);
          }
          else
          {
            stat_ = st_array_size_lf;
          }
        }
        else if( isdigit(c) || c == '-' )
        {
//          buf_.push_back(c);
          buf_.append(c);
        }
        else
        {
          stat_ = st_start;
//          return std::make_pair(i + 1, error);
          return result(error, i + 1);
        }
        break;
      case st_array_size_lf:
        if( c == '\n' )
        {
          // optimize me
//          std::string tmp(buf_.begin(), buf_.end());
//          std::string tmp(buf_.data(), buf_.size());
          assert(buf_.size() < 32);
          char numstr[32];
          std::memcpy(numstr, buf_.data(), buf_.size());
          numstr[buf_.size()] = '\n';
          long int arraySize = std::strtol(numstr, 0, 10);
          buf_.clear();
//          std::vector<RedisValue> array;
          unique_array<unique_value> array;

          if( arraySize == -1 || arraySize == 0)
          {
            stat_ = st_start;
            value_stack_.push(array);  // Empty array
//            return std::make_pair(i + 1, completed);
            return result(completed, i + 1, get_result());
          }
          else if( arraySize < 0 )
          {
            stat_ = st_start;
//            return std::make_pair(i + 1, error);
            return result(error, i + 1);
          }
          else
          {
            array.reserve(arraySize);
            array_stack_.push(arraySize);
            value_stack_.push(array);

            stat_ = st_start;

            if( i + 1 != size )
            {
//              std::pair<size_t, ParseResult> parseResult = parseArray(ptr + i + 1, size - i - 1);
              result res = decode_array(ptr + i + 1, size - i - 1);
//              parseResult.first += i + 1;
              res.size(res.size() + i + 1);
              return res;
            }
            else
            {
//              return std::make_pair(i + 1, incompleted);
              return result(incompleted, i + 1);
            }
          }
        }
        else
        {
          stat_ = st_start;
//          return std::make_pair(i + 1, error);
          return result(error, i + 1);
        }
        break;
      case st_integer:
        if( c == '\r' )
        {
          if( buf_.empty() )
          {
            stat_ = st_start;
//            return std::make_pair(i + 1, error);
            return result(error, i + 1);
          }
          else
          {
            stat_ = st_integer_lf;
          }
        }
        else if( isdigit(c) || c == '-' )
        {
//          buf_.push_back(c);
          buf_.append(c);
        }
        else
        {
          stat_ = st_start;
//          return std::make_pair(i + 1, error);
          return result(error, i + 1);
        }
        break;
      case st_integer_lf:
        if( c == '\n' )
        {
          // optimize me
//          std::string tmp(buf_.begin(), buf_.end());
//          std::string tmp(buf_.data(), buf_.size());
          assert(buf_.size() < 32);
          char numstr[32];
          std::memcpy(numstr, buf_.data(), buf_.size());
          numstr[buf_.size()] = '\n';
          long int value = std::strtol(numstr, 0, 10);

          buf_.clear();

          value_stack_.push(value);
          stat_ = st_start;

//          return std::make_pair(i + 1, completed);
          return result(completed, i + 1, get_result());
        }
        else
        {
          stat_ = st_start;
//          return std::make_pair(i + 1, error);
          return result(error, i + 1);
        }
        break;
      default:
        stat_ = st_start;
//        return std::make_pair(i + 1, error);
        return result(error, i + 1);
      }
    }

//    return std::make_pair(i, incompleted);
    return result(incompleted, i);
  }

  unique_value get_result()
  {
    assert(!value_stack_.empty());
    unique_value uv = value_stack_.top();
    value_stack_.pop();
    return uv;
  }

private:
  state stat_;
  long int bulk_size_;
//  std::vector<char> buf_;
  buffer buf_;
  std::stack<long int> array_stack_;
  std::stack<unique_value> value_stack_;
};
}

#endif /// RESP_DECODER_HPP
