#ifndef GCE_DETAIL_OBJECT_ACCESS_HPP
#define GCE_DETAIL_OBJECT_ACCESS_HPP

#include <gce/config.hpp>

namespace gce
{
namespace detail
{
class object_access
{
public:
  template <typename T>
  inline static T* get_next(T* obj)
  {
    return obj->next_obj_;
  }

  template <typename T>
  inline static void set_next(T* obj, T* next)
  {
    obj->next_obj_ = next;
  }
};
}
}

#endif /// GCE_DETAIL_OBJECT_ACCESS_HPP
