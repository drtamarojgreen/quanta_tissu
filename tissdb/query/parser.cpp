#include "parser.h"
#include "../common/log.h"
#include <stdexcept>
#include <cctype>
#include <algorithm>
#include <cstdio>
#include <optional>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace TissDB {
namespace Query {

namespace {
// Helper to parse YYYY-MM-DD
std::optional<Date> parse_date_string(const std::string& s) {
    Date d;
    int year, month, day;
    if (sscanf(s.c_str(), "%d-%d-%d", &year, &month, &day) == 3) {
        d.year = year;
        d.month = month;
        d.day = day;
        return d;
    }
    return std::nullopt;
}

// Helper to parse HH:MM:SS
std::optional<Time> parse_time_string(const std::string& s) {
    Time t;
    int hour, minute, second;
    if (sscanf(s.c_str(), "%d:%d:%d", &hour, &minute, &second) == 3) {
        t.hour = hour;
        t.minute = minute;
        t.second = second;
        return t;
    }
    return std::nullopt;
}

// Helper to parse YYYY-MM-DD HH:MM:SS
std::optional<DateTime> parse_datetime_string(const std::string& s) {
    std::tm tm = {};
    std::stringstream ss(s);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (!ss.fail()) {
        time_t time;
        #ifdef _WIN32
            time = _mkgmtime(&tm);
        #else
            time = timegm(&tm);
        #endif
        if (time == -1) return std::nullopt;
        return std::chrono::system_clock::from_time_t(time);
    }
    return std::nullopt;
}

// Helper to parse ISO 8601 timestamp strings
std::optional<Timestamp> try_parse_timestamp(const std::string& s) {
    std::tm tm = {};
    std::stringstream ss(s);

    // Attempt to parse the common ISO 8601 format
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

    if (ss.fail()) {
        return std::nullopt;
    }

    // Check for fractional seconds
    double fractional_seconds = 0.0;
    if (ss.peek() == '.') {
        ss.ignore();
        ss >> fractional_seconds;
    }

    // Handle timezone
    char last_char = s.back();
    if (last_char == 'Z') {
        // UTC
    } else {
        // Check for +HH:MM or -HH:MM, not implemented for simplicity
    }

    // Convert to time_t and then to system_clock::time_point
    tm.tm_isdst = 0; // Assuming UTC, no DST
    time_t time = timegm(&tm);
    if (time == -1) {
        return std::nullopt;
    }

    auto time_point = std::chrono::system_clock::from_time_t(time);
    auto micro_seconds = std::chrono::duration_cast<std::chrono::microseconds>(time_point.time_since_epoch()).count();
    micro_seconds += static_cast<int64_t>(fractional_seconds * 1e6);

    return Timestamp{micro_seconds};
}

} // anonymous namespace


std::optional<Timestamp> Parser::try_parse_timestamp(const std::string& literal) {
    std::tm tm = {};
    std::stringstream ss(literal);

    // Try to parse the main part of the timestamp
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) {
        return std::nullopt; // Does not match the base format
    }

    // Check for fractional seconds
    long microseconds = 0;
    if (ss.peek() == '.') {
        ss.ignore(); // consume '.'
        std::string fractional_part;
        while (std::isdigit(ss.peek())) {
            fractional_part += ss.get();
        }
        if (!fractional_part.empty()) {
            // Pad with zeros to make it 6 digits for microseconds
            if (fractional_part.length() < 6) {
                fractional_part.append(6 - fractional_part.length(), '0');
            } else {
                fractional_part = fractional_part.substr(0, 6);
            }
            microseconds = std::stoi(fractional_part);
        }
    }

    // After parsing numbers, what's left should be the timezone part.
    std::string remaining_str;
    ss >> remaining_str;

    time_t time = -1;
    // The calendar time to UTC, this function is non-standard but available on Linux/macOS
    #ifdef _WIN32
        time = _mkgmtime(&tm);
    #else
        time = timegm(&tm);
    #endif

    if (time == -1) {
        return std::nullopt;
    }

    long long total_seconds = time;

