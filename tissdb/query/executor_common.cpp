#include "executor_common.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <functional>
#include <cmath>
#include <algorithm>
#include <regex>
#include <chrono>
#include <ctime>
#include <cctype>
#include <cstdio>

namespace TissDB {
namespace Query {

// --- Helper function for LIKE to regex conversion ---
std::string like_to_regex(std::string pattern) {
    std::string regex_pattern;
    for (char c : pattern) {
        if (c == '%') {
            regex_pattern += ".*";
        } else if (c == '_') {
            regex_pattern += ".";
        } else if (c == '.' || c == '+' || c == '*' || c == '?' || c == '^' || c == '$' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == '|') {
            regex_pattern += R"(\)"; // Escape special regex characters
            regex_pattern += c;
        } else {
            regex_pattern += c;
        }
    }
    return regex_pattern;
}

// --- Conversion helpers to make evaluation more robust ---
std::optional<double> get_as_numeric(const Value& val) {
    if (const auto* num_val = std::get_if<double>(&val)) {
        return *num_val;
    }
    if (const auto* str_val = std::get_if<std::string>(&val)) {
        try {
            return std::stod(*str_val);
        } catch (const std::invalid_argument&) {
            return std::nullopt;
        }
    }
    return std::nullopt;
}

std::optional<std::string> get_as_string(const Value& val) {
    if (const auto* str_val = std::get_if<std::string>(&val)) {
        return *str_val;
    }
    if (const auto* num_val = std::get_if<double>(&val)) {
        return std::to_string(*num_val);
    }
    if (const auto* bool_val = std::get_if<bool>(&val)) {
        return *bool_val ? "true" : "false";
    }
    if (std::holds_alternative<std::nullptr_t>(val)) {
        return "null";
    }
    if (const auto* ts_val = std::get_if<Timestamp>(&val)) {
        // This is a simple conversion for now. A more robust implementation
        // would format it as an ISO 8601 string.
        return std::to_string(ts_val->microseconds_since_epoch_utc);
    }
    // Note: No automatic conversion from Date/Time/DateTime to string for now
    return std::nullopt; // Incompatible types for string comparison
}

namespace {
int64_t interval_to_microseconds(const IntervalLiteral& interval) {
    std::string unit = interval.unit;
    std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
    if (unit == "microsecond" || unit == "microseconds") return static_cast<int64_t>(interval.value);
    if (unit == "millisecond" || unit == "milliseconds") return static_cast<int64_t>(interval.value * 1000.0);
    if (unit == "second" || unit == "seconds") return static_cast<int64_t>(interval.value * 1000000.0);
    if (unit == "minute" || unit == "minutes") return static_cast<int64_t>(interval.value * 60.0 * 1000000.0);
    if (unit == "hour" || unit == "hours") return static_cast<int64_t>(interval.value * 3600.0 * 1000000.0);
    if (unit == "day" || unit == "days") return static_cast<int64_t>(interval.value * 86400.0 * 1000000.0);
    return static_cast<int64_t>(interval.value * 1000000.0);
}

Timestamp now_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    return Timestamp{micros};
}

Date timestamp_to_date(const Timestamp& ts) {
    std::time_t tt = static_cast<std::time_t>(ts.microseconds_since_epoch_utc / 1000000);
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &tt);
#else
    gmtime_r(&tt, &tm);
#endif
    return Date{static_cast<uint16_t>(tm.tm_year + 1900), static_cast<uint8_t>(tm.tm_mon + 1), static_cast<uint8_t>(tm.tm_mday)};
}


Time timestamp_to_time(const Timestamp& ts) {
    std::time_t tt = static_cast<std::time_t>(ts.microseconds_since_epoch_utc / 1000000);
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &tt);
#else
    gmtime_r(&tt, &tm);
#endif
    return Time{static_cast<uint8_t>(tm.tm_hour), static_cast<uint8_t>(tm.tm_min), static_cast<uint8_t>(tm.tm_sec)};
}

double extract_part_from_timestamp(const std::string& part, const Timestamp& ts) {
    std::time_t tt = static_cast<std::time_t>(ts.microseconds_since_epoch_utc / 1000000);
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &tt);
#else
    gmtime_r(&tt, &tm);
