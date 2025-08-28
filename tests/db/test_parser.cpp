
#include "test_framework.h"
#include "../../tissdb/query/parser.h"

TEST_CASE(ParserSelectAll) {
    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT * FROM users");
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::SelectStatement>(ast));
    auto& select_stmt = std::get<TissDB::Query::SelectStatement>(ast);
    ASSERT_EQ(1, select_stmt.fields.size());
    ASSERT_EQ("*", std::get<std::string>(select_stmt.fields[0]));
    ASSERT_EQ("users", select_stmt.from_collection);
    ASSERT_FALSE(select_stmt.where_clause.has_value());
}

TEST_CASE(ParserSelectSpecificFields) {
    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT name, age FROM employees");
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::SelectStatement>(ast));
    auto& select_stmt = std::get<TissDB::Query::SelectStatement>(ast);
    ASSERT_EQ(2, select_stmt.fields.size());
    ASSERT_EQ("name", std::get<std::string>(select_stmt.fields[0]));
    ASSERT_EQ("age", std::get<std::string>(select_stmt.fields[1]));
    ASSERT_EQ("employees", select_stmt.from_collection);
    ASSERT_FALSE(select_stmt.where_clause.has_value());
}

TEST_CASE(ParserSelectWithWhereClause) {
    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT * FROM products WHERE price > 100");
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::SelectStatement>(ast));
    auto& select_stmt = std::get<TissDB::Query::SelectStatement>(ast);
    ASSERT_TRUE(select_stmt.where_clause.has_value());

    auto& expr = select_stmt.where_clause.value();
    ASSERT_TRUE(std::holds_alternative<std::shared_ptr<TissDB::Query::BinaryExpression>>(expr));
    auto& binary_expr = std::get<std::shared_ptr<TissDB::Query::BinaryExpression>>(expr);

    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Identifier>(binary_expr->left));
    ASSERT_EQ("price", std::get<TissDB::Query::Identifier>(binary_expr->left).name);

    ASSERT_EQ(">", binary_expr->op);

    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Literal>(binary_expr->right));
    ASSERT_EQ(100.0, std::get<double>(std::get<TissDB::Query::Literal>(binary_expr->right)));
}

TEST_CASE(ParserSelectWithLogicalOperators) {
    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT * FROM orders WHERE status = 'shipped' AND total > 50");
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::SelectStatement>(ast));
    auto& select_stmt = std::get<TissDB::Query::SelectStatement>(ast);
    ASSERT_TRUE(select_stmt.where_clause.has_value());

    auto& expr = select_stmt.where_clause.value();
    ASSERT_TRUE(std::holds_alternative<std::shared_ptr<TissDB::Query::LogicalExpression>>(expr));
    auto& logical_expr = std::get<std::shared_ptr<TissDB::Query::LogicalExpression>>(expr);
    ASSERT_EQ("AND", logical_expr->op);

    // Check left side: status = 'shipped'
    ASSERT_TRUE(std::holds_alternative<std::shared_ptr<TissDB::Query::BinaryExpression>>(logical_expr->left));
    auto& left_expr = std::get<std::shared_ptr<TissDB::Query::BinaryExpression>>(logical_expr->left);
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Identifier>(left_expr->left));
    ASSERT_EQ("status", std::get<TissDB::Query::Identifier>(left_expr->left).name);
    ASSERT_EQ("=", left_expr->op);
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Literal>(left_expr->right));
    ASSERT_EQ("shipped", std::get<std::string>(std::get<TissDB::Query::Literal>(left_expr->right)));


    // Check right side: total > 50
    ASSERT_TRUE(std::holds_alternative<std::shared_ptr<TissDB::Query::BinaryExpression>>(logical_expr->right));
    auto& right_expr = std::get<std::shared_ptr<TissDB::Query::BinaryExpression>>(logical_expr->right);
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Identifier>(right_expr->left));
    ASSERT_EQ("total", std::get<TissDB::Query::Identifier>(right_expr->left).name);
    ASSERT_EQ(">", right_expr->op);
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Literal>(right_expr->right));
    ASSERT_EQ(50.0, std::get<double>(std::get<TissDB::Query::Literal>(right_expr->right)));
}

TEST_CASE(ParserInvalidQuery) {
    TissDB::Query::Parser parser;
    ASSERT_THROW(parser.parse("SELECT FROM users"), std::runtime_error);
    ASSERT_THROW(parser.parse("SELECT * users"), std::runtime_error);
}

