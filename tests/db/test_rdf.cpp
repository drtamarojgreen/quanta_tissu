#include "test_framework.h"
#include "../../tissdb/common/rdf.h"
#include "../../tissdb/common/ntriples.h"
#include "../../tissdb/storage/lsm_tree.h"
#include "../../tissdb/storage/triple_store.h"
#include "../../tissdb/storage/database_manager.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace rdf_test {

const std::string EX = "http://example.org/";

struct TripleStoreFixture {
    const std::string test_dir = "./test_rdf_data";
    std::unique_ptr<TissDB::Storage::LSMTree> storage;

    TripleStoreFixture() {
        std::filesystem::remove_all(test_dir);
        storage = std::make_unique<TissDB::Storage::LSMTree>(test_dir);
    }

    ~TripleStoreFixture() {
        if (storage) storage->shutdown();
        std::filesystem::remove_all(test_dir);
    }
};

void write_file(const std::string& path, const std::string& contents) {
    std::ofstream out(path, std::ios::binary);
    out << contents;
}

} // namespace rdf_test

TEST_CASE(RdfTermNTriplesEncoding) {
    using TissDB::Rdf::Term;
    namespace XSD = TissDB::Rdf::XSD;

    ASSERT_EQ(std::string("<http://example.org/a>"), Term::iri("http://example.org/a").to_ntriples());
    ASSERT_EQ(std::string("_:b0"), Term::blank("b0").to_ntriples());
    ASSERT_EQ(std::string("\"plain\""), Term::literal("plain").to_ntriples());
    ASSERT_EQ(std::string("\"42\"^^<http://www.w3.org/2001/XMLSchema#integer>"), Term::literal("42", XSD::INTEGER).to_ntriples());
    ASSERT_EQ(std::string("\"bonjour\"@fr-be"), Term::lang_literal("bonjour", "fr-BE").to_ntriples());
    ASSERT_EQ(std::string("\"say \\\"hi\\\"\\n\\\\\""), Term::literal("say \"hi\"\n\\").to_ntriples());
    ASSERT_EQ(std::string("<http://ex.org/a\\u0020b>"), Term::iri("http://ex.org/a b").to_ntriples());
}

TEST_CASE(RdfTermDatatypesAndEquality) {
    using TissDB::Rdf::Term;
    namespace XSD = TissDB::Rdf::XSD;

    // A literal without a datatype is an xsd:string; language-tagged literals are rdf:langString.
    ASSERT_TRUE(Term::literal("x") == Term::literal("x", XSD::STRING));
    ASSERT_EQ(std::string(TissDB::Rdf::Vocab::RDF_LANG_STRING), Term::lang_literal("x", "en").datatype);
    ASSERT_TRUE(Term::lang_literal("x", "EN") == Term::lang_literal("x", "en"));
    ASSERT_TRUE(Term::literal("x") != Term::lang_literal("x", "en"));
    ASSERT_TRUE(Term::iri("x") != Term::literal("x"));
    ASSERT_TRUE(Term::iri("x") != Term::blank("x"));

    ASSERT_TRUE(Term::literal("42", XSD::INTEGER).is_numeric());
    ASSERT_TRUE(Term::literal("2.5", XSD::DECIMAL).numeric_value() == 2.5);
    ASSERT_TRUE(Term::literal("1e3", XSD::DOUBLE).numeric_value() == 1000.0);
    ASSERT_FALSE(Term::literal("42").is_numeric());
    ASSERT_FALSE(Term::literal("abc", XSD::INTEGER).numeric_value().has_value());
}

TEST_CASE(NTriplesParsesAllTermKinds) {
    using TissDB::Rdf::Term;
    namespace XSD = TissDB::Rdf::XSD;

    const std::string doc =
        "# a comment line\n"
        "\n"
        "<http://example.org/alice> <http://example.org/name> \"Alice\" .\n"
        "<http://example.org/alice> <http://example.org/age> \"34\"^^<http://www.w3.org/2001/XMLSchema#integer> . # trailing comment\n"
        "_:b1 <http://example.org/label> \"caf\\u00E9 \\U0001F600\\t\\\"q\\\"\"@EN .\n"
        "<http://example.org/alice>\t<http://example.org/knows>\t_:b1.\r\n";

    auto triples = TissDB::NTriples::parse(doc);
    ASSERT_EQ(4u, triples.size());

    ASSERT_TRUE(triples[0].subject == Term::iri(rdf_test::EX + "alice"));
    ASSERT_TRUE(triples[0].predicate == Term::iri(rdf_test::EX + "name"));
    ASSERT_TRUE(triples[0].object == Term::literal("Alice"));

    ASSERT_TRUE(triples[1].object == Term::literal("34", XSD::INTEGER));

    ASSERT_TRUE(triples[2].subject == Term::blank("b1"));
    ASSERT_TRUE(triples[2].object == Term::lang_literal("caf\xC3\xA9 \xF0\x9F\x98\x80\t\"q\"", "en"));

    ASSERT_TRUE(triples[3].object == Term::blank("b1"));
}

