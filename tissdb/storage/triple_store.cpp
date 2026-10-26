#include "triple_store.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "database_manager.h"
#include "lsm_tree.h"
#include "../common/ntriples.h"

namespace TissDB {
namespace Storage {

namespace {

const std::string* get_string_element(const Document& doc, const std::string& key) {
    for (const auto& element : doc.elements) {
        if (element.key == key) return std::get_if<std::string>(&element.value);
    }
    return nullptr;
}

std::string read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open N-Triples file: " + path);
    std::stringstream contents;
    contents << file.rdbuf();
    if (file.bad()) throw std::runtime_error("Error reading N-Triples file: " + path);
    return contents.str();
}

} // anonymous namespace

TripleStore::TripleStore(LSMTree& db, const std::string& collection) : db_(db), collection_(collection) {}

void TripleStore::ensure_collection() {
    auto names = db_.list_collections();
    if (std::find(names.begin(), names.end(), collection_) == names.end()) {
        db_.create_collection(collection_, TissDB::Schema());
    }
}

bool TripleStore::contains(const std::string& key) const {
    auto doc = db_.get(collection_, key);
    return doc.has_value() && *doc != nullptr;
}

bool TripleStore::add(const Rdf::Triple& triple) {
    ensure_collection();
    Document doc = to_document(triple);
    if (contains(doc.id)) return false;
    db_.put(collection_, doc.id, doc);
    return true;
}

size_t TripleStore::load_ntriples(const std::string& document) {
    return add_all(NTriples::parse(document));
}

size_t TripleStore::load_ntriples_file(const std::string& path) {
    return load_ntriples(read_file(path));
}

size_t TripleStore::add_all(const std::vector<Rdf::Triple>& triples) {
    size_t added = 0;
    for (const auto& triple : triples) {
        if (add(triple)) added++;
    }
    return added;
}

bool TripleStore::remove(const Rdf::Triple& triple) {
    const std::string key = triple.to_ntriples();
    if (!contains(key)) return false;
    return db_.del(collection_, key);
}

std::vector<Rdf::Triple> TripleStore::match(const std::optional<Rdf::Term>& subject,
                                            const std::optional<Rdf::Term>& predicate,
                                            const std::optional<Rdf::Term>& object) const {
    std::vector<Rdf::Triple> result;
    for (const auto& doc : db_.scan(collection_)) {
        auto triple = from_document(doc);
        if (!triple) continue;
        if (subject && triple->subject != *subject) continue;
        if (predicate && triple->predicate != *predicate) continue;
        if (object && triple->object != *object) continue;
        result.push_back(std::move(*triple));
    }
    return result;
}

std::vector<Rdf::Triple> TripleStore::all() const {
    return match(std::nullopt, std::nullopt, std::nullopt);
}

size_t TripleStore::size() const {
    return all().size();
}

Document TripleStore::to_document(const Rdf::Triple& triple) {
    Document doc;
    doc.id = triple.to_ntriples();
    doc.elements.push_back({"subject", triple.subject.to_ntriples()});
    doc.elements.push_back({"predicate", triple.predicate.to_ntriples()});
    doc.elements.push_back({"object", triple.object.to_ntriples()});
    return doc;
}

std::optional<Rdf::Triple> TripleStore::from_document(const Document& doc) {
    const std::string* s = get_string_element(doc, "subject");
    const std::string* p = get_string_element(doc, "predicate");
    const std::string* o = get_string_element(doc, "object");
    if (!s || !p || !o) return std::nullopt;
    try {
        return Rdf::Triple{NTriples::parse_term(*s), NTriples::parse_term(*p), NTriples::parse_term(*o)};
    } catch (const NTriples::ParseError&) {
        return std::nullopt;
    }
}

size_t import_ntriples_file(DatabaseManager& manager, const std::string& db_name, const std::string& path) {
    // Read and parse before touching the database so a bad file creates nothing.
    std::vector<Rdf::Triple> triples = NTriples::parse(read_file(path));
    if (!manager.database_exists(db_name)) manager.create_database(db_name);
    return TripleStore(manager.get_database(db_name)).add_all(triples);
}

} // namespace Storage
} // namespace TissDB
