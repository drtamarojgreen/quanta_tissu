// @Card: verify_training_instrumentation
// @Situation Default
// @Is source_file_size_bytes > 0
// @Results rma_include_line > 0
// @Results rma_call_line > 0
// @Results source_bytes_scanned > 0

// @Card: verify_training_telemetry
// @Situation Default
// @Is analyzer_binary_size_bytes > 0
// @Results telemetry_records_captured >= 1
// @Results last_captured_loss_value > 0.0
// @Results session_execution_duration_ms > 0

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <thread>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) return "error";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int main(int argc, char** argv) {
    if (argc < 2) return 1;
    std::string card = argv[1];

    if (card == "verify_training_instrumentation") {
        char* root = std::getenv("QUANTA_ROOT");
        std::string path = (root ? std::string(root) : "../../../..") + "/quanta_tissu/tisslm/program/training/trainer.cpp";
        std::ifstream file(path);
        std::string line;
        int include_line = 0;
        int call_line = 0;
        int line_count = 0;
        size_t total_bytes = fs::file_size(path);

        while (std::getline(file, line)) {
            line_count++;
            if (line.find("#include \"quanta_tissu/tisslm/program/analyzer/error_handler.hpp\"") != std::string::npos) {
                include_line = line_count;
            }
            if (line.find("RMA_ERROR_VAL(rma::ErrorType::INFO, (double)avg_loss, msg)") != std::string::npos) {
                call_line = line_count;
            }
        }

        std::cout << "source_bytes_scanned = " << total_bytes << std::endl;
        std::cout << "rma_include_line = " << include_line << std::endl;
        std::cout << "rma_call_line = " << call_line << std::endl;
    } else if (card == "verify_training_telemetry") {
        auto start_time = std::chrono::steady_clock::now();

        // 1. Prepare environment
        std::string session_id = "12345";
        std::string log_file = "analyzer_log.txt";
        fs::remove(log_file);

        char* root_env = std::getenv("QUANTA_ROOT");
        std::string root = root_env ? std::string(root_env) : "../../../..";

        // 2. Start Analyzer and capture PID
        std::string analyzer_cmd = root + "/tests/model/analyzer/analyzer -s " + session_id + " -o " + log_file + " > /dev/null 2>&1 & echo $!";
        std::string pid_str = exec(analyzer_cmd.c_str());
        int analyzer_pid = 0;
        try { analyzer_pid = std::stoi(pid_str); } catch(...) {}
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // 3. Run Trainer
        std::string trainer_cmd = "cd " + root + " && rm -rf trained_tokenizer; TISSLM_NUM_EPOCHS=1 TISSLM_MAX_BATCHES=2 TISSLM_VOCAB_SIZE=300 TISSLM_N_EMBD=2 TISSLM_N_LAYER=1 TISSLM_N_HEAD=1 TISSLM_D_FF=2 ./train_model_exe " + session_id + " > /dev/null 2>&1";
        system(trainer_cmd.c_str());

        // 4. Stop Analyzer using captured PID
        if (analyzer_pid > 0) {
            std::string kill_cmd = "kill " + std::to_string(analyzer_pid) + " 2>/dev/null";
            system(kill_cmd.c_str());
        } else {
            system("pkill -f \"analyzer -s 12345\"");
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));

        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        // 6. Verify Log
        std::ifstream log(log_file);
        std::string log_line;
        int count = 0;
        double last_loss = -1.0;
        while (std::getline(log, log_line)) {
            size_t pos = log_line.find("Epoch loss: ");
            if (pos != std::string::npos) {
                count++;
                try {
                    last_loss = std::stod(log_line.substr(pos + 12));
                } catch (...) {}
            }
        }

        std::cout << "telemetry_records_captured = " << count << std::endl;
        std::cout << "last_captured_loss_value = " << last_loss << std::endl;
        std::cout << "session_execution_duration_ms = " << duration << std::endl;

        if (count == 0) {
            std::cerr << "Telemetry not found in " << log_file << " (count=" << count << ")" << std::endl;
            // Print log for debugging if failed
            std::ifstream log_debug(log_file);
            std::string dline;
            std::cerr << "--- LOG START ---" << std::endl;
            while(std::getline(log_debug, dline)) std::cerr << dline << std::endl;
            std::cerr << "--- LOG END ---" << std::endl;
        }
    }
    return 0;
}
