#include <gtest/gtest.h>
#include "zp_cpp/hash.hpp"
#include "zp_cpp/core.hpp"
#include "zp_cpp/files.hpp"
#include "zp_cpp/cli.hpp"
#include <chrono>
#include <cstring>
#include <filesystem>
#include <string>
#include "../cmn.hpp"

// =========================================================================================================================================
// =========================================================================================================================================
// ParseConfigPathAndLoad: Validates CLI args can specify config file path which is then loaded.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(CliFileIntegrationTest, ParseConfigPathAndLoad)
{
    const std::filesystem::path config_file = zp::test::make_temp_path("zp_cpp_config", ".txt");
    const char* config_content              = "setting=value";

    zp::span<const std::byte> config_span{reinterpret_cast<const std::byte*>(config_content), std::strlen(config_content)};
    ASSERT_EQ(zp::files::write_file(config_file, config_span), zp::Result::ZC_SUCCESS);

    std::string program    = "program";
    std::string config_arg = std::string("--config=") + config_file.string();

    char* argv[]           = {program.data(), config_arg.data()};
    auto cli_args          = zp::cli::parse_cli(2, argv);

    auto config_it         = cli_args.find("config");
    ASSERT_NE(config_it, cli_args.end());
    EXPECT_EQ(config_it->second, config_file.string());

    zp::buff<1024> buffer;
    zp::span<std::byte> read_span = buffer.as_span();
    zp::span<std::byte> actual_data;
    ASSERT_EQ(zp::files::read_file(config_it->second, read_span, &actual_data), zp::Result::ZC_SUCCESS);

    // Convert to null-terminated string for comparison
    std::string read_content{reinterpret_cast<const char*>(actual_data.p), actual_data.count};
    EXPECT_STREQ(read_content.c_str(), config_content);

    std::error_code ec;
    std::filesystem::remove(config_file, ec);
}
