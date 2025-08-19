#include "transaction_manager.h"
#include "lsm_tree.h"
#include "wal.h"
#include <stdexcept>

namespace TissDB {
namespace Transactions {

TransactionID TransactionManager::begin_transaction() {
    std::lock_guard<std::mutex> lock(mutex_);
    TransactionID new_tid = next_transaction_id_++;
    transactions_[new_tid] = std::make_unique<Transaction>(new_tid);
    return new_tid;
}

bool TransactionManager::commit_transaction(TransactionID tid) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = transactions_.find(tid);
    if (it == transactions_.end() || it->second->get_state() != Transaction::State::ACTIVE) {
        return false;
    }

    auto& transaction = it->second;
    const auto& operations = transaction->get_operations();

    // 1. Write to WAL first
    Storage::LogEntry commit_entry;
    commit_entry.type = Storage::LogEntryType::TXN_COMMIT;
    commit_entry.transaction_id = tid;
    commit_entry.operations = operations;
    lsm_tree_.get_wal().append(commit_entry);

    // 2. Apply to in-memory store
    for (const auto& op : operations) {
        if (op.type == OperationType::PUT) {
            lsm_tree_.put(op.collection_name, op.key, op.doc, -1);
        } else if (op.type == OperationType::DELETE) {
            lsm_tree_.del(op.collection_name, op.key, -1);
        }
    }

    transaction->set_state(Transaction::State::COMMITTED);
    transactions_.erase(it);
    return true;
}

bool TransactionManager::rollback_transaction(TransactionID tid) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = transactions_.find(tid);
    if (it == transactions_.end() || it->second->get_state() != Transaction::State::ACTIVE) {
        return true; // Idempotent
    }

    Storage::LogEntry abort_entry;
    abort_entry.type = Storage::LogEntryType::TXN_ABORT;
    abort_entry.transaction_id = tid;
    lsm_tree_.get_wal().append(abort_entry);

    it->second->set_state(Transaction::State::ABORTED);
    transactions_.erase(it);
    return true;
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