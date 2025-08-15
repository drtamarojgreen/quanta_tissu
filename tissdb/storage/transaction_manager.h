#ifndef TISSDB_TRANSACTION_MANAGER_H
#define TISSDB_TRANSACTION_MANAGER_H

#include <atomic>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace TissDB {
namespace Transactions {

// Represents a transaction ID
typedef int TransactionID;

// Enum for transaction states
enum class TransactionState {
    ACTIVE,
    COMMITTED,
    ABORTED
};

// Forward declaration of storage engine (LSMTree) if needed for interaction
// namespace Storage { class LSMTree; }

class TransactionManager {
public:
    TransactionManager();

    // Begins a new transaction, returns a TransactionID
    TransactionID beginTransaction();

    // Commits a transaction
    bool commitTransaction(TransactionID tid);

    // Aborts a transaction
    bool abortTransaction(TransactionID tid);

    // Retrieves the state of a transaction
    TransactionState getTransactionState(TransactionID tid) const;

private:
    std::atomic<TransactionID> next_transaction_id_;
    std::unordered_map<TransactionID, TransactionState> transaction_states_;
    std::mutex mutex_;

    // Placeholder for transaction logs or undo/redo information
    // std::vector<TransactionLogEntry> transaction_log_;

    // Concurrency control mechanism (e.g., lock table, timestamp table)
    // std::unordered_map<ResourceID, LockInfo> lock_table_;
};

} // namespace Transactions
} // namespace TissDB

#endif // TISSDB_TRANSACTION_MANAGER_H