#pragma once

#include <string>
#include <map>

namespace TissDB {
namespace TissLM {
namespace Core {

class Config {
public:
    Config(const std::string& config_path);

    std::string get_string(const std::string& key, const std::string& default_value = "") const;
    int get_int(const std::string& key, int default_value = 0) const;
    float get_float(const std::string& key, float default_value = 0.0f) const;

private:
    std::map<std::string, std::string> params_;
};

} // namespace Core
} // namespace TissLM
} // namespace TissDB
