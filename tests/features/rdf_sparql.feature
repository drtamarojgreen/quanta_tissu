Feature: TissDB RDF and SPARQL API

  Scenario: Import N-Triples and query RDF with SPARQL
    Given a running TissDB instance
    When I import this N-Triples document
      """
      <http://example.org/alice> <http://example.org/name> "Alice" .
      <http://example.org/alice> <http://example.org/age> "34"^^<http://www.w3.org/2001/XMLSchema#integer> .
      <http://example.org/bob> <http://example.org/name> "Bob" .
      """
    Then the RDF import should report 3 new triples
    When I execute this SPARQL query
      """
      PREFIX ex: <http://example.org/>
      SELECT ?name ?age
      WHERE {
        ?person ex:name ?name .
        OPTIONAL { ?person ex:age ?age }
      }
      ORDER BY ?name
      """
    Then the SPARQL results should contain the binding "name" = '"Alice"'
    And the SPARQL results should contain the binding "age" = '"34"^^<http://www.w3.org/2001/XMLSchema#integer>'
    And the SPARQL results should contain the binding "name" = '"Bob"'
    And the SPARQL results should omit the binding "age" for "Bob"

  Scenario: Reject malformed N-Triples without importing partial data
    Given a running TissDB instance
    When I import this N-Triples document
      """
      <http://example.org/alice> <http://example.org/name> "Alice" .
      this is not a triple
      """
    Then the RDF import should fail with status 400
    When I execute this SPARQL query
      """
      PREFIX ex: <http://example.org/>
      SELECT ?name WHERE { ?person ex:name ?name }
      """
    Then the SPARQL results should be empty

  Scenario: Return a client error for invalid SPARQL
    Given a running TissDB instance
    When I execute this SPARQL query
      """
      PREFIX ex: <http://example.org/>
      SELECT ?person WHERE { ?person ex:name ?name UNION { ?person ex:knows ?friend } }
      """
    Then the SPARQL query should fail with status 400
