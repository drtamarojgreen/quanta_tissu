#include "transaction_manager.h"

namespace TissDB {
namespace Storage {

// --- Transaction ---

void Transaction::add_operation(const TransactionOperation& op) {
    operations_.push_back(op);
}

const std::vector<TransactionOperation>& Transaction::get_operations() const {
    return operations_;
}

// --- TransactionManager ---

TransactionManager::TransactionManager() = default;

int TransactionManager::begin_transaction() {
    int transaction_id = next_transaction_id_++;
    transactions_[transaction_id] = std::make_unique<Transaction>();
    return transaction_id;
}

void TransactionManager::add_operation(int transaction_id, const TransactionOperation& op) {
    if (transactions_.count(transaction_id)) {
        transactions_[transaction_id]->add_operation(op);
    }
}

void TransactionManager::commit_transaction(int transaction_id) {
    if (transactions_.count(transaction_id)) {
        // In a real implementation, this would apply the operations to the storage engine.
        // For now, we just print the operations.
        for (const auto& op : transactions_[transaction_id]->get_operations()) {
            if (op.type == OperationType::PUT) {
                std::cout << "PUT " << op.key << " in " << op.collection_name << std::endl;
            } else if (op.type == OperationType::DELETE) {
                std::cout << "DELETE " << op.key << " from " << op.collection_name << std::endl;
            }
        }
        transactions_.erase(transaction_id);
    }
}

void TransactionManager::rollback_transaction(int transaction_id) {
    if (transactions_.count(transaction_id)) {
        transactions_.erase(transaction_id);
    }
}

} // namespace Storage
} // namespace TissDB
