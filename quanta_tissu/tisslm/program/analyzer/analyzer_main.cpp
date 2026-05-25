#include "error_wrapper.hpp"
#include "error_types.hpp"
#include "rpc_protocol.hpp"
#include "analyzer_queue.hpp"
#include "filter_manager.hpp"
#include "file_buffer.hpp"
#include "benchmark.hpp"
#include "keyboard_input.hpp"

#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

using namespace rma;

// Global state for signal handling
static std::atomic<bool> g_running{true};

static void signal_handler(int sig) {
    (void)sig;
    g_running.store(false);
}

// Deserialize and create appropriate ErrorNode based on type_id
static ErrorNodeBase* deserialize_error(const uint8_t* data, size_t len,
                                         uint32_t type_id) {
    if (len < ErrorWrapper<int>::min_serialized_size()) return nullptr;

    // Dispatch based on type_id
    switch (type_id) {
        case 1: { // int
            auto wrapper = ErrorWrapper<int>::deserialize(data, len);
            return new ErrorNode<int>(wrapper);
        }
        case 2: { // double
            auto wrapper = ErrorWrapper<double>::deserialize(data, len);
            return new ErrorNode<double>(wrapper);
        }
        case 3: { // float
            auto wrapper = ErrorWrapper<float>::deserialize(data, len);
            return new ErrorNode<float>(wrapper);
        }
        case 4: { // int64_t
            auto wrapper = ErrorWrapper<int64_t>::deserialize(data, len);
            return new ErrorNode<int64_t>(wrapper);
        }
        case 5: { // uint64_t
            auto wrapper = ErrorWrapper<uint64_t>::deserialize(data, len);
            return new ErrorNode<uint64_t>(wrapper);
        }
        default: { // generic/unknown - use int
            auto wrapper = ErrorWrapper<int>::deserialize(data, len);
            return new ErrorNode<int>(wrapper);
        }
    }
}

// Receiver thread - reads from RPC and enqueues
static void receiver_thread(RpcProtocol& rpc, AnalyzerQueue& queue,
                            Benchmark& bench) {
    std::vector<uint8_t> buffer(64 * 1024);

    while (g_running.load()) {
        bench.start("rpc_receive");
        uint32_t type_id = 0;
        size_t len = rpc.receive(buffer.data(), buffer.size(), &type_id);
        bench.stop("rpc_receive");

        if (len > 0) {
            bench.start("deserialize");
            ErrorNodeBase* node = deserialize_error(buffer.data(), len, type_id);
            bench.stop("deserialize");

            if (node) {
                bench.start("enqueue");
                queue.enqueue(node);
                bench.stop("enqueue");
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

// Format timestamp for display
static std::string format_timestamp(uint64_t ts) {
    // Convert to relative time (simplified - just show as counter)
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%012lu", static_cast<unsigned long>(ts % 1000000000000ULL));
    return std::string(buf);
}

int main(int argc, char* argv[]) {
    // Parse arguments
    uint32_t session_id = 0;
    std::string log_file = "analyzer_log.txt";

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-s" && i + 1 < argc) {
            session_id = static_cast<uint32_t>(std::atoi(argv[++i]));
        } else if (std::string(argv[i]) == "-o" && i + 1 < argc) {
            log_file = argv[++i];
        } else if (std::string(argv[i]) == "-h") {
            std::printf("Runtime Model Analyzer\n");
            std::printf("Usage: %s [-s session_id] [-o log_file]\n", argv[0]);
            std::printf("  -s : Session ID (default: 0)\n");
            std::printf("  -o : Output log file (default: analyzer_log.txt)\n");
            return 0;
        }
    }

    // Setup signal handling
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // Initialize components
    RpcProtocol rpc;
    int retries = 30;
    while (!rpc.init_analyzer(session_id) && retries > 0) {
        std::printf("Waiting for host session %u... (%d retries left)\n", session_id, retries);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        retries--;
    }

    if (!rpc.is_initialized()) {
        std::fprintf(stderr, "Failed to connect to host (session %u) after retries. "
                     "Is the host application running?\n", session_id);
        return 1;
    }

    std::printf("=== Runtime Model Analyzer ===\n");
    std::printf("Connected to session: %u\n", session_id);
    std::printf("Logging to: %s\n\n", log_file.c_str());

    AnalyzerQueue queue(10000);
    FilterManager filters;
    FileBuffer file_buf(log_file, 50);
    Benchmark bench;
    KeyboardInput keyboard;

    // Print initial filter status
    filters.printStatus();

    // Keyboard handler
    keyboard.start([&filters, &bench, &file_buf](char key) {
        switch (key) {
            case 'q':
            case 'Q':
                g_running.store(false);
                break;
            case 'h':
            case 'H':
                filters.printStatus();
                break;
            case 'a':
            case 'A':
                if (key == 'A') {
                    filters.enableAll();
                    std::printf("[All filters ENABLED]\n");
                } else {
                    filters.toggle('a');
                }
                break;
            case 'z':
            case 'Z':
                filters.disableAll();
                std::printf("[All filters DISABLED]\n");
                break;
            case 'b':
            case 'B':
                if (key == 'B') {
                    bench.print_report();
                } else {
                    filters.toggle('b');
                }
                break;
            case 'f':
            case 'F':
                file_buf.flush();
                std::printf("[Buffer flushed: %zu total entries]\n",
                            file_buf.total_entries());
                break;
            default:
                if (filters.toggle(key)) {
                    ErrorType t = key_to_error_type(key);
                    if (t != ErrorType::ERROR_TYPE_COUNT) {
                        std::printf("[%s: %s]\n",
                                    error_type_name(t),
                                    filters.shouldDisplay(t) ? "ON" : "OFF");
                    }
                }
                break;
        }
    });

    // Start receiver thread
    std::thread receiver(receiver_thread, std::ref(rpc), std::ref(queue),
                         std::ref(bench));

    // Main processing loop
    uint64_t processed_count = 0;
    auto last_status = std::chrono::steady_clock::now();

    while (g_running.load()) {
        bench.start("dequeue");
        ErrorNodeBase* node = queue.dequeue(std::chrono::milliseconds(100));
        bench.stop("dequeue");

        if (!node) continue;

        bench.start("process");

        ErrorType type = node->get_type();

        if (filters.shouldDisplay(type)) {
            // Format and display
            std::string ts = format_timestamp(node->get_timestamp());
            std::printf("[%s] %s\n", ts.c_str(), node->format().c_str());

            // Buffer to file
            std::string log_entry = "[" + ts + "] " + node->format();
            file_buf.append(log_entry);
        }

        ++processed_count;

        bench.stop("process");

        delete node;

        // Periodic status (every 5 seconds)
        auto now = std::chrono::steady_clock::now();
        if (now - last_status > std::chrono::seconds(5)) {
            std::printf("--- Status: %lu errors processed, %zu queued, "
                        "%zu buffered ---\n",
                        static_cast<unsigned long>(processed_count),
                        queue.size(),
                        file_buf.buffered_count());
            last_status = now;
        }
    }

    // Cleanup
    std::printf("\nShutting down...\n");

    queue.shutdown();
    keyboard.stop();

    if (receiver.joinable()) {
        receiver.join();
    }

    file_buf.flush();

    std::printf("\n");
    bench.print_report();
    std::printf("Total errors processed: %lu\n",
                static_cast<unsigned long>(processed_count));
    std::printf("Log file: %s (%zu entries)\n",
                log_file.c_str(), file_buf.total_entries());

    return 0;
}
