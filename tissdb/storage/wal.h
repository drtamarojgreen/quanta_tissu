#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <fstream>
#include <vector>
#include <optional>

#include "../common/document.h"
#include "../common/checksum.h"

namespace TissDB {
namespace Storage {

// Defines the type of operation recorded in the log.
enum class LogEntryType : uint8_t {
    PUT,
    DELETE,
    CREATE_COLLECTION,
    DELETE_COLLECTION,
    TXN_COMMIT,
    TXN_ABORT
};

// Represents a single entry in the Write-Ahead Log.
struct LogEntry {
    LogEntryType type;
    int transaction_id = -1;
    std::string collection_name; // Which collection this entry belongs to.
    std::string document_id;
    // For PUT operations, the full document is stored.
    // For DELETE, only the document_id and collection_name are needed.
    Document doc;
    std::vector<TissDB::Transactions::Operation> operations;
    std::optional<std::vector<uint8_t>> schema_data;
};

// Manages the Write-Ahead Log for ensuring durability of writes.
class WriteAheadLog {
public:
    // Creates a WAL instance, opening or creating the log file at the given path.
    explicit WriteAheadLog(const std::string& path);
    ~WriteAheadLog();

    // Appends a write (put/delete) operation to the log file.
    // This must be called before the change is applied to the memtable.
    void append(const LogEntry& entry);

    // Reads the log from disk to reconstruct the state after a crash.
    // Returns a vector of log entries that need to be replayed.
    std::vector<LogEntry> recover();

    // Clears the log file, typically after a successful flush of a memtable to disk.
    void clear();

    // Ensures the log file is properly flushed and closed.
    void shutdown();

private:
    std::string log_path;
    std::ofstream log_file;
};

} // namespace Storage
} // namespace TissDB