TEST_CASE(NTriplesSerializeRoundTrip) {
    const std::string doc =
        "<http://example.org/s> <http://example.org/p> \"line1\\nline2 \\\\ \\\"x\\\"\" .\n"
        "<http://example.org/s> <http://example.org/p> \"3.14\"^^<http://www.w3.org/2001/XMLSchema#decimal> .\n"
        "_:n <http://example.org/p> \"hola\"@es .\n"
        "<http://example.org/s> <http://example.org/p> <http://example.org/o> .\n";

    auto triples = TissDB::NTriples::parse(doc);
    std::string serialized = TissDB::NTriples::serialize(triples);
    ASSERT_EQ(doc, serialized);
    ASSERT_TRUE(TissDB::NTriples::parse(serialized) == triples);
}

TEST_CASE(NTriplesRejectsMalformedInput) {
    using TissDB::NTriples::ParseError;
    const std::vector<std::string> bad_lines = {
        "\"literal\" <http://example.org/p> <http://example.org/o> .",  // literal subject
        "<http://example.org/s> _:p <http://example.org/o> .",          // blank node predicate
        "<http://example.org/s> <http://example.org/p> <http://example.org/o>", // missing '.'
        "<http://example.org/s> <http://example.org/p> <http://example.org/o .", // unterminated IRI
        "<http://example.org/s> <http://example.org/p> \"bad \\q escape\" .",
        "<http://example.org/s> <http://example.org/p> \"x\"@ .",
        "<http://example.org/s> <http://example.org/p> <http://example.org/o> . extra",
        "<http://ex ample.org/s> <http://example.org/p> <http://example.org/o> .",
    };
    for (const auto& line : bad_lines) {
        ASSERT_THROW(TissDB::NTriples::parse_line(line, 1), ParseError);
    }

    // Errors report the 1-based line of the offending triple.
    const std::string doc =
        "<http://example.org/s> <http://example.org/p> <http://example.org/o> .\n"
        "# comment\n"
        "<http://example.org/s> <http://example.org/p> .\n";
    size_t error_line = 0;
    try {
        TissDB::NTriples::parse(doc);
    } catch (const ParseError& e) {
        error_line = e.line();
    }
    ASSERT_EQ(3u, error_line);
}

TEST_CASE(NTriplesParseSingleTerm) {
    using TissDB::Rdf::Term;
    ASSERT_TRUE(TissDB::NTriples::parse_term("<http://example.org/a>") == Term::iri("http://example.org/a"));
    ASSERT_TRUE(TissDB::NTriples::parse_term("_:x") == Term::blank("x"));
    ASSERT_TRUE(TissDB::NTriples::parse_term("\"7\"^^<http://www.w3.org/2001/XMLSchema#integer>") ==
                Term::literal("7", TissDB::Rdf::XSD::INTEGER));
    ASSERT_THROW(TissDB::NTriples::parse_term("<http://example.org/a> trailing"), TissDB::NTriples::ParseError);
}

TEST_CASE(TripleStoreLoadsAndMatchesTriples) {
    using TissDB::Rdf::Term;
    rdf_test::TripleStoreFixture fixture;
    TissDB::Storage::TripleStore store(*fixture.storage);

    const std::string doc =
        "<http://example.org/alice> <http://example.org/knows> <http://example.org/bob> .\n"
        "<http://example.org/alice> <http://example.org/knows> <http://example.org/carol> .\n"
        "<http://example.org/bob> <http://example.org/knows> <http://example.org/carol> .\n"
        "<http://example.org/alice> <http://example.org/name> \"Alice\" .\n"
        "<http://example.org/alice> <http://example.org/knows> <http://example.org/bob> .\n"; // duplicate

    ASSERT_EQ(4u, store.load_ntriples(doc));
    ASSERT_EQ(4u, store.size());
    ASSERT_EQ(0u, store.load_ntriples(doc)); // a graph is a set

    const Term alice = Term::iri(rdf_test::EX + "alice");
    const Term knows = Term::iri(rdf_test::EX + "knows");
    ASSERT_EQ(3u, store.match(std::nullopt, knows, std::nullopt).size());
    ASSERT_EQ(2u, store.match(alice, knows, std::nullopt).size());
    ASSERT_EQ(2u, store.match(std::nullopt, std::nullopt, Term::iri(rdf_test::EX + "carol")).size());
    auto names = store.match(alice, Term::iri(rdf_test::EX + "name"), std::nullopt);
    ASSERT_EQ(1u, names.size());
    ASSERT_TRUE(names[0].object == Term::literal("Alice"));

    TissDB::Rdf::Triple alice_knows_bob{alice, knows, Term::iri(rdf_test::EX + "bob")};
    ASSERT_TRUE(store.remove(alice_knows_bob));
    ASSERT_FALSE(store.remove(alice_knows_bob));
    ASSERT_EQ(3u, store.size());

    // Triples are ordinary documents in the RDF collection.
    auto docs = fixture.storage->scan(TissDB::Storage::TripleStore::DEFAULT_COLLECTION);
    ASSERT_EQ(3u, docs.size());
    ASSERT_TRUE(TissDB::Storage::TripleStore::from_document(docs[0]).has_value());
}

