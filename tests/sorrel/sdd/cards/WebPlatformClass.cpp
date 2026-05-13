// @Card: check_api_status
// @Situation Default
// @Is backend_running == true
// @Results api_status == online

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
    std::string arg = argv[1];
    if (arg == "check_api_status") {
        std::string res = exec("curl -s -o /dev/null -w \"%{http_code}\" http://localhost:8000/_health");
        if (res == "200") {
            std::cout << "api_status = online" << std::endl;
        } else {
            std::cout << "api_status = offline" << std::endl;
        }
    }
    return 0;
}
