#pragma once

#include <atomic>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

#include "../common/document.h"
#include "../common/operation.h"

namespace TissDB {

namespace Storage {
class LSMTree; // Forward declaration
}

namespace Transactions {

using TransactionID = int;

class Transaction {
public:
    enum class State {
        ACTIVE,
        COMMITTED,
        ABORTED
    };

    explicit Transaction(TransactionID id) : id_(id), state_(State::ACTIVE) {}

    void add_operation(Operation op) {
        operations_.push_back(std::move(op));
    }

    TransactionID get_id() const { return id_; }
    State get_state() const { return state_; }
    void set_state(State state) { state_ = state; }
    const std::vector<Operation>& get_operations() const { return operations_; }

private:
    TransactionID id_;
    State state_;
    std::vector<Operation> operations_;
    // In a real system, we'd have locks or other concurrency control info here
};

class TransactionManager {
public:
    explicit TransactionManager(Storage::LSMTree& lsm_tree)
        : lsm_tree_(lsm_tree), next_transaction_id_(1) {}

    TransactionID begin_transaction();
    void commit_transaction(TransactionID tid);
    void rollback_transaction(TransactionID tid);

    void add_put_operation(TransactionID tid, std::string collection, std::string key, Document doc);
    void add_delete_operation(TransactionID tid, std::string collection, std::string key);

    const std::unordered_map<TransactionID, std::unique_ptr<Transaction>>& get_transactions() const;

private:
    Storage::LSMTree& lsm_tree_;
    std::atomic<TransactionID> next_transaction_id_;
    std::unordered_map<TransactionID, std::unique_ptr<Transaction>> transactions_;
    mutable std::mutex mutex_;
};

} // namespace Transactions
} // namespace TissDB