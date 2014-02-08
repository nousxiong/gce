#ifndef GCE_IMPL_PROTOCOL_HPP
#define GCE_IMPL_PROTOCOL_HPP

#include <config.hpp>

namespace gce
{
namespace msg
{
struct header
{
  header()
    : size_(0)
    , type_(match_nil)
  {
  }

  boost::uint32_t size_;
  match_t type_;
};
}
}

GCE_PACK(gce::msg::header, (size_&sfix)(type_));

#endif /// GCE_IMPL_PROTOCOL_HPP
