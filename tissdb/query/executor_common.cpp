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
            regex_pattern += "\\";
            regex_pattern += c;
        } else {
            regex_pattern += c;
        }
    }
    return regex_pattern;
}

// Evaluate an expression against a document
bool evaluate_expression(const Expression& expr, const Document& doc) {
    if (const auto* logical_expr_ptr = std::get_if<std::shared_ptr<LogicalExpression>>(&expr)) {
        const auto& logical_expr = *logical_expr_ptr;
        if (logical_expr->op == "AND") {
            return evaluate_expression(logical_expr->left, doc) && evaluate_expression(logical_expr->right, doc);
        } else if (logical_expr->op == "OR") {
            return evaluate_expression(logical_expr->left, doc) || evaluate_expression(logical_expr->right, doc);
        }
    } else if (const auto* binary_expr_ptr = std::get_if<std::shared_ptr<BinaryExpression>>(&expr)) {
        const auto& binary_expr = *binary_expr_ptr;
        const auto* left_ident = std::get_if<Identifier>(&binary_expr->left);
        const auto* right_literal = std::get_if<Literal>(&binary_expr->right);

        if (left_ident && right_literal) {
            for (const auto& elem : doc.elements) {
                if (elem.key == left_ident->name) {
                    if (auto* str_val = std::get_if<std::string>(&elem.value)) {
                        if (auto* lit_val = std::get_if<std::string>(right_literal)) {
                            if (binary_expr->op == "=") return *str_val == *lit_val;
                            if (binary_expr->op == "!=") return *str_val != *lit_val;
                            if (binary_expr->op == "LIKE") {
                                std::regex re(like_to_regex(*lit_val));
                                return std::regex_match(*str_val, re);
                            }
                        }
                    } else if (auto* num_val = std::get_if<double>(&elem.value)) {
                        if (auto* lit_val = std::get_if<double>(right_literal)) {
                            if (binary_expr->op == "=") return *num_val == *lit_val;
                            if (binary_expr->op == "!=") return *num_val != *lit_val;
                            if (binary_expr->op == "<") return *num_val < *lit_val;
                            if (binary_expr->op == ">") return *num_val > *lit_val;
                            if (binary_expr->op == "<=") return *num_val <= *lit_val;
                            if (binary_expr->op == ">=") return *num_val >= *lit_val;
                        } else if (const auto* lit_val_str = std::get_if<std::string>(right_literal)) {
                            // The parser might interpret a number as a string, so we try to convert it.
                            try {
                                double val = std::stod(*lit_val_str);
                                if (binary_expr->op == "=") return *num_val == val;
                                if (binary_expr->op == "!=") return *num_val != val;
                                if (binary_expr->op == "<") return *num_val < val;
                                if (binary_expr->op == ">") return *num_val > val;
                                if (binary_expr->op == "<=") return *num_val <= val;
                                if (binary_expr->op == ">=") return *num_val >= val;
                            } catch (const std::invalid_argument& ia) {
                                // Not a number, so the comparison is false
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

// --- New function to evaluate expressions in an UPDATE SET clause ---
Literal evaluate_update_expression(const Expression& expr, const Document& doc) {
    if (const auto* lit_ptr = std::get_if<Literal>(&expr)) {
        return *lit_ptr;
    }

    if (const auto* ident_ptr = std::get_if<Identifier>(&expr)) {
        for (const auto& elem : doc.elements) {
            if (elem.key == ident_ptr->name) {
                if (const auto* str_val = std::get_if<std::string>(&elem.value)) {
                    return *str_val;
                } else if (const auto* num_val = std::get_if<double>(&elem.value)) {
                    return *num_val;
                }
            }
        }
        throw std::runtime_error("Field not found in document: " + ident_ptr->name);
    }

    if (const auto* binary_expr_ptr = std::get_if<std::shared_ptr<BinaryExpression>>(&expr)) {
        const auto& binary_expr = *binary_expr_ptr;
        Literal left_val = evaluate_update_expression(binary_expr->left, doc);
        Literal right_val = evaluate_update_expression(binary_expr->right, doc);

        if (const auto* left_num = std::get_if<double>(&left_val)) {
            if (const auto* right_num = std::get_if<double>(&right_val)) {
                if (binary_expr->op == "+") return *left_num + *right_num;
                if (binary_expr->op == "-") return *left_num - *right_num;
                if (binary_expr->op == "*") return *left_num * *right_num;
                if (binary_expr->op == "/") {
                    if (*right_num == 0) throw std::runtime_error("Division by zero");
                    return *left_num / *right_num;
                }
            }
        }
        // Add more type combinations if needed (e.g., string concatenation)
        throw std::runtime_error("Unsupported operator or type combination in SET clause");
    }

    throw std::runtime_error("Unsupported expression type in SET clause");
}

void process_aggregation(std::map<std::string, AggregateResult>& results_map, const std::string& result_key, const Document& doc, const AggregateFunction& agg_func) {
    if (agg_func.field_name == "*") {
        if (agg_func.function_name == "COUNT") {
            results_map[result_key].count++;
        }
        return;
    }

    for (const auto& elem : doc.elements) {
        if (elem.key == agg_func.field_name) {
            auto& result = results_map[result_key];

            // COUNT should increment for any non-null field
            if (agg_func.function_name == "COUNT") {
                result.count++;
            }

            // Handle numeric aggregations
            if (auto* num_val = std::get_if<Number>(&elem.value)) {
                if (agg_func.function_name == "SUM" || agg_func.function_name == "AVG" || agg_func.function_name == "STDDEV") {
                    result.sum += *num_val;
                    result.sum_sq += (*num_val) * (*num_val);
                }
                if (agg_func.function_name == "MIN") {
                    if (!result.min.has_value() || *num_val < result.min.value()) {
                        result.min = *num_val;
                    }
                }
                if (agg_func.function_name == "MAX") {
                    if (!result.max.has_value() || *num_val > result.max.value()) {
                        result.max = *num_val;
                    }
                }
            }
            // Handle string aggregations
            else if (auto* str_val = std::get_if<std::string>(&elem.value)) {
                if (agg_func.function_name == "MIN") {
                    if (!result.min_str.has_value() || *str_val < result.min_str.value()) {
                        result.min_str = *str_val;
                    }
                }
                if (agg_func.function_name == "MAX") {
                    if (!result.max_str.has_value() || *str_val > result.max_str.value()) {
                        result.max_str = *str_val;
                    }
                }
            }
        }
    }
}

Document combine_documents(const Document& doc1, const Document& doc2) {
    Document combined_doc;
    combined_doc.id = doc1.id + "_" + doc2.id;

    for (const auto& elem : doc1.elements) {
        combined_doc.elements.push_back(elem);
    }
    for (const auto& elem : doc2.elements) {
        bool found = false;
        for (const auto& existing_elem : combined_doc.elements) {
            if (existing_elem.key == elem.key) {
                found = true;
                break;
            }
        }
        if (!found) {
            combined_doc.elements.push_back(elem);
        }
    }
    return combined_doc;
}

void extract_equality_conditions(const Expression& expr, std::map<std::string, std::string>& conditions) {
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
    }
    return ss.str();
}

} // namespace Query
} // namespace TissDB
