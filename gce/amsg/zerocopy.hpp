///
/// Copyright (c) 2012 Ning Ding (lordoffox@gmail.com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/lordoffox/amsg for latest version.
///

#ifndef AMSG_ZEROCOPY_HPP
#define AMSG_ZEROCOPY_HPP

#include "amsg.hpp"

namespace amsg
{
  struct zero_copy_buffer : public basic_store
  {
  private:
    std::string		m_error_info;

    unsigned char const* m_read_header_ptr;
    unsigned char* m_write_header_ptr;
    unsigned char const* m_read_ptr;
    unsigned char* m_write_ptr;
    unsigned char const* m_read_tail_ptr;
    unsigned char const* m_write_tail_ptr;
    int							m_status;

  public:
    enum { good , read_overflow , write_overflow };

    zero_copy_buffer()
      : m_read_header_ptr(0)
      , m_write_header_ptr(0)
      , m_read_ptr(0)
      , m_write_ptr(0)
      , m_read_tail_ptr(0)
      , m_write_tail_ptr(0)
      , m_status(good)
    {
    }

    ~zero_copy_buffer()
    {
    }

    AMSG_INLINE void set_read(unsigned char const* buffer, ::std::size_t length)
    {
      this->m_read_header_ptr = buffer;
      this->m_read_ptr = this->m_read_header_ptr;
      this->m_read_tail_ptr = this->m_read_header_ptr + length;
      this->m_status = good;
    }

    AMSG_INLINE void set_read(char const* buffer, ::std::size_t length)
    {
      set_read((unsigned char const*)buffer, length);
    }

    AMSG_INLINE void set_write(unsigned char* buffer, ::std::size_t length)
    {
      this->m_write_header_ptr = buffer;
      this->m_write_ptr = this->m_write_header_ptr;
      this->m_write_tail_ptr = this->m_write_header_ptr + length;
      this->m_status = good;
    }

    AMSG_INLINE void set_write(char* buffer, ::std::size_t length)
    {
      set_write((unsigned char*)buffer, length);
    }

    void append_debug_info(const char * info)
    {
      m_error_info.append(info);
    }

    std::size_t read(char * buffer,std::size_t len)
    {
      if (this->m_read_ptr + len > this->m_read_tail_ptr)
      {
        this->m_status = read_overflow;
        return 0;
      }
      std::memcpy(buffer, this->m_read_ptr, len);
      this->m_read_ptr += len;
      return len;
    }

    unsigned char get_char()
    {
      if (this->m_read_ptr + 1 > this->m_read_tail_ptr)
      {
        m_status = read_overflow;
        return 0;
      }
      return *m_read_ptr++;
    }

    std::size_t write(const char * buffer,std::size_t len)
    {
      if (this->m_write_ptr + len > this->m_write_tail_ptr)
      {
        this->m_status = write_overflow;
        return 0;
      }
      std::memcpy((void*)this->m_write_ptr, buffer, len);
      this->m_write_ptr += len;
      return len;
    }

    bool bad(){ return m_status != good || basic_store::error(); }

    AMSG_INLINE unsigned char * append_write(std::size_t len)
    {
      if (this->m_write_ptr + len > this->m_write_tail_ptr)
      {
        this->m_status = write_overflow;
        return 0;
      }
      unsigned char * append_ptr = this->m_write_ptr;
      this->m_write_ptr += len;
      return append_ptr;
    }

    AMSG_INLINE unsigned char const* skip_read(std::size_t len)
    {
      if (this->m_read_ptr + len > this->m_read_tail_ptr)
      {
        this->m_status = read_overflow;
        return 0;
      }
      unsigned char const* ptr = this->m_read_ptr;
      this->m_read_ptr += len;
      return ptr;
    }

    AMSG_INLINE void clear_write()
    {
      basic_store::clear();
      this->m_write_ptr = this->m_write_header_ptr;
    }

    AMSG_INLINE void clear()
    {
      basic_store::clear();
      this->m_read_ptr = this->m_read_header_ptr;
      this->m_write_ptr = this->m_write_header_ptr;
      m_status = good;
    }

    AMSG_INLINE unsigned char const* read_ptr() const
    {
      return this->m_read_ptr;
    }

    AMSG_INLINE unsigned char* write_ptr() const
    {
      return this->m_write_ptr;
    }

