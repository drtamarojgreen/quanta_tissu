#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "../common/document.h"
#include "../common/rdf.h"

namespace TissDB {
namespace Storage {

class LSMTree;
class DatabaseManager;

// Stores RDF triples as documents in an LSMTree collection.
//
// Each triple becomes one document whose id is the triple's canonical N-Triples
// line (so a graph behaves as a set) and whose "subject", "predicate" and
// "object" elements hold the N-Triples encoding of each term.
class TripleStore {
public:
    static constexpr const char* DEFAULT_COLLECTION = "rdf_triples";

    explicit TripleStore(LSMTree& db, const std::string& collection = DEFAULT_COLLECTION);

    // Adds a triple. Returns false if it was already present.
    bool add(const Rdf::Triple& triple);

    // Parses an N-Triples document and adds every triple in it. The whole document
    // is parsed before anything is written, so malformed input stores nothing.
    // Returns the number of triples that were not already present.
    size_t load_ntriples(const std::string& document);

    // Reads an N-Triples file and loads it like load_ntriples. Throws
    // std::runtime_error if the file cannot be read.
    size_t load_ntriples_file(const std::string& path);

    // Adds already-parsed triples. Returns the number that were not already present.
    size_t add_all(const std::vector<Rdf::Triple>& triples);

    // Removes a triple. Returns false if it was not present.
    bool remove(const Rdf::Triple& triple);

    // Returns all triples matching the pattern; nullopt positions match any term.
    std::vector<Rdf::Triple> match(const std::optional<Rdf::Term>& subject,
                                   const std::optional<Rdf::Term>& predicate,
                                   const std::optional<Rdf::Term>& object) const;

    std::vector<Rdf::Triple> all() const;
    size_t size() const;

    const std::string& collection() const { return collection_; }

    static Document to_document(const Rdf::Triple& triple);
    // Returns nullopt for documents that do not describe a triple.
    static std::optional<Rdf::Triple> from_document(const Document& doc);

private:
    void ensure_collection();
    bool contains(const std::string& key) const;

    LSMTree& db_;
    std::string collection_;
};

// Imports an N-Triples file into the default RDF collection of a database,
// creating the database if it does not exist. The file is fully parsed first, so
// a missing or malformed file changes nothing. Returns the number of new triples.
size_t import_ntriples_file(DatabaseManager& manager, const std::string& db_name, const std::string& path);

} // namespace Storage
} // namespace TissDB
