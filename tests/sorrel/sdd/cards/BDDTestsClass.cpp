// @Card: bdd_state_persistence
// @Situation Default
// @Is platform == linux
// @Results persistence_verified == true

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
    // Find project root relative to this card (which is in tests/sorrel/sdd/cards/)
    std::string api_cmd = "node ../../../web_platform/frontend/js/api/platform_api.js ";

    if (card == "bdd_state_persistence") {
        // Given the platform is initialized
        exec((api_cmd + "clear_state").c_str());

        // When I switch to the "playground" tab
        exec((api_cmd + "switch_tab '\"playground\"'").c_str());

        // And I generate a response with prompt "Hello greenhouse"
        exec((api_cmd + "generate '\"Hello greenhouse\"'").c_str());

        // And I switch to the "config" tab
        exec((api_cmd + "switch_tab '\"config\"'").c_str());

        // And I switch back to the "playground" tab
        exec((api_cmd + "switch_tab '\"playground\"'").c_str());

        // Then the playground should still show "Hello greenhouse" as the last prompt
        std::string res = exec((api_cmd + "get_module_state '\"ModelModule\"'").c_str());
        if (res.find("\"lastPrompt\": \"Hello greenhouse\"") != std::string::npos) {
            std::cout << "persistence_verified = true" << std::endl;
        } else {
            std::cout << "persistence_verified = false" << std::endl;
        }
    }
    return 0;
}
