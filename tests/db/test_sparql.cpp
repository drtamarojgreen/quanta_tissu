#include "test_framework.h"
#include "../../tissdb/query/parser.h"
#include "../../tissdb/query/sparql_parser.h"
#include "../../tissdb/query/executor.h"
#include "../../tissdb/query/executor_sparql.h"
#include "../../tissdb/storage/lsm_tree.h"
#include "../../tissdb/storage/triple_store.h"
#include <algorithm>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

namespace sparql_test {

const std::string PEOPLE_NT =
    "<http://example.org/alice> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.org/Person> .\n"
    "<http://example.org/alice> <http://example.org/name> \"Alice\" .\n"
    "<http://example.org/alice> <http://example.org/age> \"34\"^^<http://www.w3.org/2001/XMLSchema#integer> .\n"
    "<http://example.org/alice> <http://example.org/knows> <http://example.org/bob> .\n"
    "<http://example.org/alice> <http://example.org/knows> <http://example.org/carol> .\n"
    "<http://example.org/bob> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.org/Person> .\n"
    "<http://example.org/bob> <http://example.org/name> \"Bob\"@en .\n"
    "<http://example.org/bob> <http://example.org/age> \"28\"^^<http://www.w3.org/2001/XMLSchema#integer> .\n"
    "<http://example.org/bob> <http://example.org/knows> <http://example.org/carol> .\n"
    "<http://example.org/carol> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.org/Person> .\n"
    "<http://example.org/carol> <http://example.org/name> \"Carol\" .\n"
    "<http://example.org/carol> <http://example.org/age> \"41\"^^<http://www.w3.org/2001/XMLSchema#integer> .\n"
    "<http://example.org/carol> <http://example.org/worksFor> _:acme .\n"
    "_:acme <http://example.org/name> \"Acme Corp\" .\n";

struct SparqlFixture {
    const std::string test_dir = "./test_sparql_data";
    std::unique_ptr<TissDB::Storage::LSMTree> storage;
    std::unique_ptr<TissDB::Query::Executor> executor;

    explicit SparqlFixture(const std::string& ntriples = PEOPLE_NT) {
        std::filesystem::remove_all(test_dir);
        storage = std::make_unique<TissDB::Storage::LSMTree>(test_dir);
        executor = std::make_unique<TissDB::Query::Executor>(*storage);
        if (!ntriples.empty()) {
            TissDB::Storage::TripleStore(*storage).load_ntriples(ntriples);
        }
    }

    ~SparqlFixture() {
        storage->shutdown();
        std::filesystem::remove_all(test_dir);
    }

    TissDB::Query::QueryResult run(const std::string& query) {
        TissDB::Query::Parser parser;
        return executor->execute(parser.parse(query), {});
    }
};

// Value of a result element, or "" if the variable is unbound in that row.
std::string field(const TissDB::Document& doc, const std::string& key) {
    for (const auto& element : doc.elements) {
        if (element.key == key) return std::get<std::string>(element.value);
    }
    return "";
}

std::multiset<std::string> column(const TissDB::Query::QueryResult& result, const std::string& key) {
    std::multiset<std::string> values;
    for (const auto& doc : result) values.insert(field(doc, key));
    return values;
}

TissDB::Query::SparqlSelectStatement parse_sparql(const std::string& query) {
    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse(query);
    auto* stmt = std::get_if<TissDB::Query::SparqlSelectStatement>(&ast);
    if (!stmt) throw std::runtime_error("query was not parsed as SPARQL");
    return *stmt;
}

// Triple patterns of the index-th element of a group (which must be a TRIPLES element).
const std::vector<TissDB::Query::SparqlTriplePattern>& triples(const TissDB::Query::SparqlGroupPattern& group, size_t index = 0) {
    const auto& element = group.elements.at(index);
    if (element.kind != TissDB::Query::SparqlGroupElement::Kind::TRIPLES) throw std::runtime_error("not a TRIPLES element");
    return element.triples;
}

const TissDB::Rdf::Term& constant(const TissDB::Query::SparqlPatternTerm& position) {
    return std::get<TissDB::Rdf::Term>(position);
}

const std::string& variable(const TissDB::Query::SparqlPatternTerm& position) {
    return std::get<TissDB::Query::SparqlVariable>(position).name;
}

} // namespace sparql_test

