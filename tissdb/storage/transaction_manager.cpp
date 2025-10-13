#include "transaction_manager.h"
#include "lsm_tree.h"
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

    const auto& operations = it->second->get_operations();
    for (const auto& op : operations) {
        if (op.type == OperationType::PUT) {
            lsm_tree_.put(op.collection_name, op.key, op.doc, -1);
        } else if (op.type == OperationType::DELETE) {
            lsm_tree_.del(op.collection_name, op.key, -1);
        }
    }

    it->second->set_state(Transaction::State::COMMITTED);
    transactions_.erase(it);
    return true;
}

bool TransactionManager::rollback_transaction(TransactionID tid) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = transactions_.find(tid);
    if (it == transactions_.end() || it->second->get_state() != Transaction::State::ACTIVE) {
<<<<<<< HEAD
         // It's okay to roll back a non-existent or already-completed transaction.
        return;
=======
        return true; // Idempotent
>>>>>>> main
    }
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

const Transaction* TransactionManager::get_transaction(TransactionID tid) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = transactions_.find(tid);
    if (it != transactions_.end()) {
        return it->second.get();
    }
    return nullptr;
}

} // namespace Transactions
} // namespace TissDB