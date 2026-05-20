// @Card: verify_process_continuity
// @Situation Default
// @Is api_available == true
// @Results process_continued == true

// @Card: verify_log_integrity
// @Situation Default
// @Is api_available == true
// @Results logs_consistent == true

#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <array>
#include <thread>
#include <chrono>

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
    std::string api_cmd = "node ../../../web_platform/frontend/js/api/platform_api.js ";

    if (card == "verify_process_continuity") {
        // Start analyzer process
        exec((api_cmd + "execute_analyzer_start").c_str());

        // Simulate tab switch by reloading state/rehydrating
        std::this_thread::sleep_for(std::chrono::seconds(2));
        exec((api_cmd + "rehydrate").c_str());

        // Verify it is still running in the canonical state
        std::string res = exec((api_cmd + "get_task_status '\"analyzer\"'").c_str());
        if (res.find("\"status\": \"running\"") != std::string::npos) {
            std::cout << "process_continued = true" << std::endl;
        }

        // Cleanup
        exec((api_cmd + "execute_analyzer_stop").c_str());
    } else if (card == "verify_log_integrity") {
        exec((api_cmd + "execute_analyzer_start").c_str());
        std::this_thread::sleep_for(std::chrono::seconds(3));

        std::string logs1 = exec((api_cmd + "get_task_status '\"analyzer\"'").c_str());

        // Simulate reload
        exec((api_cmd + "rehydrate").c_str());
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::string logs2 = exec((api_cmd + "get_task_status '\"analyzer\"'").c_str());

        // Very basic check: second log set should contain the first set (or be non-empty)
        if (logs2.find("\"logs\"") != std::string::npos && logs2.length() >= logs1.length()) {
            std::cout << "logs_consistent = true" << std::endl;
        }

        exec((api_cmd + "execute_analyzer_stop").c_str());
    }
    return 0;
}