TEST_CASE(SparqlIdentifiedByPrefixKeyword) {
    using TissDB::Query::is_sparql_query;
    ASSERT_TRUE(is_sparql_query("PREFIX ex: <http://example.org/> SELECT ?s WHERE { ?s ?p ?o }"));
    ASSERT_TRUE(is_sparql_query("  \n\tprefix ex: <http://example.org/> SELECT * { ?s ?p ?o }"));
    ASSERT_TRUE(is_sparql_query("# leading comment\nPREFIX : <http://example.org/> SELECT * { ?s ?p ?o }"));
    ASSERT_FALSE(is_sparql_query("SELECT * FROM products"));
    ASSERT_FALSE(is_sparql_query("SELECT * FROM prefix_table"));
    ASSERT_FALSE(is_sparql_query("PREFIXED_COLUMN"));
    ASSERT_FALSE(is_sparql_query(""));

    TissDB::Query::Parser parser;
    auto sparql_ast = parser.parse("PREFIX ex: <http://example.org/> SELECT ?s WHERE { ?s ex:p ?o }");
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::SparqlSelectStatement>(sparql_ast));
    auto tissql_ast = parser.parse("SELECT * FROM products WHERE price > 10");
    ASSERT_TRUE(std::holds_alternative<TissDB::Query::SelectStatement>(tissql_ast));
}

TEST_CASE(SparqlParserBuildsTriplePatterns) {
    namespace XSD = TissDB::Rdf::XSD;
    using TissDB::Rdf::Term;

    auto stmt = sparql_test::parse_sparql(
        "PREFIX ex: <http://example.org/>\n"
        "PREFIX : <http://example.org/default#>\n"
        "SELECT DISTINCT ?person ?name\n"
        "WHERE {\n"
        "  ?person a ex:Person ;\n"
        "          ex:name ?name ;\n"
        "          ex:knows ex:bob, :dave .\n"
        "  ?person ex:age 34 . ?person ex:score -1.5 . ?person ex:ratio 2e3 .\n"
        "  ?person ex:label 'it\\'s'@EN . ?person ex:flag true .\n"
        "  ?person <http://example.org/full> \"x\"^^ex:custom .\n"
        "  _:anon ex:p ?person\n"
        "}\n"
        "ORDER BY DESC(?name) ?person LIMIT 10 OFFSET 5");

    ASSERT_TRUE(stmt.distinct);
    ASSERT_FALSE(stmt.select_all);
    ASSERT_TRUE((stmt.variables == std::vector<std::string>{"person", "name"}));
    ASSERT_EQ(std::string("http://example.org/"), stmt.prefixes.at("ex"));
    ASSERT_EQ(std::string("http://example.org/default#"), stmt.prefixes.at(""));

    ASSERT_EQ(1u, stmt.where.elements.size());
    const auto& where = sparql_test::triples(stmt.where);
    ASSERT_EQ(11u, where.size());
    ASSERT_EQ(std::string("person"), sparql_test::variable(where[0].subject));
    ASSERT_TRUE(sparql_test::constant(where[0].predicate) == Term::iri(TissDB::Rdf::Vocab::RDF_TYPE));
    ASSERT_TRUE(sparql_test::constant(where[0].object) == Term::iri("http://example.org/Person"));
    ASSERT_EQ(std::string("name"), sparql_test::variable(where[1].object));
    ASSERT_TRUE(sparql_test::constant(where[2].object) == Term::iri("http://example.org/bob"));
    ASSERT_TRUE(sparql_test::constant(where[3].object) == Term::iri("http://example.org/default#dave"));
    ASSERT_TRUE(sparql_test::constant(where[4].object) == Term::literal("34", XSD::INTEGER));
    ASSERT_TRUE(sparql_test::constant(where[5].object) == Term::literal("-1.5", XSD::DECIMAL));
    ASSERT_TRUE(sparql_test::constant(where[6].object) == Term::literal("2e3", XSD::DOUBLE));
    ASSERT_TRUE(sparql_test::constant(where[7].object) == Term::lang_literal("it's", "en"));
    ASSERT_TRUE(sparql_test::constant(where[8].object) == Term::literal("true", XSD::BOOLEAN));
    ASSERT_TRUE(sparql_test::constant(where[9].predicate) == Term::iri("http://example.org/full"));
    ASSERT_TRUE(sparql_test::constant(where[9].object) == Term::literal("x", "http://example.org/custom"));
    ASSERT_EQ(std::string("_:anon"), sparql_test::variable(where[10].subject));

    ASSERT_EQ(2u, stmt.order_by.size());
    ASSERT_EQ(std::string("name"), stmt.order_by[0].variable);
    ASSERT_TRUE(stmt.order_by[0].descending);
    ASSERT_FALSE(stmt.order_by[1].descending);
    ASSERT_TRUE(stmt.limit == 10u);
    ASSERT_TRUE(stmt.offset == 5u);
}

TEST_CASE(SparqlParserHandlesBaseAndFilters) {
    auto stmt = sparql_test::parse_sparql(
        "PREFIX ex: <http://example.org/> BASE <http://example.org/> "
        "SELECT * WHERE { <alice> ?p ?o . FILTER(?o != <bob> && (isLiteral(?o) || !BOUND(?p))) FILTER regex(str(?o), \"^A\", \"i\") }");
    ASSERT_TRUE(stmt.select_all);
    const auto& where = sparql_test::triples(stmt.where);
    ASSERT_EQ(1u, where.size());
    ASSERT_TRUE(sparql_test::constant(where[0].subject) == TissDB::Rdf::Term::iri("http://example.org/alice"));
    ASSERT_EQ(2u, stmt.where.filters.size());
    ASSERT_EQ(std::string("&&"), stmt.where.filters[0]->op);
    ASSERT_EQ(std::string("REGEX"), stmt.where.filters[1]->op);
    ASSERT_EQ(3u, stmt.where.filters[1]->args.size());
}

