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
    return std::nullopt; // Incompatible types for string comparison
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
        const auto* right_literal_ptr = std::get_if<Literal>(&binary_expr->right);

        if (!left_ident || !right_literal_ptr) {
            return false; // Invalid expression structure
        }

        const auto* doc_value_ptr = get_value_from_doc(doc, left_ident->name);
        if (!doc_value_ptr) {
            return false; // Field does not exist in document
        }

        const Value& doc_value = *doc_value_ptr;
        Value literal_value;
        if (std::holds_alternative<Null>(*right_literal_ptr)) {
            literal_value = std::nullptr_t{};
        } else if (const auto* str_val = std::get_if<std::string>(right_literal_ptr)) {
            literal_value = *str_val;
        } else if (const auto* num_val = std::get_if<double>(right_literal_ptr)) {
            literal_value = *num_val;
        } else if (const auto* bool_val = std::get_if<bool>(right_literal_ptr)) {
            literal_value = *bool_val;
        }
        const std::string& op = binary_expr->op;

        // Try numeric comparison first
        auto doc_num_opt = get_as_numeric(doc_value);
        auto lit_num_opt = get_as_numeric(literal_value);

        if (doc_num_opt && lit_num_opt) {
            double doc_num = *doc_num_opt;
            double lit_num = *lit_num_opt;
            if (op == "=") return doc_num == lit_num;
            if (op == "!=") return doc_num != lit_num;
            if (op == ">") return doc_num > lit_num;
            if (op == "<") return doc_num < lit_num;
            if (op == ">=") return doc_num >= lit_num;
            if (op == "<=") return doc_num <= lit_num;
        }

        // Fallback to string comparison
        auto doc_str_opt = get_as_string(doc_value);
        auto lit_str_opt = get_as_string(literal_value);

        if (doc_str_opt && lit_str_opt) {
            const std::string& doc_str = *doc_str_opt;
            const std::string& lit_str = *lit_str_opt;
            if (op == "=") return doc_str == lit_str;
            if (op == "!=") return doc_str != lit_str;
            if (op == ">") return doc_str > lit_str;
            if (op == "<") return doc_str < lit_str;
            if (op == ">=") return doc_str >= lit_str;
            if (op == "<=") return doc_str <= lit_str;
            if (op == "LIKE") {
                try {
                    std::regex re(like_to_regex(lit_str));
                    return std::regex_match(doc_str, re);
                } catch (const std::regex_error& e) {
                    // Invalid LIKE pattern
                    return false;
                }
            }
        }
    }
    // If no comparison could be made, or if it's not a recognized expression type
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
    // Handle COUNT(*) separately. It counts every row passed to it.
    if (agg_func.type == AggregateType::COUNT && !agg_func.field_name.has_value()) {
        results_map[result_key].count++;
        return;
    }

    // For any other aggregation, a field name is required.
    if (!agg_func.field_name.has_value()) {
        // This case should ideally be prevented by the parser.
        return;
    }
    const std::string& field = agg_func.field_name.value();

    // Find the element with the specified field name in the document.
    for (const auto& elem : doc.elements) {
        if (elem.key == field) {
            auto& result = results_map[result_key];

            // COUNT(field) increments for any non-null occurrence of the field.
            if (agg_func.type == AggregateType::COUNT) {
                result.count++;
            }

            // Handle numeric aggregations.
            if (auto* num_val = std::get_if<Number>(&elem.value)) {
                switch (agg_func.type) {
                    case AggregateType::SUM:
                        result.sum += *num_val;
                        break;
                    case AggregateType::AVG:
                        result.sum += *num_val;
                        result.avg_count++; // Use dedicated counter for AVG
                        break;
                    case AggregateType::MIN:
                        if (!result.min.has_value() || *num_val < result.min.value()) {
                            result.min = *num_val;
                        }
                        break;
                    case AggregateType::MAX:
                        if (!result.max.has_value() || *num_val > result.max.value()) {
                            result.max = *num_val;
                        }
                        break;
                    default:
                        // COUNT is handled above, other types don't apply to numbers here.
                        break;
                }
            }
            // Handle string aggregations for MIN/MAX.
            else if (auto* str_val = std::get_if<std::string>(&elem.value)) {
                switch (agg_func.type) {
                    case AggregateType::MIN:
                        if (!result.min_str.has_value() || *str_val < result.min_str.value()) {
                            result.min_str = *str_val;
                        }
                        break;
                    case AggregateType::MAX:
                        if (!result.max_str.has_value() || *str_val > result.max_str.value()) {
                            result.max_str = *str_val;
                        }
                        break;
                    default:
                        // Other aggregates do not apply to strings.
                        break;
                }
            }
            // Once the field is found and processed, we can exit the loop for this document.
            return;
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
