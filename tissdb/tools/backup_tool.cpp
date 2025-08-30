#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <stdexcept>

// NOTE FOR USER:
// This tool is designed to be compiled as a separate utility.
// It requires C++17 for the <filesystem> library.
//
// Example compilation command:
// g++ -std=c++17 -o backup_tool backup_tool.cpp
//
// The master key for the Key Management System (KMS) is not handled here.
// In a real system, the backup process must also include a secure way to
// back up and restore the master key itself, as the data backups are
// useless without it.

namespace fs = std::filesystem;

void print_usage() {
    std::cout << "TissDB Secure Backup Utility" << std::endl;
    std::cout << "----------------------------" << std::endl;
    std::cout << "Usage: backup_tool <command> <args...>" << std::endl;
    std::cout << "\nCommands:" << std::endl;
    std::cout << "  backup <data_directory> <backup_directory>" << std::endl;
    std::cout << "    - Creates a backup of .db and .log files." << std::endl;
    std::cout << "  restore <backup_directory> <data_directory>" << std::endl;
    std::cout << "    - Restores files from a backup. Warning: Overwrites destination files." << std::endl;
}

void backup(const fs::path& data_dir, const fs::path& backup_dir) {
    if (!fs::is_directory(data_dir)) {
        throw std::runtime_error("Source data directory does not exist or is not a directory: " + data_dir.string());
    }
    if (!fs::exists(backup_dir)) {
        std::cout << "Backup directory does not exist. Creating it: " << backup_dir.string() << std::endl;
        fs::create_directories(backup_dir);
    }

    std::cout << "Starting backup..." << std::endl;
    int files_copied = 0;
    for (const auto& entry : fs::directory_iterator(data_dir)) {
        const auto& path = entry.path();
        if (entry.is_regular_file() && (path.extension() == ".db" || path.extension() == ".log")) {
            fs::copy(path, backup_dir / path.filename(), fs::copy_options::overwrite_existing);
            std::cout << "  - Copied " << path.filename().string() << std::endl;
            files_copied++;
        }
    }
    std::cout << "Backup complete. " << files_copied << " files copied." << std::endl;
}

void restore(const fs::path& backup_dir, const fs::path& data_dir) {
    if (!fs::is_directory(backup_dir)) {
        throw std::runtime_error("Source backup directory does not exist or is not a directory: " + backup_dir.string());
    }
    if (!fs::exists(data_dir)) {
        std::cout << "Data directory does not exist. Creating it: " << data_dir.string() << std::endl;
        fs::create_directories(data_dir);
    }

    std::cout << "Starting restore..." << std::endl;
    std::cout << "WARNING: This will overwrite existing files in " << data_dir.string() << std::endl;
    int files_restored = 0;
    for (const auto& entry : fs::directory_iterator(backup_dir)) {
        const auto& path = entry.path();
        if (entry.is_regular_file()) {
            fs::copy(path, data_dir / path.filename(), fs::copy_options::overwrite_existing);
            std::cout << "  - Restored " << path.filename().string() << std::endl;
            files_restored++;
        }
    }
    std::cout << "Restore complete. " << files_restored << " files restored." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        print_usage();
        return 1;
    }

    std::string command = argv[1];
    fs::path source = argv[2];
    fs::path destination = argv[3];

    try {
        if (command == "backup") {
            backup(source, destination);
        } else if (command == "restore") {
            // In a restore, the backup dir is the source
            restore(source, destination);
        } else {
            std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
            print_usage();
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