TEST_CASE(SparqlInvalidQueriesReportClearErrors) {
    // Each invalid query must be rejected with a message saying what is wrong and
    // where (1-based line and column). P is 33 characters long.
    const std::string P = "PREFIX ex: <http://example.org/> ";
    const std::string E = "SPARQL syntax error at line 1, column ";
    const std::vector<std::pair<std::string, std::string>> cases = {
        // Prologue and query form
        {"PREFIX ex <http://example.org/> SELECT ?s WHERE { ?s ?p ?o }",
         "SPARQL syntax error at line 1, column 8: prefix label must end with ':', e.g. 'ex:' near 'ex'"},
        {"PREFIX", "SPARQL syntax error at line 1, column 7: expected a prefix label such as 'ex:' after PREFIX at end of query"},
        {"PREFIX ex: SELECT ?s WHERE { ?s ?p ?o }",
         "SPARQL syntax error at line 1, column 12: expected an IRI in angle brackets after 'ex:' near 'SELECT'"},
        {"PREFIX ex: <http://example.org/>", E + "33: expected SELECT after the PREFIX declarations at end of query"},
        {P + "CONSTRUCT { ?s ex:p ?o } WHERE { ?s ex:p ?o }", E + "34: CONSTRUCT queries are not supported; only SELECT queries are"},
        {P + "ASK { ?s ex:p ?o }", E + "34: ASK queries are not supported; only SELECT queries are"},
        // SELECT clause
        {P + "SELECT WHERE { ?s ex:p ?o }", E + "41: expected variables or '*' after SELECT near 'WHERE'"},
        {P + "SELECT (COUNT(?s) AS ?n) WHERE { ?s ex:p ?o }",
         E + "41: expressions in SELECT (such as aggregates or AS) are not supported; list variables or use '*' near '('"},
        // WHERE clause structure
        {P + "SELECT ?s WHERE ?s ex:p ?o", E + "50: expected '{' to start the WHERE clause near '?s'"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o ", E + "63: missing '}' to close the '{' at line 1, column 50"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o . . }", E + "65: expected a triple pattern, FILTER, OPTIONAL or '}' near '.'"},
        {P + "SELECT ?s WHERE { ?s }", E + "55: triple pattern is missing its predicate and object near '}'"},
        {P + "SELECT ?s WHERE { ?s ex:p }", E + "60: triple pattern is missing its object near '}'"},
        {P + "SELECT ?s WHERE { \"lit\" ex:p ?o }", E + "52: a literal cannot be the subject of a triple pattern near '\"lit\"'"},
        {P + "SELECT ?s WHERE { ?s \"lit\" ?o }", E + "55: a predicate must be an IRI or a variable near '\"lit\"'"},
        {P + "SELECT ?s WHERE { ?s nope:p ?o }", E + "55: undeclared prefix 'nope:'; declare it with PREFIX nope: <namespace-iri>"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o OPTIONAL ?s ex:q ?x }", E + "72: expected '{' after OPTIONAL near '?s'"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o OPTIONAL { ?s ex:q ?x }", E + "86: missing '}' to close the '{' at line 1, column 50"},
        // Unsupported SPARQL features are named
        {P + "SELECT ?s WHERE { ?s ex:p ?o UNION { ?s ex:q ?x } }", E + "63: UNION is not supported"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o MINUS { ?s ex:q ?x } }", E + "63: MINUS is not supported"},
        {P + "SELECT ?s WHERE { { ?s ex:p ?o } UNION { ?s ex:q ?x } }",
         E + "52: nested group patterns are not supported; only OPTIONAL { ... } may contain a group"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o } GROUP BY ?s", E + "65: GROUP BY is not supported"},
        // FILTER expressions
        {P + "SELECT ?s WHERE { ?s ex:p ?o FILTER ?o > 1 }", E + "70: expected '(' or a function call after FILTER near '?o'"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o FILTER(?o > ) }", E + "75: expected an expression near ')'"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o FILTER(nosuchfn(?o)) }", E + "70: unknown function 'nosuchfn'"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o FILTER(regex(?o)) }", E + "70: REGEX expects 2 or 3 arguments but got 1"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o FILTER(str(?o, ?s)) }", E + "70: STR expects 1 argument but got 2"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o FILTER(BOUND(\"x\")) }", E + "70: BOUND expects a variable argument"},
        // Solution modifiers and trailing input
        {P + "SELECT ?s WHERE { ?s ex:p ?o } ORDER BY", E + "73: expected a variable, ASC(?var) or DESC(?var) after ORDER BY at end of query"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o } LIMIT -1", E + "71: LIMIT expects a non-negative integer near '-1'"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o } LIMIT ten", E + "71: LIMIT expects a non-negative integer near 'ten'"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o } LIMIT 1 LIMIT 2", E + "73: LIMIT may only be given once"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o } OFFSET 1 OFFSET 2", E + "74: OFFSET may only be given once"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o } extra", E + "65: unexpected 'extra' after the end of the query"},
        // Lexical errors
        {P + "SELECT ?s WHERE { ?s ex:p \"unterminated }", E + "60: unterminated string literal"},
        {P + "SELECT ?s WHERE { ?s ex:p \"bad \\q escape\" }", E + "65: invalid escape sequence '\\q' in string literal"},
        {P + "SELECT ?s WHERE { ?s ex:p ?o ~ }", E + "63: unexpected character '~'"},
        {P + "SELECT ?s WHERE { ?s ex:p _: }", E + "60: missing blank node label after '_:'"},
    };

    TissDB::Query::Parser parser;
    int failures = 0;
    for (const auto& [query, expected] : cases) {
        std::string actual = "(no error)";
        try {
            parser.parse(query);
        } catch (const TissDB::Query::SparqlSyntaxError& e) {
            actual = e.what();
        } catch (const std::exception& e) {
            actual = std::string("(wrong exception type) ") + e.what();
        }
        if (actual != expected) {
            failures++;
            std::cerr << "\nQuery: " << query << "\n  expected: " << expected << "\n  actual:   " << actual << std::endl;
        }
    }
    if (failures) FAIL(failures << " invalid quer" << (failures == 1 ? "y" : "ies") << " reported an unexpected error");
}