#endif
    if (part == "year") return tm.tm_year + 1900;
    if (part == "month") return tm.tm_mon + 1;
    if (part == "day") return tm.tm_mday;
    if (part == "hour") return tm.tm_hour;
    if (part == "minute") return tm.tm_min;
    if (part == "second") return tm.tm_sec;
    return 0.0;
}
}

// --- Expression Resolution ---

Value resolve_expression_to_value(const Expression& expr, const Document& doc, const std::vector<Literal>& params) {
    if (const auto* ident_ptr = std::get_if<Identifier>(&expr)) {
        std::string key = ident_ptr->name;
        // Handle qualified names like 'c.name' by splitting them
        std::string alias;
        std::string field_name;
        if (auto dot_pos = key.find('.'); dot_pos != std::string::npos) {
            alias = key.substr(0, dot_pos);
            field_name = key.substr(dot_pos + 1);
        } else {
            field_name = key;
        }

        // In a combined document, the key is already aliased, so we just look for it.
        const Value* val = get_value_from_doc(doc, key);
        return val ? *val : Value{std::nullptr_t{}};
    }
    if (const auto* lit_ptr = std::get_if<Literal>(&expr)) {
        if (const auto* str_val = std::get_if<std::string>(lit_ptr)) return *str_val;
        if (const auto* num_val = std::get_if<double>(lit_ptr)) return *num_val;
        if (const auto* bool_val = std::get_if<bool>(lit_ptr)) return *bool_val;
        if (const auto* date_val = std::get_if<Date>(lit_ptr)) return *date_val;
        if (const auto* time_val = std::get_if<Time>(lit_ptr)) return *time_val;
        if (const auto* ts_val = std::get_if<TissDB::Timestamp>(lit_ptr)) return *ts_val;
        if (const auto* dt_val = std::get_if<DateTime>(lit_ptr)) return *dt_val;
        if (const auto* ts_val = std::get_if<Timestamp>(lit_ptr)) return *ts_val;
        if (std::holds_alternative<Null>(*lit_ptr)) return std::nullptr_t{};
    }
    if (const auto* interval_ptr = std::get_if<IntervalLiteral>(&expr)) {
        return Timestamp{interval_to_microseconds(*interval_ptr)};
    }
    if (const auto* param_ptr = std::get_if<ParameterExpression>(&expr)) {
        if (param_ptr->index >= params.size()) {
            throw std::runtime_error("Parameter index out of bounds.");
        }
        const auto& param_lit = params[param_ptr->index];
        if (const auto* str_val = std::get_if<std::string>(&param_lit)) return *str_val;
        if (const auto* num_val = std::get_if<double>(&param_lit)) return *num_val;
        if (const auto* bool_val = std::get_if<bool>(&param_lit)) return *bool_val;
        if (const auto* date_val = std::get_if<Date>(&param_lit)) return *date_val;
        if (const auto* time_val = std::get_if<Time>(&param_lit)) return *time_val;
        if (const auto* ts_val = std::get_if<TissDB::Timestamp>(&param_lit)) return *ts_val;
        if (const auto* dt_val = std::get_if<DateTime>(&param_lit)) return *dt_val;
        if (const auto* ts_val = std::get_if<Timestamp>(&param_lit)) return *ts_val;
        if (std::holds_alternative<Null>(param_lit)) return std::nullptr_t{};
    }
    if (const auto* fn_ptr = std::get_if<std::shared_ptr<FunctionExpression>>(&expr)) {
        const auto& fn = *fn_ptr;
        if (fn->name == "NOW") {
            return now_timestamp();
        }
        if (fn->name == "DATE") {
            if (fn->args.empty()) {
                throw std::runtime_error("DATE() requires an argument.");
            }
            Value arg = resolve_expression_to_value(fn->args[0], doc, params);
            if (const auto* ts = std::get_if<Timestamp>(&arg)) {
                return timestamp_to_date(*ts);
            }
            if (const auto* str = std::get_if<std::string>(&arg)) {
                int y, m, d;
                if (std::sscanf(str->c_str(), "%d-%d-%d", &y, &m, &d) == 3) {
                    return Date{static_cast<uint16_t>(y), static_cast<uint8_t>(m), static_cast<uint8_t>(d)};
                }
            }
            if (const auto* d = std::get_if<Date>(&arg)) {
                return *d;
            }
            throw std::runtime_error("DATE() argument must be DATE, TIMESTAMP, or date string.");
        }
        if (fn->name == "TIME") {
            if (fn->args.empty()) {
                throw std::runtime_error("TIME() requires an argument.");
            }
            Value arg = resolve_expression_to_value(fn->args[0], doc, params);
            if (const auto* ts = std::get_if<Timestamp>(&arg)) {
                return timestamp_to_time(*ts);
            }
            if (const auto* str = std::get_if<std::string>(&arg)) {
                int h, m, sec;
                if (std::sscanf(str->c_str(), "%d:%d:%d", &h, &m, &sec) == 3) {
                    return Time{static_cast<uint8_t>(h), static_cast<uint8_t>(m), static_cast<uint8_t>(sec)};
                }
            }
            if (const auto* t = std::get_if<Time>(&arg)) {
                return *t;
            }
            throw std::runtime_error("TIME() argument must be TIME, TIMESTAMP, or time string.");
        }
        if (fn->name == "EXTRACT") {
            if (fn->args.size() != 2) {
                throw std::runtime_error("EXTRACT requires 2 arguments.");
            }
            auto* part_lit = std::get_if<Literal>(&fn->args[0]);
            if (!part_lit || !std::holds_alternative<std::string>(*part_lit)) {
                throw std::runtime_error("EXTRACT part must be string literal.");
            }
            std::string part = std::get<std::string>(*part_lit);
            Value target = resolve_expression_to_value(fn->args[1], doc, params);
            if (const auto* ts = std::get_if<Timestamp>(&target)) {
                return extract_part_from_timestamp(part, *ts);
            }
            if (const auto* d = std::get_if<Date>(&target)) {
                if (part == "year") return static_cast<double>(d->year);
                if (part == "month") return static_cast<double>(d->month);
                if (part == "day") return static_cast<double>(d->day);
                return 0.0;
            }
            throw std::runtime_error("EXTRACT target must be TIMESTAMP or DATE.");
        }
        throw std::runtime_error("Unsupported function: " + fn->name);
    }

    if (const auto* binary_expr_ptr = std::get_if<std::shared_ptr<BinaryExpression>>(&expr)) {
        const auto& binary_expr = *binary_expr_ptr;
        Value left_val = resolve_expression_to_value(binary_expr->left, doc, params);
        Value right_val = resolve_expression_to_value(binary_expr->right, doc, params);

        auto left_num_opt = get_as_numeric(left_val);
        auto right_num_opt = get_as_numeric(right_val);

        if (left_num_opt && right_num_opt) {
            if (binary_expr->op == "+") return *left_num_opt + *right_num_opt;
            if (binary_expr->op == "-") return *left_num_opt - *right_num_opt;
            if (binary_expr->op == "*") return *left_num_opt * *right_num_opt;
            if (binary_expr->op == "/") {
                if (*right_num_opt == 0) throw std::runtime_error("Division by zero");
                return *left_num_opt / *right_num_opt;
            }
        }

        if (const auto* left_ts = std::get_if<Timestamp>(&left_val)) {
            if (const auto* right_interval = std::get_if<Timestamp>(&right_val)) {
                if (binary_expr->op == "+") return Timestamp{left_ts->microseconds_since_epoch_utc + right_interval->microseconds_since_epoch_utc};
                if (binary_expr->op == "-") return Timestamp{left_ts->microseconds_since_epoch_utc - right_interval->microseconds_since_epoch_utc};
            }
        }

        if (const auto* left_date = std::get_if<Date>(&left_val)) {
            if (const auto* right_interval = std::get_if<Timestamp>(&right_val)) {
                std::tm tm{};
                tm.tm_year = left_date->year - 1900;
                tm.tm_mon = left_date->month - 1;
                tm.tm_mday = left_date->day;
#ifdef _WIN32
                std::time_t base = _mkgmtime(&tm);
#else
                std::time_t base = timegm(&tm);
#endif
                auto micros = static_cast<int64_t>(base) * 1000000;
                if (binary_expr->op == "+") micros += right_interval->microseconds_since_epoch_utc;
                else if (binary_expr->op == "-") micros -= right_interval->microseconds_since_epoch_utc;
                else throw std::runtime_error("Unsupported DATE arithmetic operation.");
                return timestamp_to_date(Timestamp{micros});
            }
        }

        throw std::runtime_error("Unsupported arithmetic operation or type mismatch.");
    }
    throw std::runtime_error("Unsupported expression type for value resolution.");
}


