#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <cstdio>
#include <memory>
#include <array>
#include <algorithm>
#include <map>

struct TestEntry {
    std::string executable;
    std::string description;
};

struct TestResult {
    std::string executable;
    std::string description;
    bool passed;
    double duration_seconds;
    std::string details;
};

std::vector<TestEntry> load_config(const std::string& config_path) {
    std::vector<TestEntry> entries;
    std::ifstream file(config_path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file: " << config_path << std::endl;
        return entries;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t delimiter_pos = line.find('|');
        if (delimiter_pos != std::string::npos) {
            entries.push_back({
                line.substr(0, delimiter_pos),
                line.substr(delimiter_pos + 1)
            });
        }
    }
    return entries;
}

std::string analyze_output(const std::string& output, int exit_code) {
    if (exit_code == 0) {
        // Try to find "Passed: X, Failed: Y"
        std::string passed_marker = "Passed: ";
        std::string failed_marker = "Failed: ";
        size_t p_pos = output.rfind(passed_marker);
        size_t f_pos = output.rfind(failed_marker);

        if (p_pos != std::string::npos && f_pos != std::string::npos) {
            size_t p_end = output.find_first_not_of("0123456789", p_pos + passed_marker.length());
            size_t f_end = output.find_first_not_of("0123456789", f_pos + failed_marker.length());
            
            std::string p_count = output.substr(p_pos + passed_marker.length(), p_end - (p_pos + passed_marker.length()));
            std::string f_count = output.substr(f_pos + failed_marker.length(), f_end - (f_pos + failed_marker.length()));
            return "Passed: " + p_count + ", Failed: " + f_count;
        }
        return "All checks passed";
    }

    if (output.find("Connection failed") != std::string::npos) return "DB Connection Failed";
    if (output.find("Shape mismatch") != std::string::npos) {
        size_t pos = output.find("Shape mismatch");
        size_t end = output.find('\n', pos);
        return output.substr(pos, end - pos);
    }
    if (output.find("Matrix dimensions are not compatible") != std::string::npos) return "Broadcasting Error";
    if (output.find("Segmentation fault") != std::string::npos) return "Segmentation Fault";
    if (output.find("Assertion failed") != std::string::npos) return "Assertion Failed";
    
    return "Unknown Failure";
}

TestResult run_test(const TestEntry& entry) {
    std::cout << "=========================================" << std::endl;
    std::cout << "Executing: ./" << entry.executable << std::endl;
    std::cout << "=========================================" << std::endl;

    auto start_time = std::chrono::high_resolution_clock::now();

    std::string command = "./" + entry.executable + " 2>&1";
    std::array<char, 128> buffer;
    std::string result_output;
    
    // Using popen to capture output
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        return {entry.executable, entry.description, false, 0.0, "Failed to launch test"};
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::cout << buffer.data(); // Stream to stdout
        result_output += buffer.data(); // Capture for analysis
    }

    int exit_code = pclose(pipe.release()); // Get exit code
    // pclose returns the wait4 status, need to extract exit code
    if (WIFEXITED(exit_code)) {
        exit_code = WEXITSTATUS(exit_code);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;

    TestResult result;
    result.executable = entry.executable;
    result.description = entry.description;
    result.passed = (exit_code == 0);
    result.duration_seconds = duration.count();
    result.details = analyze_output(result_output, exit_code);

    if (!result.passed) {
        std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cout << "!!! Test Failed: " << entry.executable << std::endl;
        std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    }

    std::cout << "=========================================" << std::endl;
    return result;
}

void print_summary(const std::vector<TestResult>& results) {
    std::cout << "\n";
    std::cout << "============================================================================================================================================" << std::endl;
    std::cout << "                                           TissLM C++ Test Suite: Executive Summary" << std::endl;
    std::cout << "============================================================================================================================================" << std::endl;
    
    // Header
    printf("%-30s | %-55s | %-6s | %-8s | %s\n", "Test Executable", "Description", "Status", "Duration", "Details");
    std::cout << "--------------------------------------------------------------------------------------------------------------------------------------------" << std::endl;

    int passed_count = 0;
    for (const auto& res : results) {
        if (res.passed) passed_count++;
        printf("%-30s | %-55s | %-6s | %-8.3fs | %s\n", 
            ("./" + res.executable).c_str(), 
            res.description.c_str(), 
            (res.passed ? "PASS" : "FAIL"), 
            res.duration_seconds, 
            res.details.c_str());
    }

    std::cout << "============================================================================================================================================" << std::endl;
    std::cout << "Overall Result: " << passed_count << " / " << results.size() << " tests passed." << std::endl;
    std::cout << "============================================================================================================================================" << std::endl;
}

int main() {
    std::vector<TestEntry> tests = load_config("test_config.txt");
    if (tests.empty()) {
        std::cerr << "No tests found in test_config.txt or file missing." << std::endl;
        return 1;
    }

    std::vector<TestResult> results;
    bool all_passed = true;

    for (const auto& test : tests) {
        TestResult res = run_test(test);
        results.push_back(res);
        if (!res.passed) all_passed = false;
    }

    print_summary(results);

    if (!all_passed) {
        std::cout << "\nActionable Recommendations:" << std::endl;
        std::cout << "---------------------------" << std::endl;
        
        int db_failures = 0;
        int shape_failures = 0;

        for (const auto& res : results) {
            if (res.details.find("Connection Failed") != std::string::npos) db_failures++;
            if (res.details.find("Shape mismatch") != std::string::npos || res.details.find("Broadcasting Error") != std::string::npos) shape_failures++;
        }

        if (db_failures > 0) {
            std::cout << "1. [Database] " << db_failures << " tests failed due to connection issues. Ensure 'tissdb' server is running on port 9876." << std::endl;
        }
        if (shape_failures > 0) {
            std::cout << "2. [Matrix Ops] " << shape_failures << " tests failed due to shape/broadcasting errors. Check tensor dimensions." << std::endl;
        }
        std::cout << "3. Review the logs above for specific assertion failures or stack traces." << std::endl;
        
        return 1;
    } else {
        std::cout << "\nAll tests completed successfully. System is stable." << std::endl;
        return 0;
    }
}
