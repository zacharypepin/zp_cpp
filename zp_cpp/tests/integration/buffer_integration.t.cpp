#include <gtest/gtest.h>
#include "zp_cpp/core.hpp"
#include "zp_cpp/buff.hpp"
#include "zp_cpp/files.hpp"
#include "zp_cpp/hash.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <system_error>
#include "../cmn.hpp"

namespace
{
    struct PacketHeader
    {
        std::uint16_t magic;
        std::uint16_t version;
        std::uint32_t payload_size;
    };
}

// =========================================================================================================================================
// =========================================================================================================================================
// PacketRoundTripViaFileIO: Validates arena allocations compose a packet that can be written, reloaded, and hashed consistently.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(BufferSpanIntegrationTest, PacketRoundTripViaFileIO)
{
    constexpr std::size_t payload_size = 6u;
    constexpr PacketHeader header{0xCAFEu, 0x0002u, static_cast<std::uint32_t>(payload_size)};
    constexpr std::array<unsigned char, payload_size> payload = {'s', 'y', 'n', 'c', 'e', 'd'};

    zp::buff<256> arena;
    zp::span<std::byte> header_span;
    zp::span<std::byte> payload_span;

    ASSERT_EQ(arena.bump(sizeof(PacketHeader), &header_span), zp::Result::ZC_SUCCESS);
    ASSERT_EQ(arena.bump(payload.size(), &payload_span), zp::Result::ZC_SUCCESS);

    ASSERT_EQ(header_span.rcv(0, sizeof(PacketHeader), &header), zp::Result::ZC_SUCCESS);
    ASSERT_EQ(payload_span.rcv(0, payload.size(), payload.data()), zp::Result::ZC_SUCCESS);

    const size_t packet_size = header_span.count + payload_span.count;
    const zp::span<const std::byte> packet_bytes{header_span.p, packet_size};
    const auto expected_hash                = zp::hash::hash_data(packet_bytes);

    const std::filesystem::path packet_path = zp::test::make_temp_path("zp_cpp_packet", ".bin");
    ASSERT_EQ(zp::files::write_file(packet_path, packet_bytes), zp::Result::ZC_SUCCESS);

    zp::buff<256> read_arena;
    zp::span<std::byte> packet_on_disk;
    ASSERT_EQ(zp::files::read_file(packet_path, read_arena.as_span(), &packet_on_disk), zp::Result::ZC_SUCCESS);
    EXPECT_EQ(packet_on_disk.count, packet_size);

    zp::span<std::byte> read_header;
    zp::span<std::byte> read_payload;
    ASSERT_EQ(packet_on_disk.split(sizeof(PacketHeader), &read_header, &read_payload), zp::Result::ZC_SUCCESS);
    ASSERT_EQ(read_payload.count, payload.size());

    PacketHeader roundtrip_header{};
    ASSERT_EQ(read_header.cpy(0, sizeof(PacketHeader), &roundtrip_header), zp::Result::ZC_SUCCESS);
    EXPECT_EQ(roundtrip_header.magic, header.magic);
    EXPECT_EQ(roundtrip_header.version, header.version);
    EXPECT_EQ(roundtrip_header.payload_size, header.payload_size);

    std::array<unsigned char, payload_size> roundtrip_payload{};
    ASSERT_EQ(read_payload.cpy(0, read_payload.count, roundtrip_payload.data()), zp::Result::ZC_SUCCESS);
    EXPECT_EQ(roundtrip_payload, payload);

    const auto roundtrip_hash = zp::hash::hash_data(zp::span<const std::byte>{packet_on_disk.p, packet_on_disk.count});
    EXPECT_TRUE(roundtrip_hash == expected_hash);

    std::error_code ec;
    std::filesystem::remove(packet_path, ec);
}