// Evaluate an expression against a document
bool evaluate_expression(const Expression& expr, const Document& doc, const std::vector<Literal>& params) {
    if (const auto* logical_expr_ptr = std::get_if<std::shared_ptr<LogicalExpression>>(&expr)) {
        const auto& logical_expr = *logical_expr_ptr;
        if (logical_expr->op == "AND") {
            return evaluate_expression(logical_expr->left, doc, params) && evaluate_expression(logical_expr->right, doc, params);
        } else if (logical_expr->op == "OR") {
            return evaluate_expression(logical_expr->left, doc, params) || evaluate_expression(logical_expr->right, doc, params);
        }
    } else if (const auto* between_expr_ptr = std::get_if<std::shared_ptr<BetweenExpression>>(&expr)) {
        const auto& between_expr = *between_expr_ptr;
        Value value = resolve_expression_to_value(between_expr->value, doc, params);
        Value lower = resolve_expression_to_value(between_expr->lower, doc, params);
        Value upper = resolve_expression_to_value(between_expr->upper, doc, params);
        bool in_range = false;

        auto v_num = get_as_numeric(value);
        auto l_num = get_as_numeric(lower);
        auto u_num = get_as_numeric(upper);
        if (v_num && l_num && u_num) {
            in_range = (*v_num >= *l_num && *v_num <= *u_num);
        } else if (auto v_date = std::get_if<Date>(&value)) {
            if (auto l_date = std::get_if<Date>(&lower)) {
                if (auto u_date = std::get_if<Date>(&upper)) {
                    in_range = !(*v_date < *l_date) && !(*u_date < *v_date);
                }
            }
        } else if (auto v_ts = std::get_if<Timestamp>(&value)) {
            if (auto l_ts = std::get_if<Timestamp>(&lower)) {
                if (auto u_ts = std::get_if<Timestamp>(&upper)) {
                    in_range = (*v_ts >= *l_ts) && (*v_ts <= *u_ts);
                }
            }
        }

        return between_expr->negated ? !in_range : in_range;
    } else if (const auto* binary_expr_ptr = std::get_if<std::shared_ptr<BinaryExpression>>(&expr)) {
        const auto& binary_expr = *binary_expr_ptr;

        Value left_value = resolve_expression_to_value(binary_expr->left, doc, params);
        Value right_value = resolve_expression_to_value(binary_expr->right, doc, params);

        const std::string& op = binary_expr->op;

        // Type-specific comparisons
        if (const auto* left_date = std::get_if<Date>(&left_value)) {
            if (const auto* right_date = std::get_if<Date>(&right_value)) {
                if (op == "=") return *left_date == *right_date;
                if (op == "!=") return !(*left_date == *right_date);
                if (op == ">") return *right_date < *left_date;
                if (op == "<") return *left_date < *right_date;
                if (op == ">=") return !(*left_date < *right_date);
                if (op == "<=") return !(*right_date < *left_date);
                return false; // Unsupported operator for Date
            }
        }

        if (const auto* left_time = std::get_if<Time>(&left_value)) {
            if (const auto* right_time = std::get_if<Time>(&right_value)) {
                if (op == "=") return *left_time == *right_time;
                if (op == "!=") return !(*left_time == *right_time);
                if (op == ">") return *right_time < *left_time;
                if (op == "<") return *left_time < *right_time;
                if (op == ">=") return !(*left_time < *right_time);
                if (op == "<=") return !(*right_time < *left_time);
                return false; // Unsupported operator for Time
            }
        }

        if (const auto* left_dt = std::get_if<DateTime>(&left_value)) {
            if (const auto* right_dt = std::get_if<DateTime>(&right_value)) {
                if (op == "=") return *left_dt == *right_dt;
                if (op == "!=") return *left_dt != *right_dt;
                if (op == ">") return *left_dt > *right_dt;
                if (op == "<") return *left_dt < *right_dt;
                if (op == ">=") return *left_dt >= *right_dt;
                if (op == "<=") return *left_dt <= *right_dt;
                return false; // Unsupported operator for DateTime
            }
        }

        if (const auto* left_ts = std::get_if<TissDB::Timestamp>(&left_value)) {
            if (const auto* right_ts = std::get_if<TissDB::Timestamp>(&right_value)) {
                if (op == "=") return *left_ts == *right_ts;
                if (op == "!=") return *left_ts != *right_ts;
                if (op == ">") return *left_ts > *right_ts;
                if (op == "<") return *left_ts < *right_ts;
                if (op == ">=") return *left_ts >= *right_ts;
                if (op == "<=") return *left_ts <= *right_ts;
                return false; // Unsupported operator for Timestamp
            }
        }

        // Fallback to numeric and string comparisons
        auto left_num_opt = get_as_numeric(left_value);
        auto right_num_opt = get_as_numeric(right_value);

        if (left_num_opt && right_num_opt) {
            double left_num = *left_num_opt;
            double right_num = *right_num_opt;
            if (op == "=") return left_num == right_num;
            if (op == "!=") return left_num != right_num;
            if (op == ">") return left_num > right_num;
            if (op == "<") return left_num < right_num;
            if (op == ">=") return left_num >= right_num;
            if (op == "<=") return left_num <= right_num;
        }

        auto left_str_opt = get_as_string(left_value);
        auto right_str_opt = get_as_string(right_value);

        if (left_str_opt && right_str_opt) {
            const std::string& left_str = *left_str_opt;
            const std::string& right_str = *right_str_opt;
            if (op == "=") return left_str == right_str;
            if (op == "!=") return left_str != right_str;
            if (op == ">") return left_str > right_str;
            if (op == "<") return left_str < right_str;
            if (op == ">=") return left_str >= right_str;
            if (op == "<=") return left_str <= right_str;
            if (op == "LIKE") {
                try {
                    std::regex re(like_to_regex(right_str));
                    return std::regex_match(left_str, re);
                } catch (const std::regex_error& e) {
                    return false;
                }
            }
        }
    }
    return false;
}

