# 🔧 PulseProto Design Requirements Document

---

## 🎯 Project Goal

Build a **C++-native, zero-dependency**, efficient, minimal UDP protocol library that replaces ENet with a leaner, saner, higher-performance alternative. No bloat. No callbacks from hell. No hand-holding. Just raw power with a clean API.

---

## 🧱 Core Requirements & Their Architecture

### 1. ✅ Client-Server Only Model

* **Requirement**: One server, N clients.
* **Design**:

  * Server holds a single bound `pulseudp::UDPSocket`.
  * Each client is tracked by a `Session` object, identified by its `UDPAddr` (IP\:port).
  * Clients are state machines:

    * `Connecting`, `Connected`, `TimedOut`, `Disconnected`.

### 2. ✅ Explicit Packet Header Formats

* **Requirement**: Small, efficient, distinguishable packet headers.
* **Design**:

  * Use a bitfield in `type_flags` to determine packet type (4 bits) and flags (4 bits).
  * Versioning and session metadata are **only in connection packets** (e.g. Hello, Challenge).
  * Runtime packet header struct layout:

    * `BaseHeader`, `ReliableHeader`, etc., stored in `packet.h`
  * Binary serialization done via raw memcpy or direct pointer reinterpretation. No serialization libraries. Ever.

### 3. ✅ Reliable Messaging

* **Requirement**: Support reliable channels with ACKs.
* **Design**:

  * Each `Session` tracks per-channel state:

    * Last sent sequence number.
    * Last acknowledged sequence number.
    * Outstanding unacked packets (with resend timers).
  * ACKs are piggybacked or bundled in ACK-only packets if needed.
  * `ReliableChannel` per ID, owned by session.
  * Resend timeout is configured per channel (as you said).

### 4. ✅ Unreliable Messaging

* **Requirement**: Raw fire-and-forget support.
* **Design**:

  * Also a `Channel`, but with zero state.
  * Used for ping, movement, fire events, etc.
  * Packet uses `BaseHeader` with no sequence or ack fields.

### 5. ❌ No Fragmentation

* **Requirement**: Drop packets > 1200 bytes.
* **Design**:

  * `SendPacket()` validates size before sending.
  * Optional callback (logging, metrics) for oversized drops.
  * This removes a whole class of bugs and buffering complexity.

### 6. ✅ Channel System (Typed Streams)

* **Requirement**: Send different types of packets independently.
* **Design**:

  * Each packet includes a `channel_id`.
  * `Session` maps `channel_id` to its corresponding channel logic (Reliable/Unreliable).
  * Channel max is 256 (fits in one byte). Could allow configurable channel-to-class mapping.

### 7. ✅ Packet Reordering & Dropped Packet Handling (Reliable Channels)

* **Requirement**: Deliver reliable packets in-order.
* **Design**:

  * Out-of-order packets are buffered until all earlier packets arrive.
  * Packet receive buffer is a ring buffer keyed on `sequence % N`.
  * Late packets are dropped after timeout or overwrite window.

### 8. ✅ Ping/Latency Measurement

* **Requirement**: Measure and smooth round-trip time.
* **Design**:

  * `PingPacket` includes a `timestamp_ns` from client.
  * Server uses its own time to calculate RTT.
  * Ping is handled via a special control channel.

### 9. ✅ Disconnect/Timeout Handling

* **Requirement**: Kill idle or nonresponsive clients.
* **Design**:

  * Each `Session` tracks last-recv timestamp.
  * If delta > timeout threshold (configurable), session is killed.
  * `GoodbyePacket` may be sent prior to disconnect, but is not guaranteed.

### 10. ✅ Hello Handshake & Versioning

* **Requirement**: Initial handshake must support future protocol features.
* **Design**:

  * `HelloPacket` has a `version_byte` and payload blob.
  * `Session` isn’t initialized until Hello/Ack completes.
  * Future versions can negotiate different handshakes if needed.

### 11. ❌ No Encryption / TLS

* **Requirement**: No crypto in the protocol.
* **Design**:

  * You want crypto? Use TLS over REST for login/auth. Not here.

### 12. ✅ Max Sessions / Max Packet Size

* **Requirement**: Control memory usage.
* **Design**:

  * Server has hard cap on max clients.
  * MTU enforced as 1200 bytes.
  * Packet pools optionally used to avoid malloc churn (later optimization).

### 13. ✅ Logging / Telemetry (Optional)

* **Requirement**: Log internal state for debug/ops.
* **Design**:

  * `ILogger` interface is injected into protocol runtime.
  * Default is noop. You want logs? Hook them up.

---

## 📦 Component Overview

### `Session`

* Owns channels.
* Tracks state, last ping, last packet recv time.
* Responsible for sending and receiving.

### `Channel`

* Base class for `ReliableChannel`, `UnreliableChannel`
* Handles packet formatting, buffering, sequencing.

### `Packet`

* Static helpers for encoding/decoding headers and payloads.

### `Protocol`

* Orchestrates send/recv loop, handles session management, packet routing.

---

## 🧪 Testing Strategy

* Unit test every component (Session, Packet, Channel)
* Simulated sockets for loopback
* Soak test for long-term connection stability (run in CI)

---

## 🚫 Things We Are NOT Doing

* ❌ NAT traversal
* ❌ Framing arbitrary TCP over UDP
* ❌ Fragmentation support
* ❌ Secure channels
* ❌ Reliable unordered (pointless)
* ❌ TLS or encryption
* ❌ Realtime replication framework