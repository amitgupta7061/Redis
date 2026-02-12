<p align="center">
  <img src="https://img.shields.io/badge/C++-17-blue?style=for-the-badge&logo=cplusplus&logoColor=white" />
  <img src="https://img.shields.io/badge/Linux-epoll-orange?style=for-the-badge&logo=linux&logoColor=white" />
  <img src="https://img.shields.io/badge/Architecture-Modular-green?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Thread_Safe-shared__mutex-purple?style=for-the-badge" />
</p>

<h1 align="center">⚡ Rediss</h1>
<p align="center">
  <strong>A Redis-like in-memory key-value database built from scratch in C++17</strong>
</p>
<p align="center">
  Production-grade architecture • epoll event loop • RESP protocol • Thread-safe design
</p>

---

## 🎯 What is Rediss?

**Rediss** is a lightweight, high-performance, Redis-compatible in-memory key-value store built entirely from scratch in modern C++17. It demonstrates real-world systems engineering — non-blocking I/O, protocol parsing, command routing, and thread-safe data storage — all without any external dependencies.

> Built for learning **system design**, **networking**, and **backend engineering** at a production level.

---

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        CLIENT (nc / redis-cli)                  │
│                      Sends: SET name amit\r\n                   │
└───────────────────────────┬─────────────────────────────────────┘
                            │ TCP Connection (port 6379)
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│                     🌐 TCP SERVER (epoll)                       │
│                                                                 │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐         │
│  │ Client 1 │  │ Client 2 │  │ Client 3 │  │ Client N │         │
│  │  Buffer  │  │  Buffer  │  │  Buffer  │  │  Buffer  │         │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘         │
│       └──────────────┴──────────────┴──────────────┘            │
│                           │                                     │
│                    Raw line: "SET name amit"                    │
└───────────────────────────┬─────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│                    📝 COMMAND PARSER                            │
│                                                                 │
│   Input:  "SET name amit\r\n"                                   │
│                    │                                            │
│        ┌───────────┼───────────┐                                │
│        ▼           ▼           ▼                                │
│    ┌───────┐  ┌────────┐  ┌────────┐                            │
│    │  SET  │  │  name  │  │  amit  │                            │
│    │(verb) │  │ (arg1) │  │ (arg2) │                            │
│    └───────┘  └────────┘  └────────┘                            │
│                                                                 │
│   Output: Command { name="SET", args=["name","amit"] }          │
└───────────────────────────┬─────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│                   🎮 COMMAND HANDLER                            │
│                                                                 │
│   ┌─────────┬─────────┬─────────┬──────────┬────────┐           │
│   │   SET   │   GET   │   DEL   │  EXISTS  │  PING  │           │
│   │ key val │   key   │   key   │   key    │        │           │
│   └────┬────┘ └───┬───┘ └───┬───┘ └───┬────┘ └──┬───┘           │
│        └──────────┴─────────┴─────────┴──────────┘              │
│                           │                                     │
└───────────────────────────┬─────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│                    💾 KV STORE (Thread-Safe)                    │
│                                                                 │
│     ┌─────────────────────────────────────────────┐             │
│     │       std::unordered_map<string, string>    │             │
│     │                                             │             │
│     │   ┌────────┬─────────┐                      │             │
│     │   │  Key   │  Value  │                      │             │
│     │   ├────────┼─────────┤                      │             │
│     │   │ name   │ amit    │                      │             │
│     │   │ count  │ 42      │                      │             │
│     │   │ lang   │ c++     │                      │             │
│     │   └────────┴─────────┘                      │             │
│     │                                             │             │
│     │   🔒 Protected by std::shared_mutex         │             │
│     │   • Concurrent reads  (shared_lock)         │             │
│     │   • Exclusive writes  (unique_lock)         │             │
│     └─────────────────────────────────────────────┘             │
│                                                                 │
│   Returns: result → Handler formats → "+OK\r\n" → Client        │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📂 Project Structure

```
rediss/
├── 📄 CMakeLists.txt              # CMake build configuration
├── 📄 Makefile                    # Alternative make build
├── 📄 .gitignore
│
└── src/
    ├── 📄 main.cpp                # Entry point + ASCII banner
    │
    ├── 📁 store/                  # Phase 1: Core Data Store
    │   ├── kv_store.h             #   Thread-safe KVStore interface
    │   └── kv_store.cpp           #   SET, GET, DEL, EXISTS impl
    │
    ├── 📁 parser/                 # Phase 2: Input Parsing
    │   ├── command_parser.h       #   Static parser interface
    │   └── command_parser.cpp     #   Tokenization + CRLF handling
    │
    ├── 📁 commands/               # Phase 3: Command Routing
    │   ├── command.h              #   Command struct definition
    │   ├── command_handler.h      #   Handler interface
    │   └── command_handler.cpp    #   Dispatch + RESP formatting
    │
    ├── 📁 network/                # Phase 4: TCP Networking
    │   ├── tcp_server.h           #   Epoll server interface
    │   └── tcp_server.cpp         #   Non-blocking event loop
    │
    └── 📁 server/                 # Orchestration Layer
        ├── server_app.h           #   App wiring interface
        └── server_app.cpp         #   Compose all components
```

---

## 🚀 Quick Start

### Prerequisites

- **g++** with C++17 support (GCC 7+ or Clang 5+)
- **Linux** (uses `epoll` for I/O multiplexing)

