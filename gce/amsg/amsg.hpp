///
/// Copyright (c) 2012 Ning Ding (lordoffox@gmail.com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/lordoffox/amsg for latest version.
///

#ifndef AMSG_AMSG_HPP
#define AMSG_AMSG_HPP

#include <boost/array.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/detail/endian.hpp>
#include <boost/mpl/if.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/numeric/conversion/bounds.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/integer.hpp>
#include <boost/predef.h>
#include <string>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <iterator>
#include <limits>

#ifndef AMSG_INLINE
# if BOOST_COMP_MSVC
#  pragma inline_recursion(on)
#  define AMSG_INLINE __forceinline
# elif BOOST_COMP_GNUC
#  define AMSG_INLINE inline __attribute__((always_inline))
# else
#  define AMSG_INLINE inline
# endif
#endif

#if defined(AMSG_STD_CXX11) || defined(AMSG_HAS_STD_ARRAY)
#include <array>
#endif

#if defined(AMSG_STD_CXX11) || defined(AMSG_HAS_STD_FORWARD_LIST)
#include <forward_list>
#endif

#if defined(AMSG_STD_CXX11) || defined(AMSG_HAS_STD_UNORDERED_MAP)
#include <unordered_map>
#endif

#if defined(AMSG_STD_CXX11) || defined(AMSG_HAS_STD_UNORDERED_SET)
#include <unordered_set>
#endif

namespace amsg{

	enum error_code_t
	{
		success = 0,
		negative_assign_to_unsigned_integer_number,
		value_too_large_to_integer_number,
		sequence_length_overflow,
		stream_buffer_overflow,
		number_of_element_not_macth
	};

	struct basic_store
	{

		basic_store()
      : m_error_code(success)
		{

		}

		AMSG_INLINE error_code_t error_code() const
		{
			return m_error_code;
		}

		AMSG_INLINE void	set_error_code(error_code_t value)
		{
			m_error_code = value;
		}

		AMSG_INLINE const char * message()
		{
			switch(m_error_code)
			{
			case success:
				break;
			case negative_assign_to_unsigned_integer_number:
				return "can't assign negative number to unsigned integer number.";
			case value_too_large_to_integer_number:
				return "value too large to integer number";
			case sequence_length_overflow:
				return "sequence length overflow";
			case stream_buffer_overflow:
				return "stream buffer overflow";
			case number_of_element_not_macth:
				return "number of element not macth";
			default:
				break;
			}
			return "";
		}

		AMSG_INLINE bool error() const // true if error
		{
			return m_error_code != success;
		}

    AMSG_INLINE void clear()
    {
      m_error_code = success;
    }

		error_code_t	m_error_code;
	};

  template< typename stream_ty, typename error_string_ty = ::std::string >
  struct store : public basic_store
  {
  public:
    store(stream_ty& stream) 
      : basic_store()
      , m_stream(stream)
    {
    }

    AMSG_INLINE const error_string_ty& info() const { return m_error_info; }

    AMSG_INLINE void append_debug_info(const char * info)
    {
      m_error_info.append(info);
    }

    AMSG_INLINE bool bad()const { return m_stream.bad(); }

    AMSG_INLINE std::size_t read(char * buffer, std::size_t len)
    {
      return m_stream.read(buffer, len);
    }

    AMSG_INLINE std::size_t write(const char * buffer, std::size_t len)
    {
      return m_stream.write(buffer, len);
    }

    AMSG_INLINE void clear()
    {
      basic_store::clear();
      m_stream.clear();
      m_error_info.clear();
    }

  private:
    stream_ty&		m_stream;
    error_string_ty		m_error_info;
  };

  template<typename stream_ty>
  AMSG_INLINE store<stream_ty> make_store(stream_ty& stream)
  {
    return store<stream_ty>(stream);
  }
}

namespace amsg{	namespace detail
{
#if BOOST_WORKAROUND(BOOST_MSVC, > 1300)
#include <intrin.h>

#define byte_swap_16(x) _byteswap_ushort(x)
#define byte_swap_32(x) _byteswap_ulong(x)
#define byte_swap_64(x) _byteswap_uint64(x)

#elif BOOST_WORKAROUND(__GNUC__, >= 3)
#include <byteswap.h>

#define byte_swap_16(x) bswap_16(x)
#define byte_swap_32(x) bswap_32(x)
#define byte_swap_64(x) bswap_64(x)
#else
	template<typename _ty>
	AMSG_INLINE _ty byte_swap_16(_ty value)
	{
		value= (value>>8) | (value<<8);
		return value;
	}

	template<typename _ty>
	AMSG_INLINE _ty byte_swap_32(_ty value)
	{
		value= ((value<<8)&0xFF00FF00) | ((value>>8)&0x00FF00FF);
		value= (value>>16) | (value<<16);
		return value;
	}

	template<typename _ty>
	AMSG_INLINE _ty byte_swap_64(_ty value)
	{
		value= ((value<< 8)&0xFF00FF00FF00FF00ULL) | ((value>> 8)&0x00FF00FF00FF00FFULL);
		value= ((value<<16)&0xFFFF0000FFFF0000ULL) | ((value>>16)&0x0000FFFF0000FFFFULL);
		return (value>>32) | (value<<32);
	}

#endif

#if defined(BOOST_LITTLE_ENDIAN)
#define host_to_little_endian16(value) (value)
#define host_to_little_endian32(value) (value)
#define host_to_little_endian64(value) (value)
#define little_endian_to_host16(value) (value)
#define little_endian_to_host32(value) (value)
#define little_endian_to_host64(value) (value)
#else
#define host_to_little_endian16(value) byte_swap_16(value)
#define host_to_little_endian32(value) byte_swap_32(value)
#define host_to_little_endian64(value) byte_swap_64(value)
#define little_endian_to_host16(value) byte_swap_16(value)
#define little_endian_to_host32(value) byte_swap_32(value)
#define little_endian_to_host64(value) byte_swap_64(value)
#endif

	enum{  const_interger_byte_msak = 0x3f, const_negative_bit_value = 0x40 , const_tag_as_value = 0x7f ,const_tag_as_type , const_store_postive_integer_byte_mask = 0x80 -2 , const_store_negative_integer_byte_mask = 0x80 + const_negative_bit_value - 2 };

	template<typename ty,int tag>
	struct byte_size_of_impl
	{
		typedef typename ::boost::remove_const<ty>::type value_type;
		static AMSG_INLINE ::boost::uint32_t size(const value_type& value);
	};

	template<typename ty , int tag>
	struct byte_size_of_enum_impl
	{
		typedef ty value_type;
		static AMSG_INLINE ::boost::uint32_t size(const value_type& value)
		{
			int64_t temp = value;
      return byte_size_of_impl<int64_t,tag>::size(temp);
		}
	};

	template <typename ty , int tag>
	struct byte_size_of
	{
		typedef ty value_type;
		typedef typename ::boost::mpl::if_<
			::boost::is_enum<value_type>,
			byte_size_of_enum_impl<value_type,tag>,
			byte_size_of_impl<value_type,tag>
		>::type impl_type;
	};

	template<typename ty, int tag>
	struct byte_size_of_unsigned_char_imp
	{
		typedef ty value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type& value)
		{
			if(value < const_tag_as_type)
			{
				return 1;
			}
			return 2;
		}
	};

	template<typename ty, int tag>
	struct byte_size_of_signed_char_imp
	{
		typedef ty value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type& value)
		{
			if(0 <= value && value < const_tag_as_type)
			{
				return 1;
			}
			return 2;
		}
	};

	template<int tag>
	struct byte_size_of< ::boost::uint8_t,tag>
	{
		typedef byte_size_of_unsigned_char_imp< ::boost::uint8_t,tag> impl_type;
	};

	template<int tag>
	struct byte_size_of< ::boost::int8_t,tag>
	{
		typedef byte_size_of_signed_char_imp< ::boost::int8_t,tag> impl_type;
	};

#if CHAR_MIN < 0
  template<int tag>
  struct byte_size_of < char, tag >
  {
    typedef byte_size_of_signed_char_imp<char, tag>  impl_type;
  };
#else
  template<int tag>
  struct byte_size_of < char, tag >
  {
    typedef byte_size_of_unsigned_char_imp<char, tag>  impl_type;
  };