TEST_CASE(SparqlSyntaxErrorReportsLineAndColumn) {
    const std::string query =
        "PREFIX ex: <http://example.org/>\n"
        "SELECT ?s\n"
        "WHERE {\n"
        "  ?s nope:p ?o\n"
        "}";
    TissDB::Query::Parser parser;
    bool caught = false;
    try {
        parser.parse(query);
    } catch (const TissDB::Query::SparqlSyntaxError& e) {
        caught = true;
        ASSERT_EQ(4u, e.line());
        ASSERT_EQ(6u, e.column());
        ASSERT_EQ(std::string("undeclared prefix 'nope:'; declare it with PREFIX nope: <namespace-iri>"), e.detail());
        ASSERT_EQ(std::string("SPARQL syntax error at line 4, column 6: ") + e.detail(), std::string(e.what()));
    }
    ASSERT_TRUE(caught);

    // Callers that catch std::runtime_error (as before SparqlSyntaxError existed) still catch it.
    bool caught_as_runtime_error = false;
    try {
        parser.parse("PREFIX ex: <http://example.org/> SELECT ?s WHERE { ?s ex:p ?o UNION }");
    } catch (const std::runtime_error&) {
        caught_as_runtime_error = true;
    }
    ASSERT_TRUE(caught_as_runtime_error);

    // Long tokens are shortened in messages.
    std::string long_literal(60, 'x');
    std::string message;
    try {
        parser.parse("PREFIX ex: <http://example.org/> SELECT ?s WHERE { \"" + long_literal + "\" ex:p ?o }");
    } catch (const TissDB::Query::SparqlSyntaxError& e) {
        message = e.detail();
    }
    ASSERT_EQ(std::string("a literal cannot be the subject of a triple pattern near '\"") + std::string(39, 'x') + "...'", message);
}

// The headline behaviour: a SPARQL SELECT over loaded N-Triples returns the matching triples.
TEST_CASE(SparqlSelectReturnsMatchingTriples) {
    sparql_test::SparqlFixture fixture;

    auto all = fixture.run("PREFIX ex: <http://example.org/> SELECT ?s ?p ?o WHERE { ?s ?p ?o }");
    ASSERT_EQ(14u, all.size());

    auto knows = fixture.run("PREFIX ex: <http://example.org/> SELECT ?s ?o WHERE { ?s ex:knows ?o }");
    ASSERT_EQ(3u, knows.size());
    std::set<std::pair<std::string, std::string>> pairs;
    for (const auto& doc : knows) {
        ASSERT_EQ(2u, doc.elements.size());
        pairs.insert({sparql_test::field(doc, "s"), sparql_test::field(doc, "o")});
    }
    std::set<std::pair<std::string, std::string>> expected = {
        {"<http://example.org/alice>", "<http://example.org/bob>"},
        {"<http://example.org/alice>", "<http://example.org/carol>"},
        {"<http://example.org/bob>", "<http://example.org/carol>"},
    };
    ASSERT_TRUE(pairs == expected);

    // Every returned row corresponds to a triple that was loaded.
    auto loaded = TissDB::NTriples::parse(sparql_test::PEOPLE_NT);
    for (const auto& doc : all) {
        TissDB::Rdf::Triple t{TissDB::NTriples::parse_term(sparql_test::field(doc, "s")),
                              TissDB::NTriples::parse_term(sparql_test::field(doc, "p")),
                              TissDB::NTriples::parse_term(sparql_test::field(doc, "o"))};
        ASSERT_TRUE(std::find(loaded.begin(), loaded.end(), t) != loaded.end());
    }
}

