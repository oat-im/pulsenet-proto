#include <pulse/net/proto/proto.h>
#include <pulse/net/udp/udp.h>
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

using namespace pulse::net;

constexpr uint16_t SERVER_PORT = 40000;
constexpr uint8_t CHANNEL_CLIENT_TO_SERVER = 1;
constexpr uint8_t CHANNEL_SERVER_TO_CLIENT = 2;

void sleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main() {
    auto listen_addr_result = udp::Addr::Create(udp::Addr::kAnyIPv4, SERVER_PORT);
    assert(listen_addr_result);
    auto& listen_addr = *listen_addr_result;
    auto server_addr_result = udp::Addr::Create("127.0.0.1", SERVER_PORT);
    assert(server_addr_result);
    auto& server_addr = *server_addr_result;

    auto server_sock_result = udp::get_socket_factory()->listen(listen_addr);
    assert(server_sock_result);
    auto& server_sock = **server_sock_result;

    auto client_sock_result = udp::get_socket_factory()->dial(server_addr);
    assert(client_sock_result);
    auto& client_sock = **client_sock_result;


    std::unique_ptr<proto::Protocol> server_proto, client_proto;

    bool server_received = false;
    bool client_received = false;
    std::string receive_msg;

    server_proto = proto::CreateProtocol(
        server_sock,
        [&](proto::Session& sess, uint8_t channel, proto::BufferView buf) {
            assert(channel == CHANNEL_CLIENT_TO_SERVER);
            assert(buf.size == 5);
            std::string msg(reinterpret_cast<const char*>(buf.data), buf.size);
            if (msg != "hello") {
                std::cerr << "Assertion failed: msg != \"hello\"\n";
                std::cerr << "msg.size() = " << msg.size() << "\n";
                for (size_t i = 0; i < msg.size(); ++i) {
                    std::cerr << "msg[" << i << "] = 0x" << std::hex << static_cast<int>(msg[i]) << "\n";
                }
                std::abort();
            } else {
                std::cout << "[Server] Received: " << msg << "\n";
            }
            server_received = true;

            // Echo back
            sess.sendReliable(CHANNEL_SERVER_TO_CLIENT, buf);
        },
        [&](const udp::Addr&) {
            std::cout << "[Server] Client disconnected.\n";
        }
    ).value();

    client_proto = proto::CreateProtocol(
        client_sock,
        [&](proto::Session& sess, uint8_t channel, proto::BufferView buf) {
            assert(channel == CHANNEL_SERVER_TO_CLIENT);
            assert(buf.size == 5);
            std::string msg(reinterpret_cast<const char*>(buf.data), buf.size);
            if (msg != "hello") {
                std::cerr << "Assertion failed: msg != \"hello\"\n";
                std::cerr << "msg.size() = " << msg.size() << "\n";
                for (size_t i = 0; i < msg.size(); ++i) {
                    std::cerr << "msg[" << i << "] = 0x" << std::hex << static_cast<int>(msg[i]) << "\n";
                }
                std::abort();
            } else {
                std::cout << "[Client] Received: " << msg << "\n";
            }
            receive_msg = msg;
            client_received = true;
        },
        [&](const udp::Addr&) {
            std::cout << "[Client] Server disconnected.\n";
        }
    ).value();

    // Manually spoof a hello by sending something from client
    auto client_session_result = client_proto->connect(server_addr);
    assert(client_session_result);
    auto client_session = *client_session_result;

    std::string hello = "hello";
    client_session->sendReliable(CHANNEL_CLIENT_TO_SERVER, proto::BufferView{
        reinterpret_cast<const uint8_t*>(hello.data()), hello.size()
    });

    // Pump both loops
    for (int i = 0; i < 20 && !(server_received && client_received); ++i) {
        client_proto->tick();
        server_proto->tick();
        sleepMs(10);
    }

    assert(server_received);
    assert(client_received);
    assert(receive_msg == "hello");

    std::cout << "[Test] Reliable message echoed successfully.\n";

    // Let idle timeout trigger
    sleepMs(12000);
    server_proto->tick();
    client_proto->tick();

    assert(server_proto->sessions().empty() || client_proto->sessions().empty());

    std::cout << "[Test] Session timeout cleanup verified.\n";
    return 0;
}