Literal evaluate_update_expression(const Expression& expr, const Document& doc, const std::vector<Literal>& params) {
    Value resolved_value = resolve_expression_to_value(expr, doc, params);
    if (const auto* str_val = std::get_if<std::string>(&resolved_value)) {
        return *str_val;
    }
    if (const auto* num_val = std::get_if<double>(&resolved_value)) {
        return *num_val;
    }
    if (const auto* bool_val = std::get_if<bool>(&resolved_value)) {
        return *bool_val;
    }
    if (const auto* date_val = std::get_if<Date>(&resolved_value)) {
        return *date_val;
    }
    if (const auto* time_val = std::get_if<Time>(&resolved_value)) {
        return *time_val;
    }
    if (const auto* ts_val = std::get_if<TissDB::Timestamp>(&resolved_value)) {
        return *ts_val;
    }
    if (const auto* dt_val = std::get_if<DateTime>(&resolved_value)) {
        return *dt_val;
    }
    if (const auto* ts_val = std::get_if<Timestamp>(&resolved_value)) {
        return *ts_val;
    }
    if (std::holds_alternative<std::nullptr_t>(resolved_value)) {
        return Null{};
    }
    throw std::runtime_error("Could not convert resolved value to Literal for update.");
}

void process_aggregation(std::map<std::string, AggregateResult>& results_map, const std::string& result_key, const Document& doc, const AggregateFunction& agg_func) {
    // This function does not currently need parameter support, so its signature remains unchanged.
    if (agg_func.type == AggregateType::COUNT && !agg_func.field_name.has_value()) {
        results_map[result_key].count++;
        return;
    }

    if (!agg_func.field_name.has_value()) {
        return;
    }
    const std::string& field = agg_func.field_name.value();

    const Value* value_ptr = get_value_from_doc(doc, field);
    if (!value_ptr) return;

    auto& result = results_map[result_key];
    if (agg_func.type == AggregateType::COUNT) {
        result.count++;
    }

    if (auto num_val_opt = get_as_numeric(*value_ptr)) {
        double num_val = *num_val_opt;
        switch (agg_func.type) {
            case AggregateType::SUM:
                result.sum += num_val;
                break;
            case AggregateType::AVG:
                result.sum += num_val;
                result.avg_count++;
                break;
            case AggregateType::MIN:
                if (!result.min.has_value() || num_val < result.min.value()) {
                    result.min = num_val;
                }
                break;
            case AggregateType::MAX:
                if (!result.max.has_value() || num_val > result.max.value()) {
                    result.max = num_val;
                }
                break;
            default:
                break;
        }
    }
    else if (auto str_val_opt = get_as_string(*value_ptr)) {
        const std::string& str_val = *str_val_opt;
        switch (agg_func.type) {
            case AggregateType::MIN:
                if (!result.min_str.has_value() || str_val < result.min_str.value()) {
                    result.min_str = str_val;
                }
                break;
            case AggregateType::MAX:
                if (!result.max_str.has_value() || str_val > result.max_str.value()) {
                    result.max_str = str_val;
                }
                break;
            default:
                break;
        }
    }
}