TEST_CASE(SparqlMatchesTypedAndTaggedLiterals) {
    sparql_test::SparqlFixture fixture;

    auto by_age = fixture.run("PREFIX ex: <http://example.org/> SELECT ?who WHERE { ?who ex:age 34 }");
    ASSERT_EQ(1u, by_age.size());
    ASSERT_EQ(std::string("<http://example.org/alice>"), sparql_test::field(by_age[0], "who"));

    auto by_lang = fixture.run("PREFIX ex: <http://example.org/> SELECT ?who WHERE { ?who ex:name \"Bob\"@en }");
    ASSERT_EQ(1u, by_lang.size());
    ASSERT_EQ(std::string("<http://example.org/bob>"), sparql_test::field(by_lang[0], "who"));

    // "Bob" without the language tag is a different term.
    auto untagged = fixture.run("PREFIX ex: <http://example.org/> SELECT ?who WHERE { ?who ex:name \"Bob\" }");
    ASSERT_EQ(0u, untagged.size());

    auto values = fixture.run("PREFIX ex: <http://example.org/> SELECT ?age WHERE { ex:carol ex:age ?age }");
    ASSERT_EQ(1u, values.size());
    ASSERT_EQ(std::string("\"41\"^^<http://www.w3.org/2001/XMLSchema#integer>"), sparql_test::field(values[0], "age"));
}

TEST_CASE(SparqlJoinsPatternsOnSharedVariables) {
    sparql_test::SparqlFixture fixture;

    auto result = fixture.run(
        "PREFIX ex: <http://example.org/>\n"
        "SELECT ?name ?friendName WHERE {\n"
        "  ?person a ex:Person ; ex:name ?name ; ex:knows ?friend .\n"
        "  ?friend ex:name ?friendName .\n"
        "}");
    ASSERT_EQ(3u, result.size());
    std::set<std::pair<std::string, std::string>> pairs;
    for (const auto& doc : result) {
        pairs.insert({sparql_test::field(doc, "name"), sparql_test::field(doc, "friendName")});
    }
    std::set<std::pair<std::string, std::string>> expected = {
        {"\"Alice\"", "\"Bob\"@en"},
        {"\"Alice\"", "\"Carol\""},
        {"\"Bob\"@en", "\"Carol\""},
    };
    ASSERT_TRUE(pairs == expected);

    // Query blank nodes act as variables and can traverse stored blank nodes.
    auto employer = fixture.run("PREFIX ex: <http://example.org/> SELECT ?org WHERE { ex:carol ex:worksFor _:o . _:o ex:name ?org }");
    ASSERT_EQ(1u, employer.size());
    ASSERT_EQ(std::string("\"Acme Corp\""), sparql_test::field(employer[0], "org"));

    // A variable repeated within one pattern must bind to the same term.
    auto self_loops = fixture.run("PREFIX ex: <http://example.org/> SELECT ?x WHERE { ?x ex:knows ?x }");
    ASSERT_EQ(0u, self_loops.size());
}

TEST_CASE(SparqlFiltersSolutions) {
    sparql_test::SparqlFixture fixture;

    auto older = fixture.run("PREFIX ex: <http://example.org/> SELECT ?p WHERE { ?p ex:age ?age FILTER(?age > 30) }");
    ASSERT_TRUE((sparql_test::column(older, "p") ==
                 std::multiset<std::string>{"<http://example.org/alice>", "<http://example.org/carol>"}));

    auto range = fixture.run("PREFIX ex: <http://example.org/> SELECT ?p WHERE { ?p ex:age ?age . FILTER(?age >= 28 && ?age < 34.5 && ?p != ex:bob) }");
    ASSERT_TRUE((sparql_test::column(range, "p") == std::multiset<std::string>{"<http://example.org/alice>"}));

    auto regex = fixture.run("PREFIX ex: <http://example.org/> SELECT ?n WHERE { ?p ex:name ?n FILTER regex(?n, \"^c\", \"i\") }");
    ASSERT_TRUE((sparql_test::column(regex, "n") == std::multiset<std::string>{"\"Carol\""}));

    auto lang = fixture.run("PREFIX ex: <http://example.org/> SELECT ?n WHERE { ?p ex:name ?n FILTER(lang(?n) = \"en\") }");
    ASSERT_TRUE((sparql_test::column(lang, "n") == std::multiset<std::string>{"\"Bob\"@en"}));

    auto blanks = fixture.run("PREFIX ex: <http://example.org/> SELECT ?s WHERE { ?s ex:name ?n FILTER(isBlank(?s)) }");
    ASSERT_TRUE((sparql_test::column(blanks, "s") == std::multiset<std::string>{"_:acme"}));

    auto contains = fixture.run("PREFIX ex: <http://example.org/> SELECT ?n WHERE { ?p ex:name ?n FILTER(CONTAINS(STR(?n), \"o\")) }");
    ASSERT_TRUE((sparql_test::column(contains, "n") == std::multiset<std::string>{"\"Bob\"@en", "\"Carol\"", "\"Acme Corp\""}));

    // Comparing a number with an IRI is a type error, which removes the solution.
    auto type_error = fixture.run("PREFIX ex: <http://example.org/> SELECT ?p WHERE { ?p ex:age ?age FILTER(?age < ex:bob) }");
    ASSERT_EQ(0u, type_error.size());
}