TEST_CASE(ParserSelectWithParentheses) {
    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT * FROM products WHERE (category = 'electronics' AND price > 1000) OR in_stock = 1");
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::SelectStatement>(ast));
    auto& select_stmt = std::get<TissDB::Query::SelectStatement>(ast);
    ASSERT_TRUE(select_stmt.where_clause.has_value());

    auto& expr = select_stmt.where_clause.value();
    ASSERT_TRUE(std::holds_alternative<std::shared_ptr<TissDB::Query::LogicalExpression>>(expr));
    auto& or_expr = std::get<std::shared_ptr<TissDB::Query::LogicalExpression>>(expr);
    ASSERT_EQ("OR", or_expr->op);

    // Check the left side of OR: (category = 'electronics' AND price > 1000)
    ASSERT_TRUE(std::holds_alternative<std::shared_ptr<TissDB::Query::LogicalExpression>>(or_expr->left));
    auto& and_expr = std::get<std::shared_ptr<TissDB::Query::LogicalExpression>>(or_expr->left);
    ASSERT_EQ("AND", and_expr->op);

    // Check the left side of AND: category = 'electronics'
    ASSERT_TRUE(std::holds_alternative<std::shared_ptr<TissDB::Query::BinaryExpression>>(and_expr->left));
    auto& cat_expr = std::get<std::shared_ptr<TissDB::Query::BinaryExpression>>(and_expr->left);
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Identifier>(cat_expr->left));
    ASSERT_EQ("category", std::get<TissDB::Query::Identifier>(cat_expr->left).name);
    ASSERT_EQ("=", cat_expr->op);
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Literal>(cat_expr->right));
    ASSERT_EQ("electronics", std::get<std::string>(std::get<TissDB::Query::Literal>(cat_expr->right)));

    // Check the right side of AND: price > 1000
    ASSERT_TRUE(std::holds_alternative<std::shared_ptr<TissDB::Query::BinaryExpression>>(and_expr->right));
    auto& price_expr = std::get<std::shared_ptr<TissDB::Query::BinaryExpression>>(and_expr->right);
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Identifier>(price_expr->left));
    ASSERT_EQ("price", std::get<TissDB::Query::Identifier>(price_expr->left).name);
    ASSERT_EQ(">", price_expr->op);
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Literal>(price_expr->right));
    ASSERT_EQ(1000.0, std::get<double>(std::get<TissDB::Query::Literal>(price_expr->right)));

    // Check the right side of OR: in_stock = 1
    ASSERT_TRUE(std::holds_alternative<std::shared_ptr<TissDB::Query::BinaryExpression>>(or_expr->right));
    auto& stock_expr = std::get<std::shared_ptr<TissDB::Query::BinaryExpression>>(or_expr->right);
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Identifier>(stock_expr->left));
    ASSERT_EQ("in_stock", std::get<TissDB::Query::Identifier>(stock_expr->left).name);
    ASSERT_EQ("=", stock_expr->op);
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Literal>(stock_expr->right));
    ASSERT_EQ(1.0, std::get<double>(std::get<TissDB::Query::Literal>(stock_expr->right)));
}

TEST_CASE(ParserSelectWithParameters) {
    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT * FROM products WHERE category = ? AND price < ?");
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::SelectStatement>(ast));
    auto& select_stmt = std::get<TissDB::Query::SelectStatement>(ast);
    ASSERT_TRUE(select_stmt.where_clause.has_value());

    auto& expr = select_stmt.where_clause.value();
    ASSERT_TRUE(std::holds_alternative<std::shared_ptr<TissDB::Query::LogicalExpression>>(expr));
    auto& logical_expr = std::get<std::shared_ptr<TissDB::Query::LogicalExpression>>(expr);
    ASSERT_EQ("AND", logical_expr->op);

    // Check left side: category = ?
    ASSERT_TRUE(std::holds_alternative<std::shared_ptr<TissDB::Query::BinaryExpression>>(logical_expr->left));
    auto& left_expr = std::get<std::shared_ptr<TissDB::Query::BinaryExpression>>(logical_expr->left);
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Identifier>(left_expr->left));
    ASSERT_EQ("category", std::get<TissDB::Query::Identifier>(left_expr->left).name);
    ASSERT_EQ("=", left_expr->op);
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::ParameterExpression>(left_expr->right));
    ASSERT_EQ(0, std::get<TissDB::Query::ParameterExpression>(left_expr->right).index);


    // Check right side: price < ?
    ASSERT_TRUE(std::holds_alternative<std::shared_ptr<TissDB::Query::BinaryExpression>>(logical_expr->right));
    auto& right_expr = std::get<std::shared_ptr<TissDB::Query::BinaryExpression>>(logical_expr->right);
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Identifier>(right_expr->left));
    ASSERT_EQ("price", std::get<TissDB::Query::Identifier>(right_expr->left).name);
    ASSERT_EQ("<", right_expr->op);
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::ParameterExpression>(right_expr->right));
    ASSERT_EQ(1, std::get<TissDB::Query::ParameterExpression>(right_expr->right).index);
}
