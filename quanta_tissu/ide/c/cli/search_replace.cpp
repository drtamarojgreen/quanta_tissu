#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <regex>
#include <filesystem> // C++17 for filesystem operations

// Function to find files matching a glob pattern
// This is a simplified version. A full implementation would require
// more robust glob matching logic or a dedicated library.
std::vector<std::string> find_files(const std::string& pattern) {
    std::vector<std::string> files;
    // Basic implementation: currently just returns the pattern itself if it's a direct file,
    // or needs to be expanded to traverse directories and match globs.
    // For a full glob, one would iterate through directories and check each file/dir name.
    // Example: std::filesystem::recursive_directory_iterator
    // For now, we'll assume a simple file path or a very basic wildcard.
    // A proper glob implementation is complex in C++ without external libraries.
    // We'll just return the pattern as a single file for now, or iterate a directory.
    // A more complete solution would involve parsing the glob pattern and
    // traversing the filesystem accordingly.
    
    // Placeholder: In a real scenario, this would expand the glob.
    // For now, if it's a directory, list its contents. If it's a file, add it.
    // This is a very basic interpretation of --files.
    if (std::filesystem::exists(pattern)) {
        if (std::filesystem::is_regular_file(pattern)) {
            files.push_back(pattern);
        } else if (std::filesystem::is_directory(pattern)) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(pattern)) {
                if (std::filesystem::is_regular_file(entry.path())) {
                    files.push_back(entry.path().string());
                }
            }
        }
    } else {
        // If it's not an existing path, treat it as a glob pattern
        // This part would need a custom glob matcher
        // For now, we'll just add it as is, expecting the caller to handle it
        // or for a more advanced glob implementation to be added here.
        // As a fallback, if the pattern contains wildcards, we'd need to implement
        // a basic wildcard matcher against filenames in the current directory/project.
        // For the purpose of this outline, we'll keep it simple.
        // A robust glob implementation is non-trivial without a library.
        // We'll assume the user provides a direct file or directory for now.
        // Or, if a wildcard is present, we'd need to iterate and match.
        // For example, if pattern is "*.txt", iterate current dir and add .txt files.
        // This is a significant simplification due to "no additional libraries".
        // A proper glob would involve parsing the pattern and traversing the filesystem.
        // For now, we'll just return an empty list if the pattern doesn't exist as a file/dir.
    }
    return files;
}

// Function to perform search and replace in a file
// Returns a tuple: (matches, original_lines, modified_lines)
// modified_lines is empty if no replacement occurred or no changes were made.
std::tuple<bool, std::string, std::string>
search_and_replace_in_file(const std::string& filepath, const std::string& search_pattern_str,
                           const std::string& replace_string, bool is_regex, bool case_sensitive) {
    std::string original_content;
    std::string modified_content;
    bool found_match = false;

    std::ifstream infile(filepath);
    if (!infile.is_open()) {
        std::cerr << "Error opening file: " << filepath << std::endl;
        return std::make_tuple(false, "", "");
    }

    // Read entire file content
    original_content.assign((std::istreambuf_iterator<char>(infile)),
                             (std::istreambuf_iterator<char>()));
    infile.close();

    std::string target_content = original_content;
    std::string pattern_to_use = search_pattern_str;

    std::regex_constants::syntax_option_type regex_flags = std::regex_constants::ECMAScript;
    if (!case_sensitive) {
        regex_flags |= std::regex_constants::icase;
    }
    // For multi-line matching with regex, std::regex_constants::dot_all is needed
    // However, ECMAScript does not support dot_all. We need to manually handle newlines
    // or use a different regex grammar if available (e.g., std::regex_constants::grep)
    // For simplicity, assuming pattern includes \n if multi-line.

    if (is_regex) {
        std::regex pattern_regex(search_pattern_str, regex_flags);
        if (std::regex_search(original_content, pattern_regex)) {
            found_match = true;
            if (!replace_string.empty()) {
                modified_content = std::regex_replace(original_content, pattern_regex, replace_string);
            }
        }
    } else {
        // Literal string search
        // For case-insensitive literal search, convert both to lower for comparison
        std::string temp_original = original_content;
        std::string temp_pattern = search_pattern_str;
        if (!case_sensitive) {
            std::transform(temp_original.begin(), temp_original.end(), temp_original.begin(), ::tolower);
            std::transform(temp_pattern.begin(), temp_pattern.end(), temp_pattern.begin(), ::tolower);
        }

        if (temp_original.find(temp_pattern) != std::string::npos) {
            found_match = true;
            if (!replace_string.empty()) {
                // Replace all occurrences of literal string
                modified_content = original_content;
                size_t pos = 0;
                while ((pos = modified_content.find(search_pattern_str, pos)) != std::string::npos) {
                    modified_content.replace(pos, search_pattern_str.length(), replace_string);
                    pos += replace_string.length();
                }
            }
        }
    }

    if (modified_content == original_content) {
        modified_content = ""; // No actual change made, set to empty to indicate no modification
    }

    return std::make_tuple(found_match, original_content, modified_content);
}

