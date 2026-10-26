# TissDB

TissDB is a lightweight, high-performance NoSQL database built from scratch in C++. It is designed to be a simple, embeddable database for C++ applications.

## Features

*   **Multi-Database Support:** TissDB can manage multiple, isolated databases on a single server instance. Each database has its own collections and data.
*   **Collection Management:** Create, delete, and list collections within each database.
*   **Write-Ahead Log (WAL):** Data is written to a WAL to ensure durability and allow for recovery upon restart.
*   **JSON-like Document Model:** TissDB stores data in a flexible, JSON-like document model.
*   **TissQL Query Language:** TissDB provides a simple, SQL-like query language called TissQL for querying data.
*   **RDF and N-Triples:** TissDB stores RDF triples (IRIs, blank nodes, and literals with XSD datatypes or language tags) and loads them from [N-Triples](https://www.w3.org/TR/n-triples/) over HTTP or from files at startup.
*   **SPARQL Queries:** RDF data can be queried with SPARQL `SELECT`, including joins across triple patterns, `FILTER`, `OPTIONAL`, `ORDER BY`, `LIMIT`, and `OFFSET`. A query is treated as SPARQL when it starts with the `PREFIX` keyword.
*   **RESTful API:** TissDB provides a RESTful API for interacting with the database.

## Building and Running

To build TissDB, you will need a C++17 compiler and `make`. You can build the project by running the `make` command from the `tissdb` directory:

```bash
# From the tissdb/ directory
make
```

This creates the executable `build/tissdb`. You can then run the database with:

```bash
./build/tissdb
```

By default, the database listens on port 9876 and stores all its data in a directory named `tissdb_data`.

### Command-Line Options

| Option | Description |
| --- | --- |
| `-h`, `--help` | Show the help message and exit. |
| `--port <port>` | Port to listen on (default: `9876`). |
| `--data-dir <path>` | Data directory (default: `tissdb_data`). |
| `--import-ntriples <file>` | Load an N-Triples file before the server starts. Can be repeated to load several files. |
| `--import-db <name>` | Database that `--import-ntriples` loads into (default: `main_db`). It is created if it does not exist. |

For example, to start the server with two N-Triples files loaded into a database called `graph`:

```bash
./build/tissdb --import-ntriples people.nt --import-ntriples places.nt --import-db graph
```

Each file is read and fully parsed before anything is stored, so a file with an error loads nothing. If a file cannot be read or contains an error, TissDB prints the problem (including the line number) and exits with status 1 instead of serving partially imported data; files listed before the failing one stay loaded.

## Querying

Queries are sent with `POST /<db>/_query` (or `POST /<db>/<collection>/_query`) and a JSON body of the form `{"query": "..."}`. All endpoints except `/_health` require an `Authorization: Bearer <token>` header.

```bash
curl -X POST -H "Authorization: Bearer static_test_token" \
     -d '{"query": "SELECT name, price FROM products WHERE price > 100"}' \
     http://localhost:9876/main_db/_query
```

The result is a JSON array with one object per row.

### TissQL

TissQL is TissDB's SQL-like language for documents (`SELECT`, `INSERT`, `UPDATE`, `DELETE`, `WHERE`, `ORDER BY`, `LIMIT`, `GROUP BY`, aggregates, `JOIN`, `UNION`, and more). See [`docs/tissql.ebnf`](../docs/tissql.ebnf) for the grammar. Adding SPARQL support did not change how TissQL queries are parsed or what they return; this is checked by regression tests (see [Testing](#testing)).

### SPARQL

A query whose first keyword is `PREFIX` (case-insensitive, after optional whitespace and `#` comments) is parsed as SPARQL; every other query is parsed as TissQL. SPARQL queries run over the database's RDF triples (see [Storing RDF](#storing-rdf)).

```sparql
PREFIX ex: <http://example.org/>
SELECT ?name ?email ?age
WHERE {
  ?person a ex:Person ;
          ex:name ?name .
  OPTIONAL { ?person ex:email ?email }
  OPTIONAL { ?person ex:age ?age FILTER(?age >= 18) }
  FILTER(regex(?name, "^A", "i"))
}
ORDER BY DESC(?age)
LIMIT 10
```

Supported:

*   **Prologue:** `PREFIX` declarations (including the empty prefix `:`) and `BASE` (relative IRIs are appended to the base).
*   **Projection:** `SELECT ?a ?b ...` or `SELECT *`, with optional `DISTINCT` (`REDUCED` is accepted and ignored).
*   **Triple patterns:** variables (`?x` or `$x`), IRIs (`<...>` or prefixed names), literals (strings with `@lang` or `^^datatype`, integers, decimals, doubles, `true`/`false`), blank nodes (`_:b`, which act as variables that are never projected), the `a` shorthand for `rdf:type`, and the `;` and `,` abbreviations.
*   **Joins:** triple patterns that share a variable are joined on it.
*   **`OPTIONAL { ... }`:** left join; solutions with no optional match are kept with the optional variables unbound. Optionals can be nested and repeated, and are evaluated in order, so `{ A OPTIONAL { B } C }` means "A, optionally B, then C". A `FILTER` inside an `OPTIONAL` only decides whether the optional part matches; a `FILTER` outside removes whole solutions. `OPTIONAL { ... } FILTER(!BOUND(?x))` finds solutions without a match.
*   **`FILTER`:** `=`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`, `!`, parentheses, and the functions `BOUND`, `STR`, `LANG`, `DATATYPE`, `isIRI`/`isURI`, `isBlank`, `isLiteral`, `isNumeric`, `REGEX` (with optional `"i"` flag), `CONTAINS`, `STRSTARTS`, `STRENDS`, `sameTerm`, and `langMatches`. Numbers compare by value, strings, dates and dateTimes lexically. As in SPARQL, an expression error (such as comparing a number with an IRI) makes the filter false.
*   **Solution modifiers:** `ORDER BY ?v`, `ASC(?v)`, `DESC(?v)` (several keys allowed), `LIMIT n`, `OFFSET n`.

Not supported: `UNION`, `MINUS`, `GRAPH`, `BIND`, `VALUES`, `SERVICE`, nested `{ ... }` groups other than `OPTIONAL`, expressions or aggregates in `SELECT`, `GROUP BY`/`HAVING`, arithmetic, property paths, and the `CONSTRUCT`, `ASK` and `DESCRIBE` query forms. These are rejected with an error that names the unsupported feature.

**Results.** Each solution is returned as one JSON object. Every bound, projected variable becomes a key (without the `?`) whose value is the term in N-Triples syntax, as in the SPARQL TSV results format: `<http://example.org/alice>`, `_:b0`, `"Alice"`, `"Bonjour"@fr`, `"34"^^<http://www.w3.org/2001/XMLSchema#integer>`. Unbound variables (for example from an `OPTIONAL` that did not match) are left out of that row. Each row also has an `_id` holding its position in the result.

```json
[
  {"_id": "0", "email": "\"alice@example.org\"", "name": "\"Alice\""},
  {"_id": "1", "name": "\"Bob\""}
]
```

**Errors.** An invalid SPARQL query is answered with `400 Bad Request` and a plain-text message that says what is wrong and where, using 1-based line and column numbers:

```text
SPARQL syntax error at line 4, column 6: undeclared prefix 'nope:'; declare it with PREFIX nope: <namespace-iri>
SPARQL syntax error at line 1, column 63: UNION is not supported
SPARQL syntax error at line 1, column 70: REGEX expects 2 or 3 arguments but got 1
SPARQL syntax error at line 1, column 63: missing '}' to close the '{' at line 1, column 50
```

In C++ the parser throws `TissDB::Query::SparqlSyntaxError` (derived from `std::runtime_error`), which also provides `line()`, `column()`, and `detail()` (the message without the position prefix).

## Storing RDF

### RDF datatypes

`common/rdf.h` defines the RDF data model: `Rdf::Term` (an IRI, blank node, or literal) and `Rdf::Triple`. Literals always carry a datatype IRI: a literal without one is an `xsd:string`, and a language-tagged literal is an `rdf:langString` with a lowercased tag, following RDF 1.1. Constants for common `xsd:` datatypes are provided in `Rdf::XSD`. Numeric literals (`xsd:integer`, `xsd:decimal`, `xsd:double`, `xsd:float`, and the derived integer types) can be read with `Term::numeric_value()`.

### N-Triples

`common/ntriples.h` parses and serializes N-Triples: one triple per line, `#` comments, blank lines, Windows (CRLF) line endings, string escapes (`\t \b \n \r \f \" \' \\`) and Unicode escapes (`\uXXXX`, `\UXXXXXXXX`). Malformed input throws `NTriples::ParseError`, whose message and `line()` give the 1-based line number.

### Triple store

Triples are stored in an ordinary collection named `rdf_triples` in each database, managed by `Storage::TripleStore` (`storage/triple_store.h`). Each triple is one document whose ID is the triple's N-Triples line, so loading the same triple twice stores it once, and whose `subject`, `predicate`, and `object` fields hold the N-Triples form of each term. The triples are therefore persisted and recovered through the WAL like any other documents.

### Loading N-Triples

*   **HTTP endpoint:** `POST /<db>/_ntriples` with an N-Triples document as the request body. The response is `{"loaded": n}`, where `n` is the number of triples that were not already present. Malformed input is rejected with `400 Bad Request` and the parse error, and nothing from that request is stored. The database must already exist.

    ```bash
    curl -X POST -H "Authorization: Bearer static_test_token" \
         --data-binary @people.nt http://localhost:9876/main_db/_ntriples
    ```

*   **File import at startup:** `--import-ntriples <file>` (see [Command-Line Options](#command-line-options)).
*   **From C++:** `TripleStore::load_ntriples(text)`, `TripleStore::load_ntriples_file(path)`, or `Storage::import_ntriples_file(manager, db_name, path)`, which creates the database if needed and only does so once the file has parsed successfully.

### Web platform

The QuantaTissu web platform (`web_platform/`) has a **Query Browser** tab that runs TissQL or SPARQL queries against a chosen database and shows the results in a sortable, paginated table, together with any error message. The same tab can import an N-Triples file into the selected database. It uses the platform endpoints `POST /api/db/query` (`{"db_name": ..., "query": ...}`) and `POST /api/db/ntriples` (`{"db_name": ..., "data": "<n-triples>"}`), which forward to the TissDB endpoints above. Uploads through the platform are limited to 10 MB; use `--import-ntriples` for larger files.

## Changes in Behavior

The following changed along with the RDF and SPARQL support:

*   **Queries starting with `PREFIX`** are now parsed as SPARQL. Previously they failed with `Unsupported statement type`. No TissQL statement starts with `PREFIX`, so TissQL queries are unaffected; collections and fields whose names merely begin with "prefix" still work as before.
*   **JSON strings are now escaped.** API responses previously wrote `"`, `\`, and control characters inside strings unescaped, which produced invalid JSON for such values. They are now escaped (`\"`, `\\`, `\n`, `\u0001`, ...). Output for strings without these characters is byte-for-byte unchanged. The JSON parser also now accepts `\uXXXX` escapes (including surrogate pairs). As a side effect, index entries for document IDs that contain quotes are now stored correctly.

## Testing

The unit tests live in `tests/db/` and are built by `make test`. The tests for these features are:

*   `test_rdf.cpp`: RDF terms and datatypes, N-Triples parsing and serialization, the triple store, file import, and persistence across restarts.
*   `test_sparql.cpp`: detection by `PREFIX`, parsing, joins, `FILTER`, `OPTIONAL`, solution modifiers, and the exact error message (including line and column) for each kind of invalid query.
*   `test_query_regression.cpp`: runs a fixed set of existing TissQL queries (selects, filters, `LIKE`, `BETWEEN`, ordering, aggregates, grouping, joins, unions, parameters, dates and timestamps, inserts, updates, deletes, and invalid queries) and checks that they return exactly the results, value types, and error messages recorded from the code before SPARQL support was added. It also checks that JSON output for ordinary values is unchanged.
*   `test_json.cpp`: JSON string escaping and `\u` escapes.

Note: `make test` does not currently build, for reasons that predate these features: `test_indexer.cpp` uses an older `Indexer::find_by_index` API, `test_timestamp.cpp` calls a parser helper that is not linkable, and `test_tissdb_client.cpp` needs the TissDB client library, which the Makefile does not build. `test_tissu_client.cpp` also waits for a running server. The remaining test files build and run when those four are left out of `tests/db/test_main.cpp`.

## Current Limitations

*   **In-Memory Storage Model:** While a Write-Ahead Log ensures durability, the primary data structures (collections) are still held in-memory. The database must replay the WAL on startup to restore state.
*   **No Index Persistence:** B-Tree indexes are not yet persistent.
*   **In-Progress Transaction Support:** The API includes endpoints for transactions, but the implementation is not yet complete.
*   **No Replication or Sharding:** The database does not yet support replication or sharding.
*   **SPARQL Scale:** SPARQL queries scan the database's `rdf_triples` collection and join with nested loops, which suits modest graphs; large graphs would need triple indexes and hash joins. Only the `SELECT` subset described above is supported.
*   **Single RDF Graph per Database:** Each database has one default graph; named graphs are not supported.

## Documentation

For API details, see the [TissDB API Guide](../docs/tissdb_api_guide.md) and the [TissQL grammar](../docs/tissql.ebnf).
