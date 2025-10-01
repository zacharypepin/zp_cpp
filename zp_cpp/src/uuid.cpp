#include "zp_cpp/uuid.hpp"

#include <cstddef>
#include <cstring>
#include <iomanip>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>

namespace
{
    thread_local std::mt19937_64 g_rng{std::random_device{}()};
    std::uniform_int_distribution<uint64_t> g_dist{0, std::numeric_limits<uint64_t>::max()};
}

// =========================================================================================================================================
// =========================================================================================================================================
// operator==: Compares two UUIDs byte-by-byte for equality.
// =========================================================================================================================================
// =========================================================================================================================================
bool zp::uuid::uuid::operator==(const uuid& other) const noexcept
{
    for (size_t i = 0; i < 16; ++i)
    {
        if (bytes[i] != other.bytes[i])
        {
            return false;
        }
    }
    return true;
}

// =========================================================================================================================================
// =========================================================================================================================================
// operator!=: Negates operator== to determine inequality between UUIDs.
// =========================================================================================================================================
// =========================================================================================================================================
bool zp::uuid::uuid::operator!=(const uuid& other) const noexcept
{
    return !(*this == other);
}

// =========================================================================================================================================
// =========================================================================================================================================
// generate: Produces a version 4 (random) RFC 4122 UUID.
// =========================================================================================================================================
// =========================================================================================================================================
zp::uuid::uuid zp::uuid::generate()
{
    uuid value{};

    const uint64_t high      = g_dist(g_rng);
    const uint64_t low       = g_dist(g_rng);

    uint64_t versioned_high  = high & 0xFFFFFFFFFFFF0FFFULL;
    versioned_high          |= 0x0000000000004000ULL;

    uint64_t variant_low     = low & 0x3FFFFFFFFFFFFFFFULL;
    variant_low             |= 0x8000000000000000ULL;

    std::memcpy(value.bytes, &versioned_high, sizeof(versioned_high));
    std::memcpy(value.bytes + sizeof(versioned_high), &variant_low, sizeof(variant_low));

    return value;
}

// =========================================================================================================================================
// =========================================================================================================================================
// to_str: Converts a UUID into its canonical lowercase hexadecimal string representation.
// =========================================================================================================================================
// =========================================================================================================================================
std::string zp::uuid::to_str(const uuid& value)
{
    std::ostringstream stream;
    stream << std::hex << std::setfill('0');

    for (int i = 0; i < 16; ++i)
    {
        if (i == 4 || i == 6 || i == 8 || i == 10)
        {
            stream << '-';
        }

        const unsigned byte_value = std::to_integer<unsigned>(value.bytes[i]);
        stream << std::setw(2) << byte_value;
    }

    return stream.str();
}

// =========================================================================================================================================
// =========================================================================================================================================
// from_str: Parses a canonical lowercase hexadecimal UUID string into its binary representation.
// =========================================================================================================================================
// =========================================================================================================================================
zp::uuid::uuid zp::uuid::from_str(const std::string& value)
{
    if (value.length() != 36 || value[8] != '-' || value[13] != '-' || value[18] != '-' || value[23] != '-')
    {
        throw std::invalid_argument("Invalid UUID format");
    }

    uuid parsed{};
    size_t byte_index = 0;

    for (size_t i = 0; i < value.length();)
    {
        if (value[i] == '-')
        {
            ++i;
            continue;
        }

        if (i + 1 >= value.length())
        {
            throw std::invalid_argument("Incomplete byte in UUID");
        }

        const char high_char = value[i];
        const char low_char  = value[i + 1];

        std::byte high_nibble{};
        // =================================================================================================
        // =================================================================================================
        // Convert the high hex digit into a 4-bit value.
        // =================================================================================================
        // =================================================================================================
        {
            if (high_char >= '0' && high_char <= '9')
            {
                high_nibble = static_cast<std::byte>(high_char - '0');
            }
            else if (high_char >= 'a' && high_char <= 'f')
            {
                high_nibble = static_cast<std::byte>(10 + (high_char - 'a'));
            }
            else if (high_char >= 'A' && high_char <= 'F')
            {
                high_nibble = static_cast<std::byte>(10 + (high_char - 'A'));
            }
            else
            {
                throw std::invalid_argument("Invalid hex character");
            }
        }

        std::byte low_nibble{};
        // =================================================================================================
        // =================================================================================================
        // Convert the low hex digit into a 4-bit value.
        // =================================================================================================
        // =================================================================================================
        {
            if (low_char >= '0' && low_char <= '9')
            {
                low_nibble = static_cast<std::byte>(low_char - '0');
            }
            else if (low_char >= 'a' && low_char <= 'f')
            {
                low_nibble = static_cast<std::byte>(10 + (low_char - 'a'));
            }
            else if (low_char >= 'A' && low_char <= 'F')
            {
                low_nibble = static_cast<std::byte>(10 + (low_char - 'A'));
            }
            else
            {
                throw std::invalid_argument("Invalid hex character");
            }
        }

        const unsigned combined     = (std::to_integer<unsigned>(high_nibble) << 4) | std::to_integer<unsigned>(low_nibble);
        parsed.bytes[byte_index++]  = static_cast<std::byte>(combined);
        i                          += 2;
    }

    if (byte_index != 16)
    {
        throw std::runtime_error("UUID did not contain 16 bytes");
    }

    return parsed;
}

// =========================================================================================================================================
// =========================================================================================================================================
// operator(): Hashes a UUID by XOR'ing its two 64-bit halves for use in unordered containers.
// =========================================================================================================================================
// =========================================================================================================================================
size_t std::hash<zp::uuid::uuid>::operator()(const zp::uuid::uuid& id) const noexcept
{
    uint64_t high = 0;
    uint64_t low  = 0;

    std::memcpy(&high, id.bytes, sizeof(high));
    std::memcpy(&low, id.bytes + sizeof(high), sizeof(low));
    return high ^ low;
}