TEST_CASE(SparqlSolutionModifiers) {
    sparql_test::SparqlFixture fixture;

    auto ordered = fixture.run("PREFIX ex: <http://example.org/> SELECT ?p ?age WHERE { ?p ex:age ?age } ORDER BY DESC(?age)");
    ASSERT_EQ(3u, ordered.size());
    ASSERT_EQ(std::string("<http://example.org/carol>"), sparql_test::field(ordered[0], "p"));
    ASSERT_EQ(std::string("<http://example.org/alice>"), sparql_test::field(ordered[1], "p"));
    ASSERT_EQ(std::string("<http://example.org/bob>"), sparql_test::field(ordered[2], "p"));

    auto paged = fixture.run("PREFIX ex: <http://example.org/> SELECT ?p WHERE { ?p ex:age ?age } ORDER BY ?age LIMIT 1 OFFSET 1");
    ASSERT_EQ(1u, paged.size());
    ASSERT_EQ(std::string("<http://example.org/alice>"), sparql_test::field(paged[0], "p"));

    auto past_end = fixture.run("PREFIX ex: <http://example.org/> SELECT ?p WHERE { ?p ex:age ?age } OFFSET 10");
    ASSERT_EQ(0u, past_end.size());

    auto subjects = fixture.run("PREFIX ex: <http://example.org/> SELECT ?s WHERE { ?s ex:knows ?o }");
    ASSERT_EQ(3u, subjects.size());
    auto distinct = fixture.run("PREFIX ex: <http://example.org/> SELECT DISTINCT ?s WHERE { ?s ex:knows ?o }");
    ASSERT_TRUE((sparql_test::column(distinct, "s") ==
                 std::multiset<std::string>{"<http://example.org/alice>", "<http://example.org/bob>"}));
}

TEST_CASE(SparqlSelectStarProjectsPatternVariables) {
    sparql_test::SparqlFixture fixture;
    auto stmt = sparql_test::parse_sparql(
        "PREFIX ex: <http://example.org/> SELECT * WHERE { ?person ex:worksFor _:org . _:org ex:name ?orgName }");
    auto solutions = TissDB::Query::evaluate_sparql_select(*fixture.storage, stmt);
    ASSERT_EQ(1u, solutions.size());
    ASSERT_EQ(2u, solutions[0].size()); // _:org is not projected
    ASSERT_TRUE(solutions[0].at("person") == TissDB::Rdf::Term::iri("http://example.org/carol"));
    ASSERT_TRUE(solutions[0].at("orgName") == TissDB::Rdf::Term::literal("Acme Corp"));
}

TEST_CASE(SparqlOverEmptyDatabaseReturnsNothing) {
    sparql_test::SparqlFixture fixture("");
    auto result = fixture.run("PREFIX ex: <http://example.org/> SELECT ?s WHERE { ?s ?p ?o }");
    ASSERT_EQ(0u, result.size());

    // TissQL queries keep working next to SPARQL support.
    fixture.storage->create_collection("items", TissDB::Schema());
    TissDB::Document doc;
    doc.id = "1";
    doc.elements.push_back({"name", std::string("widget")});
    fixture.storage->put("items", "1", doc);
    auto tissql = fixture.run("SELECT * FROM items WHERE name = 'widget'");
    ASSERT_EQ(1u, tissql.size());
}

// --- OPTIONAL ---

namespace sparql_test {

// PEOPLE_NT plus an email address for alice only.
const std::string PEOPLE_WITH_EMAIL_NT =
    PEOPLE_NT + "<http://example.org/alice> <http://example.org/email> \"alice@example.org\" .\n";

bool has_field(const TissDB::Document& doc, const std::string& key) {
    return std::any_of(doc.elements.begin(), doc.elements.end(), [&](const TissDB::Element& e) { return e.key == key; });
}

// Rows as "name|email" strings, with "" for unbound values.
std::multiset<std::string> pairs_of(const TissDB::Query::QueryResult& result, const std::string& a, const std::string& b) {
    std::multiset<std::string> out;
    for (const auto& doc : result) out.insert(field(doc, a) + "|" + field(doc, b));
    return out;
}

} // namespace sparql_test

