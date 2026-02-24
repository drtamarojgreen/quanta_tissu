#ifndef TISSLM_BDD_FRAMEWORK_H
#define TISSLM_BDD_FRAMEWORK_H

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <regex>
#include <any>

namespace TissDB {
namespace BDD {

struct Context {
    std::map<std::string, std::any> data;

    template<typename T>
    void set(const std::string& key, T value) {
        data[key] = value;
    }

    template<typename T>
    T get(const std::string& key) {
        return std::any_cast<T>(data.at(key));
    }

    bool has(const std::string& key) {
        return data.find(key) != data.end();
    }

    void clear() {
        data.clear();
    }
};

using Table = std::vector<std::vector<std::string>>;

using StepFunc = std::function<void(Context&, const std::smatch&, const Table&)>;

class StepRegistry {
public:
    static StepRegistry& instance();
    void add_step(const std::string& pattern, StepFunc func);
    bool run_step(const std::string& step_text, const Table& table, Context& context);

private:
    std::vector<std::pair<std::string, StepFunc>> steps_;
};

struct Step {
    std::string text;
    Table table;
};

struct Scenario {
    std::string name;
    std::vector<Step> steps;
};

struct Feature {
    std::string name;
    std::vector<Scenario> scenarios;
};

class Parser {
public:
    static std::vector<Feature> parse_file(const std::string& filepath);
};

class Runner {
public:
    static bool run_feature(const Feature& feature);
};

#define BDD_CONCAT_IMPL(x, y) x##y
#define BDD_CONCAT(x, y) BDD_CONCAT_IMPL(x, y)

#define STEP(pattern) \
    static void BDD_CONCAT(step_func_, __LINE__)(::TissDB::BDD::Context&, const std::smatch&, const ::TissDB::BDD::Table&); \
    static bool BDD_CONCAT(registered_, __LINE__) = (::TissDB::BDD::StepRegistry::instance().add_step(pattern, BDD_CONCAT(step_func_, __LINE__)), true); \
    static void BDD_CONCAT(step_func_, __LINE__)(::TissDB::BDD::Context& context, const std::smatch& matches, const ::TissDB::BDD::Table& table)

} // namespace BDD
} // namespace TissDB

#endif // TISSLM_BDD_FRAMEWORK_H
