#ifndef TISSDB_AUDIT_LOGGER_H
#define TISSDB_AUDIT_LOGGER_H

#include <string>
#include <vector>
#include <chrono>
#include "../auth/rbac.h" // For Role enum

namespace TissDB {
namespace Audit {

enum class EventType {
    AuthSuccess,
    AuthFailure,
    RequestBegin,
    RequestEnd,
    DbCreate,
    DbDelete,
    CollectionCreate,
    CollectionDelete,
    DocRead,
    DocWrite,
    DocDelete,
    PermissionCheckFailure
};

struct AuditLogEntry {
    std::chrono::system_clock::time_point timestamp;
    std::string user_token_id; // Or a hash of the token
    std::string source_ip;
    EventType event_type;
    std::string resource_accessed; // e.g., db_name/collection_name/doc_id
    bool success;
    std::string description;
};

class AuditLogger {
public:
    // The logger would be initialized with a path to its log file.
    AuditLogger(const std::string& log_file_path);
    ~AuditLogger();

    void log(const AuditLogEntry& entry);

    // Method to retrieve logs for the admin API.
    std::vector<AuditLogEntry> get_logs(
        std::chrono::system_clock::time_point start,
        std::chrono::system_clock::time_point end);

private:
    std::string log_path_;
    // In a real system, a thread-safe queue and a dedicated writer thread
    // would be used to avoid blocking request threads. For now, we'll write directly.
};

} // namespace Audit
} // namespace TissDB

#endif // TISSDB_AUDIT_LOGGER_H
