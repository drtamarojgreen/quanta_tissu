#include "executor_common.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <functional>
#include <cmath>
#include <algorithm>
#include <regex>

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

// --- Expression Resolution ---

Value resolve_expression_to_value(const Expression& expr, const Document& doc, const std::vector<Literal>& params) {
    if (const auto* ident_ptr = std::get_if<Identifier>(&expr)) {
        const Value* val = get_value_from_doc(doc, ident_ptr->name);
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


Document combine_documents(const Document& doc1, const std::string& collection1_name, const Document& doc2, const std::string& collection2_name) {
    Document combined_doc;
    combined_doc.id = doc1.id + "_" + doc2.id;

    for (const auto& elem : doc1.elements) {
        combined_doc.elements.push_back({collection1_name + "." + elem.key, elem.value});
    }

    for (const auto& elem : doc2.elements) {
        combined_doc.elements.push_back({collection2_name + "." + elem.key, elem.value});
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
