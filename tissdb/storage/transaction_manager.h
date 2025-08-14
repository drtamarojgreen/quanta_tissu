#pragma once

#include "../common/document.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace TissDB {
namespace Storage {

enum class OperationType { PUT, DELETE };

struct TransactionOperation {
    OperationType type;
    std::string collection_name;
    std::string key;
    Document doc;
};

class Transaction {
public:
    void add_operation(const TransactionOperation& op);
    const std::vector<TransactionOperation>& get_operations() const;

private:
    std::vector<TransactionOperation> operations_;
};

class TransactionManager {
public:
    TransactionManager();

    int begin_transaction();
    void add_operation(int transaction_id, const TransactionOperation& op);
    void commit_transaction(int transaction_id);
    void rollback_transaction(int transaction_id);

private:
    int next_transaction_id_ = 0;
    std::map<int, std::unique_ptr<Transaction>> transactions_;
};

} // namespace Storage
} // namespace TissDB