#endif

	template<int tag>
	struct byte_size_of_impl< ::boost::uint16_t,tag>
	{
		typedef ::boost::uint16_t value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type& value)
		{
			if(value < const_tag_as_type)
			{
				return 1;
			}
			else
			{
				if(value < 0x100)
				{
					return 2;
				}
			}
			return 3;
		}
	};

	template<int tag>
	struct byte_size_of_impl< ::boost::int16_t,tag>
	{
		typedef ::boost::int16_t value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type& value)
		{
			if(0 <= value && value < const_tag_as_type)
			{
				return 1;
			}
			else
			{
				value_type temp = value;
				if(value < 0)
				{
					temp = -value;
				}
				if(temp < 0x100)
				{
					return 2;
				}
			}
			return 3;
		}
	};

	template<int tag>
	struct byte_size_of_impl< ::boost::uint32_t,tag>
	{
		typedef ::boost::uint32_t value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type& value)
		{
			if(value < const_tag_as_type)
			{
				return 1;
			}
			else
			{
				if(value < 0x100)
				{
					return 2;
				}
				else if(value < 0x10000)
				{
					return 3;
				}
				else if(value < 0x1000000)
				{
					return 4;
				}
			}
			return 5;
		}
	};

	template<int tag>
	struct byte_size_of_impl< ::boost::int32_t,tag>
	{
		typedef ::boost::int32_t value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type& value)
		{
			if(0 <= value && value < const_tag_as_type)
			{
				return 1;
			}
			else
			{
				value_type temp = value;
				if(value < 0)
				{
					temp = -value;
				}
				if(temp < 0x100)
				{
					return 2;
				}
				else if(temp < 0x10000)
				{
					return 3;
				}
				else if(temp < 0x1000000)
				{
					return 4;
				}
			}
			return 5;
		}
	};

	template<int tag>
	struct byte_size_of_impl< ::boost::uint64_t,tag>
	{
		typedef ::boost::uint64_t value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type& value)
		{
			if(value < const_tag_as_type)
			{
				return 1;
			}
			else
			{
				if(value < 0x100)
				{
					return 2;
				}
				else if(value < 0x10000)
				{
					return 3;
				}
				else if(value < 0x1000000)
				{
					return 4;
				}
				else if(value < 0x100000000)
				{
					return 5;
				}
				else if(value < 0x10000000000LL)
				{
					return 6;
				}
				else if(value < 0x1000000000000LL)
				{
					return 7;
				}
				else if(value < 0x100000000000000LL)
				{
					return 8;
				}
			}
			return 9;
		}
	};

	template<int tag>
	struct byte_size_of_impl< ::boost::int64_t,tag>
	{
		typedef ::boost::int64_t value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type& value)
		{
			if(0 <= value && value < const_tag_as_type)
			{
				return 1;
			}
			else
			{
				value_type temp = value;
				if(value < 0)
				{
					temp = -value;
				}
				if(temp < 0x100)
				{
					return 2;
				}
				else if(temp < 0x10000)
				{
					return 3;
				}
				else if(temp < 0x1000000)
				{
					return 4;
				}
				else if(temp < 0x100000000)
				{
					return 5;
				}
				else if(temp < 0x10000000000LL)
				{
					return 6;
				}
				else if(temp < 0x1000000000000LL)
				{
					return 7;
				}
				else if(temp < 0x100000000000000LL)
				{
					return 8;
				}
			}
			return 9;
		}
	};

  template<int tag>
	struct byte_size_of_impl< bool,tag>
	{
		typedef bool value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type& value)
		{
			return 1;
		}
	};

	template<int tag>
	struct byte_size_of_impl<float,tag>
	{
		typedef float value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type&)
		{
			return sizeof(value_type);
		}
	};

	template<int tag>
	struct byte_size_of_impl<double,tag>
	{
		typedef double value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type&)
		{
			return sizeof(value_type);
		}
	};

	template<typename alloc_ty , int tag>
	struct byte_size_of_impl< ::std::basic_string<char, ::std::char_traits<char>, alloc_ty> , tag>
	{
		typedef ::std::basic_string<char, ::std::char_traits<char>, alloc_ty> value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type& value , ::std::size_t max = 0)
		{
      (max);
      ::boost::uint32_t len = (::boost::uint32_t)value.length();
      ::boost::uint32_t size = byte_size_of< ::boost::uint32_t, 0>::impl_type::size(len) + len;
			return size;
		}
	};

	template<typename alloc_ty , int tag>
	struct byte_size_of_impl< ::std::basic_string<wchar_t, ::std::char_traits<wchar_t>, alloc_ty> , tag>
	{
		typedef ::std::basic_string<wchar_t, ::std::char_traits<wchar_t>, alloc_ty> value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type& value , ::std::size_t max = 0)
		{
			::std::size_t len = value.length();
      ::std::size_t size = byte_size_of< ::boost::uint32_t, 0>::impl_type::size(len);
			for(::boost::uint32_t i = 0 ; i< len ; ++i)
			{
        size += byte_size_of<wchar_t, 0>::impl_type::size(value[i]);
			}
			return size;
		}
	};

	template<typename ty , int tag>
	struct byte_size_of_seq_container_impl
	{
		typedef ty value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type& value , ::std::size_t max = 0)
		{
      (max);
      ::boost::uint32_t len = (::boost::uint32_t)value.size();
      ::boost::uint32_t size = byte_size_of< ::boost::uint32_t, 0>::impl_type::size(len);
			for( typename value_type::const_iterator i = value.begin() ; i != value.end(); ++i )
			{
				const typename value_type::value_type& elem_value = *i;
				size += byte_size_of<typename value_type::value_type,tag>::impl_type::size(elem_value);
			}
			return size;
		}
	};

	template<typename ty,typename alloc_ty , int tag>
	struct byte_size_of< ::std::deque<ty,alloc_ty> , tag>
	{
		typedef typename ::std::deque<ty,alloc_ty> value_type;
		typedef byte_size_of_seq_container_impl<value_type,tag> impl_type;
	};

	template<typename ty,typename alloc_ty , int tag>
	struct byte_size_of< ::std::list<ty,alloc_ty> , tag>
	{
		typedef typename ::std::list<ty,alloc_ty> value_type;
		typedef byte_size_of_seq_container_impl<value_type,tag> impl_type;
	};

	template<typename ty,typename alloc_ty , int tag>
	struct byte_size_of< ::std::vector<ty,alloc_ty> , tag>
	{
		typedef typename ::std::vector<ty,alloc_ty> value_type;
		typedef byte_size_of_seq_container_impl<value_type,tag> impl_type;
	};

	template<typename ty ,std::size_t array_size ,int tag>
	struct byte_size_of< ::boost::array<ty,array_size> , tag>
	{
		typedef typename ::boost::array<ty,array_size> value_type;
		typedef byte_size_of_seq_container_impl<value_type,tag> impl_type;
	};

#if defined(AMSG_STD_CXX11)||defined(AMSG_HAS_STD_ARRAY)
	template<typename ty ,std::size_t array_size ,int tag>
	struct byte_size_of< ::std::array<ty,array_size> , tag>
	{
		typedef typename ::std::array<ty,array_size> value_type;
		typedef byte_size_of_seq_container_impl<value_type,tag> impl_type;
	};
#endif

#if defined(AMSG_STD_CXX11)||defined(AMSG_HAS_STD_FORWARD_LIST)
  template<typename ty , int tag>
	struct byte_size_of_forward_list_impl
	{
		typedef ty value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type& value , ::std::size_t max = 0)
		{
      (max);
      ::boost::uint32_t len = (::boost::uint32_t)std::distance(value.begin(), value.end());
      ::boost::uint32_t size = byte_size_of< ::boost::uint32_t, 0>::impl_type::size(len);
			for( typename value_type::const_iterator i = value.begin() ; i != value.end(); ++i )
			{
				const typename value_type::value_type& elem_value = *i;
        size += byte_size_of<typename value_type::value_type, tag>::impl_type::size(elem_value);
			}
			return size;
		}
	};

  template<typename ty,typename alloc_ty , int tag>
	struct byte_size_of< ::std::forward_list<ty,alloc_ty> , tag>
	{
		typedef typename ::std::forward_list<ty,alloc_ty> value_type;
		typedef byte_size_of_forward_list_impl<value_type,tag> impl_type;
	};
#endif

	template<typename ty , int tag>
	struct byte_size_of_unorder_container_impl
	{
		typedef ty value_type;

    static AMSG_INLINE::boost::uint32_t size(const value_type& value, ::std::size_t max = 0)
		{
      (max);
      ::boost::uint32_t len = (::boost::uint32_t)value.size();
      ::boost::uint32_t size = byte_size_of< ::boost::uint32_t, 0>::impl_type::size(len);
			for( typename value_type::const_iterator i = value.begin() ; i != value.end(); ++i )
			{
				typedef typename ::boost::remove_const<typename value_type::value_type::first_type>::type first_type;
				typedef typename ::boost::remove_const<typename value_type::value_type::second_type>::type second_type;
        size += byte_size_of<first_type, tag>::impl_type::size(i->first);
        size += byte_size_of<second_type, tag>::impl_type::size(i->second);
			}
			return size;
		}
	};

	template<typename key_ty , typename ty , typename cmp_ty ,typename alloc_ty , int tag>
	struct byte_size_of< ::std::map<key_ty,ty,cmp_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::map<key_ty,ty,cmp_ty,alloc_ty> value_type;
		typedef byte_size_of_unorder_container_impl<value_type,tag> impl_type;
	};

  template<typename key_ty , typename ty , typename cmp_ty ,typename alloc_ty , int tag>
	struct byte_size_of< ::std::multimap<key_ty,ty,cmp_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::multimap<key_ty,ty,cmp_ty,alloc_ty> value_type;
		typedef byte_size_of_unorder_container_impl<value_type,tag> impl_type;
	};

	template<typename key_ty , typename ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct byte_size_of< ::boost::unordered_map<key_ty,ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::boost::unordered_map<key_ty,ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef byte_size_of_unorder_container_impl<value_type,tag> impl_type;
	};

#if defined(AMSG_STD_CXX11) || defined(AMSG_HAS_STD_UNORDERED_MAP)
  template<typename key_ty , typename ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct byte_size_of< ::std::unordered_map<key_ty,ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::unordered_map<key_ty,ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef byte_size_of_unorder_container_impl<value_type,tag> impl_type;
	};
  
  template<typename key_ty , typename ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct byte_size_of< ::std::unordered_multimap<key_ty,ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::unordered_multimap<key_ty,ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef byte_size_of_unorder_container_impl<value_type,tag> impl_type;
	};
#endif

  template<typename ty , int tag>
	struct byte_size_of_unorder_set_impl
	{
		typedef ty value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type& value , ::std::size_t max = 0)
		{
      (max);
			::std::size_t len = value.size();
      ::std::size_t size = byte_size_of< ::boost::uint32_t, 0>::impl_type::size(len);
			for( typename value_type::const_iterator i = value.begin() ; i != value.end(); ++i )
			{
				typedef typename ::boost::remove_const<typename value_type::value_type>::type val_type;
        size += byte_size_of<val_type, tag>::impl_type::size(i->first);
			}
			return size;
		}
	};

  template<typename key_ty , typename cmp_ty ,typename alloc_ty , int tag>
	struct byte_size_of< ::std::set<key_ty,cmp_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::set<key_ty,cmp_ty,alloc_ty> value_type;
		typedef byte_size_of_unorder_set_impl<value_type,tag> impl_type;
	};

  template<typename key_ty , typename cmp_ty ,typename alloc_ty , int tag>
	struct byte_size_of< ::std::multiset<key_ty,cmp_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::multiset<key_ty,cmp_ty,alloc_ty> value_type;
		typedef byte_size_of_unorder_set_impl<value_type,tag> impl_type;
	};

	template<typename key_ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct byte_size_of< ::boost::unordered_set<key_ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::boost::unordered_set<key_ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef byte_size_of_unorder_set_impl<value_type,tag> impl_type;
	};

#if defined(AMSG_STD_CXX11) || defined(AMSG_HAS_STD_UNORDERED_SET)
  template<typename key_ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct byte_size_of< ::std::unordered_set<key_ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::unordered_set<key_ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef byte_size_of_unorder_set_impl<value_type,tag> impl_type;
	};

  template<typename key_ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct byte_size_of< ::std::unordered_multiset<key_ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::unordered_multiset<key_ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef byte_size_of_unorder_set_impl<value_type,tag> impl_type;
	};
