// Regression tests for existing TissQL queries and JSON output.
//
// The expected results below were captured by running these exact queries on the
// code from before SPARQL/RDF support was added, so any change in what existing
// queries return (rows, values, types, order or error messages) fails here.
#include "test_framework.h"
#include "../../tissdb/storage/lsm_tree.h"
#include "../../tissdb/query/parser.h"
#include "../../tissdb/query/executor.h"
#include "../../tissdb/json/json.h"
#include <cstdio>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

namespace query_regression {

struct Case {
    std::vector<std::string> statements; // run in order; results of each are recorded
    std::vector<TissDB::Query::Literal> params;
    std::string expected;
};

std::string format_number(double d) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.15g", d);
    return buf;
}

std::string canonical_value(const TissDB::Value& value);

std::string canonical_elements(const std::vector<TissDB::Element>& elements) {
    std::string out = "{";
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i) out += ", ";
        out += elements[i].key + "=" + canonical_value(elements[i].value);
    }
    return out + "}";
}

// Type-tagged, deterministic rendering of a value, so type changes are detected too.
std::string canonical_value(const TissDB::Value& value) {
    return std::visit([](const auto& v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            return "null";
        } else if constexpr (std::is_same_v<T, std::string>) {
            return "s'" + v + "'";
        } else if constexpr (std::is_same_v<T, TissDB::Number>) {
            return "n" + format_number(v);
        } else if constexpr (std::is_same_v<T, TissDB::Boolean>) {
            return v ? "true" : "false";
        } else if constexpr (std::is_same_v<T, TissDB::Date>) {
            return "date(" + std::to_string(v.year) + "-" + std::to_string(v.month) + "-" + std::to_string(v.day) + ")";
        } else if constexpr (std::is_same_v<T, TissDB::Time>) {
            return "time(" + std::to_string(v.hour) + ":" + std::to_string(v.minute) + ":" + std::to_string(v.second) + ")";
        } else if constexpr (std::is_same_v<T, TissDB::DateTime>) {
            auto secs = std::chrono::duration_cast<std::chrono::seconds>(v.time_since_epoch()).count();
            return "datetime(" + std::to_string(secs) + ")";
        } else if constexpr (std::is_same_v<T, TissDB::Timestamp>) {
            return "ts(" + std::to_string(v.microseconds_since_epoch_utc) + ")";
        } else if constexpr (std::is_same_v<T, TissDB::BinaryData>) {
            return "binary(" + std::to_string(v.size()) + ")";
        } else if constexpr (std::is_same_v<T, std::vector<TissDB::Element>>) {
            return canonical_elements(v);
        } else if constexpr (std::is_same_v<T, std::shared_ptr<TissDB::Array>>) {
            std::string out = "[";
            if (v) {
                for (size_t i = 0; i < v->values.size(); ++i) out += (i ? ", " : "") + canonical_value(v->values[i]);
            }
            return out + "]";
        } else {
            std::string out = "obj{";
            if (v) {
                bool first = true;
                for (const auto& [k, val] : v->values) {
                    out += (first ? "" : ", ") + k + "=" + canonical_value(val);
                    first = false;
                }
            }
            return out + "}";
        }
    }, value);
}

std::string canonical_result(const TissDB::Query::QueryResult& result) {
    std::string out = std::to_string(result.size()) + " row(s)";
    for (const auto& doc : result) out += "\n  " + doc.id + " " + canonical_elements(doc.elements);
    return out;
}

TissDB::Document make_doc(const std::string& id, std::vector<TissDB::Element> elements) {
    TissDB::Document doc;
    doc.id = id;
    doc.elements = std::move(elements);
    return doc;
}

struct Fixture {
    const std::string dir = "./test_query_regression_data";
    std::unique_ptr<TissDB::Storage::LSMTree> storage;