int main(int argc, char* argv[]) {

    // --- Argument Parsing ---
    // Manual argument parsing. For a robust CLI, a library like cxxopts or Boost.Program_options
    // would be used, but per "no additional libraries" constraint, we do it manually.
    int main(int argc, char* argv[]) {
    // --- Argument Parsing ---
    std::string pattern_str;
    std::string replace_str;
    std::string pattern_file;
    std::string replace_file;
    std::string files_pattern;
    bool is_regex = false;
    bool case_sensitive = false;
    bool in_place = false;
    bool dry_run = false;
    std::string output_file;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--pattern" && i + 1 < argc) {
            pattern_str = argv[++i];
        } else if (arg == "--pattern-file" && i + 1 < argc) {
            pattern_file = argv[++i];
        } else if (arg == "--replace" && i + 1 < argc) {
            replace_str = argv[++i];
        } else if (arg == "--replace-file" && i + 1 < argc) {
            replace_file = argv[++i];
        } else if (arg == "--files" && i + 1 < argc) {
            files_pattern = argv[++i];
        } else if (arg == "--regex") {
            is_regex = true;
        } else if (arg == "--case-sensitive") {
            case_sensitive = true;
        } else if (arg == "--in-place") {
            in_place = true;
        } else if (arg == "--dry-run") {
            dry_run = true;
        } else if (arg == "--output" && i + 1 < argc) {
            output_file = argv[++i];
        } else {
            std::cerr << "Unknown or incomplete argument: " << arg << std::endl;
            return 1;
        }
    }

    // Handle mutual exclusivity and read from files
    if (!pattern_str.empty() && !pattern_file.empty()) {
        std::cerr << "Error: Cannot specify both --pattern and --pattern-file." << std::endl;
        return 1;
    }
    if (pattern_str.empty() && pattern_file.empty()) {
        std::cerr << "Error: One of --pattern or --pattern-file must be specified." << std::endl;
        return 1;
    }
    if (!pattern_file.empty()) {
        std::ifstream pf_stream(pattern_file);
        if (!pf_stream.is_open()) {
            std::cerr << "Error: Could not open pattern file: " << pattern_file << std::endl;
            return 1;
        }
        std::string line;
        while (std::getline(pf_stream, line)) {
            pattern_str += line + "\n";
        }
        if (!pattern_str.empty()) {
            pattern_str.pop_back(); // Remove last newline
        }
    }

    if (!replace_str.empty() && !replace_file.empty()) {
        std::cerr << "Error: Cannot specify both --replace and --replace-file." << std::endl;
        return 1;
    }
    if (!replace_file.empty()) {
        std::ifstream rf_stream(replace_file);
        if (!rf_stream.is_open()) {
            std::cerr << "Error: Could not open replace file: " << replace_file << std::endl;
            return 1;
        }
        std::string line;
        while (std::getline(rf_stream, line)) {
            replace_str += line + "\n";
        }
        if (!replace_str.empty()) {
            replace_str.pop_back(); // Remove last newline
        }
    }

    if (pattern_str.empty()) {
        std::cerr << "Error: Pattern is empty after processing. Check --pattern or --pattern-file." << std::endl;
        return 1;
    }

    if (in_place && replace_str.empty()) {
        std::cerr << "Error: --in-place requires --replace or --replace-file to be specified." << std::endl;
        return 1;
    }
    if (dry_run && replace_str.empty()) {
        std::cerr << "Error: --dry-run requires --replace or --replace-file to be specified." << std::endl;
        return 1;
    }
    if (in_place && dry_run) {
        std::cerr << "Error: Cannot use --in-place and --dry-run together." << std::endl;
        return 1;
    }

    // --- File Discovery ---
    std::vector<std::string> files_to_process;
    if (!files_pattern.empty()) {
        files_to_process = find_files(files_pattern);
    } else {
        // Default: search current directory and subdirectories for all files
        // This needs a more robust implementation for a real project.
        // For now, we'll just assume the current directory for simplicity.
        // A full recursive search would be needed here.
        // As a placeholder, we'll just list files in the current directory.
        for (const auto& entry : std::filesystem::directory_iterator(".")) {
            if (std::filesystem::is_regular_file(entry.path())) {
                files_to_process.push_back(entry.path().string());
            }
        }
    }

    std::vector<std::string> all_results_output; // For --output or stdout
    std::vector<std::tuple<std::string, std::string, std::string>> files_with_modifications;

    for (const std::string& filepath : files_to_process) {
        auto [found_match, original_content, modified_content] = search_and_replace_in_file(
            filepath, pattern_str, replace_str, is_regex, case_sensitive
        );

        if (found_match) {
            all_results_output.push_back(filepath);
        }
        
        if (!modified_content.empty()) { // Check if modifications were actually made
            files_with_modifications.emplace_back(filepath, original_content, modified_content);
        }
    }

    // --- Output and Replacement Logic ---
    if (!replace_str.empty()) { // If replacement was requested
        if (dry_run) {
            std::cout << "--- Dry Run: Proposed Changes ---" << std::endl;
            if (files_with_modifications.empty()) {
                std::cout << "No replacements would be made." << std::endl;
            }
            for (const auto& entry : files_with_modifications) {
                const std::string& filepath = std::get<0>(entry);
                const std::string& original = std::get<1>(entry);
                const std::string& modified = std::get<2>(entry);

                std::cout << "\n--- " << filepath << " (Original) ---" << std::endl;
                std::cout << original << std::endl;
                std::cout << "\n--- " << filepath << " (Proposed) ---" << std::endl;
                std::cout << modified << std::endl;
            }
        } else if (in_place) {
            std::cout << "--- Performing In-Place Replacements ---" << std::endl;
            if (files_with_modifications.empty()) {
                std::cout << "No replacements were made." << std::endl;
            }
            for (const auto& entry : files_with_modifications) {
                const std::string& filepath = std::get<0>(entry);
                const std::string& modified = std::get<2>(entry);

                try {
                    std::ofstream outfile(filepath);
                    if (outfile.is_open()) {
                        outfile << modified; // Write modified content
                        outfile.close();
                        std::cout << "Modified: " << filepath << std::endl;
                    } else {
                        std::cerr << "Error writing to " << filepath << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error writing to " << filepath << ": " << e.what() << std::endl;
                }
            }
        } else { // If --replace is used without --dry-run or --in-place, just show files with matches
            if (!all_results_output.empty()) {
                std::cout << "Files with matches:" << std::endl;
                for (const std::string& res : all_results_output) {
                    std::cout << res << std::endl;
                }
            } else {
                std::cout << "No matches found." << std::endl;
            }
        }
    } else { // Only search, no replacement
        if (!output_file.empty()) {
            try {
                std::ofstream outfile(output_file);
                if (outfile.is_open()) {
                    for (const std::string& res : all_results_output) {
                        outfile << res << std::endl;
                    }
                    outfile.close();
                    std::cout << "Search results written to " << output_file << std::endl;
                } else {
                    std::cerr << "Error writing to output file " << output_file << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error writing to output file " << output_file << ": " << e.what() << std::endl;
            }
        } else {
            if (!all_results_output.empty()) {
                std::cout << "Files with matches:" << std::endl;
                for (const std::string& res : all_results_output) {
                    std::cout << res << std::endl;
                }
            } else {
                std::cout << "No matches found." << std::endl;
            }
        }
    }

    return 0;
}
