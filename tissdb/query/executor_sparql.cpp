#include "executor_sparql.h"
#include "../storage/triple_store.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iterator>
#include <regex>
#include <set>

namespace TissDB {
namespace Query {

namespace {

using Rdf::Term;
using OptionalTerm = std::optional<Term>;

Term make_bool(bool value) {
    return Term::literal(value ? "true" : "false", Rdf::XSD::BOOLEAN);
}

bool is_string_literal(const Term& t) {
    return t.is_literal() && (t.datatype == Rdf::XSD::STRING || t.datatype == Rdf::Vocab::RDF_LANG_STRING);
}

std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

// SPARQL effective boolean value; nullopt is a type error.
std::optional<bool> effective_boolean_value(const OptionalTerm& t) {
    if (!t || !t->is_literal()) return std::nullopt;
    if (t->datatype == Rdf::XSD::BOOLEAN) return t->value == "true" || t->value == "1";
    if (t->is_numeric()) {
        auto v = t->numeric_value();
        return v && *v != 0 && !std::isnan(*v);
    }
    if (t->datatype == Rdf::XSD::STRING) return !t->value.empty();
    return std::nullopt;
}

// Compares two terms by value. Returns <0, 0 or >0, or nullopt when the terms are
// not comparable (e.g. a number and an IRI).
std::optional<int> compare_values(const Term& a, const Term& b) {
    if (a.is_numeric() && b.is_numeric()) {
        auto x = a.numeric_value();
        auto y = b.numeric_value();
        if (!x || !y) return std::nullopt;
        return *x < *y ? -1 : (*x > *y ? 1 : 0);
    }
    if (a.is_literal() && b.is_literal() && a.datatype == b.datatype && a.language == b.language) {
        if (a.datatype == Rdf::XSD::BOOLEAN) {
            bool x = *effective_boolean_value(a);
            bool y = *effective_boolean_value(b);
            return static_cast<int>(x) - static_cast<int>(y);
        }
        if (a.datatype == Rdf::XSD::STRING || a.datatype == Rdf::Vocab::RDF_LANG_STRING ||
            a.datatype == Rdf::XSD::DATE || a.datatype == Rdf::XSD::DATE_TIME) {
            int c = a.value.compare(b.value);
            return c < 0 ? -1 : (c > 0 ? 1 : 0);
        }
    }
    return std::nullopt;
}

OptionalTerm evaluate(const SparqlExpression& expr, const SparqlSolution& solution);

OptionalTerm evaluate_function(const SparqlExpression& expr, const SparqlSolution& solution) {
    const std::string& name = expr.op;
    if (name == "BOUND") {
        return make_bool(solution.count(expr.args[0]->variable) > 0);
    }

    std::vector<OptionalTerm> args;
    for (const auto& arg : expr.args) {
        args.push_back(evaluate(*arg, solution));
        if (!args.back()) return std::nullopt;
    }
    const Term& a = *args[0];

    if (name == "ISIRI" || name == "ISURI") return make_bool(a.is_iri());
    if (name == "ISBLANK") return make_bool(a.is_blank());
    if (name == "ISLITERAL") return make_bool(a.is_literal());
    if (name == "ISNUMERIC") return make_bool(a.numeric_value().has_value());
    if (name == "STR") {
        if (a.is_blank()) return std::nullopt;
        return Term::literal(a.value);
    }
    if (name == "LANG") {
        if (!a.is_literal()) return std::nullopt;
        return Term::literal(a.language);
    }
    if (name == "DATATYPE") {
        if (!a.is_literal()) return std::nullopt;
        return Term::iri(a.datatype);
    }
    if (name == "SAMETERM") return make_bool(a == *args[1]);

    // The remaining functions operate on string literals.
    for (const auto& arg : args) {
        if (!is_string_literal(*arg)) return std::nullopt;
    }
    const std::string& text = a.value;
    const std::string& other = args[1]->value;

    if (name == "CONTAINS") return make_bool(text.find(other) != std::string::npos);
    if (name == "STRSTARTS") return make_bool(text.compare(0, other.size(), other) == 0);
    if (name == "STRENDS") {
        return make_bool(text.size() >= other.size() && text.compare(text.size() - other.size(), other.size(), other) == 0);
    }
    if (name == "LANGMATCHES") {
        std::string tag = to_lower(text);
        std::string range = to_lower(other);
        if (range == "*") return make_bool(!tag.empty());
        return make_bool(tag == range || tag.compare(0, range.size() + 1, range + "-") == 0);
    }
    if (name == "REGEX") {
        auto flags = std::regex::ECMAScript;
        if (args.size() == 3 && args[2]->value.find('i') != std::string::npos) flags |= std::regex::icase;
        try {
            return make_bool(std::regex_search(text, std::regex(other, flags)));
        } catch (const std::regex_error&) {
            return std::nullopt;
        }
    }
    return std::nullopt;
}

OptionalTerm evaluate(const SparqlExpression& expr, const SparqlSolution& solution) {
    switch (expr.kind) {
        case SparqlExpression::Kind::TERM:
            return expr.term;
        case SparqlExpression::Kind::VARIABLE: {
            auto it = solution.find(expr.variable);
            if (it == solution.end()) return std::nullopt;
            return it->second;
        }
        case SparqlExpression::Kind::UNARY: {
            auto value = effective_boolean_value(evaluate(*expr.args[0], solution));
            if (!value) return std::nullopt;
            return make_bool(!*value);
        }
        case SparqlExpression::Kind::BINARY: {
            if (expr.op == "||" || expr.op == "&&") {
                // SPARQL three-valued logic: an error only matters if it decides the result.
                auto left = effective_boolean_value(evaluate(*expr.args[0], solution));
                auto right = effective_boolean_value(evaluate(*expr.args[1], solution));
                const bool deciding = expr.op == "||";
                if ((left && *left == deciding) || (right && *right == deciding)) return make_bool(deciding);
                if (!left || !right) return std::nullopt;
                return make_bool(!deciding);
            }
            auto left = evaluate(*expr.args[0], solution);
            auto right = evaluate(*expr.args[1], solution);
            if (!left || !right) return std::nullopt;
            auto cmp = compare_values(*left, *right);
            if (expr.op == "=") return make_bool(cmp ? *cmp == 0 : *left == *right);
            if (expr.op == "!=") return make_bool(cmp ? *cmp != 0 : *left != *right);
            if (!cmp) return std::nullopt;
            if (expr.op == "<") return make_bool(*cmp < 0);
            if (expr.op == ">") return make_bool(*cmp > 0);
            if (expr.op == "<=") return make_bool(*cmp <= 0);
            if (expr.op == ">=") return make_bool(*cmp >= 0);
            return std::nullopt;
        }
        case SparqlExpression::Kind::FUNCTION:
            return evaluate_function(expr, solution);
    }
    return std::nullopt;
}

// Tries to extend a solution so that the pattern position matches the term.
bool bind(const SparqlPatternTerm& position, const Term& term, SparqlSolution& solution) {
    if (const auto* constant = std::get_if<Term>(&position)) return *constant == term;
    const std::string& name = std::get<SparqlVariable>(position).name;
    auto [it, inserted] = solution.emplace(name, term);
    return inserted || it->second == term;
}

bool is_known(const SparqlPatternTerm& position, const std::set<std::string>& bound) {
    if (std::holds_alternative<Term>(position)) return true;
    return bound.count(std::get<SparqlVariable>(position).name) > 0;
}

// Joins the given solutions with a basic graph pattern by extending each solution
// through nested-loop matching, picking at each step the pattern with the most
// constant or already-bound positions. A variable left unbound by an earlier
// OPTIONAL is free to bind here, which is exactly SPARQL join compatibility.
std::vector<SparqlSolution> evaluate_bgp(const std::vector<SparqlTriplePattern>& patterns,
                                         const std::vector<Rdf::Triple>& triples,
                                         std::vector<SparqlSolution> solutions) {
    std::vector<bool> used(patterns.size(), false);

    // Variables bound in every input solution count as known for pattern ordering.
    std::set<std::string> bound;
    if (!solutions.empty()) {
        for (const auto& [name, term] : solutions.front()) bound.insert(name);
        for (const auto& solution : solutions) {
            for (auto it = bound.begin(); it != bound.end();) {
                it = solution.count(*it) ? std::next(it) : bound.erase(it);
            }
        }
    }

    for (size_t step = 0; step < patterns.size() && !solutions.empty(); ++step) {
        size_t best = patterns.size();
        int best_score = -1;
        for (size_t i = 0; i < patterns.size(); ++i) {
            if (used[i]) continue;
            int score = is_known(patterns[i].subject, bound) + is_known(patterns[i].predicate, bound) +
                        is_known(patterns[i].object, bound);
            if (score > best_score) {
                best = i;
                best_score = score;
            }
        }
        used[best] = true;
        const SparqlTriplePattern& pattern = patterns[best];

        std::vector<SparqlSolution> next;
        for (const auto& solution : solutions) {
            for (const auto& triple : triples) {
                SparqlSolution extended = solution;
                if (bind(pattern.subject, triple.subject, extended) && bind(pattern.predicate, triple.predicate, extended) &&
                    bind(pattern.object, triple.object, extended)) {
                    next.push_back(std::move(extended));
                }
            }
        }
        solutions = std::move(next);

        for (const auto* position : {&pattern.subject, &pattern.predicate, &pattern.object}) {
            if (const auto* var = std::get_if<SparqlVariable>(position)) bound.insert(var->name);
        }
    }
    return solutions;
}

// FILTER semantics: every filter must be true; errors count as false.
bool passes_filters(const std::vector<std::shared_ptr<SparqlExpression>>& filters, const SparqlSolution& solution) {
    for (const auto& filter : filters) {
        auto value = effective_boolean_value(evaluate(*filter, solution));
        if (!value || !*value) return false;
    }
    return true;
}

// Two solutions are compatible if they agree on every variable they share.
bool compatible(const SparqlSolution& a, const SparqlSolution& b) {
    const SparqlSolution& smaller = a.size() <= b.size() ? a : b;
    const SparqlSolution& larger = a.size() <= b.size() ? b : a;
    for (const auto& [name, term] : smaller) {
        auto it = larger.find(name);
        if (it != larger.end() && it->second != term) return false;
    }
    return true;
}

// SPARQL LeftJoin: each left solution is merged with every compatible right
// solution for which the filters hold; a left solution with no such partner is
// kept unchanged.
std::vector<SparqlSolution> left_join(const std::vector<SparqlSolution>& left, const std::vector<SparqlSolution>& right,
                                      const std::vector<std::shared_ptr<SparqlExpression>>& filters) {
    std::vector<SparqlSolution> result;
    for (const auto& l : left) {
        bool extended = false;
        for (const auto& r : right) {
            if (!compatible(l, r)) continue;
            SparqlSolution merged = l;
            merged.insert(r.begin(), r.end());
            if (!passes_filters(filters, merged)) continue;
            result.push_back(std::move(merged));
            extended = true;
        }
        if (!extended) result.push_back(l);
    }
    return result;
}

// Evaluates a group pattern bottom-up. When apply_filters is false the caller
// (an OPTIONAL) uses the group's filters as its left-join condition instead.
std::vector<SparqlSolution> evaluate_group(const SparqlGroupPattern& group, const std::vector<Rdf::Triple>& triples,
                                           bool apply_filters) {
    std::vector<SparqlSolution> solutions(1);
    for (const auto& element : group.elements) {
        if (element.kind == SparqlGroupElement::Kind::TRIPLES) {
            solutions = evaluate_bgp(element.triples, triples, std::move(solutions));
        } else {
            solutions = left_join(solutions, evaluate_group(*element.group, triples, false), element.group->filters);
        }
    }
    if (apply_filters && !group.filters.empty()) {
        std::vector<SparqlSolution> kept;
        for (auto& solution : solutions) {
            if (passes_filters(group.filters, solution)) kept.push_back(std::move(solution));
        }
        solutions = std::move(kept);
    }
    return solutions;
}

// SPARQL ORDER BY ranking: unbound < blank node < IRI < literal.
int order_rank(const OptionalTerm& t) {
    if (!t) return 0;
    if (t->is_blank()) return 1;
    if (t->is_iri()) return 2;
    return 3;
}

int compare_for_order(const OptionalTerm& a, const OptionalTerm& b) {
    int ra = order_rank(a);
    int rb = order_rank(b);
    if (ra != rb || ra == 0) return ra - rb;
    if (auto cmp = compare_values(*a, *b)) return *cmp;
    if (*a < *b) return -1;
    if (*b < *a) return 1;
    return 0;
}

OptionalTerm lookup(const SparqlSolution& solution, const std::string& variable) {
    auto it = solution.find(variable);
    if (it == solution.end()) return std::nullopt;
    return it->second;
}

// Appends the group's variables in order of first appearance, skipping blank nodes.
void collect_variables(const SparqlGroupPattern& group, std::vector<std::string>& variables) {
    for (const auto& element : group.elements) {
        if (element.kind == SparqlGroupElement::Kind::OPTIONAL) {
            collect_variables(*element.group, variables);
            continue;
        }
        for (const auto& pattern : element.triples) {
            for (const auto* position : {&pattern.subject, &pattern.predicate, &pattern.object}) {
                const auto* var = std::get_if<SparqlVariable>(position);
                if (!var || var->name.compare(0, 2, "_:") == 0) continue;
                if (std::find(variables.begin(), variables.end(), var->name) == variables.end()) {
                    variables.push_back(var->name);
                }
            }
        }
    }
}

std::vector<std::string> projected_variables(const SparqlSelectStatement& stmt) {
    if (!stmt.select_all) return stmt.variables;
    std::vector<std::string> variables;
    collect_variables(stmt.where, variables);
    return variables;
}

} // anonymous namespace

std::vector<SparqlSolution> evaluate_sparql_select(Storage::LSMTree& storage_engine, const SparqlSelectStatement& stmt) {
    Storage::TripleStore store(storage_engine);
    std::vector<SparqlSolution> solutions = evaluate_group(stmt.where, store.all(), true);

    if (!stmt.order_by.empty()) {
        std::stable_sort(solutions.begin(), solutions.end(), [&](const SparqlSolution& a, const SparqlSolution& b) {
            for (const auto& condition : stmt.order_by) {
                int cmp = compare_for_order(lookup(a, condition.variable), lookup(b, condition.variable));
                if (cmp != 0) return condition.descending ? cmp > 0 : cmp < 0;
            }
            return false;
        });
    }

    // Projection, then DISTINCT, then OFFSET/LIMIT.
    const std::vector<std::string> variables = projected_variables(stmt);
    std::vector<SparqlSolution> projected;
    std::set<std::string> seen;
    for (const auto& solution : solutions) {
        SparqlSolution row;
        std::string key;
        for (const auto& variable : variables) {
            auto it = solution.find(variable);
            if (it == solution.end()) continue;
            row.emplace(variable, it->second);
            key += variable + "=" + it->second.to_ntriples() + "\n";
        }
        if (stmt.distinct && !seen.insert(key).second) continue;
        projected.push_back(std::move(row));
    }

    const size_t offset = std::min(stmt.offset.value_or(0), projected.size());
    const size_t end = stmt.limit ? offset + std::min(projected.size() - offset, *stmt.limit) : projected.size();
    return std::vector<SparqlSolution>(projected.begin() + offset, projected.begin() + end);
}

QueryResult execute_sparql_select_statement(Storage::LSMTree& storage_engine, const SparqlSelectStatement& stmt) {
    const std::vector<std::string> variables = projected_variables(stmt);
    QueryResult result;
    for (const auto& solution : evaluate_sparql_select(storage_engine, stmt)) {
        Document doc;
        doc.id = std::to_string(result.size());
        for (const auto& variable : variables) {
            auto it = solution.find(variable);
            if (it != solution.end()) doc.elements.push_back({variable, it->second.to_ntriples()});
        }
        result.push_back(std::move(doc));
    }
    return result;
}

} // namespace Query
} // namespace TissDB
