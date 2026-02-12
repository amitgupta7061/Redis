#pragma once

namespace rediss {

/// Top-level application orchestrator.
///
/// Owns the KVStore and CommandHandler, creates the TcpServer, and wires
/// the full pipeline:  raw input → parse → execute → response.
///
/// This is the only class that holds ownership of core components.
class ServerApp {
public:
    /// Start the server on the given port (blocking).
    void run(int port);
};

} // namespace rediss
