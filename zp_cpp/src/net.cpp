#include "zp_cpp/net.hpp"

#include <cstring>
#include <enet/enet.h>
#include <unordered_set>
#include <utility>

#include "zp_cpp/dbg.hpp"

struct zp::net::server::State::Internal
{
    ENetHost* host;
    std::unordered_set<ENetPeer*> connected_clients;
};

struct zp::net::client::State::Internal
{
    ENetHost* host;
    ENetPeer* server;
};

namespace
{
    // =========================================================================================================================================
    // =========================================================================================================================================
    // process_connect: Logs connection details, tracks the peer, and raises the connected event.
    // =========================================================================================================================================
    // =========================================================================================================================================
    void process_connect(zp::net::Instance* p_inst, ENetPeer* peer)
    {
        // ============================================================================================
        // ============================================================================================
        // record connection metadata for diagnostics.
        // ============================================================================================
        // ============================================================================================
        {
            char ip[zp::net::IPV4_ADDRSTRLEN];
            enet_address_get_host_ip(&peer->address, ip, sizeof(ip));
            LOG("peer: " << peer << " connected from " << ip << ":" << peer->address.port);
        }

        // ============================================================================================
        // ============================================================================================
        // track the peer in the active client set for later lookups.
        // ============================================================================================
        // ============================================================================================
        {
            p_inst->server_state.p_i->connected_clients.insert(peer);
        }

        // ============================================================================================
        // ============================================================================================
        // fire the application-facing client-connected event.
        // ============================================================================================
        // ============================================================================================
        {
            zp::net::server::ClientConnectedEvt evt = {};
            evt.client_id                           = peer;

            p_inst->server_state.on_client_connected.trigger(evt);
        }
    }

