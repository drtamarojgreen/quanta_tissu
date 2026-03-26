#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <regex>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

bool matches_wildcard(const std::string& filename, const std::string& pattern) {
    if (pattern == "*") return true;
    if (pattern.front() == '*' && pattern.back() != '*') {
        std::string suffix = pattern.substr(1);
        return filename.length() >= suffix.length() && filename.compare(filename.length() - suffix.length(), suffix.length(), suffix) == 0;
    } else if (pattern.back() == '*' && pattern.front() != '*') {
        std::string prefix = pattern.substr(0, pattern.length() - 1);
        return filename.length() >= prefix.length() && filename.compare(0, prefix.length(), prefix) == 0;
    } else if (pattern.find('*') != std::string::npos) {
        std::string regex_pattern = std::regex_replace(pattern, std::regex(R"(\.)"), R"(\.)");
        regex_pattern = std::regex_replace(regex_pattern, std::regex(R"(\*)"), R"(.*)");
        try { return std::regex_match(filename, std::regex(regex_pattern)); } catch (...) { return false; }
    }
    return filename == pattern;
}

std::vector<std::string> find_files(const std::string& pattern) {
    std::vector<std::string> files;
    if (fs::exists(pattern)) {
        if (fs::is_regular_file(pattern)) files.push_back(pattern);
        else if (fs::is_directory(pattern)) {
            for (const auto& entry : fs::recursive_directory_iterator(pattern)) if (fs::is_regular_file(entry.path())) files.push_back(entry.path().string());
        }
    } else {
        std::string dir = ".", file_pattern = pattern;
        size_t last_slash = pattern.find_last_of("/\\");
        if (last_slash != std::string::npos) { dir = pattern.substr(0, last_slash); file_pattern = pattern.substr(last_slash + 1); }
        if (fs::exists(dir) && fs::is_directory(dir)) {
            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if (fs::is_regular_file(entry.path()) && matches_wildcard(entry.path().filename().string(), file_pattern)) files.push_back(entry.path().string());
            }
        }
    }
    return files;
}

std::tuple<bool, std::string, std::string>
search_and_replace_in_file(const std::string& filepath, const std::string& search_pattern_str,
                           const std::string& replace_string, bool is_regex, bool case_sensitive) {
    std::string original_content, modified_content;
    bool found_match = false;
    std::ifstream infile(filepath);
    if (!infile.is_open()) return std::make_tuple(false, "", "");
    original_content.assign((std::istreambuf_iterator<char>(infile)), (std::istreambuf_iterator<char>()));
    infile.close();
    std::regex_constants::syntax_option_type regex_flags = std::regex_constants::ECMAScript;
    if (!case_sensitive) regex_flags |= std::regex_constants::icase;
    if (is_regex) {
        std::regex pattern_regex(search_pattern_str, regex_flags);
        if (std::regex_search(original_content, pattern_regex)) {
            found_match = true;
            if (!replace_string.empty()) modified_content = std::regex_replace(original_content, pattern_regex, replace_string);
        }
    } else {
        std::string temp_original = original_content, temp_pattern = search_pattern_str;
        if (!case_sensitive) {
            std::transform(temp_original.begin(), temp_original.end(), temp_original.begin(), ::tolower);
            std::transform(temp_pattern.begin(), temp_pattern.end(), temp_pattern.begin(), ::tolower);
        }
        if (temp_original.find(temp_pattern) != std::string::npos) {
            found_match = true;
            if (!replace_string.empty()) {
                modified_content = original_content;
                size_t pos = 0;
                while ((pos = modified_content.find(search_pattern_str, pos)) != std::string::npos) {
                    modified_content.replace(pos, search_pattern_str.length(), replace_string);
                    pos += replace_string.length();
                }
            }
        }
    }
    if (modified_content == original_content) modified_content = "";
    return std::make_tuple(found_match, original_content, modified_content);
}

int main(int argc, char* argv[]) {
    std::string pattern_str, replace_str, pattern_file, replace_file, files_pattern, output_file;
    bool is_regex = false, case_sensitive = false, in_place = false, dry_run = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--pattern" && i + 1 < argc) pattern_str = argv[++i];
        else if (arg == "--pattern-file" && i + 1 < argc) pattern_file = argv[++i];
        else if (arg == "--replace" && i + 1 < argc) replace_str = argv[++i];
        else if (arg == "--replace-file" && i + 1 < argc) replace_file = argv[++i];
        else if (arg == "--files" && i + 1 < argc) files_pattern = argv[++i];
        else if (arg == "--regex") is_regex = true;
        else if (arg == "--case-sensitive") case_sensitive = true;
        else if (arg == "--in-place") in_place = true;
        else if (arg == "--dry-run") dry_run = true;
        else if (arg == "--output" && i + 1 < argc) output_file = argv[++i];
        else { std::cerr << "Unknown arg: " << arg << std::endl; return 1; }
    }
    if (!pattern_file.empty()) {
        std::ifstream pf(pattern_file);
        if (pf.is_open()) { std::string line; while (std::getline(pf, line)) pattern_str += line + "\n"; if (!pattern_str.empty()) pattern_str.pop_back(); }
    }
    if (!replace_file.empty()) {
        std::ifstream rf(replace_file);
        if (rf.is_open()) { std::string line; while (std::getline(rf, line)) replace_str += line + "\n"; if (!replace_str.empty()) replace_str.pop_back(); }
    }
    if (pattern_str.empty()) { std::cerr << "Error: Pattern empty." << std::endl; return 1; }
    std::vector<std::string> files_to_process;
    if (!files_pattern.empty()) files_to_process = find_files(files_pattern);
    else for (const auto& entry : fs::recursive_directory_iterator(".")) if (fs::is_regular_file(entry.path())) files_to_process.push_back(entry.path().string());
    std::vector<std::string> all_results;
    std::vector<std::tuple<std::string, std::string, std::string>> modifications;
    for (const std::string& filepath : files_to_process) {
        auto [found, original, modified] = search_and_replace_in_file(filepath, pattern_str, replace_str, is_regex, case_sensitive);
        if (found) all_results.push_back(filepath);
        if (!modified.empty()) modifications.emplace_back(filepath, original, modified);
    }
    if (!replace_str.empty()) {
        if (dry_run) {
            std::cout << "--- Dry Run ---" << std::endl;
            for (const auto& entry : modifications) std::cout << "\n--- " << std::get<0>(entry) << " ---\n" << std::get<2>(entry) << std::endl;
        } else if (in_place) {
            for (const auto& entry : modifications) {
                std::ofstream out(std::get<0>(entry));
                if (out.is_open()) { out << std::get<2>(entry); std::cout << "Modified: " << std::get<0>(entry) << std::endl; }
            }
        } else {
            for (const auto& res : all_results) std::cout << res << std::endl;
        }
    } else {
        if (!output_file.empty()) {
            std::ofstream out(output_file);
            if (out.is_open()) for (const auto& res : all_results) out << res << std::endl;
        } else for (const auto& res : all_results) std::cout << res << std::endl;
    }
    return 0;
}