#endif

	template<typename store_ty , typename ty , int tag>
	struct value_read_support_impl
	{
		typedef ty value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value,::std::size_t max = 0);
	};

  template<typename store_ty, typename ty, int tag>
	struct value_write_support_impl
	{
		typedef ty value_type;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value,::std::size_t max = 0);
	};

  template<typename store_ty , typename ty>
	struct value_default_read_support_impl
	{
		typedef ty value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value);
	};

	template<typename store_ty , typename ty>
	struct value_default_write_support_impl
	{
		typedef ty value_type;
		static AMSG_INLINE void write(store_ty& store_data, value_type& value);
	};

	template<typename store_ty , typename ty , int tag>
	struct value_read_support_enum_impl
	{
		typedef ty value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			int64_t temp;
      value_default_read_support_impl<store_ty,int64_t>::read(store_data,temp);
			value = static_cast<ty>(temp);
		}
	};

	template<typename store_ty , typename ty , int tag>
	struct value_write_support_enum_impl
	{
		typedef ty value_type;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			int64_t temp = value;
      value_default_write_support_impl<store_ty,int64_t>::write(store_data,temp);
		}
	};

	template<typename store_ty , typename ty , int tag = 0>
	struct value_read_support
	{
		typedef ty value_type;
		typedef typename ::boost::mpl::if_<
			::boost::is_enum<value_type>,
			value_read_support_enum_impl<store_ty,value_type,tag>,
			value_read_support_impl<store_ty,value_type,tag>
		>::type impl_type;
	};

  template<typename store_ty, typename ty, int tag = 0>
	struct value_write_support
	{
		typedef ty value_type;
		typedef typename ::boost::mpl::if_<
			::boost::is_enum<value_type>,
			value_write_support_enum_impl<store_ty,value_type,tag>,
			value_write_support_impl<store_ty,value_type,tag>
		>::type impl_type;
	};

	template<typename store_ty,typename ty,int type_byte_size=sizeof(ty)>
	struct value_fix_size_support_impl
	{
		typedef ty value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value);
		static AMSG_INLINE void write(store_ty& store_data, value_type& value);
	};

	template<typename store_ty,typename ty>
	struct value_fix_size_support_impl<store_ty,ty,1>
	{
		typedef ty value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			store_data.read((char*)&value,1);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
		static AMSG_INLINE void write(store_ty& store_data,const value_type& value)
		{
			store_data.write((const char *)&value,1);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<typename store_ty,typename ty>
	struct value_fix_size_support_impl<store_ty,ty,2>
	{
		typedef ty value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			store_data.read((char*)&value,2);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			value = little_endian_to_host16(value);
		}
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			value_type write_value = host_to_little_endian16(value);
			store_data.write((const char *)&write_value,2);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<typename store_ty,typename ty>
	struct value_fix_size_support_impl<store_ty,ty,4>
	{
		typedef ty value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			store_data.read((char*)&value,4);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			value = little_endian_to_host32(value);
		}
		static AMSG_INLINE void write(store_ty& store_data,const value_type& value)
		{
			value_type write_value = host_to_little_endian32(value);
			store_data.write((const char *)&write_value,4);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<typename store_ty,typename ty>
	struct value_fix_size_support_impl<store_ty,ty,8>
	{
		typedef ty value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			store_data.read((char*)&value,8);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			value = little_endian_to_host64(value);
		}
		static AMSG_INLINE void write(store_ty& store_data,const value_type& value)
		{
			value_type write_value = host_to_little_endian64(value);
			store_data.write((const char *)&write_value,8);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<typename store_ty , typename ty>
	struct value_fix_size_read_support_impl
	{
		typedef ty value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			value_fix_size_support_impl<store_ty,ty,sizeof(ty)>::read(store_data,value);
		}
	};

	template<typename store_ty , typename ty>
	struct value_fix_size_write_support_impl
	{
		typedef ty value_type;
		static AMSG_INLINE void write(store_ty& store_data,const value_type& value)
		{
			value_fix_size_support_impl<store_ty,ty,sizeof(ty)>::write(store_data,value);
		}
	};

	template<typename store_ty>
	struct value_default_read_support_impl<store_ty,::boost::uint8_t>
	{
		typedef ::boost::uint8_t value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const int bytes = sizeof(value_type);
			value_type read_value[2]={0};
			store_data.read((char*)&value,1);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			if(value > const_tag_as_value)
			{
				if((long)value & const_negative_bit_value)
				{
					store_data.set_error_code(negative_assign_to_unsigned_integer_number);
					return;

				}
				int read_bytes = (value & const_interger_byte_msak) + 1;
				if( bytes < read_bytes )
				{
					store_data.set_error_code(value_too_large_to_integer_number);
					return;

				}
				store_data.read((char*)&value ,1);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
			}
		}
	};

	template<typename store_ty>
	struct value_default_write_support_impl<store_ty,::boost::uint8_t>
	{
		typedef ::boost::uint8_t value_type;

		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			value_type write_value[2]={0};
			int write_bytes = 0;
			::boost::uint8_t tag = static_cast< ::boost::uint8_t>(value);
			write_bytes = 1;
			if(value < const_tag_as_type)
			{
				write_value[0] = tag;
			}
			else
			{
				write_value[0] = 0x80;
				write_value[1] = tag;
				write_bytes = 2;
			}
			store_data.write((const char *)write_value,write_bytes);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<typename store_ty , typename char_like_ty>
	struct value_default_read_char_like_support_impl
	{
		typedef char_like_ty value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const int bytes = sizeof(value_type);
			value_type read_value[2]={0};
			store_data.read((char*)&value,1);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			if(value > const_tag_as_value)
			{
				int sign = 1;
				if((long)value & const_negative_bit_value)
				{
					sign = -1;
				}
				int read_bytes = (value & const_interger_byte_msak) + 1;
				if( bytes < read_bytes )
				{
					store_data.set_error_code(value_too_large_to_integer_number);
					return;

				}
				store_data.read((char *)&read_value[1],1);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				if(sign < 0)
				{
					value = -read_value[1];
				}
				else
				{
					value = read_value[1];
				}
			}

		}
	};

	template<typename store_ty , typename char_like_ty>
	struct value_default_write_char_like_support_impl
	{
		typedef char_like_ty value_type;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			value_type write_value[2]={0};
			int write_bytes = 0;
			::boost::uint8_t * ptr = (::boost::uint8_t *)write_value;
			::boost::uint8_t tag = static_cast< ::boost::uint8_t>(value);
			write_bytes = 1;
			if(0<= value && value < const_tag_as_type)
			{
				write_value[0] = tag;
			}
			else
			{
				::boost::uint8_t negative_bit = 0;
				value_type temp = value;
				if(value < 0)
				{
					negative_bit = const_negative_bit_value;
					temp = -value;
				}
				write_value[0] = (value_type)0x80;
				write_value[0] |= negative_bit;
				write_value[1] = temp;
				write_bytes = 2;
			}
			store_data.write((const char *)write_value,write_bytes);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

  template<typename store_ty>
	struct value_default_read_support_impl<store_ty , bool>
	{
		typedef bool value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
      ::boost::uint8_t val;
      value_default_read_support_impl<store_ty,::boost::uint8_t>::read(store_data, val);
      value = val != 0;
		}
	};

	template<typename store_ty>
	struct value_default_write_support_impl<store_ty , bool>
	{
		typedef bool value_type;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
      ::boost::uint8_t val = value ? 1 : 0;
      value_default_write_support_impl<store_ty,::boost::uint8_t>::write(store_data, val);
		}
	};

	template<typename store_ty>
	struct value_default_read_support_impl<store_ty , ::boost::uint16_t>
	{
		typedef ::boost::uint16_t value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const int bytes = sizeof(value_type);
			value_type read_value[2]={0};
			::boost::uint8_t * ptr = (::boost::uint8_t *)read_value;
			store_data.read((char*)ptr,1);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			value = *ptr;
			if(value > const_tag_as_value)
			{
				if((long)value & const_negative_bit_value)
				{
					store_data.set_error_code(negative_assign_to_unsigned_integer_number);
					return;

				}
				int read_bytes = (value & const_interger_byte_msak) + 1;
				if( bytes < read_bytes )
				{
					store_data.set_error_code(value_too_large_to_integer_number);
					return;

				}
				ptr = (::boost::uint8_t *)&read_value[1];
				store_data.read((char*)ptr,read_bytes);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				value = little_endian_to_host16(read_value[1]);
			}
		}
	};

	template<typename store_ty>
	struct value_default_write_support_impl<store_ty , ::boost::uint16_t>
	{
		typedef ::boost::uint16_t value_type;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			value_type write_value[2]={0};
			int write_bytes = 0;
			::boost::uint8_t * ptr = (::boost::uint8_t *)write_value;
			::boost::uint8_t tag = static_cast< ::boost::uint8_t>(value);
			write_bytes = 1;
			if(value < const_tag_as_type)
			{
				write_value[0] = tag;
			}
			else
			{
				write_value[1] = host_to_little_endian16(value);
				ptr = (::boost::uint8_t *)(&write_value[1]) - 1;
				if(value < 0x100)
				{
					write_bytes = 2;
				}
				else
				{
					write_bytes = 3;
				}
				*ptr = const_store_postive_integer_byte_mask + write_bytes;
			}
			store_data.write((const char *)ptr,write_bytes);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<typename store_ty>
	struct value_default_read_support_impl<store_ty , ::boost::int16_t>
	{
		typedef ::boost::int16_t value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const int bytes = sizeof(value_type);
			value_type read_value[2]={0};
			::boost::uint8_t * ptr = (::boost::uint8_t *)read_value;
			store_data.read((char*)ptr,1);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			value = *ptr;
			if(value > const_tag_as_value)
			{
				int sign = 1;
				if((long)value & const_negative_bit_value)
				{
					sign = -1;
				}
				int read_bytes = (value & const_interger_byte_msak) + 1;
				if( bytes < read_bytes )
				{
					store_data.set_error_code(value_too_large_to_integer_number);
					return;

				}
				ptr = (::boost::uint8_t *)&read_value[1];
				store_data.read((char*)ptr,read_bytes);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				if(sign < 0)
				{
					value = -(value_type)little_endian_to_host16(read_value[1]);
				}
				else
				{
					value = little_endian_to_host16(read_value[1]);
				}
			}
		}
	};

	template<typename store_ty>
	struct value_default_write_support_impl<store_ty , ::boost::int16_t>
	{
		typedef ::boost::int16_t value_type;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			value_type write_value[2]={0};
			int write_bytes = 0;
			::boost::uint8_t * ptr = (::boost::uint8_t *)write_value;
			::boost::uint8_t tag = static_cast< ::boost::uint8_t>(value);
			write_bytes = 1;
			if(0 <= value && value < const_tag_as_type)
			{
				write_value[0] = tag;
			}
			else
			{
				::boost::uint8_t negative_bit = 0;
				value_type temp = value;
				if(value < 0)
				{
					negative_bit = const_negative_bit_value;
					temp = -value ;
				}
				write_value[1] = host_to_little_endian16(temp);
				ptr = (::boost::uint8_t *)(&write_value[1]) - 1;
				if(temp < 0x100)
				{
					write_bytes = 2;
				}
				else
				{
					write_bytes = 3;
				}
				*ptr = const_store_postive_integer_byte_mask + negative_bit + write_bytes;
			}
			store_data.write((const char *)ptr,write_bytes);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<typename store_ty>
	struct value_default_read_support_impl<store_ty , ::boost::uint32_t>
	{
		typedef ::boost::uint32_t value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const int bytes = sizeof(value_type);
			value_type read_value[2]={0};
			::boost::uint8_t * ptr = (::boost::uint8_t *)read_value;
			store_data.read((char*)ptr,1);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			value = *ptr;
			if(value > const_tag_as_value)
			{
				if((long)value & const_negative_bit_value)
				{
					store_data.set_error_code(negative_assign_to_unsigned_integer_number);
					return;

				}
				int read_bytes = (value & const_interger_byte_msak) + 1;
				if( bytes < read_bytes )
				{
					store_data.set_error_code(value_too_large_to_integer_number);
					return;

				}
				ptr = (::boost::uint8_t *)&read_value[1];
				store_data.read((char*)ptr,read_bytes);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				value = little_endian_to_host32(read_value[1]);
			}
		}
	};

	template<typename store_ty>
	struct value_default_write_support_impl<store_ty , ::boost::uint32_t>
	{
		typedef ::boost::uint32_t value_type;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			value_type write_value[2]={0};
			int write_bytes = 0;
			::boost::uint8_t * ptr = (::boost::uint8_t *)write_value;
			::boost::uint8_t tag = static_cast< ::boost::uint8_t>(value);
			write_bytes = 1;
			if(value < const_tag_as_type)
			{
				write_value[0] = tag;
			}
			else
			{
				write_value[1] = host_to_little_endian32(value);
				ptr = (::boost::uint8_t *)(&write_value[1]) - 1;
				if(value < 0x100)
				{
					write_bytes = 2;
				}
				else if(value < 0x10000)
				{
					write_bytes = 3;
				}
				else if(value < 0x1000000)
				{
					write_bytes = 4;
				}
				else
				{
					write_bytes = 5;
				}
				*ptr = const_store_postive_integer_byte_mask + write_bytes;
			}
			store_data.write((const char *)ptr,write_bytes);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<typename store_ty>
	struct value_default_read_support_impl<store_ty , ::boost::int32_t>
	{
		typedef ::boost::int32_t value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const int bytes = sizeof(value_type);
			value_type read_value[2]={0};
			::boost::uint8_t * ptr = (::boost::uint8_t *)read_value;
			store_data.read((char*)ptr,1);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			value = *ptr;
			if(value > const_tag_as_value)
			{
				int sign = 1;
				if((long)value & const_negative_bit_value)
				{
					sign = -1;
				}
				int read_bytes = (value & const_interger_byte_msak) + 1;
				if( bytes < read_bytes )
				{
					store_data.set_error_code(value_too_large_to_integer_number);
					return;

				}
				ptr = (::boost::uint8_t *)&read_value[1];
				store_data.read((char*)ptr,read_bytes);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				if(sign < 0)
				{
					value = -(value_type)little_endian_to_host32(read_value[1]);
				}
				else
				{
					value = little_endian_to_host32(read_value[1]);
				}
			}
		}
	};

	template<typename store_ty>
	struct value_default_write_support_impl<store_ty , ::boost::int32_t>
	{
		typedef ::boost::int32_t value_type;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			value_type write_value[2]={0};
			int write_bytes = 0;
			::boost::uint8_t * ptr = (::boost::uint8_t *)write_value;
			::boost::uint8_t tag = static_cast< ::boost::uint8_t>(value);
			write_bytes = 1;
			if(0 <= value && value < const_tag_as_type)
			{
				write_value[0] = tag;
			}
			else
			{
				::boost::uint8_t negative_bit = 0;
				value_type temp = value;
				if(value < 0)
				{
					negative_bit = const_negative_bit_value;
					temp = -value ;
				}
				write_value[1] = host_to_little_endian32(temp);
				ptr = (::boost::uint8_t *)(&write_value[1]) - 1;
				if(temp < 0x100)
				{
					write_bytes = 2;
				}
				else if(temp < 0x10000)
				{
					write_bytes = 3;
				}
				else if(temp < 0x1000000)
				{
					write_bytes = 4;
				}
				else
				{
					write_bytes = 5;
				}
				*ptr = const_store_postive_integer_byte_mask + negative_bit + write_bytes;
			}
			store_data.write((const char *)ptr,write_bytes);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<typename store_ty>
	struct value_default_read_support_impl<store_ty , ::boost::uint64_t>
	{
		typedef ::boost::uint64_t value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const int bytes = sizeof(value_type);
			value_type read_value[2]={0};
			::boost::uint8_t * ptr = (::boost::uint8_t *)read_value;
			store_data.read((char*)ptr,1);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			value = *ptr;
			if(value > const_tag_as_value)
			{
				if((long)value & const_negative_bit_value)
				{
					store_data.set_error_code(negative_assign_to_unsigned_integer_number);
					return;

				}
				int read_bytes = int(value & const_interger_byte_msak) + 1;
				if( bytes < read_bytes )
				{
					store_data.set_error_code(value_too_large_to_integer_number);
					return;
				}
				ptr = (::boost::uint8_t *)&read_value[1];
				store_data.read((char*)ptr,read_bytes);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				value = little_endian_to_host64(read_value[1]);
			}
		}
	};

	template<typename store_ty>
	struct value_default_write_support_impl<store_ty , ::boost::uint64_t>
	{
		typedef ::boost::uint64_t value_type;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			value_type write_value[2]={0};
			int write_bytes = 0;
			::boost::uint8_t * ptr = (::boost::uint8_t *)write_value;
			::boost::uint8_t tag = static_cast< ::boost::uint8_t>(value);
			write_bytes = 1;
			if(value < const_tag_as_type)
			{
				write_value[0] = tag;
			}
			else
			{
				write_value[1] = host_to_little_endian64(value);
				ptr = (::boost::uint8_t *)(&write_value[1]) - 1;
				if(value < 0x100)
				{
					write_bytes = 2;
				}
				else if(value < 0x10000)
				{
					write_bytes = 3;
				}
				else if(value < 0x1000000)
				{
					write_bytes = 4;
				}
				else if(value < 0x100000000)
				{
					write_bytes = 5;
				}
				else if(value < 0x10000000000LL)
				{
					write_bytes = 6;
				}
				else if(value < 0x1000000000000LL)
				{
					write_bytes = 7;
				}
				else if(value < 0x100000000000000LL)
				{
					write_bytes = 8;
				}
				else
				{
					write_bytes = 9;
				}
				*ptr = const_store_postive_integer_byte_mask + write_bytes;
			}
			store_data.write((const char *)ptr,write_bytes);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<typename store_ty>
	struct value_default_read_support_impl<store_ty , ::boost::int64_t>
	{
		typedef ::boost::int64_t value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const int bytes = sizeof(value_type);
			value_type read_value[2]={0};
			::boost::uint8_t * ptr = (::boost::uint8_t *)read_value;
			store_data.read((char*)ptr,1);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			value = *ptr;
			if(value > const_tag_as_value)
			{
				int sign = 1;
				if((long)value & const_negative_bit_value)
				{
					sign = -1;
				}
				int read_bytes = int(value & const_interger_byte_msak) + 1;
				if( bytes < read_bytes )
				{
					store_data.set_error_code(value_too_large_to_integer_number);
					return;

				}
				ptr = (::boost::uint8_t *)&read_value[1];
				store_data.read((char*)ptr,read_bytes);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				if(sign < 0)
				{
					value = -(value_type)little_endian_to_host64(read_value[1]);
				}
				else
				{
					value = little_endian_to_host64(read_value[1]);
				}
			}
		}
	};

	template<typename store_ty>
	struct value_default_write_support_impl<store_ty , ::boost::int64_t>
	{
		typedef ::boost::int64_t value_type;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			value_type write_value[2]={0};
			int write_bytes = 0;
			::boost::uint8_t * ptr = (::boost::uint8_t *)write_value;
			::boost::uint8_t tag = static_cast< ::boost::uint8_t>(value);
			write_bytes = 1;
			if(0 <= value && value < const_tag_as_type)
			{
				write_value[0] = tag;
			}
			else
			{
				::boost::uint8_t negative_bit = 0;
				value_type temp = value;
				if(value < 0)
				{
					negative_bit = const_negative_bit_value;
					temp = -value ;
				}
				write_value[1] = host_to_little_endian64(temp);
				ptr = (::boost::uint8_t *)(&write_value[1]) - 1;
				if(temp < 0x100)
				{
					write_bytes = 2;
				}
				else if(temp < 0x10000)
				{
					write_bytes = 3;
				}
				else if(temp < 0x1000000)
				{
					write_bytes = 4;
				}
				else if(temp < 0x100000000)
				{
					write_bytes = 5;
				}
				else if(temp < 0x10000000000LL)
				{
					write_bytes = 6;
				}
				else if(temp < 0x1000000000000LL)
				{
					write_bytes = 7;
				}
				else if(temp < 0x100000000000000LL)
				{
					write_bytes = 8;
				}
				else
				{
					write_bytes = 9;
				}
				*ptr = const_store_postive_integer_byte_mask + negative_bit + write_bytes;
			}
			store_data.write((const char *)ptr,write_bytes);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};


	template<typename store_ty,int tag>
	struct value_read_support<store_ty,::boost::uint8_t,tag>
	{
		typedef value_default_read_support_impl<store_ty,::boost::uint8_t> impl_type;
	};

	template<typename store_ty ,int tag>
	struct value_write_support<store_ty,::boost::uint8_t,tag>
	{
		typedef value_default_write_support_impl<store_ty,::boost::uint8_t> impl_type;
	};

	template<typename store_ty,int tag>
	struct value_read_support<store_ty,::boost::int8_t,tag>
	{
		typedef value_default_read_char_like_support_impl<store_ty,::boost::int8_t> impl_type;
	};

	template<typename store_ty ,int tag>
	struct value_write_support<store_ty,::boost::int8_t,tag>
	{
		typedef value_default_write_char_like_support_impl<store_ty,::boost::int8_t> impl_type;
	};

	template<typename store_ty,int tag>
	struct value_read_support<store_ty,char,tag>
	{
		typedef value_default_read_char_like_support_impl<store_ty,char> impl_type;
	};

	template<typename store_ty ,int tag>
	struct value_write_support<store_ty,char,tag>
	{
		typedef value_default_write_char_like_support_impl<store_ty,char> impl_type;
	};

  template<typename store_ty,int tag>
	struct value_read_support<store_ty,bool,tag>
	{
		typedef value_default_read_support_impl<store_ty,bool> impl_type;
	};

	template<typename store_ty ,int tag>
	struct value_write_support<store_ty,bool,tag>
	{
		typedef value_default_write_support_impl<store_ty,bool> impl_type;
	};

	template<typename store_ty,int tag>
	struct value_read_support<store_ty,::boost::uint16_t,tag>
	{
		typedef value_default_read_support_impl<store_ty,::boost::uint16_t> impl_type;
	};

	template<typename store_ty ,int tag>
	struct value_write_support<store_ty,::boost::uint16_t,tag>
	{
		typedef value_default_write_support_impl<store_ty,::boost::uint16_t> impl_type;
	};

	template<typename store_ty,int tag>
	struct value_read_support<store_ty,::boost::int16_t,tag>
	{
		typedef value_default_read_support_impl<store_ty,::boost::int16_t> impl_type;
	};

	template<typename store_ty ,int tag>
	struct value_write_support<store_ty,::boost::int16_t,tag>
	{
		typedef value_default_write_support_impl<store_ty,::boost::int16_t> impl_type;
	};

	template<typename store_ty,int tag>
	struct value_read_support<store_ty,::boost::uint32_t,tag>
	{
		typedef value_default_read_support_impl<store_ty,::boost::uint32_t> impl_type;
	};

	template<typename store_ty ,int tag>
	struct value_write_support<store_ty,::boost::uint32_t,tag>
	{
		typedef value_default_write_support_impl<store_ty,::boost::uint32_t> impl_type;
	};

	template<typename store_ty,int tag>
	struct value_read_support<store_ty,::boost::int32_t,tag>
	{
		typedef value_default_read_support_impl<store_ty,::boost::int32_t> impl_type;
	};

	template<typename store_ty ,int tag>
	struct value_write_support<store_ty,::boost::int32_t,tag>
	{
		typedef value_default_write_support_impl<store_ty,::boost::int32_t> impl_type;
	};

	template<typename store_ty,int tag>
	struct value_read_support<store_ty,::boost::uint64_t,tag>
	{
		typedef value_default_read_support_impl<store_ty,::boost::uint64_t> impl_type;
	};

	template<typename store_ty ,int tag>
	struct value_write_support<store_ty,::boost::uint64_t,tag>
	{
		typedef value_default_write_support_impl<store_ty,::boost::uint64_t> impl_type;
	};

	template<typename store_ty,int tag>
	struct value_read_support<store_ty,::boost::int64_t,tag>
	{
		typedef value_default_read_support_impl<store_ty,::boost::int64_t> impl_type;
	};

	template<typename store_ty ,int tag>
	struct value_write_support<store_ty,::boost::int64_t,tag>
	{
		typedef value_default_write_support_impl<store_ty,::boost::int64_t> impl_type;
	};

	template<typename store_ty>
	struct value_read_support_impl<store_ty , float , 0>
	{
		typedef float value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			store_data.read((char*)&value,sizeof(value_type));
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<typename store_ty>
	struct value_write_support_impl<store_ty , float , 0>
	{
		typedef float value_type;
		static AMSG_INLINE void write(store_ty& store_data,const value_type& value)
		{
			store_data.write((const char*)&value,sizeof(value_type));
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<typename store_ty>
	struct value_read_support_impl<store_ty , double , 0>
	{
		typedef double value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			store_data.read((char*)&value,sizeof(value_type));
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<typename store_ty>
	struct value_write_support_impl<store_ty , double , 0>
	{
		typedef double value_type;
		static AMSG_INLINE void write(store_ty& store_data,const value_type& value)
		{
			store_data.write((const char*)&value,sizeof(value_type));
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

  struct class_t{};

  template< ::std::size_t size>
  struct fix_bytes_t{};

  struct string_t{};

  template<typename value_type>
  struct list_t{};

  template<typename key_type,typename value_type>
  struct map_t{};

  template<typename value_type>
  struct del_t{};

  typedef del_t<class_t> del_class_t;
  typedef del_t<int> del_int_t;
  typedef del_t<int> del_enum_t;
  typedef del_t<fix_bytes_t<1> > del_fix_int8_t;
  typedef del_t<fix_bytes_t<2> > del_fix_int16_t;
  typedef del_t<fix_bytes_t<4> > del_fix_int32_t;
  typedef del_t<fix_bytes_t<8> > del_fix_int64_t;
  typedef del_t<fix_bytes_t<4> > del_float_t;
  typedef del_t<fix_bytes_t<8> > del_double_t;
  typedef del_t<string_t> del_string_t;

  template<typename store_ty,int tag>
  struct value_read_support_impl<store_ty, del_class_t, tag>
  {
    static AMSG_INLINE void read(store_ty& store_data, const del_class_t&)
    {
      ::boost::uint32_t len_tag;
      ::std::size_t offset = store_data.read_length();
      value_read_support<store_ty, uint32_t, 0>::impl_type::read(store_data, len_tag);
      if (store_data.bad())
      {
        store_data.set_error_code(stream_buffer_overflow);
        return;
      }
      if (len_tag >= 0)
      {
      ::std::size_t read_len = store_data.read_length() - offset;
      ::std::size_t len = (::std::size_t)len_tag;
      if (len > read_len) store_data.skip_read(len - read_len);
      }
    }
  };

  template<typename store_ty,int tag>
  struct value_read_support_impl<store_ty, del_int_t, tag>
  {
    static AMSG_INLINE void read(store_ty& store_data, const del_int_t&)
    {
      ::boost::uint8_t vtag;
      store_data.read((char*)&vtag, 1);
      if (store_data.bad())
      {
        store_data.set_error_code(stream_buffer_overflow);
        return;
      }
      if (vtag > const_tag_as_value)
      {
        int read_bytes = int(vtag & const_interger_byte_msak) + 1;
        store_data.skip_read(read_bytes);
        if (store_data.bad())
        {
          store_data.set_error_code(stream_buffer_overflow);
          return;
        }
      }
    }
  };

  template<typename store_ty , ::std::size_t size , int tag>
  struct value_read_support_impl < store_ty, fix_bytes_t<size>, tag >
  {
    static AMSG_INLINE void read(store_ty& store_data, const fix_bytes_t<size>&)
    {
      store_data.skip_read(size);
      if (store_data.bad())
      {
        store_data.set_error_code(stream_buffer_overflow);
        return;
      }
    }
  };

  template<typename store_ty , int tag>
  struct value_read_support_impl < store_ty, del_string_t, tag >
  {
    static AMSG_INLINE void read(store_ty& store_data, const del_string_t&)
    {
      ::boost::uint32_t len_tag;
      value_read_support<store_ty, uint32_t, 0>::impl_type::read(store_data, len_tag);
      if (store_data.bad())
      {
        store_data.set_error_code(stream_buffer_overflow);
        return;
      }
      store_data.skip_read(len_tag);
      if (store_data.bad())
      {
        store_data.set_error_code(stream_buffer_overflow);
        return;
      }
    }
  };

  template<typename store_ty , typename value_type, int tag>
  struct value_read_support_impl < store_ty, del_t<list_t<value_type> >, tag >
  {
    static AMSG_INLINE void read(store_ty& store_data, const del_t<list_t<value_type> >&)
    {
      ::boost::uint32_t len_tag;
      value_read_support<store_ty, uint32_t, 0>::impl_type::read(store_data, len_tag);
      if (store_data.bad())
      {
        store_data.set_error_code(stream_buffer_overflow);
        return;
      }
      value_type v;
      for (::boost::uint32_t i = 0; i < len_tag; ++i)
      {
        value_read_support<store_ty, value_type, tag>::impl_type::read(store_data, v);
      }
    }
  };

  template<typename store_ty, typename key_type , typename value_type, int tag>
  struct value_read_support_impl < store_ty, del_t<map_t<key_type,value_type> >, tag >
  {
    static AMSG_INLINE void read(store_ty& store_data, const del_t<map_t<key_type, value_type> >&)
    {
      ::boost::uint32_t len_tag;
      value_read_support<store_ty, uint32_t, 0>::impl_type::read(store_data, len_tag);
      if (store_data.bad())
      {
        store_data.set_error_code(stream_buffer_overflow);
        return;
      }
      key_type k;
      value_type v;
      for (::boost::uint32_t i = 0; i < len_tag; ++i)
      {
        value_read_support<store_ty, key_type, tag>::impl_type::read(store_data, k);
        value_read_support<store_ty, value_type, tag>::impl_type::read(store_data, v);
      }
    }
  };

  template<typename store_ty, typename value_type , int tag>
  struct value_write_support_impl < store_ty, del_t<value_type>, tag >
  {
    static AMSG_INLINE void write(store_ty& , const del_t<value_type>&)
    {}
  };


	static size_t to_str( const ::boost::uint32_t& Value , char * resultbuffer , size_t len)
	{
		::boost::uint32_t temp = Value;
		resultbuffer[len-1] = 0;
		size_t pos = len-2;
		if(temp ==0)
		{
			resultbuffer[pos--]='0';
		}
		while( temp )
		{
			resultbuffer[pos--]	= (char)((temp%10)+'0');
			temp = temp/10;
		}
		++pos;
		memmove( resultbuffer,resultbuffer+pos,(len-pos));
		return len - pos;
	}

	template<typename store_ty,typename string_ty , int tag>
	struct value_string_read_support_impl
	{
		typedef string_ty value_type;
		static void read(store_ty& store_data, value_type& value,::std::size_t max=0)
		{
			::boost::uint32_t len;
			value_read_support<store_ty,::boost::uint32_t,0>::impl_type::read(store_data,len);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			if (max > 0 && max < len)
			{
				store_data.set_error_code(sequence_length_overflow);
				return;
			}
			value.resize(len);
			store_data.read((char*)value.data(),len);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<typename store_ty,typename string_ty , int tag>
	struct value_string_write_support_impl
	{
		typedef string_ty value_type;
		static void write(store_ty& store_data, const value_type& value,::std::size_t max=0)
		{
      ::boost::uint32_t len = (::boost::uint32_t)value.length();
			if (max > 0 && max < len)
			{
				store_data.set_error_code(sequence_length_overflow);
				return;
			}
			value_write_support<store_ty,::std::size_t,0>::impl_type::write(store_data,len);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			store_data.write(value.data(),len);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};


	template<typename store_ty,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::std::basic_string<char, ::std::char_traits<char>, alloc_ty> , tag>
	{
		typedef ::std::basic_string<char, ::std::char_traits<char>, alloc_ty> value_type;
		typedef value_string_read_support_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::std::basic_string<char, ::std::char_traits<char>, alloc_ty> , tag>
	{
		typedef ::std::basic_string<char, ::std::char_traits<char>, alloc_ty> value_type;
		typedef value_string_write_support_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty,typename wstring_ty , int tag>
	struct value_wstring_read_support_impl
	{
		typedef wstring_ty value_type;
		static void read(store_ty& store_data, value_type& value,::std::size_t max=0)
		{
			::boost::uint32_t len;
			value_read_support<store_ty,::boost::uint32_t,0>::impl_type::read(store_data,len);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			if (max > 0 && max < len)
			{
				store_data.set_error_code(sequence_length_overflow);
				return;
			}
			value.resize(len);
			wchar_t data;
			for(::boost::uint32_t i = 0 ; i< len ; ++i)
			{
				value_read_support<store_ty,wchar_t,0>::impl_type::read(store_data,value[i]);
				if(store_data.error())
				{
					char buffer[64];
					to_str(i,buffer,64);
					store_data.append_debug_info("[");
					store_data.append_debug_info(buffer);
					store_data.append_debug_info("]");
					return;
				}
				value.append(1,data);
			}
		}
	};

	template<typename store_ty,typename wstring_ty , int tag>
	struct value_wstring_write_support_impl
	{
		typedef wstring_ty value_type;
		static void write(store_ty& store_data, const value_type& value,::std::size_t max=0)
		{
      ::boost::uint32_t len = (::boost::uint32_t)value.length();
			if (max > 0 && max < len)
			{
				store_data.set_error_code(sequence_length_overflow);
				return;
			}
			value_write_support<store_ty,::boost::uint32_t,0>::impl_type::write(store_data,len);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			for(::boost::uint32_t i = 0 ; i< len ; ++i)
			{
				value_write_support<store_ty,wchar_t,0>::impl_type::write(store_data,value[i]);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
			}
		}
	};

	template<typename store_ty,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::std::basic_string<wchar_t, ::std::char_traits<wchar_t>, alloc_ty> , tag>
	{
		typedef ::std::basic_string<wchar_t, ::std::char_traits<wchar_t>, alloc_ty> value_type;
		typedef value_string_read_support_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::std::basic_string<wchar_t, ::std::char_traits<wchar_t>, alloc_ty> , tag>
	{
		typedef ::std::basic_string<wchar_t, ::std::char_traits<wchar_t>, alloc_ty> value_type;
		typedef value_string_write_support_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename ty , int tag>
	struct value_support_read_seq_container_impl
	{
		typedef ty value_type;
		static void read(store_ty& store_data, value_type& value,std::size_t max=0)\
		{
			::boost::uint32_t len;
			value_read_support<store_ty,::boost::uint32_t,tag>::impl_type::read(store_data,len);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			if (max > 0 && max < len)
			{
				store_data.set_error_code(sequence_length_overflow);
				return;
			}
			value.resize(len);
			::boost::uint32_t c = 0;
			for( typename value_type::iterator i = value.begin() ; i != value.end(); ++i,++c )
			{
				typename value_type::value_type& elem_value = *i;
				value_read_support<store_ty,typename value_type::value_type,tag>::impl_type::read(store_data,elem_value);
				if(store_data.error())
				{
					char buffer[64];
					to_str(c,buffer,64);
					store_data.append_debug_info("[");
					store_data.append_debug_info(buffer);
					store_data.append_debug_info("]");
					return;
				}
			}
		}
	};

	template<typename store_ty , typename ty , int tag>
	struct value_support_write_seq_container_impl
	{
		typedef ty value_type;
		static void write(store_ty& store_data, const value_type& value,std::size_t max=0)
		{
      ::boost::uint32_t len = (::boost::uint32_t)value.size();
			if (max > 0 && max < len)
			{
				store_data.set_error_code(sequence_length_overflow);
				return;
			}
			value_write_support<store_ty,::std::size_t,tag>::impl_type::write(store_data,len);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			for( typename value_type::const_iterator i = value.begin() ; i != value.end(); ++i )
			{
				const typename value_type::value_type& elem_value = *i;
				value_write_support<store_ty,typename value_type::value_type,tag>::impl_type::write(store_data,elem_value);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
			}
		}
	};

	template<typename store_ty , typename ty,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::std::deque<ty,alloc_ty> , tag>
	{
		typedef typename ::std::deque<ty,alloc_ty> value_type;
		typedef value_support_read_seq_container_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename ty,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::std::deque<ty,alloc_ty> , tag>
	{
		typedef typename ::std::deque<ty,alloc_ty> value_type;
		typedef value_support_write_seq_container_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename ty,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::std::list<ty,alloc_ty> , tag>
	{
		typedef typename ::std::list<ty,alloc_ty> value_type;
		typedef value_support_read_seq_container_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename ty,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::std::list<ty,alloc_ty> , tag>
	{
		typedef typename ::std::list<ty,alloc_ty> value_type;
		typedef value_support_write_seq_container_impl<store_ty,value_type,tag> impl_type;
	};

#if defined(AMSG_STD_CXX11) || defined(AMSG_HAS_STD_FORWARD_LIST)
  template<typename store_ty , typename ty,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::std::forward_list<ty,alloc_ty> , tag>
	{
		typedef typename ::std::forward_list<ty,alloc_ty> value_type;
		typedef value_support_read_seq_container_impl<store_ty,value_type,tag> impl_type;
	};

  template<typename store_ty , typename ty , int tag>
	struct value_support_write_forward_list_impl
	{
		typedef ty value_type;
		static void write(store_ty& store_data, const value_type& value,std::size_t max=0)
		{
      ::boost::uint32_t len = (::boost::uint32_t)std::distance(value.begin(), value.end());
			if (max > 0 && max < len)
			{
				store_data.set_error_code(sequence_length_overflow);
				return;
			}
			value_write_support<store_ty,::std::size_t,tag>::impl_type::write(store_data,len);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			for( typename value_type::const_iterator i = value.begin() ; i != value.end(); ++i )
			{
				const typename value_type::value_type& elem_value = *i;
				value_write_support<store_ty,typename value_type::value_type,tag>::impl_type::write(store_data,elem_value);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
			}
		}
	};

	template<typename store_ty , typename ty,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::std::forward_list<ty,alloc_ty> , tag>
	{
		typedef typename ::std::forward_list<ty,alloc_ty> value_type;
		typedef value_support_write_forward_list_impl<store_ty,value_type,tag> impl_type;
	};
#endif

	template<typename store_ty , typename ty , int tag>
	struct value_support_read_array_impl
	{
		typedef ty value_type;
		static void read(store_ty& store_data, value_type& value,std::size_t max=0)\
		{
			::boost::uint32_t len;
			value_read_support<store_ty,::boost::uint32_t,tag>::impl_type::read(store_data,len);
			if (max > 0 && max < len)
			{
				store_data.set_error_code(sequence_length_overflow);
				return;
			}
			if( len > value.size() )
			{
				store_data.set_error_code(number_of_element_not_macth);
				return;
			}
			for( ::boost::uint32_t i = 0 ; i < value.size(); ++i)
			{
				typename value_type::value_type& elem_value = value[i];
				if(i < len)
				{
					value_read_support<store_ty,typename value_type::value_type,tag>::impl_type::read(store_data,elem_value);
					if(store_data.error())
					{
						char buffer[64];
						to_str(i,buffer,64);
						store_data.append_debug_info("[");
						store_data.append_debug_info(buffer);
						store_data.append_debug_info("]");
						return;
					}
				}
				else
				{
					elem_value = typename ty::value_type ();
				}
			}
		}
	};

	template<typename store_ty , typename ty, int tag>
	struct value_support_write_array_impl
	{
		typedef ty value_type;
		static void write(store_ty& store_data, const value_type& value,std::size_t max=0)
		{
      ::boost::uint32_t len = (::boost::uint32_t)value.size();
			if (max > 0 && max < len)
			{
				store_data.set_error_code(sequence_length_overflow);
				return;
			}
			value_write_support<store_ty,::boost::uint32_t,tag>::impl_type::write(store_data,len);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			for( ::boost::uint32_t i = 0 ; i < value.size(); ++i)
			{
				const typename value_type::value_type& elem_value = value[i];
				value_write_support<store_ty,typename value_type::value_type,tag>::impl_type::write(store_data,elem_value);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
			}
		}
	};

	template<typename store_ty , typename ty,std::size_t array_size , int tag>
	struct value_read_support<store_ty , ::boost::array<ty,array_size> , tag>
	{
		typedef typename ::boost::array<ty,array_size> value_type;
		typedef value_support_read_array_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename ty,std::size_t array_size , int tag>
	struct value_write_support<store_ty , ::boost::array<ty,array_size> , tag>
	{
		typedef typename ::boost::array<ty,array_size> value_type;
		typedef value_support_write_array_impl<store_ty,value_type,tag> impl_type;
	};

#if defined(AMSG_STD_CXX11)||defined(AMSG_HAS_STD_ARRAY)
	template<typename store_ty , typename ty,std::size_t array_size , int tag>
	struct value_read_support<store_ty , ::std::array<ty,array_size> , tag>
	{
		typedef typename ::std::array<ty,array_size> value_type;
		typedef value_support_read_array_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename ty,std::size_t array_size , int tag>
	struct value_write_support<store_ty , ::std::array<ty,array_size> , tag>
	{
		typedef typename ::std::array<ty,array_size> value_type;
		typedef value_support_write_array_impl<store_ty,value_type,tag> impl_type;
	};
#endif

	template<typename store_ty , typename ty,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::std::vector<ty,alloc_ty> , tag>
	{
		typedef typename ::std::vector<ty,alloc_ty> value_type;
		typedef value_support_read_seq_container_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename ty,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::std::vector<ty,alloc_ty> , tag>
	{
		typedef typename ::std::vector<ty,alloc_ty> value_type;
		typedef value_support_write_seq_container_impl<store_ty,value_type,tag> impl_type;
	};


	template<typename store_ty , typename ty , int tag>
	struct value_support_read_unorder_container_impl
	{
		typedef ty value_type;
		static void read(store_ty& store_data, value_type& value,std::size_t max=0)
		{
			::boost::uint32_t len;
			value_read_support<store_ty,::boost::uint32_t,tag>::impl_type::read(store_data,len);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			if (max > 0 && max < len)
			{
				store_data.set_error_code(sequence_length_overflow);
				return;
			}
			for( ::boost::uint32_t i = 0; i < len ; ++i)
			{
				typedef typename ::boost::remove_const<typename value_type::value_type::first_type>::type first_type;
				typedef typename ::boost::remove_const<typename value_type::value_type::second_type>::type second_type;
				first_type first;
				second_type second;
				value_read_support<store_ty,first_type,tag>::impl_type::read(store_data,first);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				if(store_data.error())
				{
					char buffer[64];
					to_str(i,buffer,64);
					store_data.append_debug_info("[key:");
					store_data.append_debug_info(buffer);
					store_data.append_debug_info("]");
					return;
				}
				value_read_support<store_ty,second_type,tag>::impl_type::read(store_data,second);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				if(store_data.error())
				{
					char buffer[64];
					to_str(i,buffer,64);
					store_data.append_debug_info("[value:");
					store_data.append_debug_info(buffer);
					store_data.append_debug_info("]");
					return;
				}
				value.insert(std::make_pair(first,second));
			}
		}
	};

	template<typename store_ty , typename ty , int tag>
	struct value_support_write_unorder_container_impl
	{
		typedef ty value_type;
		static void write(store_ty& store_data, const value_type& value,std::size_t max=0)
		{
			::boost::uint32_t len = (::boost::uint32_t)value.size();
			if (max > 0 && max < len)
			{
				store_data.set_error_code(sequence_length_overflow);
				return;
			}
			value_write_support<store_ty,::boost::uint32_t,tag>::impl_type::write(store_data,len);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			for( typename value_type::const_iterator i = value.begin() ; i != value.end(); ++i )
			{
				typedef typename ::boost::remove_const<typename value_type::value_type::first_type>::type first_type;
				typedef typename ::boost::remove_const<typename value_type::value_type::second_type>::type second_type;
				value_write_support<store_ty,first_type,tag>::impl_type::write(store_data,i->first);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				value_write_support<store_ty,second_type,tag>::impl_type::write(store_data,i->second);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
			}
		}
	};

	template<typename store_ty , typename key_ty , typename ty , typename cmp_ty ,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::std::map<key_ty,ty,cmp_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::map<key_ty,ty,cmp_ty,alloc_ty> value_type;
		typedef value_support_read_unorder_container_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename key_ty , typename ty , typename cmp_ty ,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::std::map<key_ty,ty,cmp_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::map<key_ty,ty,cmp_ty,alloc_ty> value_type;
		typedef value_support_write_unorder_container_impl<store_ty,value_type,tag> impl_type;
	};

  template<typename store_ty , typename key_ty , typename ty , typename cmp_ty ,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::std::multimap<key_ty,ty,cmp_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::multimap<key_ty,ty,cmp_ty,alloc_ty> value_type;
		typedef value_support_read_unorder_container_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename key_ty , typename ty , typename cmp_ty ,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::std::multimap<key_ty,ty,cmp_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::multimap<key_ty,ty,cmp_ty,alloc_ty> value_type;
		typedef value_support_write_unorder_container_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename key_ty , typename ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::boost::unordered_map<key_ty,ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::boost::unordered_map<key_ty,ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef value_support_read_unorder_container_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename key_ty , typename ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::boost::unordered_map<key_ty,ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::boost::unordered_map<key_ty,ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef value_support_write_unorder_container_impl<store_ty,value_type,tag> impl_type;
	};

#if defined(AMSG_STD_CXX11) || defined(AMSG_HAS_STD_UNORDERED_MAP)
  template<typename store_ty , typename key_ty , typename ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::std::unordered_map<key_ty,ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::unordered_map<key_ty,ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef value_support_read_unorder_container_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename key_ty , typename ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::std::unordered_map<key_ty,ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::unordered_map<key_ty,ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef value_support_write_unorder_container_impl<store_ty,value_type,tag> impl_type;
	};

  template<typename store_ty , typename key_ty , typename ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::std::unordered_multimap<key_ty,ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::unordered_multimap<key_ty,ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef value_support_read_unorder_container_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename key_ty , typename ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::std::unordered_multimap<key_ty,ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::unordered_multimap<key_ty,ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef value_support_write_unorder_container_impl<store_ty,value_type,tag> impl_type;
	};
#endif


  template<typename store_ty , typename ty , int tag>
	struct value_support_read_unorder_set_impl
	{
		typedef ty value_type;
		static void read(store_ty& store_data, value_type& value,std::size_t max=0)
		{
			::boost::uint32_t len;
			value_read_support<store_ty,::boost::uint32_t,tag>::impl_type::read(store_data,len);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			if (max > 0 && max < len)
			{
				store_data.set_error_code(sequence_length_overflow);
				return;
			}
			for( ::boost::uint32_t i = 0; i < len ; ++i)
			{
				typedef typename ::boost::remove_const<typename value_type::value_type>::type val_type;
				val_type val;
				value_read_support<store_ty,val_type,tag>::impl_type::read(store_data,val);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				if(store_data.error())
				{
					char buffer[64];
					to_str(i,buffer,64);
					store_data.append_debug_info("[key:");
					store_data.append_debug_info(buffer);
					store_data.append_debug_info("]");
					return;
				}
				value.insert(val);
			}
		}
	};

	template<typename store_ty , typename ty , int tag>
	struct value_support_write_unorder_set_impl
	{
		typedef ty value_type;
		static void write(store_ty& store_data, const value_type& value,std::size_t max=0)
		{
			::boost::uint32_t len = (::boost::uint32_t)value.size();
			if (max > 0 && max < len)
			{
				store_data.set_error_code(sequence_length_overflow);
				return;
			}
			value_write_support<store_ty,::boost::uint32_t,tag>::impl_type::write(store_data,len);
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			for( typename value_type::const_iterator i = value.begin() ; i != value.end(); ++i )
			{
				typedef typename ::boost::remove_const<typename value_type::value_type>::type val_type;
				value_write_support<store_ty,val_type,tag>::impl_type::write(store_data,*i);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
			}
		}
	};

  template<typename store_ty , typename key_ty , typename cmp_ty ,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::std::set<key_ty,cmp_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::set<key_ty,cmp_ty,alloc_ty> value_type;
		typedef value_support_read_unorder_set_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename key_ty , typename cmp_ty ,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::std::set<key_ty,cmp_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::set<key_ty,cmp_ty,alloc_ty> value_type;
		typedef value_support_write_unorder_set_impl<store_ty,value_type,tag> impl_type;
	};

  template<typename store_ty , typename key_ty , typename cmp_ty ,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::std::multiset<key_ty,cmp_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::multiset<key_ty,cmp_ty,alloc_ty> value_type;
		typedef value_support_read_unorder_set_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename key_ty , typename cmp_ty ,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::std::multiset<key_ty,cmp_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::multiset<key_ty,cmp_ty,alloc_ty> value_type;
		typedef value_support_write_unorder_set_impl<store_ty,value_type,tag> impl_type;
	};

  template<typename store_ty , typename key_ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::boost::unordered_set<key_ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::boost::unordered_set<key_ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef value_support_read_unorder_set_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename key_ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::boost::unordered_set<key_ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::boost::unordered_set<key_ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef value_support_write_unorder_set_impl<store_ty,value_type,tag> impl_type;
	};

#if defined(AMSG_STD_CXX11) || defined(AMSG_HAS_STD_UNORDERED_SET)
  template<typename store_ty , typename key_ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::std::unordered_set<key_ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::unordered_set<key_ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef value_support_read_unorder_set_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename key_ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::std::unordered_set<key_ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::unordered_set<key_ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef value_support_write_unorder_set_impl<store_ty,value_type,tag> impl_type;
	};

  template<typename store_ty , typename key_ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct value_read_support<store_ty , ::std::unordered_multiset<key_ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::unordered_multiset<key_ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef value_support_read_unorder_set_impl<store_ty,value_type,tag> impl_type;
	};

	template<typename store_ty , typename key_ty , typename hash_ty ,typename pred_ty ,typename alloc_ty , int tag>
	struct value_write_support<store_ty , ::std::unordered_multiset<key_ty,hash_ty,pred_ty,alloc_ty> ,tag>
	{
		typedef typename ::std::unordered_multiset<key_ty,hash_ty,pred_ty,alloc_ty> value_type;
		typedef value_support_write_unorder_set_impl<store_ty,value_type,tag> impl_type;
	};
#endif


	template <typename ty>
	struct smax_valid
	{
		typedef ty value_type;
		::std::size_t size;
		value_type& obj;
		smax_valid(::std::size_t max,value_type& value)
			:size(max),obj(value)
		{}
		smax_valid(const smax_valid& rv)
			:size(rv.size),obj(rv.obj)
		{}

		smax_valid& operator = (const smax_valid& rv)
		{
			size = rv.size;
			obj = rv.obj;
		}
	};

	struct smax
	{
		::std::size_t size;
		smax(::std::size_t max=0)
			:size(max)
		{}
	};

	template<typename ty>
	AMSG_INLINE smax_valid<ty> operator & (ty& value,const smax& sm)
	{
		smax_valid<ty> valid(sm.size,value);
		return valid;
	}

	template <typename store_ty , typename ty , int tag>
	struct value_read_support_impl<store_ty,smax_valid<ty>,tag>
	{
		typedef smax_valid<ty> value_type;
		static AMSG_INLINE void read(store_ty& store_data,const value_type& value)
		{
			typedef typename ::boost::remove_const<ty>::type ref_type;
			value_read_support<store_ty,ref_type,tag>::impl_type::read(store_data,value.obj,value.size);
		}
	};

	template <typename store_ty , typename ty , int tag>
	struct value_write_support_impl<store_ty,smax_valid<ty>,tag>
	{
		typedef smax_valid<ty> value_type;
		static AMSG_INLINE void write(store_ty& store_data,const value_type& value)
		{
			typedef typename ::boost::remove_const<ty>::type ref_type;
			value_write_support<store_ty,ref_type,tag>::impl_type::write(store_data,value.obj,value.size);
		}
	};

	template <typename ty , int tag>
	struct byte_size_of_impl<smax_valid<ty>,tag>
	{
		typedef smax_valid<ty> value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type& value)
		{
			typedef typename ::boost::remove_const<ty>::type ref_type;
			return byte_size_of<ref_type,tag>::impl_type::size(value.obj , value.size);
		}
	};


	template <typename ty>
	struct sfix_op
	{
		typedef ty value_type;
		value_type& obj;
		sfix_op(value_type& value)
			:obj(value)
		{}
		sfix_op(const sfix_op& rv)
			:obj(rv.obj)
		{}

		sfix_op& operator = (const sfix_op& rv)
		{
			obj = rv.obj;
		}
	};

	struct sfix_def{};

	namespace
	{
		static sfix_def sfix = sfix_def();
	}

	template<typename ty>
	AMSG_INLINE sfix_op<ty> operator & (ty& value,const sfix_def&)
	{
		sfix_op<ty> op(value);
		return op;
	}

	template <typename store_ty , typename ty , int tag>
	struct value_read_support_impl<store_ty,sfix_op<ty>,tag>
	{
		typedef sfix_op<ty> value_type;
		static AMSG_INLINE void read(store_ty& store_data,const value_type& value)
		{
			typedef typename ::boost::remove_const<ty>::type ref_type;
			value_fix_size_read_support_impl<store_ty,ref_type>::read(store_data,value.obj);
		}
	};

	template <typename store_ty , typename ty , int tag>
	struct value_write_support_impl<store_ty,sfix_op<ty>,tag>
	{
		typedef sfix_op<ty> value_type;
		static AMSG_INLINE void write(store_ty& store_data,const value_type& value)
		{
			typedef typename ::boost::remove_const<ty>::type ref_type;
			value_fix_size_write_support_impl<store_ty,ref_type>::write(store_data,value.obj);
		}
	};

	template <typename ty , int tag>
	struct byte_size_of_impl<sfix_op<ty>,tag>
	{
		typedef sfix_op<ty> value_type;

		static AMSG_INLINE ::boost::uint32_t size(const value_type&)
		{
			typedef typename ::boost::remove_const<ty>::type ref_type;
			return sizeof(ref_type);
		}
	};

  template <typename ty, int tag>
  struct byte_size_of_impl < del_t<ty>, tag >
  {
    typedef del_t<ty> value_type;

    static AMSG_INLINE::boost::uint32_t size(const value_type&)
    {
      return 0;
    }
  };

  template <typename ty>
  struct can_skip_opt
  {
    static AMSG_INLINE bool check(const ty&)
    {
      return false;
    }
  };

  template<typename alloc_ty>
  struct can_skip_opt< ::std::basic_string<char, ::std::char_traits<char>, alloc_ty> >
  {
    typedef ::std::basic_string<char, ::std::char_traits<char>, alloc_ty> value_type;
    static AMSG_INLINE bool check(const value_type& value)
    {
      return value.empty();
    }
  };

  template<typename ty, typename alloc_ty>
  struct can_skip_opt < ::std::list<ty, alloc_ty> >
  {
    typedef ::std::list<ty, alloc_ty> value_type;
    static AMSG_INLINE bool check(const value_type& value)
    {
      return value.empty();
    }
  };

  template<typename ty, typename alloc_ty>
  struct can_skip_opt < ::std::deque<ty, alloc_ty> >
  {
    typedef ::std::deque<ty, alloc_ty> value_type;
    static AMSG_INLINE bool check(const value_type& value)
    {
      return value.empty();
    }
  };

  template<typename ty, typename alloc_ty>
  struct can_skip_opt < ::std::vector<ty, alloc_ty> >
  {
    typedef ::std::vector<ty, alloc_ty> value_type;
    static AMSG_INLINE bool check(const value_type& value)
    {
      return value.empty();
    }
  };

#if defined(AMSG_STD_CXX11) || defined(AMSG_HAS_STD_FORWARD_LIST)
  template<typename ty, typename alloc_ty>
  struct can_skip_opt < ::std::forward_list<ty, alloc_ty> >
  {
    typedef ::std::forward_list<ty, alloc_ty> value_type;
    static AMSG_INLINE bool check(const value_type& value)
    {
      return value.empty();
    }
  };
#endif

  template<typename key_ty, typename ty, typename cmp_ty, typename alloc_ty>
  struct can_skip_opt < ::std::map<key_ty, ty, cmp_ty, alloc_ty> >
  {
    typedef typename ::std::map<key_ty, ty, cmp_ty, alloc_ty> value_type;
    static AMSG_INLINE bool check(const value_type& value)
    {
      return value.empty();
    }
  };

  template<typename key_ty, typename ty, typename cmp_ty, typename alloc_ty>
  struct can_skip_opt < ::std::multimap<key_ty, ty, cmp_ty, alloc_ty> >
  {
    typedef typename ::std::multimap<key_ty, ty, cmp_ty, alloc_ty> value_type;
    static AMSG_INLINE bool check(const value_type& value)
    {
      return value.empty();
    }
  };

  template<typename key_ty, typename ty, typename cmp_ty, typename alloc_ty>
  struct can_skip_opt < ::boost::unordered::unordered_map<key_ty, ty, cmp_ty, alloc_ty> >
  {
    typedef typename ::boost::unordered::unordered_map<key_ty, ty, cmp_ty, alloc_ty> value_type;
    static AMSG_INLINE bool check(const value_type& value)
    {
      return value.empty();
    }
  };

#if defined(AMSG_STD_CXX11) || defined(AMSG_HAS_STD_UNORDERED_MAP)
  template<typename key_ty, typename ty, typename cmp_ty, typename alloc_ty>
  struct can_skip_opt < ::std::unordered_map<key_ty, ty, cmp_ty, alloc_ty> >
  {
    typedef typename ::std::unordered_map<key_ty, ty, cmp_ty, alloc_ty> value_type;
    static AMSG_INLINE bool check(const value_type& value)
    {
      return value.empty();
    }
  };

  template<typename key_ty, typename ty, typename cmp_ty, typename alloc_ty>
  struct can_skip_opt < ::std::unordered_multimap<key_ty, ty, cmp_ty, alloc_ty> >
  {
    typedef typename ::std::unordered_multimap<key_ty, ty, cmp_ty, alloc_ty> value_type;
    static AMSG_INLINE bool check(const value_type& value)
    {
      return value.empty();
    }
  };

  template<typename ty>
  struct can_skip_opt <sfix_op<ty> >
  {
    static AMSG_INLINE bool check(const sfix_op<ty>& value)
    {
      return can_skip_opt<ty>::check(value.obj);
    }
  };

  template<typename ty>
  struct can_skip_opt < smax_valid<ty> >
  {
    static AMSG_INLINE bool check(const smax_valid<ty>& value)
    {
      return can_skip_opt<ty>::check(value.obj);
    }
  };

  template<typename ty>
  struct can_skip_opt < del_t<ty> >
  {
    static AMSG_INLINE bool check(const del_t<ty>&)
    {
      return true;
    }
  };


#endif
}

template <int tag , typename store_ty , typename ty>
AMSG_INLINE void read_x(store_ty& store_data, ty& value)
{
	::amsg::detail::value_read_support<store_ty,ty,tag>::impl_type::read(store_data,value);
}

template <int tag , typename store_ty , typename ty>
AMSG_INLINE void read_x(store_ty& store_data, const ty& value)
{
	::amsg::detail::value_read_support<store_ty,ty,tag>::impl_type::read(store_data,value);
}

template <int tag , typename store_ty , typename ty>
AMSG_INLINE void write_x(store_ty& store_data,const ty& value)
{
	::amsg::detail::value_write_support<store_ty,ty,tag>::impl_type::write(store_data,value);
}

template <typename store_ty , typename ty>
AMSG_INLINE void read(store_ty& store_data, ty& value)
{
	::amsg::detail::value_read_support<store_ty,ty,0>::impl_type::read(store_data,value);
}

template <typename store_ty , typename ty>
AMSG_INLINE void read(store_ty& store_data, const ty& value)
{
	::amsg::detail::value_read_support<store_ty,ty,0>::impl_type::read(store_data,value);
}

template <typename store_ty , typename ty>
AMSG_INLINE void write(store_ty& store_data,const ty& value)
{
	::amsg::detail::value_write_support<store_ty,ty,0>::impl_type::write(store_data,value);
}

template <int tag , typename ty>
AMSG_INLINE ::boost::uint32_t size_of_x(const ty& value)
{
	return ::amsg::detail::byte_size_of<ty,tag>::impl_type::size(value);
}

template <typename ty>
AMSG_INLINE ::boost::uint32_t size_of(const ty& value)
{
	return ::amsg::detail::byte_size_of<ty,0>::impl_type::size(value);
}

template <typename ty>
AMSG_INLINE bool can_skip(const ty& value)
{
  return ::amsg::detail::can_skip_opt<ty>::check(value);
}

}

#define AMSG_TAG_MEMBER_X( r ,v , elem ) \
  if(!can_skip(elem))	\
  {\
    wtag |= mask;\
  }\
  mask <<= 1;

#define AMSG_SIZE_MEMBER_X( r ,v , elem ) \
	if(!can_skip(elem))	\
  {\
    stag |= mask;\
    size += ::amsg::size_of_x<tag>(elem);\
  }\
  mask <<= 1;

#define AMSG_READ_MEMBER_X( r , v , elem ) \
	if(rtag&mask)\
  {\
    ::amsg::read_x<tag>(store_data,elem);\
	  if(store_data.error())\
    {\
	    store_data.append_debug_info(".");\
	    store_data.append_debug_info(BOOST_PP_STRINGIZE(elem));\
	    return;\
    }\
  }\
  mask <<= 1;

#define AMSG_WRITE_MEMBER_X( r ,v , elem ) \
	if(wtag&mask)\
  {\
    ::amsg::write_x<tag>(store_data,elem);\
	  if(store_data.error())\
    {\
	    store_data.append_debug_info(".");\
	    store_data.append_debug_info(BOOST_PP_STRINGIZE(elem));\
	    return;\
    }\
  }\
  mask <<= 1;

#define AMSG_X(TYPE, MEMBERS,X)\
	namespace amsg { namespace detail {\
	template<int tag>	\
  struct byte_size_of_impl<TYPE,tag>	\
  {\
	  typedef TYPE value_type;\
	  static AMSG_INLINE ::boost::uint32_t size(const value_type& v)\
    {\
      uint32_t size = 0;\
      uint64_t stag = 0;\
      uint64_t mask = 1;\
	    BOOST_PP_SEQ_FOR_EACH( AMSG_SIZE_MEMBER_X , v , MEMBERS ) \
      size += size_of(stag);\
      size += size_of(size + size_of(size));\
      return size;\
    }\
  };\
  template<typename store_ty,int tag>	\
  struct value_read_support_impl<store_ty,TYPE,tag>	\
  {\
	  typedef TYPE value_type;\
	  static AMSG_INLINE void read(store_ty& store_data, value_type& v)\
    {\
        ::std::size_t offset = store_data.read_length(); \
        uint32_t len_tag = 0; \
        uint64_t rtag = 0; \
        uint64_t mask = 1; \
        ::amsg::read_x<tag>(store_data, len_tag); \
        if (store_data.error()){ return; }\
        ::amsg::read_x<tag>(store_data, rtag); \
        if (store_data.error()){ return; }\
        BOOST_PP_SEQ_FOR_EACH(AMSG_READ_MEMBER_X, v, MEMBERS) \
        if (len_tag >= 0)\
        {\
          ::std::size_t read_len = store_data.read_length() - offset; \
          ::std::size_t len = (::std::size_t)len_tag; \
          if (len > read_len) store_data.skip_read(len - read_len); \
        }\
    }\
  };\
	template<typename store_ty,int tag>	\
  struct value_write_support_impl<store_ty,TYPE,tag>	\
  {\
	  typedef TYPE value_type;\
	  static AMSG_INLINE void write(store_ty& store_data, const value_type& v)\
    {\
      uint32_t size = size_of(v); \
      uint64_t wtag = 0; \
      uint64_t mask = 1; \
      BOOST_PP_SEQ_FOR_EACH(AMSG_TAG_MEMBER_X, v, MEMBERS) \
      ::amsg::write_x<tag>(store_data, size); \
      if (store_data.error()){ return; }\
      ::amsg::write_x<tag>(store_data, wtag); \
      if (store_data.error()){ return; }\
      mask = 1; \
      BOOST_PP_SEQ_FOR_EACH(AMSG_WRITE_MEMBER_X, v, MEMBERS) \
    }\
  };\
}}

#define AMSG(TYPE, MEMBERS) AMSG_X(TYPE, MEMBERS,0)

#define AMSGF_X(TYPE,X)	\
	template <typename store_ty,typename ty=TYPE,int tag = X> friend struct ::amsg::detail::value_read_support;\
	template <typename store_ty,typename ty=TYPE,int tag = X> friend struct ::amsg::detail::value_write_support;

#define  AMSGF(TYPE) AMSGF_X(TYPE,0)

#endif
