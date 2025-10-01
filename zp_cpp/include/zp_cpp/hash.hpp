#pragma once

#include "core.hpp"
#include "buff.hpp"

#include <cstdint>
#include <cstddef>
#include <string>

namespace zp::hash
{
    struct hash256
    {
        std::byte bytes[32];
    };

    std::size_t hash_value(const hash256& h) noexcept;

    hash256 hash_data(const void* data, std::uint64_t size) noexcept;
    hash256 hash_data(zp::span<const std::byte> data) noexcept;

    std::string to_str(const hash256& h);

    bool operator==(const hash256& a, const hash256& b) noexcept;

    bool operator!=(const hash256& a, const hash256& b) noexcept;
}

namespace std
{
    template <> struct hash<zp::hash::hash256>
    {
        size_t operator()(const zp::hash::hash256& h) const noexcept;
    };
}
