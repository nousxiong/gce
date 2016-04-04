///
/// encoder.hpp
///

#ifndef RESP_ENCODER_HPP
#define RESP_ENCODER_HPP

#include "config.hpp"
#include <cstdio>
#include <cassert>

namespace resp
{
/// Encoder of resp.
/**
 * @param Buffer Must match concept:
 *  #1 copyable
 *  #2 has member method Buffer::append(Buffer const&/char/char const*), like std::string
 *  #3 has non-explicit constructors, like std::string
 */
template <typename Buffer>
class encoder
{
  typedef Buffer buffer_t;
  typedef encoder<buffer_t> self_t;

public:
  explicit encoder()
    : buffers_(0)
  {
  }

  ~encoder()
  {
  }

public:
  /// Encode redis command.
  /**
   * @param cmd Redis command.
   * @return At least one buffer, may multi, if multi, user could use scatter-gather io to send them.
   */
  std::vector<buffer_t> encode(buffer_t const& cmd)
  {
    std::vector<buffer_t> buffers;
    buffers.reserve(1);
    append_size(buffers, '*', 1);
    append(buffers, cmd);
    return buffers;
  }

  /// Encode redis command + arg(s).
  std::vector<buffer_t> encode(
    buffer_t const& cmd,
    buffer_t const& arg1
    )
  {
    std::vector<buffer_t> buffers;
    buffers.reserve(2);
    append_size(buffers, '*', 2);
    append(buffers, cmd);
    append(buffers, arg1);
    return buffers;
  }

  /// Encode redis command + arg(s).
  std::vector<buffer_t> encode(
    buffer_t const& cmd,
    buffer_t const& arg1,
    buffer_t const& arg2
    )
  {
    std::vector<buffer_t> buffers;
    buffers.reserve(3);
    append_size(buffers, '*', 3);
    append(buffers, cmd);
    append(buffers, arg1);
    append(buffers, arg2);
    return buffers;
  }

  /// Encode redis command + arg(s).
  std::vector<buffer_t> encode(
    buffer_t const& cmd,
    buffer_t const& arg1,
    buffer_t const& arg2,
    buffer_t const& arg3
    )
  {
    std::vector<buffer_t> buffers;
    buffers.reserve(3);
    append_size(buffers, '*', 3);
    append(buffers, cmd);
    append(buffers, arg1);
    append(buffers, arg2);
    append(buffers, arg3);
    return buffers;
  }

  /// Encode redis command + arg(s).
  std::vector<buffer_t> encode(
    buffer_t const& cmd,
    buffer_t const& arg1,
    buffer_t const& arg2,
    buffer_t const& arg3,
    buffer_t const& arg4
    )
  {
    std::vector<buffer_t> buffers;
    buffers.reserve(4);
    append_size(buffers, '*', 4);
    append(buffers, cmd);
    append(buffers, arg1);
    append(buffers, arg2);
    append(buffers, arg3);
    append(buffers, arg4);
    return buffers;
  }

  /// Encode redis command + arg(s).
  std::vector<buffer_t> encode(
    buffer_t const& cmd,
    buffer_t const& arg1,
    buffer_t const& arg2,
    buffer_t const& arg3,
    buffer_t const& arg4,
    buffer_t const& arg5
    )
  {
    std::vector<buffer_t> buffers;
    buffers.reserve(5);
    append_size(buffers, '*', 5);
    append(buffers, cmd);
    append(buffers, arg1);
    append(buffers, arg2);
    append(buffers, arg3);
    append(buffers, arg4);
    append(buffers, arg5);
    return buffers;
  }

  /// Encode redis command + arg(s).
  std::vector<buffer_t> encode(buffer_t const& cmd, std::vector<buffer_t> const& args)
  {
    std::vector<buffer_t> buffers;
    buffers.reserve(args.size() + 1);
    encode(buffers, cmd, args);
    return buffers;
  }

public:
  class command
  {
  public:
    command()
       : enc_(0)
    {
    }
    
    command(self_t& enc, buffer_t const& name)
      : enc_(&enc)
      , name_(name)
    {
    }
    
    command(command const& other)
      : enc_(other.enc_)
      , name_(other.name_)
    {
    }
    
    command& operator=(command const& rhs)
    {
      if (this != &rhs)
      {
        enc_ = rhs.enc_;
        name_ = rhs.name_;
      }
      return *this;
    }

    ~command()
    {
    }

  public:
    command& arg(buffer_t const& a)
    {
      assert(enc_ != 0);
      enc_->queued_buffer(a);
      return *this;
    }

    self_t& end()
    {
      assert(enc_ != 0);
      std::vector<buffer_t>& buffers = enc_->get_buffers();
      std::vector<buffer_t>& args = enc_->get_cmd_args();
      self_t::encode(buffers, name_, args);
      args.clear();
      return *enc_;
    }

  private:
    self_t* enc_;
    buffer_t name_;
  };

  self_t& begin(std::vector<buffer_t>& buffers)
  {
    assert(buffers_ == 0);
    buffers.clear();
    buffers_ = &buffers;
    return *this;
  }

  void end()
  {
    assert(buffers_ != 0);
    buffers_ = 0;
  }

  std::vector<buffer_t>& get_buffers()
  {
    assert(buffers_ != 0);
    return *buffers_;
  }

  command cmd(buffer_t const& name)
  {
    assert(cmd_args_.empty());
    return command(*this, name);
  }

public:
  static void append(std::vector<buffer_t>& buffers, buffer_t const& arg)
  {
    append_size(buffers, '$', arg.size());
    if (arg.size() > RESP_LARGE_BUFFER_SIZE)
    {
      buffers.push_back(arg);
      buffers.push_back("\r\n");
    }
    else
    {
      buffer_t& buffer = buffers.back();
      buffer.append(arg);
      buffer.append("\r\n");
    }
  }

  static void append(std::vector<buffer_t>& buffers, char meta)
  {
    buffers.back().append(meta);
  }

  static void append(std::vector<buffer_t>& buffers, char const* meta)
  {
    buffers.back().append(meta);
  }

  static void append_size(std::vector<buffer_t>& buffers, char ty, size_t size)
  {
    if (buffers.empty())
    {
      buffers.push_back(buffer_t());
    }

    buffer_t& buffer = buffers.back();
    buffer.append(ty);

    char size_str[24];
    std::sprintf(size_str, "%u", (unsigned int)size);
    buffer.append(size_str);
    buffer.append("\r\n");
  }

  static void encode(std::vector<buffer_t>& buffers, buffer_t const& cmd, std::vector<buffer_t> const& args)
  {
    append_size(buffers, '*', args.size() + 1);
    append(buffers, cmd);
    for (size_t i=0; i<args.size(); ++i)
    {
      append(buffers, args[i]);
    }
  }

  void queued_buffer(buffer_t const& buf)
  {
    cmd_args_.push_back(buf);
  }

  std::vector<buffer_t>& get_cmd_args()
  {
    return cmd_args_;
  }

private:
  std::vector<buffer_t>* buffers_;
  std::vector<buffer_t> cmd_args_;
};
}

#endif /// RESP_ENCODER_HPP
