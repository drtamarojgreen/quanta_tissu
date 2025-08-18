#pragma once

#include <memory>

// Forward declaration of the database manager to avoid including the full header.
namespace TissDB { namespace Storage { class DatabaseManager; } }

namespace TissDB {
namespace API {

// Provides an HTTP interface to the TissDB storage engine.
class HttpServer {
public:
    // Constructor takes a reference to the database manager and the port to listen on.
    HttpServer(Storage::DatabaseManager& db_manager, int port);

    // Destructor must be defined in the .cpp file when using PIMPL with std::unique_ptr.
    ~HttpServer();

    // Starts the server. This will block the calling thread.
    void start();

    // Stops the running server.
    void stop();

private:
    // PIMPL (Pointer to Implementation) idiom to hide the low-level socket
    // implementation details from this public header file. This reduces compile
    // times and decouples our interface from the specific implementation.
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace API
} // namespace TissDB
