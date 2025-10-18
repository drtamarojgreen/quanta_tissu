
#ifndef TISSDB_JOIN_ALGORITHMS_H
#define TISSDB_JOIN_ALGORITHMS_H

#include <vector>
#include <string>
#include "../common/document.h"

namespace TissDB {
namespace Query {

// Forward declarations for potential data structures
// struct TableData; // Represents data from a table/collection

class JoinAlgorithms {
public:
    // Placeholder for Nested Loop Join
    static std::vector<Document> nestedLoopJoin(
        const std::vector<Document>& left_table,
        const std::string& left_collection_name,
        const std::vector<Document>& right_table,
        const std::string& right_collection_name,
        const std::string& left_join_key,
        const std::string& right_join_key
    );

    // Placeholder for Hash Join
    static std::vector<Document> hashJoin(
        const std::vector<Document>& left_table,
        const std::string& left_collection_name,
        const std::vector<Document>& right_table,
        const std::string& right_collection_name,
        const std::string& left_join_key,
        const std::string& right_join_key
    );

    // Placeholder for Sort-Merge Join
    static std::vector<Document> sortMergeJoin(
        std::vector<Document> left_table,
        const std::string& left_collection_name,
        std::vector<Document> right_table,
        const std::string& right_collection_name,
        const std::string& left_join_key,
        const std::string& right_join_key
    );

private:
    // Helper methods for joining, if any
};

} // namespace Query
} // namespace TissDB

#endif // TISSDB_JOIN_ALGORITHMS_H
