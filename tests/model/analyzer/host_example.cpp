// Example host application demonstrating ErrorHandler usage
#include "error_handler.hpp"
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>

using namespace rma;

// Simulate game loop with various errors
void game_loop() {
    int frame = 0;
    int* dangerous_ptr = nullptr;
    float precision_test = 0.1f;

    while (frame < 100) {
        ++frame;

        // Simulate various error conditions
        if (frame % 10 == 0) {
            RMA_DEBUG("Frame checkpoint");
        }

        if (frame == 15) {
            RMA_WARNING("Memory usage increasing");
        }

        if (frame == 25) {
            dangerous_ptr = nullptr;
            RMA_NULL_CHECK(dangerous_ptr);
        }

        if (frame == 35) {
            int array_size = 10;
            int bad_index = 15;
            RMA_BOUNDS_CHECK(bad_index, array_size);
        }

        if (frame == 45) {
            precision_test += 0.1f;
            if (precision_test != 0.5f) { // Known float issue
                ErrorHandler::instance().report_float(
                    ErrorType::FLOAT_PRECISION,
                    static_cast<double>(precision_test),
                    __FILE__, __LINE__,
                    "Float precision loss detected");
            }
        }

        if (frame == 55) {
            RMA_ERROR_FMT(ErrorType::LOGIC_BUG,
                          "Unexpected state at frame %d", frame);
        }

        if (frame == 65) {
            RMA_ERROR(ErrorType::UNDEFINED_METHOD,
                      "Called deprecated API");
        }

        if (frame == 75) {
            RMA_ERROR_VAL(ErrorType::ILLEGAL_KEYWORD,
                          42, "Invalid token encountered");
        }

        if (frame % 20 == 0) {
            RMA_INFO("Periodic info message");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main(int argc, char* argv[]) {
    uint32_t session_id = 0;
    if (argc > 1) {
        session_id = static_cast<uint32_t>(std::atoi(argv[1]));
    }

    std::printf("Host Application Starting (session %u)\n", session_id);
    std::printf("Start analyzer with: ./analyzer -s %u\n\n", session_id);

    if (!RMA_INIT(session_id)) {
        std::fprintf(stderr, "Warning: Could not initialize error handler. "
                     "Errors will not be transmitted.\n");
    }

    RMA_INFO("Host application started");

    game_loop();

    RMA_INFO("Host application finished");

    std::printf("Host application complete.\n");
    return 0;
}
