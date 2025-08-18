
#include "test_framework.h"
#include "../../tissdb/query/executor.h"
#include "../../tissdb/query/parser.h"
#include <set>
#include "../../tissdb/storage/lsm_tree.h"
#include <filesystem>
#include <set>
#include "../../tissdb/storage/transaction_manager.h"

// Mock LSMTree for testing executor in isolation
class MockLSMTree : public TissDB::Storage::LSMTree {
public:
    MockLSMTree() : TissDB::Storage::LSMTree() {}

    void create_collection(const std::string& name, const TissDB::Schema& schema) override {
        (void)schema; // Unused in mock
        mock_data_[name] = {};
    }

    void put(const std::string& collection_name, const std::string& key, const TissDB::Document& doc, TissDB::Transactions::TransactionID tid = -1) override {
        (void)tid; // Unused in mock
        mock_data_[collection_name][key] = doc;
    }

    std::optional<std::shared_ptr<TissDB::Document>> get(const std::string& collection_name, const std::string& key, TissDB::Transactions::TransactionID tid = -1) override {
        (void)tid; // Unused in mock
        if (mock_data_.count(collection_name) && mock_data_[collection_name].count(key)) {
            return std::make_shared<TissDB::Document>(mock_data_[collection_name][key]);
        }
        return std::nullopt;
    }

    std::vector<TissDB::Document> scan(const std::string& collection_name) override {
        std::vector<TissDB::Document> docs;
        if (mock_data_.count(collection_name)) {
            for (const auto& pair : mock_data_[collection_name]) {
                docs.push_back(pair.second);
            }
        }
        return docs;
    }

    // Override create_index to just record that an index was created
    void create_index(const std::string& collection_name, const std::vector<std::string>& field_names) override {
        mock_indexes_[collection_name].insert(field_names[0]); // Simple mock: only use first field
    }

    std::vector<std::string> find_by_index(const std::string& collection_name, const std::string& field_name, const std::string& value) override {
        std::vector<std::string> result_ids;
        if (mock_indexes_.count(collection_name) && mock_indexes_[collection_name].count(field_name)) {
            if (mock_data_.count(collection_name)) {
                for (const auto& pair : mock_data_[collection_name]) {
                    for (const auto& elem : pair.second.elements) {
                        if (elem.key == field_name) {
                            if (auto* str_val = std::get_if<std::string>(&elem.value)) {
                                if (*str_val == value) {
                                    result_ids.push_back(pair.first);
                                }
                            }
                        }
                    }
                }
            }
        }
        return result_ids;
    }

    // Mock data storage
    std::map<std::string, std::map<std::string, TissDB::Document>> mock_data_;
    std::map<std::string, std::set<std::string>> mock_indexes_;
};

