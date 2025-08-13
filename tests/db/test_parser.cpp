
#include "test_main.cpp"
#include "../../../tissdb/query/parser.h"

TEST_CASE(ParserSelectAll) {
    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT * FROM users");
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::SelectStatement>(ast));
    TissDB::Query::SelectStatement select_stmt = std::get<TissDB::Query::SelectStatement>(ast);
    ASSERT_EQ(1, select_stmt.fields.size());
    ASSERT_EQ("*", select_stmt.fields[0]);
    ASSERT_EQ("users", select_stmt.collection_name);
    ASSERT_FALSE(select_stmt.where_clause.has_value());
}

TEST_CASE(ParserSelectSpecificFields) {
    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT name, age FROM employees");
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::SelectStatement>(ast));
    TissDB::Query::SelectStatement select_stmt = std::get<TissDB::Query::SelectStatement>(ast);
    ASSERT_EQ(2, select_stmt.fields.size());
    ASSERT_EQ("name", select_stmt.fields[0]);
    ASSERT_EQ("age", select_stmt.fields[1]);
    ASSERT_EQ("employees", select_stmt.collection_name);
    ASSERT_FALSE(select_stmt.where_clause.has_value());
}

TEST_CASE(ParserSelectWithWhereClause) {
    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT * FROM products WHERE price > 100");
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::SelectStatement>(ast));
    TissDB::Query::SelectStatement select_stmt = std::get<TissDB::Query::SelectStatement>(ast);
    ASSERT_TRUE(select_stmt.where_clause.has_value());

    auto& expr = select_stmt.where_clause.value();
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::BinaryExpression>(expr));
    auto& binary_expr = std::get<TissDB::Query::BinaryExpression>(expr);

    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Identifier>(*binary_expr.left));
    ASSERT_EQ("price", std::get<TissDB::Query::Identifier>(*binary_expr.left).name);

    ASSERT_EQ(">", binary_expr.op);

    ASSERT_TRUE(std::holds_alternative<TissDB::Query::Literal>(*binary_expr.right));
    ASSERT_EQ(100.0, std::get<double>(std::get<TissDB::Query::Literal>(*binary_expr.right)));
}

TEST_CASE(ParserSelectWithLogicalOperators) {
    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT * FROM orders WHERE status = 'shipped' AND total > 50");
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::SelectStatement>(ast));
    TissDB::Query::SelectStatement select_stmt = std::get<TissDB::Query::SelectStatement>(ast);
    ASSERT_TRUE(select_stmt.where_clause.has_value());

    auto& expr = select_stmt.where_clause.value();
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::LogicalExpression>(expr));
    auto& logical_expr = std::get<TissDB::Query::LogicalExpression>(expr);
    ASSERT_EQ("AND", logical_expr.op);
}

TEST_CASE(ParserInvalidQuery) {
    TissDB::Query::Parser parser;
    ASSERT_THROW(parser.parse("SELECT FROM users"), std::runtime_error);
    ASSERT_THROW(parser.parse("SELECT * users"), std::runtime_error);
}
