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
    auto listenAddr = udp::Addr(udp::Addr::AnyIPv4, SERVER_PORT);
    auto serverAddr = udp::Addr("127.0.0.1", SERVER_PORT);

    auto serverSockResult = udp::Listen(listenAddr);
    assert(serverSockResult);
    auto& serverSock = **serverSockResult;

    auto clientSockResult = udp::Dial(serverAddr);
    assert(clientSockResult);
    auto& clientSock = **clientSockResult;


    std::unique_ptr<proto::Protocol> serverProto, clientProto;

    bool serverReceived = false;
    bool clientReceived = false;
    std::string receivedMsg;

    serverProto = proto::CreateProtocol(
        serverSock,
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
            serverReceived = true;

            // Echo back
            sess.sendReliable(CHANNEL_SERVER_TO_CLIENT, buf);
        },
        [&](const udp::Addr&) {
            std::cout << "[Server] Client disconnected.\n";
        }
    ).value();

    clientProto = proto::CreateProtocol(
        clientSock,
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
            receivedMsg = msg;
            clientReceived = true;
        },
        [&](const udp::Addr&) {
            std::cout << "[Client] Server disconnected.\n";
        }
    ).value();

    // Manually spoof a hello by sending something from client
    auto clientSessionResult = clientProto->connect(serverAddr);
    assert(clientSessionResult);
    auto clientSession = *clientSessionResult;

    std::string hello = "hello";
    clientSession->sendReliable(CHANNEL_CLIENT_TO_SERVER, proto::BufferView{
        reinterpret_cast<const uint8_t*>(hello.data()), hello.size()
    });

    // Pump both loops
    for (int i = 0; i < 20 && !(serverReceived && clientReceived); ++i) {
        clientProto->tick();
        serverProto->tick();
        sleepMs(10);
    }

    assert(serverReceived);
    assert(clientReceived);
    assert(receivedMsg == "hello");

    std::cout << "[Test] Reliable message echoed successfully.\n";

    // Let idle timeout trigger
    sleepMs(12000);
    serverProto->tick();
    clientProto->tick();

    assert(serverProto->sessions().empty() || clientProto->sessions().empty());

    std::cout << "[Test] Session timeout cleanup verified.\n";
    return 0;
}