TEST_CASE(ExecutorSelectAll) {
    MockLSMTree mock_lsm_tree;
    TissDB::Schema empty_schema;
    mock_lsm_tree.create_collection("users", TissDB::Schema{});

    mock_lsm_tree.put("users", "user1", TissDB::Document{"user1", {{"name", std::string("Alice")}}});
    mock_lsm_tree.put("users", "user2", TissDB::Document{"user2", {{"name", std::string("Bob")}}});

    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT * FROM users");

    TissDB::Query::Executor executor(mock_lsm_tree);
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(2, result.size());
    // Check content (order might vary)
    bool found_user1 = false;
    bool found_user2 = false;
    for (const auto& doc : result) {
        if (doc.id == "user1") found_user1 = true;
        if (doc.id == "user2") found_user2 = true;
    }
    ASSERT_TRUE(found_user1);
    ASSERT_TRUE(found_user2);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorUpdateReturnValue) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("users", TissDB::Schema{});
    mock_lsm_tree.put("users", "1", TissDB::Document{"1", {{"age", 20.0}}});
    mock_lsm_tree.put("users", "2", TissDB::Document{"2", {{"age", 30.0}}});
    mock_lsm_tree.put("users", "3", TissDB::Document{"3", {{"age", 40.0}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("UPDATE users SET age = 21 WHERE age > 25");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(1, result.size());
    const auto& summary_doc = result[0];
    ASSERT_EQ(1, summary_doc.elements.size());
    ASSERT_EQ("updated_count", summary_doc.elements[0].key);
    ASSERT_EQ(2.0, std::get<double>(summary_doc.elements[0].value));
}

TEST_CASE(ExecutorDeleteReturnValue) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("users", TissDB::Schema{});
    mock_lsm_tree.put("users", "1", TissDB::Document{"1", {{"status", std::string("active")}}});
    mock_lsm_tree.put("users", "2", TissDB::Document{"2", {{"status", std::string("inactive")}}});
    mock_lsm_tree.put("users", "3", TissDB::Document{"3", {{"status", std::string("inactive")}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("DELETE FROM users WHERE status = 'inactive'");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(1, result.size());
    const auto& summary_doc = result[0];
    ASSERT_EQ(1, summary_doc.elements.size());
    ASSERT_EQ("deleted_count", summary_doc.elements[0].key);
    ASSERT_EQ(2.0, std::get<double>(summary_doc.elements[0].value));
}

TEST_CASE(ExecutorUpdateModifyValue) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("users", TissDB::Schema{});
    mock_lsm_tree.put("users", "user1", TissDB::Document{"user1", {{"level", 5.0}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("UPDATE users SET level = 6.0 WHERE level = 5.0");
    executor.execute(ast);

    auto updated_doc_opt = mock_lsm_tree.get("users", "user1");
    ASSERT_TRUE(updated_doc_opt.has_value());
    const auto& updated_doc = updated_doc_opt.value();
    for (const auto& elem : updated_doc->elements) {
        if (elem.key == "level") {
            ASSERT_EQ(std::get<double>(elem.value), 6.0);
        }
    }
}

TEST_CASE(ExecutorAggregateGroupBy) {
    MockLSMTree mock_lsm_tree;
    TissDB::Schema empty_schema;
    mock_lsm_tree.create_collection("sales", empty_schema);

    // Setup initial data
    mock_lsm_tree.put("sales", "1", TissDB::Document{"1", {{"category", std::string("books")}, {"amount", 15.0}}});
    mock_lsm_tree.put("sales", "2", TissDB::Document{"2", {{"category", std::string("electronics")}, {"amount", 100.0}}});
    mock_lsm_tree.put("sales", "3", TissDB::Document{"3", {{"category", std::string("books")}, {"amount", 25.0}}});
    mock_lsm_tree.put("sales", "4", TissDB::Document{"4", {{"category", std::string("electronics")}, {"amount", 150.0}}});
    mock_lsm_tree.put("sales", "5", TissDB::Document{"5", {{"category", std::string("books")}, {"amount", 20.0}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    // Execute the query
    TissDB::Query::AST ast = parser.parse("SELECT category, SUM(amount), COUNT(amount) FROM sales GROUP BY category");
    TissDB::Query::QueryResult result = executor.execute(ast);

    // Verify the results
    ASSERT_EQ(2, result.size()); // Two groups: books and electronics

    bool found_books = false;
    bool found_electronics = false;

    for (const auto& doc : result) {
        bool is_books = false;
        for(const auto& el : doc.elements) {
            if(el.key == "category" && std::get<std::string>(el.value) == "books") {
                is_books = true;
                break;
            }
        }

        if (is_books) {
            found_books = true;
            ASSERT_EQ(3, doc.elements.size()); // category, SUM(amount), COUNT(amount)
            for (const auto& elem : doc.elements) {
                if (elem.key == "SUM(amount)") ASSERT_EQ(std::get<double>(elem.value), 60.0);
                if (elem.key == "COUNT(amount)") ASSERT_EQ(std::get<double>(elem.value), 3.0);
            }
        } else {
            found_electronics = true;
            ASSERT_EQ(3, doc.elements.size()); // category, SUM(amount), COUNT(amount)
            for (const auto& elem : doc.elements) {
                if (elem.key == "SUM(amount)") ASSERT_EQ(std::get<double>(elem.value), 250.0);
                if (elem.key == "COUNT(amount)") ASSERT_EQ(std::get<double>(elem.value), 2.0);
            }
        }
    }

    ASSERT_TRUE(found_books);
    ASSERT_TRUE(found_electronics);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorAggregateNoGroupBy) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("sales", TissDB::Schema{});

    mock_lsm_tree.put("sales", "1", TissDB::Document{"1", {{"amount", 10.0}}});
    mock_lsm_tree.put("sales", "2", TissDB::Document{"2", {{"amount", 20.0}}});
    mock_lsm_tree.put("sales", "3", TissDB::Document{"3", {{"amount", 30.0}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("SELECT SUM(amount), AVG(amount) FROM sales");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(1, result.size());
    const auto& doc = result[0];
    ASSERT_EQ(2, doc.elements.size());

    bool found_sum = false;
    bool found_avg = false;
    for (const auto& elem : doc.elements) {
        if (elem.key == "SUM(amount)") {
            ASSERT_EQ(std::get<double>(elem.value), 60.0);
            found_sum = true;
        }
        if (elem.key == "AVG(amount)") {
            ASSERT_EQ(std::get<double>(elem.value), 20.0);
            found_avg = true;
        }
    }
    ASSERT_TRUE(found_sum);
    ASSERT_TRUE(found_avg);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorAggregateCountStar) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("users", TissDB::Schema{});

    mock_lsm_tree.put("users", "1", TissDB::Document{"1", {{"name", std::string("A")}}});
    mock_lsm_tree.put("users", "2", TissDB::Document{"2", {{"name", std::string("B")}}});
    mock_lsm_tree.put("users", "3", TissDB::Document{"3", {{"name", std::string("C")}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("SELECT COUNT(*) FROM users");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(1, result.size());
    const auto& doc = result[0];
    ASSERT_EQ(1, doc.elements.size());
    ASSERT_EQ(doc.elements[0].key, "COUNT(*)");
    ASSERT_EQ(std::get<double>(doc.elements[0].value), 3.0);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorAggregateEmptyResult) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("sales", TissDB::Schema{});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("SELECT SUM(amount) FROM sales WHERE amount > 100");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(1, result.size());
    const auto& doc = result[0];
    ASSERT_EQ(1, doc.elements.size());
    ASSERT_EQ(doc.elements[0].key, "SUM(amount)");
    ASSERT_EQ(std::get<double>(doc.elements[0].value), 0.0); // SUM of empty set is 0

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorDeleteAll) {
    MockLSMTree mock_lsm_tree;
    TissDB::Schema empty_schema;
    mock_lsm_tree.create_collection("users", empty_schema);

    // 1. Setup initial data
    mock_lsm_tree.put("users", "user1", TissDB::Document{"user1", {{"name", std::string("Victor")}}});
    mock_lsm_tree.put("users", "user2", TissDB::Document{"user2", {{"name", std::string("Wendy")}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    // 2. Execute the DELETE query
    TissDB::Query::AST ast = parser.parse("DELETE FROM users");
    executor.execute(ast);

    // 3. Verify the data was deleted from mock storage
    ASSERT_EQ(0, mock_lsm_tree.mock_data_["users"].size());

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorDeleteWithWhere) {
    MockLSMTree mock_lsm_tree;
    TissDB::Schema empty_schema;
    mock_lsm_tree.create_collection("users", empty_schema);

    // 1. Setup initial data
    mock_lsm_tree.put("users", "user_to_delete", TissDB::Document{"user_to_delete", {{"name", std::string("Mallory")}}});
    mock_lsm_tree.put("users", "user_to_keep", TissDB::Document{"user_to_keep", {{"name", std::string("Trudy")}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    // 2. Execute the DELETE query
    TissDB::Query::AST ast = parser.parse("DELETE FROM users WHERE name = 'Mallory'");
    executor.execute(ast);

    // 3. Verify the data was deleted from mock storage
    auto deleted_doc_opt = mock_lsm_tree.get("users", "user_to_delete");
    ASSERT_FALSE(deleted_doc_opt.has_value());

    // 4. Verify that other documents were not affected
    auto other_doc_opt = mock_lsm_tree.get("users", "user_to_keep");
    ASSERT_TRUE(other_doc_opt.has_value());

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorUpdateAddField) {
    MockLSMTree mock_lsm_tree;
    TissDB::Schema empty_schema;
    mock_lsm_tree.create_collection("users", empty_schema);

    mock_lsm_tree.put("users", "user1", TissDB::Document{"user1", {{"name", std::string("Frank")}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("UPDATE users SET status = 'active' WHERE name = 'Frank'");
    executor.execute(ast);

    auto updated_doc_opt = mock_lsm_tree.get("users", "user1");
    ASSERT_TRUE(updated_doc_opt.has_value());
    const auto& updated_doc = updated_doc_opt.value();

    ASSERT_EQ(2, updated_doc->elements.size()); // name and status
    bool status_is_added = false;
    for (const auto& elem : updated_doc->elements) {
        if (elem.key == "status") {
            if (auto* str_val = std::get_if<std::string>(&elem.value)) {
                if (*str_val == "active") {
                    status_is_added = true;
                }
            }
        }
    }
    ASSERT_TRUE(status_is_added);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorUpdateAll) {
    MockLSMTree mock_lsm_tree;
    TissDB::Schema empty_schema;
    mock_lsm_tree.create_collection("users", empty_schema);

    mock_lsm_tree.put("users", "user1", TissDB::Document{"user1", {{"name", std::string("Grace")}, {"level", 5.0}}});
    mock_lsm_tree.put("users", "user2", TissDB::Document{"user2", {{"name", std::string("Heidi")}, {"level", 6.0}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("UPDATE users SET level = 10.0");
    executor.execute(ast);

    // Verify doc1 was updated
    auto updated_doc1_opt = mock_lsm_tree.get("users", "user1");
    ASSERT_TRUE(updated_doc1_opt.has_value());
    for (const auto& elem : updated_doc1_opt.value()->elements) {
        if (elem.key == "level") {
            ASSERT_EQ(std::get<double>(elem.value), 10.0);
        }
    }

    // Verify doc2 was updated
    auto updated_doc2_opt = mock_lsm_tree.get("users", "user2");
    ASSERT_TRUE(updated_doc2_opt.has_value());
    for (const auto& elem : updated_doc2_opt.value()->elements) {
        if (elem.key == "level") {
            ASSERT_EQ(std::get<double>(elem.value), 10.0);
        }
    }

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorUpdateWithWhere) {
    MockLSMTree mock_lsm_tree;
    TissDB::Schema empty_schema;
    mock_lsm_tree.create_collection("users", empty_schema);

    // 1. Setup initial data
    mock_lsm_tree.put("users", "user1", TissDB::Document{"user1", {{"name", std::string("David")}, {"age", 40.0}}});
    mock_lsm_tree.put("users", "user2", TissDB::Document{"user2", {{"name", std::string("Eve")}, {"age", 50.0}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    // 2. Execute the UPDATE query
    TissDB::Query::AST ast = parser.parse("UPDATE users SET age = 41.0 WHERE name = 'David'");
    executor.execute(ast);

    // 3. Verify the data was updated in mock storage
    auto updated_doc_opt = mock_lsm_tree.get("users", "user1");
    ASSERT_TRUE(updated_doc_opt.has_value());
    const auto& updated_doc = updated_doc_opt.value();

    bool age_is_updated = false;
    for (const auto& elem : updated_doc->elements) {
        if (elem.key == "age") {
            if (auto* num_val = std::get_if<double>(&elem.value)) {
                if (*num_val == 41.0) {
                    age_is_updated = true;
                }
            }
        }
    }
    ASSERT_TRUE(age_is_updated);

    // 4. Verify that other documents were not affected
    auto other_doc_opt = mock_lsm_tree.get("users", "user2");
    ASSERT_TRUE(other_doc_opt.has_value());
    const auto& other_doc = *other_doc_opt.value();
     for (const auto& elem : other_doc.elements) {
        if (elem.key == "age") {
            if (auto* num_val = std::get_if<double>(&elem.value)) {
                ASSERT_EQ(*num_val, 50.0);
            }
        }
    }

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorInsert) {
    MockLSMTree mock_lsm_tree;
    TissDB::Schema empty_schema;
    mock_lsm_tree.create_collection("users", empty_schema);

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    // 1. Execute the INSERT query
    TissDB::Query::AST ast = parser.parse("INSERT INTO users (name, age) VALUES ('Charlie', 30.0)");
    TissDB::Query::QueryResult result = executor.execute(ast);

    // 2. INSERT should return an empty result, but the QueryResult struct is not designed for that.
    // We will check the side-effect in the mock storage instead.

    // 3. Verify the data was inserted into the mock storage
    const auto& users_collection = mock_lsm_tree.mock_data_["users"];
    ASSERT_EQ(1, users_collection.size());

    // 4. Verify the content of the inserted document
    const auto& inserted_doc = users_collection.begin()->second;
    ASSERT_EQ(2, inserted_doc.elements.size());

    bool found_name = false;
    bool found_age = false;
    for (const auto& elem : inserted_doc.elements) {
        if (elem.key == "name") {
            if (auto* str_val = std::get_if<std::string>(&elem.value)) {
                if (*str_val == "Charlie") {
                    found_name = true;
                }
            }
        } else if (elem.key == "age") {
            if (auto* num_val = std::get_if<double>(&elem.value)) {
                if (*num_val == 30.0) {
                    found_age = true;
                }
            }
        }
    }
    ASSERT_TRUE(found_name);
    ASSERT_TRUE(found_age);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorSelectWithWhere) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("products", TissDB::Schema{});

    mock_lsm_tree.put("products", "prod1", TissDB::Document{"prod1", {{"price", 150.0}}});
    mock_lsm_tree.put("products", "prod2", TissDB::Document{"prod2", {{"price", 50.0}}});

    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT * FROM products WHERE price > 100");

    TissDB::Query::Executor executor(mock_lsm_tree);
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(1, result.size());
    ASSERT_EQ("prod1", result[0].id);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorSelectWithLike) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("users", TissDB::Schema{});

    mock_lsm_tree.put("users", "user1", TissDB::Document{"user1", {{"name", std::string("Alice")}}});
    mock_lsm_tree.put("users", "user2", TissDB::Document{"user2", {{"name", std::string("Bob")}}});
    mock_lsm_tree.put("users", "user3", TissDB::Document{"user3", {{"name", std::string("Charlie")}}});
    mock_lsm_tree.put("users", "user4", TissDB::Document{"user4", {{"name", std::string("Alicia")}}});


    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    // Test case 1: Starts with 'Ali'
    TissDB::Query::AST ast1 = parser.parse("SELECT name FROM users WHERE name LIKE 'Ali%'");
    TissDB::Query::QueryResult result1 = executor.execute(ast1);
    ASSERT_EQ(2, result1.size());

    // Test case 2: Ends with 'e'
    TissDB::Query::AST ast2 = parser.parse("SELECT name FROM users WHERE name LIKE '%e'");
    TissDB::Query::QueryResult result2 = executor.execute(ast2);
    ASSERT_EQ(2, result2.size()); // Alice, Charlie

    // Test case 3: Contains 'li'
    TissDB::Query::AST ast3 = parser.parse("SELECT name FROM users WHERE name LIKE '%li%'");
    TissDB::Query::QueryResult result3 = executor.execute(ast3);
    ASSERT_EQ(2, result3.size());

    // Test case 4: Single character wildcard
    TissDB::Query::AST ast4 = parser.parse("SELECT name FROM users WHERE name LIKE 'Ali_e'");
    TissDB::Query::QueryResult result4 = executor.execute(ast4);
    ASSERT_EQ(1, result4.size());
    ASSERT_EQ("Alice", std::get<std::string>(result4[0].elements[0].value));

    // Test case 5: No wildcards (exact match)
    TissDB::Query::AST ast5 = parser.parse("SELECT name FROM users WHERE name LIKE 'Bob'");
    TissDB::Query::QueryResult result5 = executor.execute(ast5);
    ASSERT_EQ(1, result5.size());
    ASSERT_EQ("Bob", std::get<std::string>(result5[0].elements[0].value));

    // Test case 6: No match
    TissDB::Query::AST ast6 = parser.parse("SELECT name FROM users WHERE name LIKE 'D%'");
    TissDB::Query::QueryResult result6 = executor.execute(ast6);
    ASSERT_EQ(0, result6.size());
}

TEST_CASE(ExecutorSelectWithIndex) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("users", TissDB::Schema{});
    mock_lsm_tree.create_index("users", {"name"});

    mock_lsm_tree.put("users", "user1", TissDB::Document{"user1", {{"name", std::string("Alice")}}});
    mock_lsm_tree.put("users", "user2", TissDB::Document{"user2", {{"name", std::string("Bob")}}});

    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT * FROM users WHERE name = 'Alice'");

    TissDB::Query::Executor executor(mock_lsm_tree);
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(1, result.size());
    ASSERT_EQ("user1", result[0].id);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorSelectWithAnd) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("users", TissDB::Schema{});

    mock_lsm_tree.put("users", "user1", TissDB::Document{"user1", {{"name", std::string("Alice")}, {"age", 30.0}, {"city", std::string("New York")}}});
    mock_lsm_tree.put("users", "user2", TissDB::Document{"user2", {{"name", std::string("Bob")}, {"age", 40.0}, {"city", std::string("New York")}}});
    mock_lsm_tree.put("users", "user3", TissDB::Document{"user3", {{"name", std::string("Charlie")}, {"age", 40.0}, {"city", std::string("London")}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("SELECT * FROM users WHERE age = 40.0 AND city = 'New York'");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(1, result.size());
    ASSERT_EQ("user2", result[0].id);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorSelectWithOr) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("users", TissDB::Schema{});

    mock_lsm_tree.put("users", "user1", TissDB::Document{"user1", {{"name", std::string("Alice")}, {"city", std::string("New York")}}});
    mock_lsm_tree.put("users", "user2", TissDB::Document{"user2", {{"name", std::string("Bob")}, {"city", std::string("London")}}});
    mock_lsm_tree.put("users", "user3", TissDB::Document{"user3", {{"name", std::string("Charlie")}, {"city", std::string("Paris")}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("SELECT * FROM users WHERE city = 'New York' OR city = 'London'");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(2, result.size());
    bool found_user1 = false;
    bool found_user2 = false;
    for (const auto& doc : result) {
        if (doc.id == "user1") found_user1 = true;
        if (doc.id == "user2") found_user2 = true;
    }
    ASSERT_TRUE(found_user1);
    ASSERT_TRUE(found_user2);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorSelectOrderBy) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("users", TissDB::Schema{});

    mock_lsm_tree.put("users", "user1", TissDB::Document{"user1", {{"age", 30.0}}});
    mock_lsm_tree.put("users", "user2", TissDB::Document{"user2", {{"age", 20.0}}});
    mock_lsm_tree.put("users", "user3", TissDB::Document{"user3", {{"age", 40.0}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    // Test ASC
    TissDB::Query::AST ast_asc = parser.parse("SELECT * FROM users ORDER BY age ASC");
    TissDB::Query::QueryResult result_asc = executor.execute(ast_asc);

    ASSERT_EQ(3, result_asc.size());
    ASSERT_EQ("user2", result_asc[0].id);
    ASSERT_EQ("user1", result_asc[1].id);
    ASSERT_EQ("user3", result_asc[2].id);

    // Test DESC
    TissDB::Query::AST ast_desc = parser.parse("SELECT * FROM users ORDER BY age DESC");
    TissDB::Query::QueryResult result_desc = executor.execute(ast_desc);

    ASSERT_EQ(3, result_desc.size());
    ASSERT_EQ("user3", result_desc[0].id);
    ASSERT_EQ("user1", result_desc[1].id);
    ASSERT_EQ("user2", result_desc[2].id);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorSelectLimit) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("users", TissDB::Schema{});

    mock_lsm_tree.put("users", "user1", TissDB::Document{"user1", {{"name", std::string("A")}}});
    mock_lsm_tree.put("users", "user2", TissDB::Document{"user2", {{"name", std::string("B")}}});
    mock_lsm_tree.put("users", "user3", TissDB::Document{"user3", {{"name", std::string("C")}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("SELECT * FROM users LIMIT 2");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(2, result.size());

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorSelectNotEqual) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("users", TissDB::Schema{});

    mock_lsm_tree.put("users", "user1", TissDB::Document{"user1", {{"status", std::string("active")}}});
    mock_lsm_tree.put("users", "user2", TissDB::Document{"user2", {{"status", std::string("inactive")}}});
    mock_lsm_tree.put("users", "user3", TissDB::Document{"user3", {{"status", std::string("active")}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("SELECT * FROM users WHERE status != 'active'");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(1, result.size());
    ASSERT_EQ("user2", result[0].id);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorSelectLessThanOrEqual) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("products", TissDB::Schema{});

    mock_lsm_tree.put("products", "prod1", TissDB::Document{"prod1", {{"price", 10.0}}});
    mock_lsm_tree.put("products", "prod2", TissDB::Document{"prod2", {{"price", 20.0}}});
    mock_lsm_tree.put("products", "prod3", TissDB::Document{"prod3", {{"price", 30.0}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("SELECT * FROM products WHERE price <= 20.0");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(2, result.size());
    bool found_prod1 = false;
    bool found_prod2 = false;
    for (const auto& doc : result) {
        if (doc.id == "prod1") found_prod1 = true;
        if (doc.id == "prod2") found_prod2 = true;
    }
    ASSERT_TRUE(found_prod1);
    ASSERT_TRUE(found_prod2);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorSelectGreaterThanOrEqual) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("products", TissDB::Schema{});

    mock_lsm_tree.put("products", "prod1", TissDB::Document{"prod1", {{"price", 10.0}}});
    mock_lsm_tree.put("products", "prod2", TissDB::Document{"prod2", {{"price", 20.0}}});
    mock_lsm_tree.put("products", "prod3", TissDB::Document{"prod3", {{"price", 30.0}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("SELECT * FROM products WHERE price >= 20.0");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(2, result.size());
    bool found_prod2 = false;
    bool found_prod3 = false;
    for (const auto& doc : result) {
        if (doc.id == "prod2") found_prod2 = true;
        if (doc.id == "prod3") found_prod3 = true;
    }
    ASSERT_TRUE(found_prod2);
    ASSERT_TRUE(found_prod3);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorSelectWithNot) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("users", TissDB::Schema{});

    mock_lsm_tree.put("users", "user1", TissDB::Document{"user1", {{"is_admin", true}}});
    mock_lsm_tree.put("users", "user2", TissDB::Document{"user2", {{"is_admin", false}}});
    mock_lsm_tree.put("users", "user3", TissDB::Document{"user3", {{"is_admin", true}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("SELECT * FROM users WHERE NOT is_admin");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(1, result.size());
    ASSERT_EQ("user2", result[0].id);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorAggregateMinMax) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("data", TissDB::Schema{});

    mock_lsm_tree.put("data", "1", TissDB::Document{"1", {{"value", 10.0}}});
    mock_lsm_tree.put("data", "2", TissDB::Document{"2", {{"value", 5.0}}});
    mock_lsm_tree.put("data", "3", TissDB::Document{"3", {{"value", 15.0}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("SELECT MIN(value), MAX(value) FROM data");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(1, result.size());
    const auto& doc = result[0];
    ASSERT_EQ(2, doc.elements.size());

    bool found_min = false;
    bool found_max = false;
    for (const auto& elem : doc.elements) {
        if (elem.key == "MIN(value)") {
            ASSERT_EQ(std::get<double>(elem.value), 5.0);
            found_min = true;
        }
        if (elem.key == "MAX(value)") {
            ASSERT_EQ(std::get<double>(elem.value), 15.0);
            found_max = true;
        }
    }
    ASSERT_TRUE(found_min);
    ASSERT_TRUE(found_max);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorSelectSpecificFields) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("users", TissDB::Schema{});

    mock_lsm_tree.put("users", "user1", TissDB::Document{"user1", {{"name", std::string("Alice")}, {"age", 30.0}, {"city", std::string("New York")}}});
    mock_lsm_tree.put("users", "user2", TissDB::Document{"user2", {{"name", std::string("Bob")}, {"age", 25.0}, {"city", std::string("London")}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("SELECT name, city FROM users WHERE age > 28");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(1, result.size());
    const auto& doc = result[0];
    ASSERT_EQ(2, doc.elements.size()); // Should only have name and city

    bool found_name = false;
    bool found_city = false;
    for (const auto& elem : doc.elements) {
        if (elem.key == "name") {
            ASSERT_EQ(std::get<std::string>(elem.value), "Alice");
            found_name = true;
        }
        if (elem.key == "city") {
            ASSERT_EQ(std::get<std::string>(elem.value), "New York");
            found_city = true;
        }
    }
    ASSERT_TRUE(found_name);
    ASSERT_TRUE(found_city);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorAggregateHaving) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("orders", TissDB::Schema{});

    mock_lsm_tree.put("orders", "1", TissDB::Document{"1", {{"customer_id", std::string("A")}, {"amount", 10.0}}});
    mock_lsm_tree.put("orders", "2", TissDB::Document{"2", {{"customer_id", std::string("B")}, {"amount", 50.0}}});
    mock_lsm_tree.put("orders", "3", TissDB::Document{"3", {{"customer_id", std::string("A")}, {"amount", 20.0}}});
    mock_lsm_tree.put("orders", "4", TissDB::Document{"4", {{"customer_id", std::string("C")}, {"amount", 5.0}}});
    mock_lsm_tree.put("orders", "5", TissDB::Document{"5", {{"customer_id", std::string("B")}, {"amount", 30.0}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("SELECT customer_id, SUM(amount) FROM orders GROUP BY customer_id HAVING SUM(amount) > 40.0");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(1, result.size());
    const auto& doc = result[0];
    ASSERT_EQ(2, doc.elements.size());

    bool found_customer_id = false;
    bool found_sum_amount = false;
    for (const auto& elem : doc.elements) {
        if (elem.key == "customer_id") {
            ASSERT_EQ(std::get<std::string>(elem.value), "B");
            found_customer_id = true;
        }
        if (elem.key == "SUM(amount)") {
            ASSERT_EQ(std::get<double>(elem.value), 80.0); // 50 + 30
            found_sum_amount = true;
        }
    }
    ASSERT_TRUE(found_customer_id);
    ASSERT_TRUE(found_sum_amount);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorSelectDistinct) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("products", TissDB::Schema{});

    mock_lsm_tree.put("products", "1", TissDB::Document{"1", {{"category", std::string("electronics")}}});
    mock_lsm_tree.put("products", "2", TissDB::Document{"2", {{"category", std::string("books")}}});
    mock_lsm_tree.put("products", "3", TissDB::Document{"3", {{"category", std::string("electronics")}}});
    mock_lsm_tree.put("products", "4", TissDB::Document{"4", {{"category", std::string("books")}}});
    mock_lsm_tree.put("products", "5", TissDB::Document{"5", {{"category", std::string("clothing")}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    TissDB::Query::AST ast = parser.parse("SELECT DISTINCT category FROM products");
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(3, result.size());
    std::set<std::string> categories;
    for (const auto& doc : result) {
        for (const auto& elem : doc.elements) {
            if (elem.key == "category") {
                categories.insert(std::get<std::string>(elem.value));
            }
        }
    }
    ASSERT_TRUE(categories.count("electronics"));
    ASSERT_TRUE(categories.count("books"));
    ASSERT_TRUE(categories.count("clothing"));
    ASSERT_EQ(3, categories.size());

    std::filesystem::remove_all("mock_data");
}
