#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>

#include "zp_cpp/net.hpp"

// =========================================================================================================================================
// =========================================================================================================================================
// InitAndExit: Validates net::init() and net::exit() sequence succeeds without touching server/client state.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(NetTest, InitAndExit)
{
    zp::net::Instance instance = {};

    EXPECT_TRUE(zp::net::init(&instance));
    zp::net::exit(&instance);
}

// =========================================================================================================================================
// =========================================================================================================================================
// ServerStartsAndStops: Validates server startup on ephemeral port and clean shutdown release resources.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(NetTest, ServerStartsAndStops)
{
    zp::net::Instance instance = {};

    ASSERT_TRUE(zp::net::init(&instance));

    instance.server_config.port = 0;

    ASSERT_TRUE(zp::net::server::start_server(&instance));

    zp::net::server::stop_server(&instance);
    zp::net::exit(&instance);
}

// =========================================================================================================================================
// =========================================================================================================================================
// ClientConnectsAndDisconnects: Validates client connects to local server and performs clean disconnect.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(NetTest, ClientConnectsAndDisconnects)
{
    zp::net::Instance instance = {};

    ASSERT_TRUE(zp::net::init(&instance));

    instance.server_config.port = 0;
    ASSERT_TRUE(zp::net::server::start_server(&instance));

    ASSERT_TRUE(zp::net::client::start_client(&instance));

    instance.client_config.server_addr = "127.0.0.1";
    instance.client_config.server_port = zp::net::server::get_server_port(&instance);

    // Give the server a moment to be ready
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Start the connection attempt in a separate thread so we can process server events
    std::atomic<bool> connection_result = false;
    std::thread client_thread([&]() { connection_result = zp::net::client::connect_to_server(&instance); });

    // Process server events while client is connecting
    auto start_time = std::chrono::steady_clock::now();
    while (!connection_result && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count() < 5000)
    {
        zp::net::server::handle_incoming(&instance);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    client_thread.join();
    ASSERT_TRUE(connection_result);

    for (int i = 0; i < 5; ++i)
    {
        zp::net::server::handle_incoming(&instance);
        zp::net::client::handle_incoming(&instance);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    EXPECT_TRUE(zp::net::client::is_connected_to_server(&instance));

    zp::net::client::disconnect_from_server(&instance);

    for (int i = 0; i < 5; ++i)
    {
        zp::net::server::handle_incoming(&instance);
        zp::net::client::handle_incoming(&instance);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    EXPECT_FALSE(zp::net::client::is_connected_to_server(&instance));

    zp::net::client::stop_client(&instance);
    zp::net::server::stop_server(&instance);
    zp::net::exit(&instance);
}
