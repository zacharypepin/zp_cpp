#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "zp_cpp/events.hpp"

namespace zp::net
{
    constexpr std::uint32_t STARTING_NET_ENT_VAL = 1;
    constexpr std::size_t IPV4_ADDRSTRLEN        = 16;

    using EventId                                = std::uint16_t;

    struct Instance;

    namespace server
    {
        using NetId                                               = void*;

        constexpr std::uint32_t DISCONNECT_ALL_CLIENTS_TIMEOUT_MS = 2000;

        struct ClientConnectedEvt
        {
            NetId client_id;
        };

        struct ClientDisconnectedEvt
        {
            NetId client_id;
        };

        struct NetEventOut
        {
            std::vector<NetId> dest;
            EventId event_id;
            std::vector<std::uint8_t> param_bytes;
        };

        struct NetEventIn
        {
            NetId src;
            EventId event_id;
            std::vector<std::uint8_t> param_bytes;
        };

        struct Config
        {
            std::uint16_t port;
        };

        struct State
        {
            struct Internal;

            struct Transient
            {
                std::vector<NetEventIn> incoming;
                std::vector<NetEventOut> outgoing;
            };

            Internal* p_i;

            Transient transient;

            zp::Event<ClientConnectedEvt> on_client_connected;
            zp::Event<ClientDisconnectedEvt> on_client_disconnected;
        };

        bool start_server(Instance* p_inst);
        void stop_server(Instance* p_inst);

        void disconnect_client(Instance* p_inst, NetId client);
        void disconnect_all_clients(Instance* p_inst);

        void handle_incoming(Instance* p_inst);
        void handle_outgoing(Instance* p_inst);

        void send_event(Instance* p_inst, NetEventOut net_event);

        std::uint16_t get_server_port(Instance* p_inst);
    }

    namespace client
    {
        struct NetEvent
        {
            EventId event_id;
            std::vector<std::uint8_t> param_bytes;
        };

        struct Config
        {
            std::string server_addr;
            std::uint16_t server_port;
        };

        struct State
        {
            struct Internal;

            struct Transient
            {
                std::vector<NetEvent> incoming;
                std::vector<NetEvent> outgoing;
            };

            Internal* p_i;

            Transient transient;

            zp::VoidEvent on_connected_to_server;
            zp::VoidEvent on_disconnected_from_server;
        };

        bool start_client(Instance* p_inst);
        void stop_client(Instance* p_inst);

        bool connect_to_server(Instance* p_inst);
        void disconnect_from_server(Instance* p_inst);

        void handle_incoming(Instance* p_inst);
        void handle_outgoing(Instance* p_inst);

        bool is_connected_to_server(Instance* p_inst);
        void send_event(Instance* p_inst, NetEvent net_event);
    }

    constexpr std::uint32_t EVENT_PROCESSING_TIMEOUT_MS = 1;

    struct Instance
    {
        server::Config server_config;
        server::State server_state;
        client::Config client_config;
        client::State client_state;
    };

    bool init(Instance* p_inst);
    void exit(Instance* p_inst);
}
