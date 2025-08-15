#include "transaction_manager.h"
#include <iostream>

namespace TissDB {
namespace Transactions {

TransactionManager::TransactionManager() : next_transaction_id_(1) {
    // Constructor implementation
}

TransactionID TransactionManager::beginTransaction() {
    std::lock_guard<std::mutex> lock(mutex_);
    TransactionID new_tid = next_transaction_id_++;
    transaction_states_[new_tid] = TransactionState::ACTIVE;
    std::cout << "Transaction " << new_tid << " started." << std::endl;
    return new_tid;
}

bool TransactionManager::commitTransaction(TransactionID tid) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (transaction_states_.count(tid) && transaction_states_[tid] == TransactionState::ACTIVE) {
        transaction_states_[tid] = TransactionState::COMMITTED;
        std::cout << "Transaction " << tid << " committed." << std::endl;
        // Here, actual changes would be made durable
        return true;
    }
    std::cerr << "Error: Cannot commit transaction " << tid << ". It's not active or doesn't exist." << std::endl;
    return false;
}

bool TransactionManager::abortTransaction(TransactionID tid) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (transaction_states_.count(tid) && transaction_states_[tid] == TransactionState::ACTIVE) {
        transaction_states_[tid] = TransactionState::ABORTED;
        std::cout << "Transaction " << tid << " aborted." << std::endl;
        // Here, any changes made by the transaction would be rolled back
        return true;
    }
    std::cerr << "Error: Cannot abort transaction " << tid << ". It's not active or doesn't exist." << std::endl;
    return false;
}

TransactionState TransactionManager::getTransactionState(TransactionID tid) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = transaction_states_.find(tid);
    if (it != transaction_states_.end()) {
        return it->second;
    }
    return TransactionState::ABORTED; // Or some other error state
}

} // namespace Transactions
} // namespace TissDB