### Build

```bash
git clone https://github.com/amitgupta7061/Redis.git
cd Redis

# Option 1: Make
make

# Option 2: CMake
mkdir build && cd build
cmake .. && make -j$(nproc)
```

### Run

```bash
# Start server (default port: 6379)
./rediss

# Or specify a custom port
./rediss 6380
```

You'll see:
```
  ____          _ _
 |  _ \ ___  __| (_)___ ___
 | |_) / _ \/ _` | / __/ __|
 |  _ <  __/ (_| | \__ \__ \
 |_| \_\___|\__,_|_|___/___/

Rediss server listening on port 6379
```

### Connect & Use

```bash
# In another terminal
nc localhost 6379
```

---

## 📖 Supported Commands

| Command | Syntax | Description | Response |
|---------|--------|-------------|----------|
| **PING** | `PING` | Health check | `+PONG` |
| **SET** | `SET key value` | Store a key-value pair | `+OK` |
| **GET** | `GET key` | Retrieve value by key | `$4\r\namit` or `$-1` (nil) |
| **DEL** | `DEL key [key ...]` | Delete one or more keys | `:1` (count deleted) |
| **EXISTS** | `EXISTS key` | Check if key exists | `:1` or `:0` |

### Example Session

```
$ nc localhost 6379

PING
+PONG

SET name amit
+OK

GET name
$4
amit

SET language cpp
+OK

EXISTS language
:1

DEL name language
:2

GET name
$-1
```

---

## 🔧 Request-Response Flow

```
 Client                    Server
   │                         │
   │   TCP Connect           │
   │ ──────────────────────► │
   │                         │  ← epoll registers new fd
   │                         │
   │   "SET name amit\n"     │
   │ ──────────────────────► │
   │                         │  1. TcpServer reads into buffer
   │                         │  2. Finds \n → extracts line
   │                         │  3. CommandParser::parse()
   │                         │     → Command{SET, [name, amit]}
   │                         │  4. CommandHandler::execute()
   │                         │     → store_.set("name", "amit")
   │                         │  5. Format: "+OK\r\n"
   │   "+OK\r\n"             │
   │ ◄────────────────────── │
   │                         │
   │   "GET name\n"          │
   │ ──────────────────────► │
   │                         │  1. Parse → Command{GET, [name]}
   │                         │  2. store_.get("name") → "amit"
   │                         │  3. Format: "$4\r\namit\r\n"
   │   "$4\r\namit\r\n"      │
   │ ◄────────────────────── │
   │                         │
   │   Disconnect            │
   │ ──────────────────────► │
   │                         │  ← epoll detects, cleanup fd
```

---

## 🧱 Design Decisions

| Decision | Why |
|----------|-----|
| **`shared_mutex`** | Allows concurrent `GET`/`EXISTS` reads without blocking. Only `SET`/`DEL` take exclusive locks. Future-ready for multithreading. |
| **`epoll`** | O(1) event notification vs O(n) for `select`/`poll`. Scales to thousands of connections. Industry standard on Linux. |
| **Per-client buffers** | TCP is a byte stream — data arrives in arbitrary chunks. Buffers accumulate until `\n` forms a complete command. |
| **RESP format** | Compatible with `redis-cli` and Redis client libraries. Enables drop-in testing with real Redis tools. |
| **Stateless parser** | Pure function — input in, `Command` out. No side effects, trivially testable, easily swappable. |
| **No global state** | `ServerApp` owns everything. Components are injected via references. Clean dependency graph. |

---

## 📊 Component Deep Dive

### KVStore — The Heart

```cpp
class KVStore {
    mutable std::shared_mutex mutex_;          // Read-write lock
    std::unordered_map<std::string, std::string> data_;  // O(1) avg lookup

public:
    void set(key, value);                     // unique_lock (exclusive)
    std::optional<std::string> get(key);      // shared_lock (concurrent)
    bool del(key);                            // unique_lock (exclusive)
    bool exists(key);                         // shared_lock (concurrent)
};
```

### TcpServer — The Engine

```
                    epoll_wait()
                         │
              ┌──────────┴──────────┐
              ▼                     ▼
        Listener FD            Client FD
     (new connection)        (data ready)
              │                     │
              ▼                     ▼
         accept()              read() → buffer
         set non-blocking      find '\n'?
         register with epoll     │
                            ┌────┴────┐
                            │ Yes     │ No
                            ▼         ▼
                     process line   wait for
                     write response more data
```

---

## 🛣️ Roadmap (Phase 5)

- [ ] ⏰ **TTL Expiration** — `SET key value EX seconds` with lazy + periodic cleanup
- [ ] 💾 **Persistence** — AOF (append-only file) or RDB (point-in-time snapshots)
- [ ] 🧵 **Multithreading** — Thread pool for command execution (store already thread-safe)
- [ ] ⚡ **More Commands** — `MSET`, `MGET`, `INCR`, `DECR`, `KEYS`, `FLUSHALL`, `TTL`
- [ ] 📡 **Pub/Sub** — Publish-subscribe messaging channels
- [ ] 📋 **Data Structures** — Lists, Sets, Sorted Sets, Hashes

---

## 📄 License

This project is open source and available under the [MIT License](LICENSE).

---

<p align="center">
  Built with ❤️ for learning system design and backend engineering
</p>