    // Handle timezone offset
    if (remaining_str == "Z") {
        // It's UTC, no offset to apply
    } else if (remaining_str.length() >= 6 && (remaining_str[0] == '+' || remaining_str[0] == '-')) {
        try {
            int offset_hours = std::stoi(remaining_str.substr(1, 2));
            int offset_minutes = std::stoi(remaining_str.substr(4, 2));
            int offset_seconds = (offset_hours * 3600) + (offset_minutes * 60);

            if (remaining_str[0] == '-') {
                total_seconds += offset_seconds;
            } else {
                total_seconds -= offset_seconds;
            }
        } catch (const std::exception& e) {
            return std::nullopt; // Invalid offset format
        }
    } else if (!remaining_str.empty()) {
        return std::nullopt; // Unrecognized characters at the end
    }

    return Timestamp{total_seconds * 1000000 + microseconds};
}


// --- Tokenizer ---

std::vector<Token> Parser::tokenize(const std::string& query_string) {
    std::vector<Token> new_tokens;
    for (size_t i = 0; i < query_string.length(); ++i) {
        if (std::isspace(query_string[i])) continue;

        if (std::isalpha(query_string[i]) || query_string[i] == '_') {
            size_t start = i;
            while (i + 1 < query_string.length() && (std::isalnum(query_string[i + 1]) || query_string[i + 1] == '_')) {
                i++;
            }
            std::string value = query_string.substr(start, i - start + 1);
            std::string upper_value = value;
            std::transform(upper_value.begin(), upper_value.end(), upper_value.begin(), ::toupper);

<<<<<<< HEAD
            if (upper_value == "SELECT" || upper_value == "FROM" || upper_value == "WHERE" || upper_value == "AND" || upper_value == "OR" || upper_value == "UPDATE" || upper_value == "DELETE" || upper_value == "SET" || upper_value == "GROUP" || upper_value == "BY" || upper_value == "COUNT" || upper_value == "AVG" || upper_value == "SUM" || upper_value == "MIN" || upper_value == "MAX" || upper_value == "INSERT" || upper_value == "INTO" || upper_value == "VALUES" || upper_value == "STDDEV" || upper_value == "LIKE" || upper_value == "ORDER" || upper_value == "LIMIT" || upper_value == "JOIN" || upper_value == "ON" || upper_value == "UNION" || upper_value == "ALL" || upper_value == "ASC" || upper_value == "DESC" || upper_value == "WITH" || upper_value == "DRILLDOWN" || upper_value == "TRUE" || upper_value == "FALSE" || upper_value == "NULL" || upper_value == "DATE" || upper_value == "TIME" || upper_value == "DATETIME" || upper_value == "TIMESTAMP") {
=======
            if (upper_value == "SELECT" || upper_value == "FROM" || upper_value == "WHERE" || upper_value == "AND" || upper_value == "OR" || upper_value == "UPDATE" || upper_value == "DELETE" || upper_value == "SET" || upper_value == "GROUP" || upper_value == "BY" || upper_value == "COUNT" || upper_value == "AVG" || upper_value == "SUM" || upper_value == "MIN" || upper_value == "MAX" || upper_value == "INSERT" || upper_value == "INTO" || upper_value == "VALUES" || upper_value == "STDDEV" || upper_value == "LIKE" || upper_value == "ORDER" || upper_value == "LIMIT" || upper_value == "JOIN" || upper_value == "ON" || upper_value == "UNION" || upper_value == "ALL" || upper_value == "ASC" || upper_value == "DESC" || upper_value == "WITH" || upper_value == "DRILLDOWN" || upper_value == "TRUE" || upper_value == "FALSE" || upper_value == "NULL" || upper_value == "DATE" || upper_value == "TIME") {
>>>>>>> feat/timestamp-object
                new_tokens.push_back(Token{Token::Type::KEYWORD, upper_value});
            } else {
                new_tokens.push_back(Token{Token::Type::IDENTIFIER, value});
            }
        } else if (std::isdigit(query_string[i])) {
            size_t start = i;
            while (i + 1 < query_string.length() && (std::isdigit(query_string[i + 1]) || query_string[i + 1] == '.')) {
                i++;
            }
            std::string val = query_string.substr(start, i - start + 1);
            if (val.empty()) {
                throw std::runtime_error("Tokenizer created an empty numeric literal.");
            }
            new_tokens.push_back(Token{Token::Type::NUMERIC_LITERAL, val});
        } else if (query_string[i] == '\'') {
            size_t start = ++i;
            while (i < query_string.length() && query_string[i] != '\'') {
                i++;
            }
            new_tokens.push_back(Token{Token::Type::STRING_LITERAL, query_string.substr(start, i - start)});
        } else if (query_string[i] == '=' || query_string[i] == '!' || query_string[i] == '<' || query_string[i] == '>' || query_string[i] == '+' || query_string[i] == '-' || query_string[i] == '*' || query_string[i] == '/') {
            size_t start = i;
            if (i + 1 < query_string.length() && query_string[i + 1] == '=') { // Handles ==, !=, <=, >=
                i++;
            }
            new_tokens.push_back(Token{Token::Type::OPERATOR, query_string.substr(start, i - start + 1)});
        } else if (query_string[i] == ',' || query_string[i] == '(' || query_string[i] == ')') {
            new_tokens.push_back(Token{Token::Type::OPERATOR, std::string(1, query_string[i])});
        } else if (query_string[i] == '?') {
            new_tokens.push_back(Token{Token::Type::PARAM_PLACEHOLDER, "?"});
        }
    }
    new_tokens.push_back(Token{Token::Type::EOI, ""});
    return new_tokens;
}


// --- Parser ---

Parser::Parser() = default;

AST Parser::parse(const std::string& query_string) {
    LOG_INFO("Parsing query: " + query_string);
    tokens = tokenize(query_string);
    pos = 0;
    param_index = 0;

    if (peek().type == Token::Type::KEYWORD) {
        if (peek().value == "SELECT") {
            auto ast = parse_select_statement();
            LOG_DEBUG("Successfully parsed SELECT statement.");
            return ast;
        } else if (peek().value == "UPDATE") {
            auto ast = parse_update_statement();
            LOG_DEBUG("Successfully parsed UPDATE statement.");
            return ast;
        } else if (peek().value == "DELETE") {
            auto ast = parse_delete_statement();
            LOG_DEBUG("Successfully parsed DELETE statement.");
            return ast;
        } else if (peek().value == "INSERT") {
            auto ast = parse_insert_statement();
            LOG_DEBUG("Successfully parsed INSERT statement.");
            return ast;
        }
    }

    LOG_ERROR("Unsupported statement type at start of query.");
    throw std::runtime_error("Unsupported statement type");
}

SelectStatement Parser::parse_select_statement() {
    // This function parses a single "SELECT ... FROM ... etc." block.
    expect(Token::Type::KEYWORD, "SELECT");
    auto fields = parse_select_list();
    expect(Token::Type::KEYWORD, "FROM");
    auto table = parse_table_name();
    auto join = parse_join_clause();
    auto where = parse_where_clause();
    auto group_by = parse_group_by_clause();
    auto order_by = parse_order_by_clause();
    auto limit = parse_limit_clause();
    auto drilldown = parse_drilldown_clause();

    // Create the SelectStatement for the query parsed so far.
    auto current_select = SelectStatement{fields, table, std::move(where), group_by, order_by, limit, std::move(join), std::nullopt, std::move(drilldown)};

    // Check if this statement is followed by a UNION.
    if (peek().type == Token::Type::KEYWORD && peek().value == "UNION") {
        consume(); // consume UNION
        bool all = false;
        if (peek().type == Token::Type::KEYWORD && peek().value == "ALL") {
            consume(); // consume ALL
            all = true;
        }

        // The statement we just built is the left side of the.
        auto left_select_ptr = std::make_unique<SelectStatement>(std::move(current_select));

        // The right side of the union is the result of the next recursive call.
        auto right_select_ptr = std::make_unique<SelectStatement>(parse_select_statement());

        // Create the union clause.
        auto union_clause = UnionClause{std::move(left_select_ptr), std::move(right_select_ptr), all};

        // Return a new, "wrapper" select statement that only holds the union clause.
        // The executor must check for the presence of union_clause first.
        return SelectStatement{{}, "", std::nullopt, {}, {}, std::nullopt, std::nullopt, std::make_optional(std::move(union_clause)), {}};
    }

    // If there was no UNION, just return the single statement we parsed.
    return current_select;
}

UpdateStatement Parser::parse_update_statement() {
    expect(Token::Type::KEYWORD, "UPDATE");
    auto table = parse_table_name();
    expect(Token::Type::KEYWORD, "SET");
    auto set = parse_set_clause();
    auto where = parse_where_clause();
    return {table, set, std::move(where)};
}

DeleteStatement Parser::parse_delete_statement() {
    expect(Token::Type::KEYWORD, "DELETE");
    expect(Token::Type::KEYWORD, "FROM");
    auto table = parse_table_name();
    auto where = parse_where_clause();
    return {table, std::move(where)};
}

InsertStatement Parser::parse_insert_statement() {
    expect(Token::Type::KEYWORD, "INSERT");
    expect(Token::Type::KEYWORD, "INTO");
    auto table = parse_table_name();
    std::vector<std::string> columns;
    if (peek().type == Token::Type::OPERATOR && peek().value == "(") {
        consume();
        columns = parse_column_list();
        expect(Token::Type::OPERATOR, ")");
    }
    expect(Token::Type::KEYWORD, "VALUES");
    expect(Token::Type::OPERATOR, "(");
    auto values = parse_value_list();
    expect(Token::Type::OPERATOR, ")");
    return {table, columns, values};
}

std::vector<std::variant<std::string, AggregateFunction>> Parser::parse_select_list() {
    std::vector<std::variant<std::string, AggregateFunction>> fields;
    if (peek().type == Token::Type::OPERATOR && peek().value == "*") {
        consume();
        // The executor will see a single "*" string and know to select all fields.
        fields.push_back("*");
        return fields;
    }

    do {
        if (peek().type == Token::Type::KEYWORD && (peek().value == "COUNT" || peek().value == "AVG" || peek().value == "SUM" || peek().value == "MIN" || peek().value == "MAX")) {
            fields.push_back(parse_aggregate_function());
        } else {
            // It's a regular column name
            fields.push_back(consume().value);
        }

        if (peek().type == Token::Type::OPERATOR && peek().value == ",") {
            consume();
        } else {
            break;
        }
    } while (true);
    return fields;
}

AggregateFunction Parser::parse_aggregate_function() {
    auto func_token = consume();
    std::string func_name = func_token.value;

    AggregateType type;
    if (func_name == "COUNT") type = AggregateType::COUNT;
    else if (func_name == "AVG") type = AggregateType::AVG;
    else if (func_name == "SUM") type = AggregateType::SUM;
    else if (func_name == "MIN") type = AggregateType::MIN;
    else if (func_name == "MAX") type = AggregateType::MAX;
    else {
        throw std::runtime_error("Unknown aggregate function: " + func_name);
    }

    expect(Token::Type::OPERATOR, "(");

    if (peek().type == Token::Type::OPERATOR && peek().value == "*") {
        consume(); // consume '*'
        if (type != AggregateType::COUNT) {
            throw std::runtime_error("'*' argument is only valid for COUNT");
        }
        expect(Token::Type::OPERATOR, ")");
        return {type, std::nullopt};
    } else {
        expect(Token::Type::IDENTIFIER);
        std::string field_name = tokens[pos-1].value;
        expect(Token::Type::OPERATOR, ")");
        return {type, field_name};
    }
}

std::string Parser::parse_table_name() {
    return consume().value;
}

std::vector<std::string> Parser::parse_column_list() {
    std::vector<std::string> columns;
    do {
        columns.push_back(consume().value);
        if (peek().type == Token::Type::OPERATOR && peek().value == ",") {
            consume();
        } else {
            break;
        }
    } while (true);
    return columns;
}

std::vector<Literal> Parser::parse_value_list() {
    std::vector<Literal> values;
    do {
        auto expr = parse_primary_expression();
        if (std::holds_alternative<Literal>(expr)) {
            values.push_back(std::get<Literal>(expr));
        } else {
            LOG_ERROR("Parse error: Expected a literal value in value list.");
            throw std::runtime_error("Expected a literal value in value list, but got other expression type.");
        }

        if (peek().type == Token::Type::OPERATOR && peek().value == ",") {
            consume();
        } else {
            break;
        }
    } while (true);
    return values;
}

std::optional<Expression> Parser::parse_where_clause() {
    if (peek().type == Token::Type::KEYWORD && peek().value == "WHERE") {
        consume();
        return parse_expression();
    }
    return std::nullopt;
}

std::vector<std::pair<std::string, Expression>> Parser::parse_set_clause() {
    std::vector<std::pair<std::string, Expression>> set_clause;
    do {
        auto identifier = consume();
        expect(Token::Type::OPERATOR, "=");
        auto value_expr = parse_expression();
        set_clause.push_back({identifier.value, std::move(value_expr)});
        if (peek().type == Token::Type::OPERATOR && peek().value == ",") {
            consume();
        } else {
            break;
        }
    } while (true);
    return set_clause;
}

std::vector<std::string> Parser::parse_group_by_clause() {
    std::vector<std::string> group_by_fields;
    if (peek().type == Token::Type::KEYWORD && peek().value == "GROUP") {
        consume();
        expect(Token::Type::KEYWORD, "BY");
        do {
            group_by_fields.push_back(consume().value);
            if (peek().type == Token::Type::OPERATOR && peek().value == ",") {
                consume();
            } else {
                break;
            }
        } while (true);
    }
    return group_by_fields;
}

std::vector<std::pair<std::string, std::string>> Parser::parse_order_by_clause() {
    std::vector<std::pair<std::string, std::string>> order_by_clause;
    if (peek().type == Token::Type::KEYWORD && peek().value == "ORDER") {
        consume();
        expect(Token::Type::KEYWORD, "BY");
        do {
            std::string field = consume().value;
            std::string direction = "ASC"; // Default
            if (peek().type == Token::Type::KEYWORD && (peek().value == "ASC" || peek().value == "DESC")) {
                direction = consume().value;
            }
            order_by_clause.push_back({field, direction});
            if (peek().type == Token::Type::OPERATOR && peek().value == ",") {
                consume();
            } else {
                break;
            }
        } while (true);
    }
    return order_by_clause;
}

std::optional<double> Parser::parse_limit_clause() {
    if (peek().type == Token::Type::KEYWORD && peek().value == "LIMIT") {
        consume();
        auto token = consume();
        if (token.type == Token::Type::NUMERIC_LITERAL) {
            return std::stod(token.value);
        } else {
            throw std::runtime_error("Expected numeric literal for LIMIT clause.");
        }
    }
    return std::nullopt;
}

std::optional<JoinClause> Parser::parse_join_clause() {
    JoinType type = JoinType::INNER;
    bool is_join = false;

    if (peek().type == Token::Type::KEYWORD) {
        if (peek().value == "INNER" || peek().value == "LEFT" || peek().value == "RIGHT" || peek().value == "FULL" || peek().value == "CROSS") {
            std::string join_kw = consume().value;
            if (join_kw == "INNER") type = JoinType::INNER;
            else if (join_kw == "LEFT") type = JoinType::LEFT;
            else if (join_kw == "RIGHT") type = JoinType::RIGHT;
            else if (join_kw == "FULL") type = JoinType::FULL;
            else if (join_kw == "CROSS") type = JoinType::CROSS;
            is_join = true;
        }
    }

    if ((peek().type == Token::Type::KEYWORD && peek().value == "JOIN")) {
        consume(); // Consume "JOIN"
        is_join = true;
    }

    if (is_join) {
        std::string collection_name = parse_table_name();
        if (type == JoinType::CROSS) {
            // CROSS JOIN does not have an ON clause.
            return JoinClause{collection_name, type, Expression{}};
        }
        expect(Token::Type::KEYWORD, "ON");
        Expression on_condition = parse_expression();
        return JoinClause{collection_name, type, std::move(on_condition)};
    }

    return std::nullopt;
}

std::optional<UnionClause> Parser::parse_union_clause() {
    // This function is not called directly.
    // The logic is handled inside parse_select_statement to manage the recursive structure.
    return std::nullopt;
}

std::optional<DrilldownClause> Parser::parse_drilldown_clause() {
    if (peek().type == Token::Type::KEYWORD && peek().value == "WITH") {
        consume(); // consume "WITH"
        expect(Token::Type::KEYWORD, "DRILLDOWN");
        expect(Token::Type::OPERATOR, "(");
        DrilldownClause clause;
        clause.fields = parse_column_list();
        expect(Token::Type::OPERATOR, ")");
        return clause;
    }
    return std::nullopt;
}

Expression Parser::parse_expression(int precedence) {
    auto left = parse_primary_expression();

    while (true) {
        auto op = peek().value;
        int new_precedence = 0;
        if (op == "AND" || op == "OR") new_precedence = 1;
        else if (op == "=" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=" || op == "LIKE") new_precedence = 2;
        else if (op == "+" || op == "-") new_precedence = 3;
        else if (op == "*" || op == "/") new_precedence = 4;

        if (new_precedence == 0 || new_precedence <= precedence) break;

        consume();
        auto right = parse_expression(new_precedence);
        if (op == "AND" || op == "OR") {
            left = std::make_shared<LogicalExpression>(LogicalExpression{std::move(left), op, std::move(right)});
        } else {
            left = std::make_shared<BinaryExpression>(BinaryExpression{std::move(left), op, std::move(right)});
        }
    }

    return left;
}

Expression Parser::parse_primary_expression() {
    if (peek().type == Token::Type::KEYWORD && (peek().value == "DATE" || peek().value == "TIME" || peek().value == "DATETIME" || peek().value == "TIMESTAMP")) {
        std::string keyword = consume().value;
        auto token = consume();
        if (token.type != Token::Type::STRING_LITERAL) {
            throw std::runtime_error("Expected a string literal after " + keyword);
        }

        if (keyword == "DATE") {
            if (auto date = parse_date_string(token.value)) return Literal{*date};
            throw std::runtime_error("Invalid DATE format: " + token.value);
        } else if (keyword == "TIME") {
            if (auto time = parse_time_string(token.value)) return Literal{*time};
            throw std::runtime_error("Invalid TIME format: " + token.value);
        } else if (keyword == "DATETIME") {
            if (auto dt = parse_datetime_string(token.value)) return Literal{*dt};
            throw std::runtime_error("Invalid DATETIME format: " + token.value);
        } else { // TIMESTAMP
            if (auto ts = try_parse_timestamp(token.value)) return Literal{*ts};
            throw std::runtime_error("Invalid TIMESTAMP format: " + token.value);
        }
    }

    auto token = consume();
    if (token.type == Token::Type::IDENTIFIER) {
        return Identifier{token.value};
    } else if (token.type == Token::Type::NUMERIC_LITERAL) {
        return Literal{std::stod(token.value)};
    } else if (token.type == Token::Type::STRING_LITERAL) {
        if (auto ts = try_parse_timestamp(token.value)) {
            return Literal{*ts};
        }
        return Literal{token.value};
    } else if (token.type == Token::Type::KEYWORD && token.value == "TRUE") {
        return Literal{true};
    } else if (token.type == Token::Type::KEYWORD && token.value == "FALSE") {
        return Literal{false};
    } else if (token.type == Token::Type::KEYWORD && token.value == "NULL") {
        return Literal{Null{}};
    } else if (token.type == Token::Type::PARAM_PLACEHOLDER) {
        return ParameterExpression{param_index++};
    }
    LOG_ERROR("Parse error: Unexpected token in expression: " + token.value);
    throw std::runtime_error("Unexpected token in expression");
}


// --- Helper methods ---

Token Parser::peek() {
    return tokens[pos];
}

Token Parser::consume() {
    return tokens[pos++];
}

void Parser::expect(Token::Type type, const std::string& value) {
    auto token = consume();
    if (token.type != type || (!value.empty() && token.value != value)) {
        std::string error_msg = "Expected token " + value + " but got " + token.value;
        LOG_ERROR("Parse error: " + error_msg);
        throw std::runtime_error(error_msg);
    }
}

} // namespace Query
} // namespace TissDB