    Fixture() {
        std::filesystem::remove_all(dir);
        storage = std::make_unique<TissDB::Storage::LSMTree>(dir);
        using E = TissDB::Element;
        storage->create_collection("products", TissDB::Schema());
        storage->put("products", "p1", make_doc("p1", {E{"name", std::string("Laptop")}, E{"brand", std::string("TechGear")}, E{"category", std::string("electronics")}, E{"price", 1200.0}, E{"stock", 5.0}, E{"active", true}}));
        storage->put("products", "p2", make_doc("p2", {E{"name", std::string("Headphones")}, E{"brand", std::string("AudioPhonic")}, E{"category", std::string("electronics")}, E{"price", 199.99}, E{"stock", 50.0}, E{"active", true}}));
        storage->put("products", "p3", make_doc("p3", {E{"name", std::string("Desk")}, E{"brand", std::string("WoodWorks")}, E{"category", std::string("furniture")}, E{"price", 350.0}, E{"stock", 0.0}, E{"active", false}}));
        storage->put("products", "p4", make_doc("p4", {E{"name", std::string("Chair")}, E{"brand", std::string("WoodWorks")}, E{"category", std::string("furniture")}, E{"price", 120.0}, E{"stock", 12.0}, E{"active", true}}));
        storage->put("products", "p5", make_doc("p5", {E{"name", std::string("Speaker")}, E{"brand", std::string("AudioPhonic")}, E{"category", std::string("electronics")}, E{"price", 89.5}, E{"stock", 30.0}, E{"active", true}}));
        storage->put("products", "p6", make_doc("p6", {E{"name", std::string("O'Brien Lamp")}, E{"brand", std::string("Lumen")}, E{"category", std::string("lighting")}, E{"price", 45.0}, E{"stock", 7.0}, E{"active", true}}));

        storage->create_collection("brands", TissDB::Schema());
        storage->put("brands", "b1", make_doc("b1", {E{"brand", std::string("TechGear")}, E{"country", std::string("US")}}));
        storage->put("brands", "b2", make_doc("b2", {E{"brand", std::string("AudioPhonic")}, E{"country", std::string("DE")}}));
        storage->put("brands", "b3", make_doc("b3", {E{"brand", std::string("WoodWorks")}, E{"country", std::string("SE")}}));

        storage->create_collection("events", TissDB::Schema());
        storage->put("events", "e1", make_doc("e1", {E{"label", std::string("launch")}, E{"day", TissDB::Date{2024, 3, 15}}, E{"ts", TissDB::Timestamp{1710460800000000}}}));
        storage->put("events", "e2", make_doc("e2", {E{"label", std::string("review")}, E{"day", TissDB::Date{2024, 6, 1}}, E{"ts", TissDB::Timestamp{1717200000000000}}}));

        // A collection whose name starts with "prefix" must still be queried with TissQL.
        storage->create_collection("prefix_items", TissDB::Schema());
        storage->put("prefix_items", "x1", make_doc("x1", {E{"prefix", std::string("ab")}, E{"value", 1.0}}));
    }

    ~Fixture() {
        storage->shutdown();
        std::filesystem::remove_all(dir);
    }

    std::string run(const Case& c) {
        TissDB::Query::Parser parser;
        TissDB::Query::Executor executor(*storage);
        std::string out;
        std::vector<std::string> generated_ids; // INSERT assigns random ids
        for (const auto& statement : c.statements) {
            if (!out.empty()) out += "\n";
            out += "> " + statement + "\n";
            try {
                auto result = executor.execute(parser.parse(statement), c.params);
                for (const auto& doc : result) {
                    for (const auto& element : doc.elements) {
                        const auto* id = std::get_if<std::string>(&element.value);
                        if (element.key == "inserted_id" && id) generated_ids.push_back(*id);
                    }
                }
                out += canonical_result(result);
            } catch (const std::exception& e) {
                out += std::string("ERROR: ") + e.what();
            }
        }
        for (const auto& id : generated_ids) {
            for (size_t at = out.find(id); at != std::string::npos; at = out.find(id, at)) out.replace(at, id.size(), "<generated-id>");
        }
        return out;
    }
};

