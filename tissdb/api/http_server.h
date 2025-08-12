#pragma once

#include <memory>

// Forward declaration of the storage engine to avoid including the full header.
namespace TissDB { namespace Storage { class LSMTree; } }

namespace TissDB {
namespace API {

// Provides an HTTP interface to the TissDB storage engine.
class HttpServer {
public:
    // Constructor takes a reference to the storage engine and the port to listen on.
    HttpServer(Storage::LSMTree& storage_engine, int port);

    // Destructor must be defined in the .cpp file when using PIMPL with std::unique_ptr.
    ~HttpServer();

    // Starts the server. This will block the calling thread.
    void start();

    // Stops the running server.
    void stop();

private:
    // A private method to configure all the API endpoints.
    void setup_routes();

    // PIMPL (Pointer to Implementation) idiom to hide the httplib implementation
    // details from this public header file. This reduces compile times and
    // decouples our interface from the specific library used.
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace API
} // namespace TissDB
