#include "config.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace TissDB {
namespace TissLM {
namespace Core {

Config::Config(const std::string& config_path) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        std::cerr << "Error: Could not open config file: " << config_path << std::endl;
        return;
    }

    std::string line;
    while (std::getline(config_file, line)) {
        // Ignore comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            // Trim whitespace from key and value
            key.erase(0, key.find_first_not_of(" \t\n\r"));
            key.erase(key.find_last_not_of(" \t\n\r") + 1);
            value.erase(0, value.find_first_not_of(" \t\n\r"));
            value.erase(value.find_last_not_of(" \t\n\r") + 1);
            params_[key] = value;
        }
    }
}

std::string Config::get_string(const std::string& key, const std::string& default_value) const {
    auto it = params_.find(key);
    if (it != params_.end()) {
        return it->second;
    }
    return default_value;
}

int Config::get_int(const std::string& key, int default_value) const {
    auto it = params_.find(key);
    if (it != params_.end()) {
        try {
            return std::stoi(it->second);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error: Invalid integer value for key '" << key << "': " << it->second << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: Integer value for key '" << key << "' is out of range: " << it->second << std::endl;
        }
    }
    return default_value;
}

float Config::get_float(const std::string& key, float default_value) const {
    auto it = params_.find(key);
    if (it != params_.end()) {
        try {
            return std::stof(it->second);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error: Invalid float value for key '" << key << "': " << it->second << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: Float value for key '" << key << "' is out of range: " << it->second << std::endl;
        }
    }
    return default_value;
}

} // namespace Core
} // namespace TissLM
} // namespace TissDB