    // =========================================================================================================================================
    // =========================================================================================================================================
    // process_disconnect: Logs disconnection, updates the peer registry, and raises the disconnected event.
    // =========================================================================================================================================
    // =========================================================================================================================================
    void process_disconnect(zp::net::Instance* p_inst, ENetPeer* peer)
    {
        // ============================================================================================
        // ============================================================================================
        // record disconnect for diagnostic purposes.
        // ============================================================================================
        // ============================================================================================
        {
            LOG("peer: " << peer << " disconnected.");
        }

        // ============================================================================================
        // ============================================================================================
        // remove the peer from the active client registry.
        // ============================================================================================
        // ============================================================================================
        {
            p_inst->server_state.p_i->connected_clients.erase(peer);
        }

        // ============================================================================================
        // ============================================================================================
        // fire the application-facing client-disconnected event.
        // ============================================================================================
        // ============================================================================================
        {
            zp::net::server::ClientDisconnectedEvt evt = {};
            evt.client_id                              = peer;

            p_inst->server_state.on_client_disconnected.trigger(evt);
        }
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// start_server: Creates the ENet host for accepting client connections and primes server state.
// =========================================================================================================================================
// =========================================================================================================================================
bool zp::net::server::start_server(zp::net::Instance* p_inst)
{
    // ============================================================================================
    // ============================================================================================
    // allocate state internals used by the ENet server backend.
    // ============================================================================================
    // ============================================================================================
    {
        p_inst->server_state.p_i       = new zp::net::server::State::Internal();
        p_inst->server_state.p_i->host = nullptr;
    }

    ENetAddress addr               = {};
    addr.host                      = ENET_HOST_ANY;
    addr.port                      = p_inst->server_config.port;

    p_inst->server_state.p_i->host = enet_host_create(&addr, 32, 2, 0, 0);

    if (p_inst->server_state.p_i->host == nullptr)
    {
        WARN("error occurred while trying to create an ENet server host");
        delete p_inst->server_state.p_i;
        p_inst->server_state.p_i = nullptr;
        return false;
    }

    LOG("Server started on port " << p_inst->server_state.p_i->host->address.port << " (requested: " << addr.port << ")...");

    return true;
}

// =========================================================================================================================================
// =========================================================================================================================================
// stop_server: Terminates the ENet host, disconnecting all clients and releasing resources.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::net::server::stop_server(zp::net::Instance* p_inst)
{
    if (p_inst->server_state.p_i == nullptr)
    {
        return;
    }

    WARN("stop_server");

    disconnect_all_clients(p_inst);

    enet_host_flush(p_inst->server_state.p_i->host);
    enet_host_destroy(p_inst->server_state.p_i->host);

    // ============================================================================================
    // ============================================================================================
    // release server internals allocated during start_server.
    // ============================================================================================
    // ============================================================================================
    {
        delete p_inst->server_state.p_i;
        p_inst->server_state.p_i = nullptr;
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// disconnect_client: Attempts graceful ENet disconnection for a single client and falls back to force reset.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::net::server::disconnect_client(zp::net::Instance* p_inst, zp::net::server::NetId client)
{
    WARN("disconnecting: " << client);

    ENetPeer* peer = static_cast<ENetPeer*>(client);

    enet_peer_disconnect(peer, 0);

    ENetEvent evt;
    if (enet_host_service(p_inst->server_state.p_i->host, &evt, 2000) && evt.type == ENET_EVENT_TYPE_DISCONNECT)
    {
        WARN("clean disconnect");
    }
    else
    {
        WARN("no disconnect ack, force reset");
        enet_peer_reset(peer);
    }

    process_disconnect(p_inst, peer);
}

// =========================================================================================================================================
// =========================================================================================================================================
// disconnect_all_clients: Walks all active peers requesting graceful disconnect, then force resets leftovers.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::net::server::disconnect_all_clients(zp::net::Instance* p_inst)
{
    WARN("disconnect_all_clients");

    for (const auto& peer : p_inst->server_state.p_i->connected_clients)
    {
        enet_peer_disconnect(peer, 0);
    }

    ENetEvent evt;
    std::uint32_t timeLeft    = zp::net::server::DISCONNECT_ALL_CLIENTS_TIMEOUT_MS;
    const std::uint32_t slice = 100;
    while (timeLeft > 0 && enet_host_service(p_inst->server_state.p_i->host, &evt, slice) > 0)
    {
        if (evt.type == ENET_EVENT_TYPE_DISCONNECT)
        {
            process_disconnect(p_inst, evt.peer);
        }
        timeLeft = (timeLeft > slice ? timeLeft - slice : 0);
    }

    std::vector<ENetPeer*> leftovers;
    // ============================================================================================
    // ============================================================================================
    // gather peers that did not disconnect during the timeout window for force reset.
    // ============================================================================================
    // ============================================================================================
    {
        leftovers.reserve(p_inst->server_state.p_i->connected_clients.size());
        for (const auto& peer : p_inst->server_state.p_i->connected_clients)
        {
            leftovers.push_back(peer);
        }
    }

    for (auto* peer : leftovers)
    {
        enet_peer_reset(peer);
        process_disconnect(p_inst, peer);
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// handle_incoming: Pumps the ENet server host to capture connects, disconnects, and inbound packets.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::net::server::handle_incoming(zp::net::Instance* p_inst)
{
    static ENetEvent event;

    p_inst->server_state.transient.incoming.clear();

    while (enet_host_service(p_inst->server_state.p_i->host, &event, zp::net::EVENT_PROCESSING_TIMEOUT_MS) > 0)
    {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
            {
                process_connect(p_inst, event.peer);
            }
            break;

            case ENET_EVENT_TYPE_RECEIVE:
            {
                // ============================================================================================
                // ============================================================================================
                // decode wire format payload into NetEventIn stored on the transient queue.
                // ============================================================================================
                // ============================================================================================
                {
                    static std::size_t running_offset;

                    std::uint8_t* p_bytes = event.packet->data;
                    running_offset        = 0;

                    zp::net::EventId event_id;
                    std::memcpy(&event_id, p_bytes + running_offset, sizeof(zp::net::EventId));
                    running_offset += sizeof(zp::net::EventId);

                    std::uint16_t params_size;
                    std::memcpy(&params_size, p_bytes + running_offset, sizeof(std::uint16_t));
                    running_offset += sizeof(std::uint16_t);

                    std::vector<std::uint8_t> param_bytes(params_size);
                    std::memcpy(param_bytes.data(), p_bytes + running_offset, params_size);
                    running_offset                      += params_size;

                    zp::net::server::NetEventIn net_evt  = {};
                    net_evt.src                          = event.peer;
                    net_evt.event_id                     = event_id;
                    net_evt.param_bytes                  = std::move(param_bytes);

                    p_inst->server_state.transient.incoming.push_back(std::move(net_evt));
                }

                enet_packet_destroy(event.packet);
            }
            break;

            case ENET_EVENT_TYPE_DISCONNECT:
            {
                process_disconnect(p_inst, event.peer);
            }
            break;

            default: break;
        }
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// handle_outgoing: Drains queued outbound events by sending them through ENet and clears the queue.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::net::server::handle_outgoing(zp::net::Instance* p_inst)
{
    for (auto&& event : p_inst->server_state.transient.outgoing)
    {
        send_event(p_inst, std::move(event));
    }
    p_inst->server_state.transient.outgoing.clear();
}

// =========================================================================================================================================
// =========================================================================================================================================
// send_event: Serializes server outbound payload and transmits to the requested destination peers reliably.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::net::server::send_event(zp::net::Instance* p_inst, zp::net::server::NetEventOut net_event)
{
    ENetPacket* packet;
    // ============================================================================================
    // ============================================================================================
    // encode event metadata followed by payload bytes into a reusable packet buffer.
    // ============================================================================================
    // ============================================================================================
    {
        static std::size_t running_offset;
        static unsigned char buffer[UINT16_MAX];

        running_offset = 0;

        std::memcpy(buffer + running_offset, &net_event.event_id, sizeof(zp::net::EventId));
        running_offset     += sizeof(zp::net::EventId);

        std::uint16_t size  = static_cast<std::uint16_t>(net_event.param_bytes.size());
        std::memcpy(buffer + running_offset, &size, sizeof(std::uint16_t));
        running_offset += sizeof(std::uint16_t);

        std::memcpy(buffer + running_offset, net_event.param_bytes.data(), net_event.param_bytes.size());
        running_offset += net_event.param_bytes.size();

        packet          = enet_packet_create(buffer, running_offset, ENET_PACKET_FLAG_RELIABLE);
    }

    // ============================================================================================
    // ============================================================================================
    // send packet to each destination peer and flush the host to ensure transmission.
    // ============================================================================================
    // ============================================================================================
    {
        for (auto&& net_id : net_event.dest)
        {
            enet_peer_send(static_cast<ENetPeer*>(net_id), 0, packet);
        }
        enet_host_flush(p_inst->server_state.p_i->host);
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// get_server_port: Returns the actual port the server is listening on.
// =========================================================================================================================================
// =========================================================================================================================================
std::uint16_t zp::net::server::get_server_port(zp::net::Instance* p_inst)
{
    if (p_inst->server_state.p_i == nullptr || p_inst->server_state.p_i->host == nullptr)
    {
        return 0;
    }

    return p_inst->server_state.p_i->host->address.port;
}

// =========================================================================================================================================
// =========================================================================================================================================
// start_client: Initializes an ENet client host ready to connect to remote servers.
// =========================================================================================================================================
// =========================================================================================================================================
bool zp::net::client::start_client(zp::net::Instance* p_inst)
{
    // ============================================================================================
    // ============================================================================================
    // allocate state internals used by the ENet client backend.
    // ============================================================================================
    // ============================================================================================
    {
        p_inst->client_state.p_i         = new zp::net::client::State::Internal();
        p_inst->client_state.p_i->host   = nullptr;
        p_inst->client_state.p_i->server = nullptr;
    }

    p_inst->client_state.p_i->host = enet_host_create(nullptr, 1, 2, 0, 0);

    if (p_inst->client_state.p_i->host == nullptr)
    {
        WARN("An error occurred while trying to create an ENet client host.");
        delete p_inst->client_state.p_i;
        p_inst->client_state.p_i = nullptr;
        return false;
    }

    return true;
}

// =========================================================================================================================================
// =========================================================================================================================================
// stop_client: Disconnects from the server if needed and releases ENet client resources.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::net::client::stop_client(zp::net::Instance* p_inst)
{
    if (p_inst->client_state.p_i == nullptr)
    {
        return;
    }

    WARN("stop_client");

    if (p_inst->client_state.p_i->server)
    {
        disconnect_from_server(p_inst);
    }

    enet_host_flush(p_inst->client_state.p_i->host);
    enet_host_destroy(p_inst->client_state.p_i->host);

    // ============================================================================================
    // ============================================================================================
    // release client internals allocated during start_client.
    // ============================================================================================
    // ============================================================================================
    {
        delete p_inst->client_state.p_i;
        p_inst->client_state.p_i = nullptr;
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// connect_to_server: Resolves server address, establishes ENet connection, and records connection state.
// =========================================================================================================================================
// =========================================================================================================================================
bool zp::net::client::connect_to_server(zp::net::Instance* p_inst)
{
    ENetAddress address;
    ENetEvent event;

    enet_address_set_host(&address, p_inst->client_config.server_addr.c_str());
    address.port          = p_inst->client_config.server_port;

    ENetPeer* server_peer = enet_host_connect(p_inst->client_state.p_i->host, &address, 2, 0);
    if (server_peer == nullptr)
    {
        WARN("No available peers for initiating an ENet connection.");
        return false;
    }

    // ============================================================================================
    // ============================================================================================
    // block until connection succeeds or timeout to determine handshake result.
    // ============================================================================================
    // ============================================================================================
    {
        if (enet_host_service(p_inst->client_state.p_i->host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
        {
            LOG("Connection to server succeeded.");
        }
        else
        {
            WARN("Connection to server failed.");
            enet_peer_reset(server_peer);
            return false;
        }
    }

    p_inst->client_state.transient   = {};
    p_inst->client_state.p_i->server = server_peer;

    p_inst->client_state.on_connected_to_server.trigger({});

    return true;
}

// =========================================================================================================================================
// =========================================================================================================================================
// disconnect_from_server: Initiates ENet disconnect handshake and falls back to reset when needed.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::net::client::disconnect_from_server(zp::net::Instance* p_inst)
{
    enet_peer_disconnect(p_inst->client_state.p_i->server, 0);

    ENetEvent evt;
    if (enet_host_service(p_inst->client_state.p_i->host, &evt, 2000) && evt.type == ENET_EVENT_TYPE_DISCONNECT)
    {
        WARN("clean disconnect");
    }
    else
    {
        WARN("no disconnect ack, force reset");
        enet_peer_reset(p_inst->client_state.p_i->server);
    }

    p_inst->client_state.transient   = {};
    p_inst->client_state.p_i->server = nullptr;

    p_inst->client_state.on_disconnected_from_server.trigger({});
}

// =========================================================================================================================================
// =========================================================================================================================================
// handle_incoming: Pumps the ENet client host capturing inbound packets and server disconnects.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::net::client::handle_incoming(zp::net::Instance* p_inst)
{
    static ENetEvent event;

    p_inst->client_state.transient.incoming.clear();

    while (enet_host_service(p_inst->client_state.p_i->host, &event, zp::net::EVENT_PROCESSING_TIMEOUT_MS) > 0)
    {
        if (event.type == ENET_EVENT_TYPE_RECEIVE)
        {
            // ============================================================================================
            // ============================================================================================
            // decode payload into NetEvent and store on the transient queue for consumer processing.
            // ============================================================================================
            // ============================================================================================
            {
                static std::size_t running_offset;

                std::uint8_t* p_bytes = event.packet->data;
                running_offset        = 0;

                zp::net::EventId event_id;
                std::memcpy(&event_id, p_bytes + running_offset, sizeof(zp::net::EventId));
                running_offset += sizeof(zp::net::EventId);

                std::uint16_t params_size;
                std::memcpy(&params_size, p_bytes + running_offset, sizeof(std::uint16_t));
                running_offset += sizeof(std::uint16_t);

                std::vector<std::uint8_t> param_bytes(params_size);
                std::memcpy(param_bytes.data(), p_bytes + running_offset, params_size);
                running_offset                    += params_size;

                zp::net::client::NetEvent net_evt  = {};
                net_evt.event_id                   = event_id;
                net_evt.param_bytes                = std::move(param_bytes);

                p_inst->client_state.transient.incoming.push_back(std::move(net_evt));
            }

            enet_packet_destroy(event.packet);
        }
        else if (event.type == ENET_EVENT_TYPE_DISCONNECT)
        {
            WARN("server disconnected client");

            p_inst->client_state.transient   = {};
            p_inst->client_state.p_i->server = nullptr;

            p_inst->client_state.on_disconnected_from_server.trigger({});
        }
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// handle_outgoing: Drains queued outbound client events through ENet then clears the queue.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::net::client::handle_outgoing(zp::net::Instance* p_inst)
{
    for (auto&& event : p_inst->client_state.transient.outgoing)
    {
        send_event(p_inst, std::move(event));
    }
    p_inst->client_state.transient.outgoing.clear();
}

// =========================================================================================================================================
// =========================================================================================================================================
// is_connected_to_server: Reports whether the client currently tracks an active ENet server peer.
// =========================================================================================================================================
// =========================================================================================================================================
bool zp::net::client::is_connected_to_server(zp::net::Instance* p_inst)
{
    return p_inst->client_state.p_i->server != nullptr;
}

// =========================================================================================================================================
// =========================================================================================================================================
// send_event: Serializes client payloads reliably to the connected server peer.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::net::client::send_event(zp::net::Instance* p_inst, zp::net::client::NetEvent net_event)
{
    ENetPacket* packet;
    // ============================================================================================
    // ============================================================================================
    // encode event metadata and payload into an ENet packet for transmission.
    // ============================================================================================
    // ============================================================================================
    {
        static std::size_t running_offset;
        static unsigned char buffer[UINT16_MAX];

        running_offset = 0;

        std::memcpy(buffer + running_offset, &net_event.event_id, sizeof(zp::net::EventId));
        running_offset     += sizeof(zp::net::EventId);

        std::uint16_t size  = static_cast<std::uint16_t>(net_event.param_bytes.size());
        std::memcpy(buffer + running_offset, &size, sizeof(std::uint16_t));
        running_offset += sizeof(std::uint16_t);

        std::memcpy(buffer + running_offset, net_event.param_bytes.data(), net_event.param_bytes.size());
        running_offset += net_event.param_bytes.size();

        packet          = enet_packet_create(buffer, running_offset, ENET_PACKET_FLAG_RELIABLE);
    }

    // ============================================================================================
    // ============================================================================================
    // send packet to the connected server peer and flush immediately.
    // ============================================================================================
    // ============================================================================================
    {
        enet_peer_send(p_inst->client_state.p_i->server, 0, packet);
        enet_host_flush(p_inst->client_state.p_i->host);
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// init: Initializes global ENet state required by client and server APIs.
// =========================================================================================================================================
// =========================================================================================================================================
bool zp::net::init(zp::net::Instance* p_inst)
{
    if (enet_initialize() != 0)
    {
        WARN("An error occurred while initializing ENet.");
        return false;
    }
    return true;
}

// =========================================================================================================================================
// =========================================================================================================================================
// exit: Tears down global ENet state initialized by init().
// =========================================================================================================================================
// =========================================================================================================================================
void zp::net::exit(zp::net::Instance* /*p_inst*/)
{
    enet_deinitialize();
}
