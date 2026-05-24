// @Card: verify_training_instrumentation
// @Situation Default
// @Is source_available == true
// @Results rma_include_found == 1
// @Results rma_call_found == 1

// @Card: verify_training_telemetry
// @Situation Default
// @Is analyzer_built == true
// @Results telemetry_records_found >= 1

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <thread>
#include <chrono>
#include <cstdio>
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
        std::ifstream file("../../../quanta_tissu/tisslm/program/training/trainer.cpp");
        std::string line;
        bool has_include = false;
        bool has_rma_call = false;

        while (std::getline(file, line)) {
            if (line.find("#include \"tests/model/analyzer/error_handler.hpp\"") != std::string::npos) {
                has_include = true;
            }
            if (line.find("RMA_ERROR_VAL(rma::ErrorType::INFO, (double)avg_loss, \"Epoch loss reported\")") != std::string::npos) {
                has_rma_call = true;
            }
        }

        std::cout << "rma_include_found = " << (has_include ? 1 : 0) << std::endl;
        std::cout << "rma_call_found = " << (has_rma_call ? 1 : 0) << std::endl;
    } else if (card == "verify_training_telemetry") {
        // 1. Prepare environment
        std::string session_id = "12345";
        std::string log_file = "analyzer_log.txt";
        fs::remove(log_file); // Clean previous logs

        // 2. Start Analyzer
        std::string analyzer_cmd = "../../../tests/model/analyzer/analyzer -s " + session_id + " -o " + log_file + " > /dev/null 2>&1 &";
        system(analyzer_cmd.c_str());
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 3. Run Trainer for a very short duration
        // We use minimal parameters to ensure it completes within SDD timeout
        std::string trainer_cmd = "cd ../../../ && rm -rf trained_tokenizer; TISSLM_NUM_EPOCHS=1 TISSLM_MAX_BATCHES=2 TISSLM_VOCAB_SIZE=300 TISSLM_N_EMBD=2 TISSLM_N_LAYER=1 TISSLM_N_HEAD=1 TISSLM_D_FF=2 ./train_model_exe " + session_id + " > /dev/null 2>&1";
        system(trainer_cmd.c_str());

        // 4. Stop Analyzer
        system("pkill -f \"analyzer -s 12345\"");
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 6. Verify Log
        std::ifstream log(log_file);
        std::string log_line;
        bool found_telemetry = false;
        int count = 0;
        while (std::getline(log, log_line)) {
            if (log_line.find("Epoch loss reported") != std::string::npos) {
                found_telemetry = true;
                count++;
            }
        }

        std::cout << "telemetry_records_found = " << count << std::endl;

        if (!found_telemetry) {
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
