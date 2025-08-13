#include "executor.h"
#include <stdexcept>
#include <iostream>

// Required for the executor to interact with storage
#include "../storage/lsm_tree.h"

namespace TissDB {
namespace Query {

// --- Helper functions ---

// Evaluate an expression against a document
bool evaluate_expression(const Expression& expr, const Document& doc) {
    if (auto* logical_expr = std::get_if<LogicalExpression>(&expr)) {
        if (logical_expr->op == "AND") {
            return evaluate_expression(*logical_expr->left, doc) && evaluate_expression(*logical_expr->right, doc);
        } else if (logical_expr->op == "OR") {
            return evaluate_expression(*logical_expr->left, doc) || evaluate_expression(*logical_expr->right, doc);
        }
    } else if (auto* binary_expr = std::get_if<BinaryExpression>(&expr)) {
        auto* left_ident = std::get_if<Identifier>(&*binary_expr->left);
        auto* right_literal = std::get_if<Literal>(&*binary_expr->right);

        if (left_ident && right_literal) {
            for (const auto& elem : doc.elements) {
                if (elem.key == left_ident->name) {
                    if (auto* str_val = std::get_if<std::string>(&elem.value)) {
                        if (auto* lit_val = std::get_if<std::string>(right_literal)) {
                            if (binary_expr->op == "=") return *str_val == *lit_val;
                            if (binary_expr->op == "!=") return *str_val != *lit_val;
                        }
                    } else if (auto* num_val = std::get_if<double>(&elem.value)) {
                        if (auto* lit_val = std::get_if<double>(right_literal)) {
                            if (binary_expr->op == "=") return *num_val == *lit_val;
                            if (binary_expr->op == "!=") return *num_val != *lit_val;
                            if (binary_expr->op == "<") return *num_val < *lit_val;
                            if (binary_expr->op == ">") return *num_val > *lit_val;
                            if (binary_expr->op == "<=") return *num_val <= *lit_val;
                            if (binary_expr->op == ">=") return *num_val >= *lit_val;
                        }
                    }
                }
            }
        }
    }
    return false;
}

// --- Executor ---


Executor::Executor(Storage::LSMTree& storage) : storage_engine(storage) {}

QueryResult Executor::execute(const AST& ast) {
    if (auto* select_stmt = std::get_if<SelectStatement>(&ast)) {
        std::vector<Document> result_docs;
        std::vector<std::string> doc_ids_from_index;

        // Check if we can use an index
        bool used_index = false;
        if (select_stmt->where_clause) {
            if (auto* binary_expr = std::get_if<BinaryExpression>(&*select_stmt->where_clause)) {
                auto* left_ident = std::get_if<Identifier>(&*binary_expr->left);
                auto* right_literal = std::get_if<Literal>(&*binary_expr->right);

                if (left_ident && right_literal && binary_expr->op == "=") {
                    if (auto* lit_val = std::get_if<std::string>(right_literal)) {
                        // Assuming collection name is the first part of the path
                        std::string collection_name = select_stmt->collection_name;
                        doc_ids_from_index = storage_engine.find_by_index(collection_name, left_ident->name, *lit_val);
                        used_index = true;
                    }
                }
            }
        }

        if (used_index) {
            for (const auto& doc_id : doc_ids_from_index) {
                auto doc_opt = storage_engine.get(select_stmt->collection_name, doc_id);
                if (doc_opt) {
                    result_docs.push_back(*doc_opt);
                }
            }
        } else {
            // Full table scan
            std::vector<Document> all_docs = storage_engine.scan(select_stmt->collection_name);
            for (const auto& doc : all_docs) {
                if (select_stmt->where_clause) {
                    if (evaluate_expression(*select_stmt->where_clause, doc)) {
                        result_docs.push_back(doc);
                    }
                } else {
                    result_docs.push_back(doc);
                }
            }
        }

        // Project the requested fields
        if (select_stmt->fields.size() == 1 && select_stmt->fields[0] == "*") {
            return {result_docs};
        } else {
            std::vector<Document> projected_docs;
            for (const auto& doc : result_docs) {
                Document projected_doc;
                projected_doc.id = doc.id;
                for (const auto& field : select_stmt->fields) {
                    for (const auto& elem : doc.elements) {
                        if (elem.key == field) {
                            projected_doc.elements.push_back(elem);
                        }
                    }
                }
                projected_docs.push_back(projected_doc);
            }
            return {projected_docs};
        }
    }
    return {};
}


} // namespace Query
} // namespace TissDB