TEST_CASE(TripleStoreRejectsMalformedDocumentAtomically) {
    rdf_test::TripleStoreFixture fixture;
    TissDB::Storage::TripleStore store(*fixture.storage);
    const std::string doc =
        "<http://example.org/a> <http://example.org/p> <http://example.org/b> .\n"
        "<http://example.org/a> <http://example.org/p> not-a-term .\n";
    ASSERT_THROW(store.load_ntriples(doc), TissDB::NTriples::ParseError);
    ASSERT_EQ(0u, store.size());
}

TEST_CASE(TripleStoreSurvivesReopen) {
    rdf_test::TripleStoreFixture fixture;
    {
        TissDB::Storage::TripleStore store(*fixture.storage);
        store.load_ntriples(
            "<http://example.org/a> <http://example.org/p> \"v\"@en .\n"
            "<http://example.org/a> <http://example.org/q> \"5\"^^<http://www.w3.org/2001/XMLSchema#integer> .\n");
    }
    fixture.storage->shutdown();
    fixture.storage = std::make_unique<TissDB::Storage::LSMTree>(fixture.test_dir);

    TissDB::Storage::TripleStore reopened(*fixture.storage);
    auto triples = reopened.all();
    std::sort(triples.begin(), triples.end());
    ASSERT_EQ(2u, triples.size());
    ASSERT_TRUE(triples[0].object == TissDB::Rdf::Term::lang_literal("v", "en"));
    ASSERT_TRUE(triples[1].object == TissDB::Rdf::Term::literal("5", TissDB::Rdf::XSD::INTEGER));
}

TEST_CASE(TripleStoreLoadsNTriplesFile) {
    rdf_test::TripleStoreFixture fixture;
    const std::string path = fixture.test_dir + "/people.nt";
    rdf_test::write_file(path,
        "# exported graph\r\n"
        "<http://example.org/a> <http://example.org/name> \"A\" .\r\n"
        "<http://example.org/a> <http://example.org/knows> <http://example.org/b> .\r\n"
        "<http://example.org/a> <http://example.org/name> \"A\" .\n"); // duplicate, no trailing CR

    TissDB::Storage::TripleStore store(*fixture.storage);
    ASSERT_EQ(2u, store.load_ntriples_file(path));
    ASSERT_EQ(2u, store.size());
    ASSERT_EQ(0u, store.load_ntriples_file(path));

    std::string error;
    try {
        store.load_ntriples_file(fixture.test_dir + "/missing.nt");
    } catch (const std::runtime_error& e) {
        error = e.what();
    }
    ASSERT_TRUE(error.find("Cannot open N-Triples file") != std::string::npos);
}

TEST_CASE(ImportNTriplesFileCreatesDatabase) {
    const std::string dir = "./test_rdf_import_data";
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    const std::string good = dir + "/good.nt";
    const std::string bad = dir + "/bad.nt";
    rdf_test::write_file(good,
        "<http://example.org/a> <http://example.org/p> \"1\"^^<http://www.w3.org/2001/XMLSchema#integer> .\n"
        "<http://example.org/b> <http://example.org/p> _:x .\n");
    rdf_test::write_file(bad,
        "<http://example.org/c> <http://example.org/p> <http://example.org/d> .\n"
        "<http://example.org/c> <http://example.org/p> oops .\n");

    {
        TissDB::Storage::DatabaseManager manager(dir + "/data");
        ASSERT_FALSE(manager.database_exists("graph_db"));
        ASSERT_EQ(2u, TissDB::Storage::import_ntriples_file(manager, "graph_db", good));
        ASSERT_TRUE(manager.database_exists("graph_db"));
        ASSERT_EQ(2u, TissDB::Storage::TripleStore(manager.get_database("graph_db")).size());

        // A malformed file reports its line and changes nothing, including not creating the database.
        size_t error_line = 0;
        try {
            TissDB::Storage::import_ntriples_file(manager, "other_db", bad);
        } catch (const TissDB::NTriples::ParseError& e) {
            error_line = e.line();
        }
        ASSERT_EQ(2u, error_line);
        ASSERT_FALSE(manager.database_exists("other_db"));

        bool missing_rejected = false;
        try {
            TissDB::Storage::import_ntriples_file(manager, "other_db", dir + "/missing.nt");
        } catch (const std::runtime_error&) {
            missing_rejected = true;
        }
        ASSERT_TRUE(missing_rejected);
        ASSERT_FALSE(manager.database_exists("other_db"));
        manager.shutdown();
    }

    // Imported triples persist and are queryable after reopening.
    {
        TissDB::Storage::DatabaseManager manager(dir + "/data");
        ASSERT_TRUE(manager.database_exists("graph_db"));
        ASSERT_EQ(2u, TissDB::Storage::TripleStore(manager.get_database("graph_db")).size());
        manager.shutdown();
    }
    std::filesystem::remove_all(dir);
}
