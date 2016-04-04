///
/// config.hpp
///

#ifndef RESP_CONFIG_HPP
#define RESP_CONFIG_HPP

/// Suppress some vc warnings.
#if _MSC_VER
# pragma warning(disable : 4996)
#endif

/// If resp::buffer capacity > RESP_SMALL_BUFFER_SIZE, then use dynamic alloc.
#ifndef RESP_SMALL_BUFFER_SIZE
# define RESP_SMALL_BUFFER_SIZE 64
#endif

/// If resp::encoder encode arg size > RESP_LARGE_BUFFER_SIZE, then don't copy, instead as ref using it directly.
#ifndef RESP_LARGE_BUFFER_SIZE
# define RESP_LARGE_BUFFER_SIZE 1024
#endif

/// Define unique_array node capacity.
#ifndef RESP_UNIQUE_ARRAY_NODE_CAPACITY
# define RESP_UNIQUE_ARRAY_NODE_CAPACITY 8
#endif

#endif // RESP_CONFIG_HPP
