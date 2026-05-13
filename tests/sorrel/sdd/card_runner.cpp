#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <map>
#include <cstdio>
#include <memory>
#include <array>
#include <chrono>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <sstream>

namespace fs = std::filesystem;

std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::string execute_command(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) return "ERROR: popen failed";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int main(int argc, char* argv[]) {
    fs::path sdd_dir = fs::current_path();
    if (sdd_dir.filename() != "sdd") {
        sdd_dir = sdd_dir / "tests/sorrel/sdd";
    }
    fs::path facts_file_path = sdd_dir / "facts/web_platform.facts";

    if (argc > 1) facts_file_path = fs::path(argv[1]);

    std::map<std::string, std::map<std::string, std::map<std::string, std::string>>> indexed_facts;
    std::string current_situation = "Default";

    std::ifstream facts_file(facts_file_path);
    if (facts_file.is_open()) {
        std::string line;
        while (std::getline(facts_file, line)) {
            std::string trimmed_line = trim(line);
            if (trimmed_line.empty() || trimmed_line[0] == '#') continue;
            if (trimmed_line.find("Situation:") == 0) {
                current_situation = trim(trimmed_line.substr(10));
                continue;
            }
            size_t level_end = trimmed_line.find(' ');
            if (level_end != std::string::npos) {
                std::string level = trimmed_line.substr(0, level_end);
                std::string rest = trim(trimmed_line.substr(level_end + 1));
                size_t eq_pos = rest.find('=');
                if (eq_pos != std::string::npos) {
                    std::string key = trim(rest.substr(0, eq_pos));
                    std::string value = trim(rest.substr(eq_pos + 1));
                    indexed_facts[current_situation][level][key] = value;
                }
            }
        }
    }

    struct LogicalCard {
        std::string name;
        fs::path executablePath;
        std::map<std::string, std::string> decorators;
    };

    std::vector<LogicalCard> discovered_cards;
    for (const auto& entry : fs::directory_iterator(sdd_dir / "cards")) {
        if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
            fs::path exec_path = entry.path().parent_path() / entry.path().stem();
            if (fs::exists(exec_path)) {
                std::ifstream source(entry.path());
                std::string sline;
                LogicalCard* current_card = nullptr;
                while (std::getline(source, sline)) {
                    std::string ts = trim(sline);
                    if (ts.find("// @Card:") == 0) {
                        discovered_cards.push_back({trim(ts.substr(9)), exec_path, {}});
                        current_card = &discovered_cards.back();
                    } else if (ts.find("// @") == 0 && current_card) {
                        std::string dec = ts.substr(4);
                        size_t sp = dec.find(' ');
                        if (sp != std::string::npos) {
                            current_card->decorators[dec.substr(0, sp)] = trim(dec.substr(sp + 1));
                        }
                    }
                }
            }
        }
    }

    for (const auto& card : discovered_cards) {
        std::string sit = card.decorators.count("Situation") ? card.decorators.at("Situation") : "Default";
        if (indexed_facts.count(sit)) {
            const auto& facts = indexed_facts.at(sit);
            bool match = true;
            if (card.decorators.count("Is")) {
                std::string cond = card.decorators.at("Is");
                size_t eq = cond.find("==");
                if (eq != std::string::npos) {
                    std::string k = trim(cond.substr(0, eq));
                    std::string v = trim(cond.substr(eq + 2));
                    if (!facts.count("Is") || !facts.at("Is").count(k) || facts.at("Is").at(k) != v) match = false;
                }
            }
            if (match) {
                std::cout << "Executing Card: " << card.name << std::endl;
                std::string out = execute_command(card.executablePath.string() + " " + card.name);
                std::cout << out;
            }
        }
    }
    return 0;
}
