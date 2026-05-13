// @Card: verify_persistence
// @Situation Default
// @Is api_available == true
// @Results state_persisted == true

// @Card: verify_platform_logs
// @Situation Default
// @Is api_available == true
// @Results collection_created == true

#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <array>

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

    if (card == "verify_persistence") {
        exec((api_cmd + "clear_state").c_str());
        exec((api_cmd + "switch_tab '\"playground\"'").c_str());
        exec((api_cmd + "generate '\"Persistence Test\"'").c_str());
        exec((api_cmd + "switch_tab '\"config\"'").c_str());
        exec((api_cmd + "switch_tab '\"playground\"'").c_str());

        std::string res = exec((api_cmd + "get_module_state '\"ModelModule\"'").c_str());
        if (res.find("\"lastPrompt\": \"Persistence Test\"") != std::string::npos) {
            std::cout << "state_persisted = true" << std::endl;
        }
    } else if (card == "verify_platform_logs") {
        std::string res = exec((api_cmd + "create_platform_logs").c_str());
        // Since backend might return non-JSON on error or text, find keyword
        if (res.find("created") != std::string::npos || res.find("exists") != std::string::npos || res.find("success") != std::string::npos) {
            std::cout << "collection_created = true" << std::endl;
        }
    }
    return 0;
}
