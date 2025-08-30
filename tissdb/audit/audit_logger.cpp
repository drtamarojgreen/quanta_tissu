#include "audit_logger.h"
#include <fstream>
#include <iostream>
#include <iomanip> // For std::put_time
#include <sstream>

namespace TissDB {
namespace Audit {

// Helper to convert EventType to string
std::string event_type_to_string(EventType type) {
    switch (type) {
        case EventType::AuthSuccess: return "AuthSuccess";
        case EventType::AuthFailure: return "AuthFailure";
        case EventType::RequestBegin: return "RequestBegin";
        case EventType::RequestEnd: return "RequestEnd";
        case EventType::DbCreate: return "DbCreate";
        case EventType::DbDelete: return "DbDelete";
        case EventType::CollectionCreate: return "CollectionCreate";
        case EventType::CollectionDelete: return "CollectionDelete";
        case EventType::DocRead: return "DocRead";
        case EventType::DocWrite: return "DocWrite";
        case EventType::DocDelete: return "DocDelete";
        case EventType::PermissionCheckFailure: return "PermissionCheckFailure";
        default: return "Unknown";
    }
}

AuditLogger::AuditLogger(const std::string& log_file_path) : log_path_(log_file_path) {}

AuditLogger::~AuditLogger() {}

void AuditLogger::log(const AuditLogEntry& entry) {
    // In a real system, this should be thread-safe.
    // For now, we open, append, and close on each call.
    std::ofstream log_file(log_path_, std::ios::app);
    if (!log_file.is_open()) {
        std::cerr << "CRITICAL: Could not open audit log file: " << log_path_ << std::endl;
        return;
    }

    // Format as JSON for structured logging
    std::time_t time = std::chrono::system_clock::to_time_t(entry.timestamp);
    std::tm tm = *std::gmtime(&time); // Using gmtime for UTC

    log_file << "{";
    log_file << "\"timestamp\":\"" << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ") << "\",";
    log_file << "\"user\":\"" << entry.user_token_id << "\",";
    log_file << "\"source_ip\":\"" << entry.source_ip << "\",";
    log_file << "\"event_type\":\"" << event_type_to_string(entry.event_type) << "\",";
    log_file << "\"resource\":\"" << entry.resource_accessed << "\",";
    log_file << "\"success\":" << (entry.success ? "true" : "false") << ",";
    log_file << "\"description\":\"" << entry.description << "\"";
    log_file << "}" << std::endl; // newline for each entry
}

std::vector<AuditLogEntry> AuditLogger::get_logs(
    std::chrono::system_clock::time_point start,
    std::chrono::system_clock::time_point end) {
    // Placeholder: A real implementation would need to parse the JSON log file
    // and filter by the time range. This is non-trivial and left for a future
    // developer with a test environment.
    throw std::logic_error("AuditLogger::get_logs() is not implemented.");
}

} // namespace Audit
} // namespace TissDB
