#include <gtest/gtest.h>
#include "zp_cpp/hash.hpp"
#include "zp_cpp/core.hpp"
#include "zp_cpp/files.hpp"
#include "zp_cpp/cli.hpp"
#include "zp_cpp/buff.hpp"
#include <chrono>
#include <cstring>
#include <filesystem>
#include <string>
#include "../cmn.hpp"

// =========================================================================================================================================
// =========================================================================================================================================
// CompleteDataProcessingPipeline: Validates complete workflow from CLI parsing to file I/O and hashing.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(EndToEndTest, CompleteDataProcessingPipeline)
{
    const std::filesystem::path input_file  = zp::test::make_temp_path("zp_cpp_input", ".txt");
    const std::filesystem::path output_file = zp::test::make_temp_path("zp_cpp_output", ".txt");

    std::string program                     = "program";
    std::string input_arg                   = std::string("--input=") + input_file.string();
    std::string output_arg                  = std::string("--output=") + output_file.string();

    char* argv[]                            = {program.data(), input_arg.data(), output_arg.data()};
    auto args                               = zp::cli::parse_cli(3, argv);

    auto input_it                           = args.find("input");
    ASSERT_NE(input_it, args.end());
    auto output_it = args.find("output");
    ASSERT_NE(output_it, args.end());

    const char* input_data = "Input data to process";
    zp::span<const std::byte> input_span{reinterpret_cast<const std::byte*>(input_data), std::strlen(input_data)};
    ASSERT_EQ(zp::files::write_file(input_it->second, input_span), zp::Result::ZC_SUCCESS);

    zp::buff<1024> buffer;
    zp::buff<1024> read_buffer;
    zp::span<std::byte> read_span = read_buffer.as_span();
    zp::span<std::byte> actual_data;
    ASSERT_EQ(zp::files::read_file(input_it->second, read_span, &actual_data), zp::Result::ZC_SUCCESS);

    zp::span<std::byte> data_span;
    ASSERT_EQ(buffer.bump(actual_data.count, &data_span), zp::Result::ZC_SUCCESS);
    ASSERT_EQ(data_span.rcv(0, actual_data.count, actual_data.p), zp::Result::ZC_SUCCESS);

    zp::span<const std::byte> const_data_span{data_span.p, data_span.count};
    auto hash            = zp::hash::hash_data(const_data_span);
    std::string hash_str = zp::hash::to_str(hash);

    zp::span<const std::byte> output_span{reinterpret_cast<const std::byte*>(hash_str.c_str()), hash_str.length()};
    ASSERT_EQ(zp::files::write_file(output_it->second, output_span), zp::Result::ZC_SUCCESS);

    zp::buff<1024> output_buffer;
    zp::span<std::byte> output_read_span = output_buffer.as_span();
    zp::span<std::byte> output_actual_data;
    ASSERT_EQ(zp::files::read_file(output_it->second, output_read_span, &output_actual_data), zp::Result::ZC_SUCCESS);

    std::string output_str{reinterpret_cast<const char*>(output_actual_data.p), output_actual_data.count};
    EXPECT_EQ(output_str, hash_str);

    std::error_code ec1;
    std::filesystem::remove(input_it->second, ec1);
    std::error_code ec2;
    std::filesystem::remove(output_it->second, ec2);
}