TEST_CASE(SparqlParserBuildsOptionalGroups) {
    using Kind = TissDB::Query::SparqlGroupElement::Kind;
    auto stmt = sparql_test::parse_sparql(
        "PREFIX ex: <http://example.org/>\n"
        "SELECT * WHERE {\n"
        "  ?p a ex:Person .\n"
        "  optional { ?p ex:worksFor ?org OPTIONAL { ?org ex:name ?orgName } FILTER(isBlank(?org)) }\n"
        "  OPTIONAL { ?p ex:email ?email } .\n"
        "  ?p ex:name ?name ; ex:age ?age\n"
        "  FILTER(?age > 20)\n"
        "}");

    const auto& top = stmt.where;
    ASSERT_EQ(4u, top.elements.size());
    ASSERT_TRUE(top.elements[0].kind == Kind::TRIPLES);
    ASSERT_TRUE(top.elements[1].kind == Kind::OPTIONAL);
    ASSERT_TRUE(top.elements[2].kind == Kind::OPTIONAL);
    ASSERT_TRUE(top.elements[3].kind == Kind::TRIPLES);
    ASSERT_EQ(2u, top.elements[3].triples.size());
    ASSERT_EQ(1u, top.filters.size());

    // The FILTER inside the first OPTIONAL belongs to that group, not the outer one.
    const auto& works_for = *top.elements[1].group;
    ASSERT_EQ(2u, works_for.elements.size());
    ASSERT_EQ(std::string("org"), sparql_test::variable(sparql_test::triples(works_for)[0].object));
    ASSERT_TRUE(works_for.elements[1].kind == Kind::OPTIONAL);
    ASSERT_EQ(std::string("orgName"), sparql_test::variable(sparql_test::triples(*works_for.elements[1].group)[0].object));
    ASSERT_EQ(1u, works_for.filters.size());
    ASSERT_EQ(std::string("ISBLANK"), works_for.filters[0]->op);
}

TEST_CASE(SparqlOptionalKeepsSolutionsWithoutAMatch) {
    sparql_test::SparqlFixture fixture(sparql_test::PEOPLE_WITH_EMAIL_NT);

    auto result = fixture.run(
        "PREFIX ex: <http://example.org/> "
        "SELECT ?name ?email WHERE { ?p a ex:Person ; ex:name ?name OPTIONAL { ?p ex:email ?email } }");
    ASSERT_EQ(3u, result.size());
    ASSERT_TRUE((sparql_test::pairs_of(result, "name", "email") ==
                 std::multiset<std::string>{"\"Alice\"|\"alice@example.org\"", "\"Bob\"@en|", "\"Carol\"|"}));
    for (const auto& doc : result) {
        // Unbound variables are left out of the row rather than given a placeholder value.
        ASSERT_EQ(sparql_test::field(doc, "name") == "\"Alice\"", sparql_test::has_field(doc, "email"));
    }

    // OPTIONAL plus !BOUND finds solutions with no match (negation as failure).
    auto no_email = fixture.run(
        "PREFIX ex: <http://example.org/> "
        "SELECT ?name WHERE { ?p a ex:Person ; ex:name ?name OPTIONAL { ?p ex:email ?e } FILTER(!BOUND(?e)) }");
    ASSERT_TRUE((sparql_test::column(no_email, "name") == std::multiset<std::string>{"\"Bob\"@en", "\"Carol\""}));

    // An OPTIONAL that matches several times yields one row per match.
    auto friends = fixture.run(
        "PREFIX ex: <http://example.org/> "
        "SELECT ?name ?friend WHERE { ?p ex:name ?name OPTIONAL { ?p ex:knows ?friend } }");
    ASSERT_EQ(5u, friends.size()); // alice x2, bob x1, carol and Acme unmatched
}

TEST_CASE(SparqlFilterInsideOptionalOnlyRestrictsTheOptionalPart) {
    sparql_test::SparqlFixture fixture;

    // Inside the OPTIONAL, the filter decides whether ?age is bound; every person is kept.
    auto inside = fixture.run(
        "PREFIX ex: <http://example.org/> "
        "SELECT ?name ?age WHERE { ?p a ex:Person ; ex:name ?name OPTIONAL { ?p ex:age ?age FILTER(?age > 30) } }");
    ASSERT_TRUE((sparql_test::pairs_of(inside, "name", "age") == std::multiset<std::string>{
        "\"Alice\"|\"34\"^^<http://www.w3.org/2001/XMLSchema#integer>",
        "\"Bob\"@en|",
        "\"Carol\"|\"41\"^^<http://www.w3.org/2001/XMLSchema#integer>"}));

    // The filter can also refer to variables bound outside the OPTIONAL.
    auto outer_vars = fixture.run(
        "PREFIX ex: <http://example.org/> "
        "SELECT ?name ?age WHERE { ?p a ex:Person ; ex:name ?name OPTIONAL { ?q ex:age ?age FILTER(?q = ?p && ?age < 30) } }");
    ASSERT_TRUE((sparql_test::pairs_of(outer_vars, "name", "age") == std::multiset<std::string>{
        "\"Alice\"|", "\"Bob\"@en|\"28\"^^<http://www.w3.org/2001/XMLSchema#integer>", "\"Carol\"|"}));

    // Outside the OPTIONAL, the same filter removes whole solutions.
    auto outside = fixture.run(
        "PREFIX ex: <http://example.org/> "
        "SELECT ?name WHERE { ?p a ex:Person ; ex:name ?name OPTIONAL { ?p ex:age ?age } FILTER(?age > 30) }");
    ASSERT_TRUE((sparql_test::column(outside, "name") == std::multiset<std::string>{"\"Alice\"", "\"Carol\""}));
}

