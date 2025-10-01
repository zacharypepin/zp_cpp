#pragma once

#include "core.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <cstddef>
#include <type_traits>

namespace zp
{
    template <typename T> struct span
    {
        T* p;
        size_t count;

        constexpr T* begin() noexcept;
        constexpr T* end() noexcept;
        constexpr const T* begin() const noexcept;
        constexpr const T* end() const noexcept;
        constexpr const T* cbegin() const noexcept;
        constexpr const T* cend() const noexcept;
        bool operator==(span<T> const& o) const noexcept;

        Result split(size_t size, span<T>* p_first, span<T>* p_second) const;
        Result cpy(size_t offset, size_t count, void* dst);
        Result rcv(size_t offset, size_t count, const void* src);
    };

    template <size_t SIZE> struct buff
    {
        std::byte bytes[SIZE];
        size_t count = 0;

        void reset();
        Result bump(size_t size, span<std::byte>* p_out);
        span<std::byte> as_span();
    };

    template <typename T> std::string to_str(span<T> span);
}

// =========================================================================================================================================
// =========================================================================================================================================
// span iterator methods
// =========================================================================================================================================
// =========================================================================================================================================
template <typename T> constexpr T* zp::span<T>::begin() noexcept
{
    return p;
}

template <typename T> constexpr T* zp::span<T>::end() noexcept
{
    return p + count;
}

template <typename T> constexpr const T* zp::span<T>::begin() const noexcept
{
    return p;
}

template <typename T> constexpr const T* zp::span<T>::end() const noexcept
{
    return p + count;
}

template <typename T> constexpr const T* zp::span<T>::cbegin() const noexcept
{
    return p;
}

template <typename T> constexpr const T* zp::span<T>::cend() const noexcept
{
    return p + count;
}

template <typename T> bool zp::span<T>::operator==(span<T> const& o) const noexcept
{
    static_assert(std::is_trivially_copyable_v<T>, "span<T>::operator== requires trivially copyable T");

    return count == o.count && std::memcmp(p, o.p, count * sizeof(T)) == 0;
}

// =========================================================================================================================================
// =========================================================================================================================================
// split: Split the span into two parts at the specified size boundary.
// =========================================================================================================================================
// =========================================================================================================================================
template <typename T> zp::Result zp::span<T>::split(size_t size, zp::span<T>* p_first, zp::span<T>* p_second) const
{
    if (size > this->count)
    {
        return Result::ZC_OUT_OF_BOUNDS;
    }

    p_first->p      = this->p;
    p_first->count  = size;

    p_second->p     = this->p + size;
    p_second->count = this->count - size;

    return Result::ZC_SUCCESS;
}

// =========================================================================================================================================
// =========================================================================================================================================
// cpy: Copy data from this span to the destination buffer.
// =========================================================================================================================================
// =========================================================================================================================================
template <typename T> zp::Result zp::span<T>::cpy(size_t offset, size_t count, void* dst)
{
    static_assert(std::is_trivially_copyable_v<T>, "span<T>::cpy requires trivially copyable T");

    if (offset > this->count || count > this->count - offset)
    {
        return Result::ZC_OUT_OF_BOUNDS;
    }

    memcpy(dst, this->p + offset, count * sizeof(T));

    return Result::ZC_SUCCESS;
}

// =========================================================================================================================================
// =========================================================================================================================================
// rcv: Copy data from the source buffer into this span.
// =========================================================================================================================================
// =========================================================================================================================================
template <typename T> zp::Result zp::span<T>::rcv(size_t offset, size_t count, const void* src)
{
    static_assert(std::is_trivially_copyable_v<T>, "span<T>::rcv requires trivially copyable T");

    if (offset > this->count || count > this->count - offset)
    {
        return Result::ZC_OUT_OF_BOUNDS;
    }

    memcpy(this->p + offset, src, count * sizeof(T));

    return Result::ZC_SUCCESS;
}

// =========================================================================================================================================
// =========================================================================================================================================
// reset: Reset the buffer count to zero.
// =========================================================================================================================================
// =========================================================================================================================================
template <size_t SIZE> void zp::buff<SIZE>::reset()
{
    this->count = 0;
}

// =========================================================================================================================================
// =========================================================================================================================================
// bump: Allocate space in the buffer and return a span to the allocated region.
// =========================================================================================================================================
// =========================================================================================================================================
template <size_t SIZE> zp::Result zp::buff<SIZE>::bump(size_t size, span<std::byte>* p_out)
{
    if (this->count + size > SIZE)
    {
        return Result::ZC_OUT_OF_BOUNDS;
    }

    p_out->p      = &this->bytes[this->count];
    p_out->count  = size;

    this->count  += size;

    return Result::ZC_SUCCESS;
}

// =========================================================================================================================================
// =========================================================================================================================================
// as_span: Return a span view of the buffer data.
// =========================================================================================================================================
// =========================================================================================================================================
template <size_t SIZE> zp::span<std::byte> zp::buff<SIZE>::as_span()
{
    return span<std::byte>{&this->bytes[0], SIZE};
}

// =========================================================================================================================================
// =========================================================================================================================================
// to_str: Convert span data to a string representation.
// =========================================================================================================================================
// =========================================================================================================================================
template <typename T> std::string zp::to_str(zp::span<T> span)
{
    const size_t size = span.count * sizeof(T);

    std::string result;
    result.reserve(size * 4);

    uint8_t* bytes = reinterpret_cast<uint8_t*>(span.p);
    for (size_t i = 0; i < size; ++i)
    {
        result += std::to_string(bytes[i]);
        if (i + 1 < size) result.push_back(' ');
    }
    return result;
}
