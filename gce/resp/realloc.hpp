///
/// realloc.hpp
///

#ifndef RESP_REALLOC_HPP
#define RESP_REALLOC_HPP

#include "config.hpp"
#include <cstdlib>
#include <cstring>

namespace resp
{
static void* realloc(void* ptr, size_t old_size, size_t new_size)
{
  if (new_size > old_size || ptr == 0)
  {
    void* p = std::malloc(new_size);
    if (p == 0)
    {
      return 0;
    }

    if (ptr != 0)
    {
      std::memcpy(p, ptr, old_size);
      std::free(ptr);
    }
    ptr = p;
  }
  return ptr;
}
}

#endif // RESP_REALLOC_HPP