TEST_CASE(SparqlNestedAndMultipleOptionals) {
    sparql_test::SparqlFixture fixture(sparql_test::PEOPLE_WITH_EMAIL_NT);

    auto result = fixture.run(
        "PREFIX ex: <http://example.org/> "
        "SELECT ?name ?email ?orgName WHERE {"
        "  ?p a ex:Person ; ex:name ?name ."
        "  OPTIONAL { ?p ex:email ?email }"
        "  OPTIONAL { ?p ex:worksFor ?org OPTIONAL { ?org ex:name ?orgName } }"
        "} ORDER BY ?name");
    ASSERT_EQ(3u, result.size());
    ASSERT_EQ(std::string("\"Alice\""), sparql_test::field(result[0], "name"));
    ASSERT_EQ(std::string("\"alice@example.org\""), sparql_test::field(result[0], "email"));
    ASSERT_FALSE(sparql_test::has_field(result[0], "orgName"));
    ASSERT_EQ(std::string("\"Bob\"@en"), sparql_test::field(result[1], "name"));
    ASSERT_EQ(1u, result[1].elements.size());
    ASSERT_EQ(std::string("\"Carol\""), sparql_test::field(result[2], "name"));
    ASSERT_FALSE(sparql_test::has_field(result[2], "email"));
    ASSERT_EQ(std::string("\"Acme Corp\""), sparql_test::field(result[2], "orgName"));
}

TEST_CASE(SparqlPatternsAfterOptionalJoinInOrder) {
    sparql_test::SparqlFixture fixture;

    // { A OPTIONAL { B } C } is Join(LeftJoin(A, B), C): when B does not match, ?friend
    // stays unbound and C may bind it to anything with a name.
    auto result = fixture.run(
        "PREFIX ex: <http://example.org/> "
        "SELECT ?p ?friend WHERE { ?p a ex:Person OPTIONAL { ?p ex:knows ?friend } ?friend ex:name ?fname }");
    std::multiset<std::string> carol_friends;
    for (const auto& doc : result) {
        if (sparql_test::field(doc, "p") == "<http://example.org/carol>") carol_friends.insert(sparql_test::field(doc, "friend"));
    }
    ASSERT_EQ(7u, result.size()); // alice->bob, alice->carol, bob->carol, carol->each of 4 named nodes
    ASSERT_TRUE((carol_friends == std::multiset<std::string>{
        "<http://example.org/alice>", "<http://example.org/bob>", "<http://example.org/carol>", "_:acme"}));
}

TEST_CASE(SparqlOptionalWithSelectStarAndLimit) {
    sparql_test::SparqlFixture fixture(sparql_test::PEOPLE_WITH_EMAIL_NT);

    auto stmt = sparql_test::parse_sparql(
        "PREFIX ex: <http://example.org/> "
        "SELECT * WHERE { ?p ex:name ?name OPTIONAL { ?p ex:email ?email } } ORDER BY ?name LIMIT 2");
    auto solutions = TissDB::Query::evaluate_sparql_select(*fixture.storage, stmt);
    ASSERT_EQ(2u, solutions.size());
    ASSERT_TRUE(solutions[0].at("name") == TissDB::Rdf::Term::literal("Acme Corp"));
    ASSERT_EQ(0u, solutions[0].count("email"));
    ASSERT_TRUE(solutions[1].at("name") == TissDB::Rdf::Term::literal("Alice"));
    ASSERT_TRUE(solutions[1].at("email") == TissDB::Rdf::Term::literal("alice@example.org"));

    // SELECT * projects variables that appear only inside the OPTIONAL.
    auto rows = fixture.run(
        "PREFIX ex: <http://example.org/> "
        "SELECT * WHERE { ex:alice ex:name ?name OPTIONAL { ex:alice ex:email ?email } }");
    ASSERT_EQ(1u, rows.size());
    ASSERT_EQ(std::string("\"alice@example.org\""), sparql_test::field(rows[0], "email"));
}