const std::vector<Case>& cases() {
    static const std::vector<Case> all = {
        {{"SELECT * FROM products"}, {}, R"golden(> SELECT * FROM products
6 row(s)
  p1 {name=s'Laptop', brand=s'TechGear', category=s'electronics', price=n1200, stock=n5, active=true}
  p2 {name=s'Headphones', brand=s'AudioPhonic', category=s'electronics', price=n199.99, stock=n50, active=true}
  p3 {name=s'Desk', brand=s'WoodWorks', category=s'furniture', price=n350, stock=n0, active=false}
  p4 {name=s'Chair', brand=s'WoodWorks', category=s'furniture', price=n120, stock=n12, active=true}
  p5 {name=s'Speaker', brand=s'AudioPhonic', category=s'electronics', price=n89.5, stock=n30, active=true}
  p6 {name=s'O'Brien Lamp', brand=s'Lumen', category=s'lighting', price=n45, stock=n7, active=true})golden"},
        {{"SELECT name, price FROM products"}, {}, R"golden(> SELECT name, price FROM products
6 row(s)
  p1 {name=s'Laptop', price=n1200}
  p2 {name=s'Headphones', price=n199.99}
  p3 {name=s'Desk', price=n350}
  p4 {name=s'Chair', price=n120}
  p5 {name=s'Speaker', price=n89.5}
  p6 {name=s'O'Brien Lamp', price=n45})golden"},
        {{"SELECT * FROM products WHERE brand = 'WoodWorks'"}, {}, R"golden(> SELECT * FROM products WHERE brand = 'WoodWorks'
2 row(s)
  p3 {name=s'Desk', brand=s'WoodWorks', category=s'furniture', price=n350, stock=n0, active=false}
  p4 {name=s'Chair', brand=s'WoodWorks', category=s'furniture', price=n120, stock=n12, active=true})golden"},
        {{"SELECT name FROM products WHERE price > 150"}, {}, R"golden(> SELECT name FROM products WHERE price > 150
3 row(s)
  p1 {name=s'Laptop'}
  p2 {name=s'Headphones'}
  p3 {name=s'Desk'})golden"},
        {{"SELECT name FROM products WHERE price >= 120 AND price <= 350"}, {}, R"golden(> SELECT name FROM products WHERE price >= 120 AND price <= 350
3 row(s)
  p2 {name=s'Headphones'}
  p3 {name=s'Desk'}
  p4 {name=s'Chair'})golden"},
        {{"SELECT name FROM products WHERE price < 100 OR stock = 0"}, {}, R"golden(> SELECT name FROM products WHERE price < 100 OR stock = 0
3 row(s)
  p3 {name=s'Desk'}
  p5 {name=s'Speaker'}
  p6 {name=s'O'Brien Lamp'})golden"},
        {{"SELECT name FROM products WHERE category != 'electronics'"}, {}, R"golden(> SELECT name FROM products WHERE category != 'electronics'
3 row(s)
  p3 {name=s'Desk'}
  p4 {name=s'Chair'}
  p6 {name=s'O'Brien Lamp'})golden"},
        {{"SELECT name FROM products WHERE (category = 'electronics' AND price > 100) OR brand = 'Lumen'"}, {}, R"golden(> SELECT name FROM products WHERE (category = 'electronics' AND price > 100) OR brand = 'Lumen'
3 row(s)
  p1 {name=s'Laptop'}
  p2 {name=s'Headphones'}
  p6 {name=s'O'Brien Lamp'})golden"},
        {{"SELECT name FROM products WHERE NOT active"}, {}, R"golden(> SELECT name FROM products WHERE NOT active
ERROR: Unexpected token in expression)golden"},
        {{"SELECT name FROM products WHERE active = true"}, {}, R"golden(> SELECT name FROM products WHERE active = true
5 row(s)
  p1 {name=s'Laptop'}
  p2 {name=s'Headphones'}
  p4 {name=s'Chair'}
  p5 {name=s'Speaker'}
  p6 {name=s'O'Brien Lamp'})golden"},
        {{"SELECT name FROM products WHERE name LIKE '%e%'"}, {}, R"golden(> SELECT name FROM products WHERE name LIKE '%e%'
4 row(s)
  p2 {name=s'Headphones'}
  p3 {name=s'Desk'}
  p5 {name=s'Speaker'}
  p6 {name=s'O'Brien Lamp'})golden"},
        {{"SELECT name FROM products WHERE name LIKE 'S_eaker'"}, {}, R"golden(> SELECT name FROM products WHERE name LIKE 'S_eaker'
1 row(s)
  p5 {name=s'Speaker'})golden"},
        {{"SELECT name FROM products WHERE name = 'O''Brien Lamp'"}, {}, R"golden(> SELECT name FROM products WHERE name = 'O''Brien Lamp'
1 row(s)
  p6 {name=s'O'Brien Lamp'})golden"},
        {{"SELECT name FROM products WHERE price BETWEEN 50 AND 200"}, {}, R"golden(> SELECT name FROM products WHERE price BETWEEN 50 AND 200
3 row(s)
  p2 {name=s'Headphones'}
  p4 {name=s'Chair'}
  p5 {name=s'Speaker'})golden"},
        {{"SELECT name FROM products WHERE price NOT BETWEEN 50 AND 200"}, {}, R"golden(> SELECT name FROM products WHERE price NOT BETWEEN 50 AND 200
3 row(s)
  p1 {name=s'Laptop'}
  p3 {name=s'Desk'}
  p6 {name=s'O'Brien Lamp'})golden"},
        {{"SELECT name, price FROM products ORDER BY price ASC"}, {}, R"golden(> SELECT name, price FROM products ORDER BY price ASC
6 row(s)
  p6 {name=s'O'Brien Lamp', price=n45}
  p5 {name=s'Speaker', price=n89.5}
  p4 {name=s'Chair', price=n120}
  p2 {name=s'Headphones', price=n199.99}
  p3 {name=s'Desk', price=n350}
  p1 {name=s'Laptop', price=n1200})golden"},
        {{"SELECT name, price FROM products ORDER BY price DESC LIMIT 3"}, {}, R"golden(> SELECT name, price FROM products ORDER BY price DESC LIMIT 3
6 row(s)
  p1 {name=s'Laptop', price=n1200}
  p3 {name=s'Desk', price=n350}
  p2 {name=s'Headphones', price=n199.99}
  p4 {name=s'Chair', price=n120}
  p5 {name=s'Speaker', price=n89.5}
  p6 {name=s'O'Brien Lamp', price=n45})golden"},
        {{"SELECT * FROM products ORDER BY name LIMIT 2"}, {}, R"golden(> SELECT * FROM products ORDER BY name LIMIT 2
6 row(s)
  p4 {name=s'Chair', brand=s'WoodWorks', category=s'furniture', price=n120, stock=n12, active=true}
  p3 {name=s'Desk', brand=s'WoodWorks', category=s'furniture', price=n350, stock=n0, active=false}
  p2 {name=s'Headphones', brand=s'AudioPhonic', category=s'electronics', price=n199.99, stock=n50, active=true}
  p1 {name=s'Laptop', brand=s'TechGear', category=s'electronics', price=n1200, stock=n5, active=true}
  p6 {name=s'O'Brien Lamp', brand=s'Lumen', category=s'lighting', price=n45, stock=n7, active=true}
  p5 {name=s'Speaker', brand=s'AudioPhonic', category=s'electronics', price=n89.5, stock=n30, active=true})golden"},
        {{"SELECT COUNT(*) FROM products"}, {}, R"golden(> SELECT COUNT(*) FROM products
1 row(s)
  aggregate {COUNT(*)=n6})golden"},
        {{"SELECT SUM(price), AVG(price), MIN(price), MAX(price) FROM products"}, {}, R"golden(> SELECT SUM(price), AVG(price), MIN(price), MAX(price) FROM products
1 row(s)
  aggregate {SUM(price)=n2004.49, AVG(price)=n334.081666666667, MIN(price)=n45, MAX(price)=n1200})golden"},
        {{"SELECT STDDEV(price) FROM products"}, {}, R"golden(> SELECT STDDEV(price) FROM products
ERROR: Expected token FROM but got ()golden"},
        {{"SELECT category, COUNT(name), SUM(stock) FROM products GROUP BY category"}, {}, R"golden(> SELECT category, COUNT(name), SUM(stock) FROM products GROUP BY category
3 row(s)
  electronics {category=s'electronics', COUNT(name)=n3, SUM(stock)=n85}
  furniture {category=s'furniture', COUNT(name)=n2, SUM(stock)=n12}
  lighting {category=s'lighting', COUNT(name)=n1, SUM(stock)=n7})golden"},
        {{"SELECT brand, SUM(price) FROM products GROUP BY brand HAVING SUM(price) > 300"}, {}, R"golden(> SELECT brand, SUM(price) FROM products GROUP BY brand HAVING SUM(price) > 300
4 row(s)
  AudioPhonic {brand=s'AudioPhonic', SUM(price)=n289.49}
  Lumen {brand=s'Lumen', SUM(price)=n45}
  TechGear {brand=s'TechGear', SUM(price)=n1200}
  WoodWorks {brand=s'WoodWorks', SUM(price)=n470})golden"},
        {{"SELECT DISTINCT category FROM products"}, {}, R"golden(> SELECT DISTINCT category FROM products
ERROR: Expected token FROM but got category)golden"},
        {{"SELECT * FROM products JOIN brands ON products.brand = brands.brand"}, {}, R"golden(> SELECT * FROM products JOIN brands ON products.brand = brands.brand
5 row(s)
  p1_b1 {products._id=s'p1', products.name=s'Laptop', products.brand=s'TechGear', products.category=s'electronics', products.price=n1200, products.stock=n5, products.active=true, brands._id=s'b1', brands.brand=s'TechGear', brands.country=s'US'}
  p2_b2 {products._id=s'p2', products.name=s'Headphones', products.brand=s'AudioPhonic', products.category=s'electronics', products.price=n199.99, products.stock=n50, products.active=true, brands._id=s'b2', brands.brand=s'AudioPhonic', brands.country=s'DE'}
  p3_b3 {products._id=s'p3', products.name=s'Desk', products.brand=s'WoodWorks', products.category=s'furniture', products.price=n350, products.stock=n0, products.active=false, brands._id=s'b3', brands.brand=s'WoodWorks', brands.country=s'SE'}
  p4_b3 {products._id=s'p4', products.name=s'Chair', products.brand=s'WoodWorks', products.category=s'furniture', products.price=n120, products.stock=n12, products.active=true, brands._id=s'b3', brands.brand=s'WoodWorks', brands.country=s'SE'}
  p5_b2 {products._id=s'p5', products.name=s'Speaker', products.brand=s'AudioPhonic', products.category=s'electronics', products.price=n89.5, products.stock=n30, products.active=true, brands._id=s'b2', brands.brand=s'AudioPhonic', brands.country=s'DE'})golden"},
        {{"SELECT * FROM products LEFT JOIN brands ON products.brand = brands.brand WHERE products.category = 'lighting'"}, {}, R"golden(> SELECT * FROM products LEFT JOIN brands ON products.brand = brands.brand WHERE products.category = 'lighting'
0 row(s))golden"},
        {{"SELECT name FROM products WHERE brand = 'Lumen' UNION SELECT name FROM products WHERE stock = 0"}, {}, R"golden(> SELECT name FROM products WHERE brand = 'Lumen' UNION SELECT name FROM products WHERE stock = 0
2 row(s)
  p3 {name=s'Desk'}
  p6 {name=s'O'Brien Lamp'})golden"},
        {{"SELECT brand FROM products UNION ALL SELECT brand FROM brands"}, {}, R"golden(> SELECT brand FROM products UNION ALL SELECT brand FROM brands
9 row(s)
  p1 {brand=s'TechGear'}
  p2 {brand=s'AudioPhonic'}
  p3 {brand=s'WoodWorks'}
  p4 {brand=s'WoodWorks'}
  p5 {brand=s'AudioPhonic'}
  p6 {brand=s'Lumen'}
  b1 {brand=s'TechGear'}
  b2 {brand=s'AudioPhonic'}
  b3 {brand=s'WoodWorks'})golden"},
        {{"SELECT name FROM products WHERE brand = ? AND price > ?"}, {std::string("AudioPhonic"), 100.0}, R"golden(> SELECT name FROM products WHERE brand = ? AND price > ?
1 row(s)
  p2 {name=s'Headphones'})golden"},
        {{"SELECT * FROM events WHERE day > DATE '2024-04-01'"}, {}, R"golden(> SELECT * FROM events WHERE day > DATE '2024-04-01'
1 row(s)
  e2 {label=s'review', day=date(2024-6-1), ts=ts(1717200000000000)})golden"},
        {{"SELECT label FROM events WHERE ts < TIMESTAMP '2024-05-01T00:00:00Z'"}, {}, R"golden(> SELECT label FROM events WHERE ts < TIMESTAMP '2024-05-01T00:00:00Z'
1 row(s)
  e1 {label=s'launch'})golden"},
        {{"SELECT * FROM prefix_items"}, {}, R"golden(> SELECT * FROM prefix_items
1 row(s)
  x1 {prefix=s'ab', value=n1})golden"},
        {{"SELECT prefix FROM prefix_items WHERE prefix = 'ab'"}, {}, R"golden(> SELECT prefix FROM prefix_items WHERE prefix = 'ab'
1 row(s)
  x1 {prefix=s'ab'})golden"},
        {{"SELECT * FROM missing_collection"}, {}, R"golden(> SELECT * FROM missing_collection
0 row(s))golden"},
        {{"INSERT INTO products (name, brand, price) VALUES ('Monitor', 'TechGear', 300)",
          "SELECT name, brand, price FROM products WHERE brand = 'TechGear'"}, {}, R"golden(> INSERT INTO products (name, brand, price) VALUES ('Monitor', 'TechGear', 300)
1 row(s)
  summary {inserted_count=n1, inserted_id=s'<generated-id>'}
> SELECT name, brand, price FROM products WHERE brand = 'TechGear'
2 row(s)
  <generated-id> {name=s'Monitor', brand=s'TechGear', price=n300}
  p1 {name=s'Laptop', brand=s'TechGear', price=n1200})golden"},
        {{"UPDATE products SET stock = 99 WHERE brand = 'WoodWorks'",
          "SELECT name, stock FROM products WHERE brand = 'WoodWorks'"}, {}, R"golden(> UPDATE products SET stock = 99 WHERE brand = 'WoodWorks'
1 row(s)
  summary {updated_count=n2}
> SELECT name, stock FROM products WHERE brand = 'WoodWorks'
2 row(s)
  p3 {name=s'Desk', stock=n99}
  p4 {name=s'Chair', stock=n99})golden"},
        {{"UPDATE products SET price = 10, active = false WHERE name = 'Desk'",
          "SELECT * FROM products WHERE name = 'Desk'"}, {}, R"golden(> UPDATE products SET price = 10, active = false WHERE name = 'Desk'
1 row(s)
  summary {updated_count=n1}
> SELECT * FROM products WHERE name = 'Desk'
1 row(s)
  p3 {name=s'Desk', brand=s'WoodWorks', category=s'furniture', price=n10, stock=n0, active=false})golden"},
        {{"DELETE FROM products WHERE category = 'electronics'",
          "SELECT name FROM products"}, {}, R"golden(> DELETE FROM products WHERE category = 'electronics'
1 row(s)
  summary {deleted_count=n3}
> SELECT name FROM products
3 row(s)
  p3 {name=s'Desk'}
  p4 {name=s'Chair'}
  p6 {name=s'O'Brien Lamp'})golden"},
        {{"DELETE FROM brands", "SELECT * FROM brands"}, {}, R"golden(> DELETE FROM brands
1 row(s)
  summary {deleted_count=n3}
> SELECT * FROM brands
0 row(s))golden"},
        // Invalid TissQL keeps reporting the same errors.
        {{"SELECT FROM products"}, {}, R"golden(> SELECT FROM products
ERROR: Expected token FROM but got products)golden"},
        {{"SELECT * products"}, {}, R"golden(> SELECT * products
ERROR: Expected token FROM but got products)golden"},
        {{"DROP TABLE products"}, {}, R"golden(> DROP TABLE products
ERROR: Unsupported statement type)golden"},
        {{"select name from products where price > 1000"}, {}, R"golden(> select name from products where price > 1000
1 row(s)
  p1 {name=s'Laptop'})golden"},
    };
    return all;
}

std::string sample_json() {
    TissDB::Json::JsonObject obj;
    obj["_id"] = TissDB::Json::JsonValue("p1");
    obj["name"] = TissDB::Json::JsonValue("Laptop Pro 15");
    obj["price"] = TissDB::Json::JsonValue(1200.5);
    obj["stock"] = TissDB::Json::JsonValue(5.0);
    obj["active"] = TissDB::Json::JsonValue(true);
    obj["notes"] = TissDB::Json::JsonValue();
    obj["unicode"] = TissDB::Json::JsonValue(std::string("caf\xC3\xA9 / <tag> & 'single'"));
    TissDB::Json::JsonArray tags;
    tags.push_back(TissDB::Json::JsonValue("a"));
    tags.push_back(TissDB::Json::JsonValue(2.0));
    obj["tags"] = TissDB::Json::JsonValue(tags);
    return TissDB::Json::JsonValue(obj).serialize();
}

} // namespace query_regression

TEST_CASE(RegressionExistingTissQLQueriesReturnSameResults) {
    int mismatches = 0;
    for (const auto& c : query_regression::cases()) {
        query_regression::Fixture fixture;
        std::string actual = fixture.run(c);
        if (actual != c.expected) {
            mismatches++;
            std::cerr << "\nResult changed for: " << c.statements.front() << "\n--- expected\n" << c.expected
                      << "\n--- actual\n" << actual << std::endl;
        }
    }
    if (mismatches) FAIL(mismatches << " existing quer" << (mismatches == 1 ? "y" : "ies") << " returned different results");
}

TEST_CASE(RegressionJsonSerializationUnchangedForPlainValues) {
    // Ordinary values must serialize byte-for-byte as before; only strings that
    // need escaping (quotes, backslashes, control characters) serialize differently.
    ASSERT_EQ(std::string(R"golden({"_id":"p1","active":true,"name":"Laptop Pro 15","notes":null,"price":1200.5,"stock":5,"tags":["a",2],"unicode":"café / <tag> & 'single'"})golden"), query_regression::sample_json());
}