Document combine_documents(const Document& doc1, const std::string& alias1, const Document& doc2, const std::string& alias2) {
    Document combined_doc;
    combined_doc.id = doc1.id + "_" + doc2.id;

    combined_doc.elements.push_back({alias1 + "._id", doc1.id});
    for (const auto& elem : doc1.elements) {
        combined_doc.elements.push_back({alias1 + "." + elem.key, elem.value});
    }

    combined_doc.elements.push_back({alias2 + "._id", doc2.id});
    for (const auto& elem : doc2.elements) {
        combined_doc.elements.push_back({alias2 + "." + elem.key, elem.value});
    }

    return combined_doc;
}

void extract_equality_conditions(const Expression& expr, std::map<std::string, std::string>& conditions) {
    // This function is used for potential query optimization and does not support parameters.
    // It only extracts simple `field = 'literal'` conditions.
    if (const auto* logical_expr_ptr = std::get_if<std::shared_ptr<LogicalExpression>>(&expr)) {
        const auto& logical_expr = *logical_expr_ptr;
        if (logical_expr->op == "AND") {
            extract_equality_conditions(logical_expr->left, conditions);
            extract_equality_conditions(logical_expr->right, conditions);
        }
    } else if (const auto* binary_expr_ptr = std::get_if<std::shared_ptr<BinaryExpression>>(&expr)) {
        const auto& binary_expr = *binary_expr_ptr;
        if (binary_expr->op == "=") {
            const auto* left_ident = std::get_if<Identifier>(&binary_expr->left);
            const auto* right_literal = std::get_if<Literal>(&binary_expr->right);
            if (left_ident && right_literal) {
                if (const auto* str_lit = std::get_if<std::string>(right_literal)) {
                    conditions[left_ident->name] = *str_lit;
                }
            }
        }
    }
}

const Value* get_value_from_doc(const Document& doc, const std::string& key) {
    if (key == "id" || key == "_id") {
        static thread_local Value id_val;
        id_val = doc.id;
        return &id_val;
    }
    for (const auto& elem : doc.elements) {
        if (elem.key == key) {
            return &elem.value;
        }
    }
    return nullptr;
}

std::string value_to_string(const Value& value) {
    std::stringstream ss;
    if (const auto* str_val = std::get_if<std::string>(&value)) {
        ss << *str_val;
    } else if (const auto* num_val = std::get_if<double>(&value)) {
        ss << *num_val;
    } else if (const auto* bool_val = std::get_if<bool>(&value)) {
        ss << (*bool_val ? "true" : "false");
    } else if (std::holds_alternative<std::nullptr_t>(value)) {
        ss << "null";
    } else if (const auto* ts_val = std::get_if<Timestamp>(&value)) {
        ss << ts_val->microseconds_since_epoch_utc;
    }
    // TODO: Add support for printing Date, Time, DateTime
    return ss.str();
}

} // namespace Query
} // namespace TissDB
