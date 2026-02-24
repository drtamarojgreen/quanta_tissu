#include "bdd_framework.h"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <feature_file_or_directory>" << std::endl;
        return 1;
    }

    std::string path = argv[1];
    std::vector<std::string> feature_files;

    if (fs::is_directory(path)) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.path().extension() == ".feature") {
                feature_files.push_back(entry.path().string());
            }
        }
    } else {
        feature_files.push_back(path);
    }

    bool all_passed = true;
    for (const auto& file : feature_files) {
        std::cout << "Running features from: " << file << std::endl;
        auto features = TissDB::BDD::Parser::parse_file(file);
        for (const auto& feature : features) {
            if (!TissDB::BDD::Runner::run_feature(feature)) {
                all_passed = false;
            }
        }
    }

    if (all_passed) {
        std::cout << "All BDD features passed!" << std::endl;
        return 0;
    } else {
        std::cerr << "Some BDD features failed." << std::endl;
        return 1;
    }
}
