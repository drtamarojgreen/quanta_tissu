#include "bdd_framework.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace TissDB {
namespace BDD {

StepRegistry& StepRegistry::instance() {
    static StepRegistry reg;
    return reg;
}

void StepRegistry::add_step(const std::string& pattern, StepFunc func) {
    steps_.push_back({pattern, func});
}

bool StepRegistry::run_step(const std::string& step_text, const Table& table, Context& context) {
    // Remove Given/When/Then/And prefix
    std::string text = step_text;
    std::regex prefix_regex("^(Given|When|Then|And|But)\\s+");
    text = std::regex_replace(text, prefix_regex, "");

    for (auto& pair : steps_) {
        std::regex reg(pair.first);
        std::smatch matches;
        if (std::regex_match(text, matches, reg)) {
            pair.second(context, matches, table);
            return true;
        }
    }
    return false;
}

std::vector<Feature> Parser::parse_file(const std::string& filepath) {
    std::vector<Feature> features;
    std::ifstream file(filepath);
    if (!file.is_open()) return features;

    std::string line;
    Feature* current_feature = nullptr;
    Scenario* current_scenario = nullptr;
    Step* current_step = nullptr;

    while (std::getline(file, line)) {
        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t"));
        trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

        if (trimmed.empty()) continue;

        if (trimmed.find("Feature:") == 0) {
            features.push_back({trimmed.substr(8), {}});
            current_feature = &features.back();
            current_scenario = nullptr;
            current_step = nullptr;
        } else if (trimmed.find("Scenario:") == 0) {
            if (current_feature) {
                current_feature->scenarios.push_back({trimmed.substr(9), {}});
                current_scenario = &current_feature->scenarios.back();
                current_step = nullptr;
            }
        } else if (trimmed.find("|") == 0) {
            if (current_step) {
                std::vector<std::string> row;
                std::stringstream ss(trimmed);
                std::string cell;
                while (std::getline(ss, cell, '|')) {
                    std::string tcell = cell;
                    tcell.erase(0, tcell.find_first_not_of(" \t"));
                    tcell.erase(tcell.find_last_not_of(" \t") + 1);
                    if (!cell.empty() || ss.tellg() != -1) { // Avoid leading/trailing empty cells from |
                        if (cell.empty() && row.empty() && trimmed[0] == '|') {
                             // skip first empty
                        } else {
                             row.push_back(tcell);
                        }
                    }
                }
                // Fix row parsing logic for | cell | cell |
                row.clear();
                size_t start = 1;
                size_t end = trimmed.find('|', start);
                while(end != std::string::npos) {
                    std::string tcell = trimmed.substr(start, end-start);
                    tcell.erase(0, tcell.find_first_not_of(" \t"));
                    tcell.erase(tcell.find_last_not_of(" \t") + 1);
                    row.push_back(tcell);
                    start = end + 1;
                    end = trimmed.find('|', start);
                }

                if (!row.empty()) current_step->table.push_back(row);
            }
        } else if (trimmed.find("Given ") == 0 || trimmed.find("When ") == 0 ||
                   trimmed.find("Then ") == 0 || trimmed.find("And ") == 0 ||
                   trimmed.find("But ") == 0) {
            if (current_scenario) {
                current_scenario->steps.push_back({trimmed, {}});
                current_step = &current_scenario->steps.back();
            }
        }
    }
    return features;
}

bool Runner::run_feature(const Feature& feature) {
    std::cout << "Feature: " << feature.name << std::endl;
    bool all_scenarios_passed = true;
    Context context;

    for (const auto& scenario : feature.scenarios) {
        std::cout << "  Scenario: " << scenario.name << std::endl;
        context.clear();
        bool scenario_passed = true;
        for (const auto& step : scenario.steps) {
            try {
                if (StepRegistry::instance().run_step(step.text, step.table, context)) {
                    std::cout << "    \033[32m[PASSED]\033[0m " << step.text << std::endl;
                } else {
                    std::cout << "    \033[33m[SKIPPED]\033[0m " << step.text << " (No match)" << std::endl;
                    scenario_passed = false;
                    break;
                }
            } catch (const std::exception& e) {
                std::cout << "    \033[31m[FAILED]\033[0m " << step.text << std::endl;
                std::cout << "      Error: " << e.what() << std::endl;
                scenario_passed = false;
                break;
            }
        }
        if (scenario_passed) {
            std::cout << "  Scenario: \033[32mPASSED\033[0m" << std::endl;
        } else {
            std::cout << "  Scenario: \033[31mFAILED\033[0m" << std::endl;
            all_scenarios_passed = false;
        }
        std::cout << std::endl;
    }
    return all_scenarios_passed;
}

} // namespace BDD
} // namespace TissDB
