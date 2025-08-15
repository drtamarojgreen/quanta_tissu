#include "transaction_manager.h"
#include <stdexcept>

namespace TissDB {
namespace Transactions {

TransactionID TransactionManager::begin_transaction() {
    std::lock_guard<std::mutex> lock(mutex_);
    TransactionID new_tid = next_transaction_id_++;
    transactions_[new_tid] = std::make_unique<Transaction>(new_tid);
    return new_tid;
}

void TransactionManager::commit_transaction(TransactionID tid) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = transactions_.find(tid);
    if (it == transactions_.end() || it->second->get_state() != Transaction::State::ACTIVE) {
        throw std::runtime_error("Cannot commit transaction: not active or does not exist.");
    }
    it->second->set_state(Transaction::State::COMMITTED);
    // In a real system, we might keep committed transactions for a while for recovery.
    // Here we'll just remove it.
    transactions_.erase(it);
}

void TransactionManager::rollback_transaction(TransactionID tid) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = transactions_.find(tid);
    if (it == transactions_.end() || it->second->get_state() != Transaction::State::ACTIVE) {
         // It's okay to roll back a non-existent or already-completed transaction.
        return;
    }
    it->second->set_state(Transaction::State::ABORTED);
    transactions_.erase(it);
}

void TransactionManager::add_put_operation(TransactionID tid, std::string collection, std::string key, Document doc) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = transactions_.find(tid);
    if (it == transactions_.end() || it->second->get_state() != Transaction::State::ACTIVE) {
        throw std::runtime_error("Cannot add operation: transaction is not active.");
    }
    it->second->add_operation({OperationType::PUT, std::move(collection), std::move(key), std::move(doc)});
}

void TransactionManager::add_delete_operation(TransactionID tid, std::string collection, std::string key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = transactions_.find(tid);
    if (it == transactions_.end() || it->second->get_state() != Transaction::State::ACTIVE) {
        throw std::runtime_error("Cannot add operation: transaction is not active.");
    }
    it->second->add_operation({OperationType::DELETE, std::move(collection), std::move(key), {}});
}

const std::unordered_map<TransactionID, std::unique_ptr<Transaction>>& TransactionManager::get_transactions() const {
    return transactions_;
}

} // namespace Transactions
} // namespace TissDB