    AMSG_INLINE unsigned char* write_data() const
    {
      return this->m_write_header_ptr;
    }

    AMSG_INLINE ::std::size_t read_length() const
    {
      return this->m_read_ptr - this->m_read_header_ptr;
    }

    AMSG_INLINE ::std::size_t write_length() const
    {
      return this->m_write_ptr - this->m_write_header_ptr;
    }
  };

  namespace detail
  {
	template<typename store_ty , typename ty>
	struct value_read_support_zerocopy_impl
	{
		typedef ty value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value);
	};

	template<typename store_ty , typename ty>
	struct value_write_support_zerocopy_impl
	{
		typedef ty value_type;
		static AMSG_INLINE void write(store_ty& store_data, value_type& value);
	};

	template<typename store_ty,typename char_like_ty>
	struct value_read_unsigned_char_like_support_zerocopy_impl
	{
		typedef char_like_ty value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const int bytes = sizeof(value_type);
			value = store_data.get_char();
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
				value = store_data.get_char();
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
			}
		}
	};

	template<typename store_ty,typename char_like_ty>
	struct value_write_unsigned_char_like_support_zerocopy_impl
	{
		typedef char_like_ty value_type;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			if(value < const_tag_as_type)
			{
				::boost::uint8_t * ptr = store_data.append_write(1);
				*ptr = value;
			}
			else
			{
				::boost::uint8_t * ptr = store_data.append_write(2);
				ptr[0] = 0x80;
				ptr[1] = value;
			}
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<typename store_ty,typename char_like_ty>
	struct value_read_signed_char_like_support_zerocopy_impl
	{
		typedef char_like_ty value_type;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const int bytes = sizeof(value_type);
			::boost::uint8_t tag = store_data.get_char();
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			if(tag > const_tag_as_value)
			{
				int sign = 1;
				if((long)tag & const_negative_bit_value)
				{
					sign = -1;
				}
				int read_bytes = (int(tag) & const_interger_byte_msak) + 1;
				if( bytes < read_bytes )
				{
					store_data.set_error_code(value_too_large_to_integer_number);
					return;

				}
				value = store_data.get_char();
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				if(sign < 0)
				{
					value = -value;
				}
			}
			else
			{
				value = tag;
			}
		}
	};

	template<typename store_ty,typename char_like_ty>
	struct value_write_signed_char_like_support_zerocopy_impl
	{
		typedef char_like_ty value_type;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			if(0 <= value && value < const_tag_as_type)
			{
				::boost::uint8_t * ptr = store_data.append_write(1);
				*ptr = value;
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
				::boost::uint8_t * ptr = store_data.append_write(2);
				ptr[0] = 0x80|negative_bit;
				ptr[1] = temp;
			}
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

  template <>
	struct value_read_support_zerocopy_impl<zero_copy_buffer, ::boost::uint16_t>
	{
		typedef ::boost::uint16_t value_type;
		typedef zero_copy_buffer store_ty;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const int bytes = sizeof(value_type);
			value = store_data.get_char();
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
				int read_bytes = (int(value) & const_interger_byte_msak) + 1;
				if( bytes < read_bytes )
				{
					store_data.set_error_code(value_too_large_to_integer_number);
					return;

				}
				store_data.read((char*)&value,read_bytes);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				value = little_endian_to_host16(value);
			}
		}
	};

  template <>
	struct value_write_support_zerocopy_impl<zero_copy_buffer, ::boost::uint16_t>
	{
		typedef ::boost::uint16_t value_type;
		typedef zero_copy_buffer store_ty;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			if(value < const_tag_as_type)
			{
				::boost::uint8_t * ptr = store_data.append_write(1);
				*ptr = (::boost::uint8_t)value;
			}
			else
			{
				value_type temp = host_to_little_endian16(value);
				::boost::uint8_t * ptr = (::boost::uint8_t *)(&temp);
				if(value < 0x100)
				{
					::boost::uint8_t * wptr = store_data.append_write(2);
					wptr[0] = 0x80;
					wptr[1] = ptr[0];
				}
				else
				{
					::boost::uint8_t * wptr = store_data.append_write(3);
					wptr[0] = 0x81;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
				}
			}
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

  template <>
	struct value_read_support_zerocopy_impl<zero_copy_buffer,::boost::int16_t>
	{
		typedef ::boost::int16_t value_type;
		typedef zero_copy_buffer store_ty;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const int bytes = sizeof(value_type);
			::boost::uint8_t tag = store_data.get_char();
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			if(tag > const_tag_as_value)
			{
				int sign = 1;
				if((long)tag & const_negative_bit_value)
				{
					sign = -1;
				}
				int read_bytes = (int(tag) & const_interger_byte_msak) + 1;
				if( bytes < read_bytes )
				{
					store_data.set_error_code(value_too_large_to_integer_number);
					return;

				}
				value = 0;
				store_data.read((char*)&value,read_bytes);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				value = little_endian_to_host16(value);
				if(sign < 0)
				{
					value = -value;
				}
			}
			else
			{
				value = tag;
			}
		}
	};

  template <>
	struct value_write_support_zerocopy_impl<zero_copy_buffer,::boost::int16_t>
	{
		typedef ::boost::int16_t value_type;
		typedef zero_copy_buffer store_ty;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			if(0 <= value && value < const_tag_as_type)
			{
				::boost::uint8_t * ptr = store_data.append_write(1);
				*ptr = (::boost::uint8_t)value;
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
				value_type temp1 = host_to_little_endian16(temp);
				::boost::uint8_t * ptr = (::boost::uint8_t *)(&temp1);
				if(temp < 0x100)
				{
					::boost::uint8_t * wptr = store_data.append_write(2);
					wptr[0] = 0x80+negative_bit;
					wptr[1] = ptr[0];
				}
				else
				{
					::boost::uint8_t * wptr = store_data.append_write(3);
					wptr[0] = 0x81+negative_bit;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
				}
			}
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

  template <>
	struct value_read_support_zerocopy_impl<zero_copy_buffer, ::boost::uint32_t>
	{
		typedef ::boost::uint32_t value_type;
		typedef zero_copy_buffer store_ty;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const ::std::size_t bytes = sizeof(value_type);
			value = store_data.get_char();
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
				::std::size_t read_bytes = (::std::size_t)(value & const_interger_byte_msak) + 1;
				if( bytes < read_bytes )
				{
					store_data.set_error_code(value_too_large_to_integer_number);
					return;

				}
				store_data.read((char*)&value,read_bytes);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				value = little_endian_to_host32(value);
			}
		}
	};

  template <>
	struct value_write_support_zerocopy_impl<zero_copy_buffer, ::boost::uint32_t>
	{
		typedef ::boost::uint32_t value_type;
		typedef zero_copy_buffer store_ty;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			if(value < const_tag_as_type)
			{
				::boost::uint8_t * ptr = store_data.append_write(1);
				*ptr = (::boost::uint8_t)value;
			}
			else
			{
				value_type temp = host_to_little_endian32(value);
				::boost::uint8_t * ptr = (::boost::uint8_t *)(&temp);
				if(value < 0x100)
				{
					::boost::uint8_t * wptr = store_data.append_write(2);
					wptr[0] = 0x80;
					wptr[1] = ptr[0];
				}
				else if(value < 0x10000)
				{
					::boost::uint8_t * wptr = store_data.append_write(3);
					wptr[0] = 0x81;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
				}
				else if(value < 0x1000000)
				{
					::boost::uint8_t * wptr = store_data.append_write(4);
					wptr[0] = 0x82;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
				}
				else
				{
					::boost::uint8_t * wptr = store_data.append_write(5);
					wptr[0] = 0x83;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
					wptr[4] = ptr[3];
				}
			}
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

  template <>
	struct value_read_support_zerocopy_impl<zero_copy_buffer,::boost::int32_t>
	{
		typedef ::boost::int32_t value_type;
		typedef zero_copy_buffer store_ty;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const int bytes = sizeof(value_type);
			::boost::uint8_t tag = store_data.get_char();
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			if(tag > const_tag_as_value)
			{
				int sign = 1;
				if((long)tag & const_negative_bit_value)
				{
					sign = -1;
				}
				int read_bytes = (int(tag) & const_interger_byte_msak) + 1;
				if( bytes < read_bytes )
				{
					store_data.set_error_code(value_too_large_to_integer_number);
					return;

				}
				value = 0;
				store_data.read((char*)&value,read_bytes);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				value = little_endian_to_host32(value);
				if(sign < 0)
				{
					value = -value;
				}
			}
			else
			{
				value = tag;
			}
		}
	};

  template <>
	struct value_write_support_zerocopy_impl<zero_copy_buffer,::boost::int32_t>
	{
		typedef ::boost::int32_t value_type;
		typedef zero_copy_buffer store_ty;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			if(0 <= value && value < const_tag_as_type)
			{
				::boost::uint8_t * ptr = store_data.append_write(1);
				*ptr = (::boost::uint8_t)value;
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
				value_type temp1 = host_to_little_endian32(temp);
				::boost::uint8_t * ptr = (::boost::uint8_t *)(&temp1);
				if(temp < 0x100)
				{
					::boost::uint8_t * wptr = store_data.append_write(2);
					wptr[0] = 0x80+negative_bit;
					wptr[1] = ptr[0];
				}
				else if(temp < 0x10000)
				{
					::boost::uint8_t * wptr = store_data.append_write(3);
					wptr[0] = 0x81+negative_bit;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
				}
				else if(temp < 0x1000000)
				{
					::boost::uint8_t * wptr = store_data.append_write(4);
					wptr[0] = 0x82+negative_bit;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
				}
				else
				{
					::boost::uint8_t * wptr = store_data.append_write(5);
					wptr[0] = 0x83+negative_bit;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
					wptr[4] = ptr[3];
				}
			}
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

  template <>
	struct value_read_support_zerocopy_impl<zero_copy_buffer, ::boost::uint64_t>
	{
		typedef ::boost::uint64_t value_type;
		typedef zero_copy_buffer store_ty;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const int bytes = sizeof(value_type);
			value = store_data.get_char();
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
				int read_bytes = (int)(value & const_interger_byte_msak) + 1;
				if( bytes < read_bytes )
				{
					store_data.set_error_code(value_too_large_to_integer_number);
					return;

				}
				store_data.read((char*)&value,read_bytes);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				value = little_endian_to_host64(value);
			}
		}
	};

  template <>
	struct value_write_support_zerocopy_impl<zero_copy_buffer, ::boost::uint64_t>
	{
		typedef ::boost::uint64_t value_type;
		typedef zero_copy_buffer store_ty;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			if(value < const_tag_as_type)
			{
				::boost::uint8_t * ptr = store_data.append_write(1);
				*ptr = (::boost::uint8_t)value;
			}
			else
			{
				value_type temp = host_to_little_endian64(value);
				::boost::uint8_t * ptr = (::boost::uint8_t *)(&temp);
				if(value < 0x100)
				{
					::boost::uint8_t * wptr = store_data.append_write(2);
					wptr[0] = 0x80;
					wptr[1] = ptr[0];
				}
				else if(value < 0x10000)
				{
					::boost::uint8_t * wptr = store_data.append_write(3);
					wptr[0] = 0x81;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
				}
				else if(value < 0x1000000)
				{
					::boost::uint8_t * wptr = store_data.append_write(4);
					wptr[0] = 0x82;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
				}
				else if(value < 0x100000000)
				{
					::boost::uint8_t * wptr = store_data.append_write(5);
					wptr[0] = 0x83;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
					wptr[4] = ptr[3];
				}
				else if(value < 0x10000000000LL)
				{
					::boost::uint8_t * wptr = store_data.append_write(6);
					wptr[0] = 0x84;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
					wptr[4] = ptr[3];
					wptr[5] = ptr[4];
				}
				else if(value < 0x1000000000000LL)
				{
					::boost::uint8_t * wptr = store_data.append_write(7);
					wptr[0] = 0x85;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
					wptr[4] = ptr[3];
					wptr[5] = ptr[4];
					wptr[6] = ptr[5];
				}
				else if(value < 0x100000000000000LL)
				{
					::boost::uint8_t * wptr = store_data.append_write(8);
					wptr[0] = 0x86;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
					wptr[4] = ptr[3];
					wptr[5] = ptr[4];
					wptr[6] = ptr[5];
					wptr[7] = ptr[6];
				}
				else
				{
					::boost::uint8_t * wptr = store_data.append_write(9);
					wptr[0] = 0x87;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
					wptr[4] = ptr[3];
					wptr[5] = ptr[4];
					wptr[6] = ptr[5];
					wptr[7] = ptr[6];
					wptr[8] = ptr[7];
				}
			}
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

  template <>
	struct value_read_support_zerocopy_impl<zero_copy_buffer,::boost::int64_t>
	{
		typedef ::boost::int64_t value_type;
		typedef zero_copy_buffer store_ty;
		static AMSG_INLINE void read(store_ty& store_data, value_type& value)
		{
			const int bytes = sizeof(value_type);
			::boost::uint8_t tag = store_data.get_char();
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
			if(tag > const_tag_as_value)
			{
				int sign = 1;
				if((long)tag & const_negative_bit_value)
				{
					sign = -1;
				}
				int read_bytes = (int(tag) & const_interger_byte_msak) + 1;
				if( bytes < read_bytes )
				{
					store_data.set_error_code(value_too_large_to_integer_number);
					return;

				}
				value = 0;
				store_data.read((char*)&value,read_bytes);
				if(store_data.bad())
				{
					store_data.set_error_code(stream_buffer_overflow);
					return;
				}
				value = little_endian_to_host64(value);
				if(sign < 0)
				{
					value = -value;
				}
			}
			else
			{
				value = tag;
			}
		}
	};

  template <>
	struct value_write_support_zerocopy_impl<zero_copy_buffer,::boost::int64_t>
	{
		typedef ::boost::int64_t value_type;
		typedef zero_copy_buffer store_ty;
		static AMSG_INLINE void write(store_ty& store_data, const value_type& value)
		{
			if(0 <= value && value < const_tag_as_type)
			{
				::boost::uint8_t * ptr = store_data.append_write(1);
				*ptr = (::boost::uint8_t)value;
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
				value_type temp1 = host_to_little_endian64(temp);
				::boost::uint8_t * ptr = (::boost::uint8_t *)(&temp1);
				if(temp < 0x100)
				{
					::boost::uint8_t * wptr = store_data.append_write(2);
					wptr[0] = 0x80+negative_bit;
					wptr[1] = ptr[0];
				}
				else if(temp < 0x10000)
				{
					::boost::uint8_t * wptr = store_data.append_write(3);
					wptr[0] = 0x81+negative_bit;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
				}
				else if(temp < 0x1000000)
				{
					::boost::uint8_t * wptr = store_data.append_write(4);
					wptr[0] = 0x82+negative_bit;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
				}
				else if(temp < 0x100000000)
				{
					::boost::uint8_t * wptr = store_data.append_write(5);
					wptr[0] = 0x83+negative_bit;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
					wptr[4] = ptr[3];
				}
				else if(temp < 0x10000000000LL)
				{
					::boost::uint8_t * wptr = store_data.append_write(6);
					wptr[0] = 0x84+negative_bit;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
					wptr[4] = ptr[3];
					wptr[5] = ptr[4];
				}
				else if(temp < 0x1000000000000LL)
				{
					::boost::uint8_t * wptr = store_data.append_write(7);
					wptr[0] = 0x85+negative_bit;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
					wptr[4] = ptr[3];
					wptr[5] = ptr[4];
					wptr[6] = ptr[5];
				}
				else if(temp < 0x100000000000000LL)
				{
					::boost::uint8_t * wptr = store_data.append_write(8);
					wptr[0] = 0x86+negative_bit;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
					wptr[4] = ptr[3];
					wptr[5] = ptr[4];
					wptr[6] = ptr[5];
					wptr[7] = ptr[6];
				}
				else
				{
					::boost::uint8_t * wptr = store_data.append_write(9);
					wptr[0] = 0x87+negative_bit;
					wptr[1] = ptr[0];
					wptr[2] = ptr[1];
					wptr[3] = ptr[2];
					wptr[4] = ptr[3];
					wptr[5] = ptr[4];
					wptr[6] = ptr[5];
					wptr[7] = ptr[6];
					wptr[8] = ptr[7];
				}
			}
			if(store_data.bad())
			{
				store_data.set_error_code(stream_buffer_overflow);
				return;
			}
		}
	};

	template<int tag>
	struct value_read_support<zero_copy_buffer,::boost::uint8_t,tag>
	{
		typedef value_read_unsigned_char_like_support_zerocopy_impl<zero_copy_buffer,::boost::uint8_t> impl_type;
	};

	template<int tag>
	struct value_write_support<zero_copy_buffer,::boost::uint8_t,tag>
	{
		typedef value_write_unsigned_char_like_support_zerocopy_impl<zero_copy_buffer,::boost::uint8_t> impl_type;
	};

	template<int tag>
	struct value_read_support<zero_copy_buffer,::boost::int8_t,tag>
	{
		typedef value_read_signed_char_like_support_zerocopy_impl<zero_copy_buffer,::boost::int8_t> impl_type;
	};

	template<int tag>
	struct value_write_support<zero_copy_buffer,::boost::int8_t,tag>
	{
		typedef value_write_signed_char_like_support_zerocopy_impl<zero_copy_buffer,::boost::int8_t> impl_type;
	};

	template<int tag>
	struct value_read_support<zero_copy_buffer,char,tag>
	{
		typedef char value_type;
		typedef typename ::boost::mpl::if_<
			::boost::is_signed<value_type>,
			value_read_signed_char_like_support_zerocopy_impl<zero_copy_buffer,value_type>,
			value_read_unsigned_char_like_support_zerocopy_impl<zero_copy_buffer,value_type>
		>::type impl_type;
	};

	template<int tag>
	struct value_write_support<zero_copy_buffer,char,tag>
	{
		typedef char value_type;
		typedef typename ::boost::mpl::if_<
			::boost::is_signed<value_type>,
			value_write_signed_char_like_support_zerocopy_impl<zero_copy_buffer,value_type>,
			value_write_unsigned_char_like_support_zerocopy_impl<zero_copy_buffer,value_type>
		>::type impl_type;
	};


	template<int tag>
	struct value_read_support<zero_copy_buffer,::boost::uint16_t,tag>
	{
		typedef value_read_support_zerocopy_impl<zero_copy_buffer,::boost::uint16_t> impl_type;
	};

	template<int tag>
	struct value_write_support<zero_copy_buffer,::boost::uint16_t,tag>
	{
		typedef value_write_support_zerocopy_impl<zero_copy_buffer,::boost::uint16_t> impl_type;
	};

	template<int tag>
	struct value_read_support<zero_copy_buffer,::boost::int16_t,tag>
	{
		typedef value_read_support_zerocopy_impl<zero_copy_buffer,::boost::int16_t> impl_type;
	};

	template<int tag>
	struct value_write_support<zero_copy_buffer,::boost::int16_t,tag>
	{
		typedef value_write_support_zerocopy_impl<zero_copy_buffer,::boost::int16_t> impl_type;
	};

	template<int tag>
	struct value_read_support<zero_copy_buffer,::boost::uint32_t,tag>
	{
		typedef value_read_support_zerocopy_impl<zero_copy_buffer,::boost::uint32_t> impl_type;
	};

	template<int tag>
	struct value_write_support<zero_copy_buffer,::boost::uint32_t,tag>
	{
		typedef value_write_support_zerocopy_impl<zero_copy_buffer,::boost::uint32_t> impl_type;
	};

	template<int tag>
	struct value_read_support<zero_copy_buffer,::boost::int32_t,tag>
	{
		typedef value_read_support_zerocopy_impl<zero_copy_buffer,::boost::int32_t> impl_type;
	};

	template<int tag>
	struct value_write_support<zero_copy_buffer,::boost::int32_t,tag>
	{
		typedef value_write_support_zerocopy_impl<zero_copy_buffer,::boost::int32_t> impl_type;
	};

	template<int tag>
	struct value_read_support<zero_copy_buffer,::boost::uint64_t,tag>
	{
		typedef value_read_support_zerocopy_impl<zero_copy_buffer,::boost::uint64_t> impl_type;
	};

	template<int tag>
	struct value_write_support<zero_copy_buffer,::boost::uint64_t,tag>
	{
		typedef value_write_support_zerocopy_impl<zero_copy_buffer,::boost::uint64_t> impl_type;
	};

	template<int tag>
	struct value_read_support<zero_copy_buffer,::boost::int64_t,tag>
	{
		typedef value_read_support_zerocopy_impl<zero_copy_buffer,::boost::int64_t> impl_type;
	};

	template<int tag>
	struct value_write_support<zero_copy_buffer,::boost::int64_t,tag>
	{
		typedef value_write_support_zerocopy_impl<zero_copy_buffer,::boost::int64_t> impl_type;
	};


}}

#endif
