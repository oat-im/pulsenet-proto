<div align="center">
  <img src="https://pulsenet.dev/images/pulse-networking-social.png" alt="Pulse Networking" width="1200">
  <h1>pulse::net::proto</h1>
  <p><strong>Minimal, modern UDP protocol runtime — a leaner alternative to ENet.</strong></p>
  <p>
    <a href="#features">Features</a> •
    <a href="#why">Why?</a> •
    <a href="#usage">Usage</a> •
    <a href="#design">Design</a> •
    <a href="#build-requirements">Build Requirements</a> •
    <a href="#fetchcontent">FetchContent</a> •
    <a href="#license">License</a>
  </p>
</div>

---

`pulse::net::proto` is a C++23-native, zero-dependency, zero-bloat UDP protocol library with full support for reliable messaging, channels, latency measurement, and stateful sessions — all in a sane, modern API.

It does what ENet tried to do, but without the legacy cruft, broken reliability layer, or “callback hell” API.

**Status**: 🔬 Early alpha (v0.1.0)

## 🚀 Features

- ✅ Lightweight client-server session manager over UDP
- ✅ Reliable and unreliable packet delivery
- ✅ Explicit per-channel state
- ✅ Per-session RTT measurement (ping/pong)
- ✅ Packet reordering and receive window handling
- ✅ No fragmentation, no TCP emulation nonsense
- ✅ Pure C++23 (`std::expected`, no exceptions, no macros)
- ✅ Easily mockable interfaces (`Session`, `Protocol`)
- ✅ Pluggable logger + metrics
- ✅ No dynamic memory overhead from fragmentation or excessive buffering
- ✅ Fully deterministic, testable integration model

## 🧠 Why?

Because networking code has been rotting in C++ since before Steam existed. ENet was fine in 2004. It's not 2004.

This library is for building **real-time game servers**, **low-latency control loops**, or anything else where UDP is required and you still want sane abstractions.

It doesn’t reinvent IP. It doesn’t try to guarantee delivery of your dreams. But it **does** give you:

- Reliability with proper ACKs
- Session state machines
- No-crap packet headers
- Predictable flow

If you want a TLS-based RPC layer, you’re lost.

## ⚙️ Usage

```cpp
#include <pulse/net/proto/proto.h>
#include <pulse/net/udp/udp.h>

using namespace pulse::net;

int main() {
    auto serverAddr = udp::Addr("127.0.0.1", 40000);
    auto listenResult = udp::Listen(serverAddr);
    if (!listenResult) return 1;
    auto& socket = **listenResult;

    auto proto = proto::CreateProtocol(
        socket,
        [](proto::Session& session, uint8_t channel, proto::BufferView view) {
            // Process incoming message
        },
        [](const udp::Addr& disconnected) {
            // Handle disconnect
        }
    );

    while (true) {
        proto->tick(); // recv, dispatch, flush
    }
}
````

### Sending a message

```cpp
session.sendReliable(1, proto::BufferView(data, len));
session.sendUnreliable(2, proto::BufferView(data, len));
```

## 🧱 Design

See [`drd.md`](./drd.md) for the full design doc. Here's the short version:

| Feature                    | Description                                                    |
| -------------------------- | -------------------------------------------------------------- |
| Sessions                   | One per client IP\:port, tracked automatically                 |
| Reliable delivery          | Per-channel resend queues, ACK-based delivery                  |
| Unreliable fire-and-forget | Zero-state, minimal header                                     |
| Channels                   | Up to 256, independently managed                               |
| Ping/Pong                  | RTT measurement, smoothed                                      |
| No Fragmentation           | Max packet size is hard-capped at 1200 bytes                   |
| Timeouts                   | Idle sessions are culled automatically                         |
| Protocol versioning        | Handshake has a `version` field for future use                 |
| Logging                    | Pluggable logger interface                                     |
| Metrics                    | Pluggable metrics interface                                    |
| Zero exceptions            | Only constructors throw — everything else uses `std::expected` |

You want handshakes? It has those. You want encryption? Go layer TLS on top.

## 🏗 Build Requirements

* **C++23**
* **CMake ≥ 3.15**
* **pulse::net::udp** (see [here](https://git.pulsenet.dev/pulse/udp))

## 📦 FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
  pulsenet_proto
  GIT_REPOSITORY https://git.pulsenet.dev/pulse/proto
  GIT_TAG        main
)

FetchContent_MakeAvailable(pulsenet_proto)

target_link_libraries(your_target PRIVATE pulsenet::proto)
```

Note: Depends FetchContent will pull the `pulse::net::udp` library as well from the tip of `main` branch.

## ⚖️ License

**AGPLv3**.

Yes, it's intentional.

Want to use this in a proprietary product? [Buy a commercial license](https://pulsenet.dev/) or go write your own UDP protocol stack.

## ❌ Not Included (by design)

* ❌ NAT traversal
* ❌ TLS or encryption
* ❌ Arbitrary TCP-over-UDP framing
* ❌ Fragmentation / jumbo packet support
* ❌ Unreliable unordered delivery
* ❌ Handholding

## 🧪 Testing

Includes standalone integration tests:

```sh
mkdir build && cd build
cmake .. -DPULSENET_PROTO_STANDALONE_BUILD=ON
cmake --build .
./pulsenet_proto_test
```

## Version

**pulse::net::proto v0.1.0**