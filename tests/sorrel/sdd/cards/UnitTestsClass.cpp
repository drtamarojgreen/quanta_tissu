// @Card: unit_switch_tab
// @Is platform == linux
// @Results switch_tab_success == true

// @Card: unit_get_state
// @Is platform == linux
// @Results get_state_success == true

// @Card: unit_list_tasks
// @Is platform == linux
// @Results list_tasks_success == true

// @Card: unit_train
// @Is platform == linux
// @Results train_started == true

// @Card: unit_generate
// @Is platform == linux
// @Results generate_success == true

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

    if (card == "unit_switch_tab") {
        std::string res = exec((api_cmd + "switch_tab '\"playground\"'").c_str());
        if (res.find("\"status\": \"success\"") != std::string::npos) {
            std::cout << "switch_tab_success = true" << std::endl;
        }
    } else if (card == "unit_get_state") {
        std::string res = exec((api_cmd + "get_state").c_str());
        if (res.find("\"tabs\"") != std::string::npos) {
            std::cout << "get_state_success = true" << std::endl;
        }
    } else if (card == "unit_list_tasks") {
        std::string res = exec((api_cmd + "list_tasks").c_str());
        if (res.find("{") != std::string::npos) {
            std::cout << "list_tasks_success = true" << std::endl;
        }
    } else if (card == "unit_train") {
        std::string res = exec((api_cmd + "train '{\"epochs\": 1}'").c_str());
        if (res.find("\"status\": \"started\"") != std::string::npos) {
            std::cout << "train_started = true" << std::endl;
        }
    } else if (card == "unit_generate") {
        std::string res = exec((api_cmd + "generate '\"Test prompt\"'").c_str());
        if (res.find("\"status\": \"success\"") != std::string::npos) {
            std::cout << "generate_success = true" << std::endl;
        }
    }
    return 0;
}
