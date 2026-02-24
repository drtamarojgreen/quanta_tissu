#include "stellar_reporter.h"
#include <iostream>

namespace TissLM {
namespace Stellar {

void StellarReporter::record_section(const std::string& title, const std::string& content) {
    sections_.push_back({title, content});
}

bool StellarReporter::save_report(const std::string& filepath) {
    std::ofstream ofs(filepath);
    if (!ofs.is_open()) {
        std::cerr << "[STELLAR] Error: Could not open report file for writing: " << filepath << std::endl;
        return false;
    }

    ofs << "================================================================" << std::endl;
    ofs << "    QuantaTissu Frontier - STELLAR ANALYSIS REPORT" << std::endl;
    ofs << "================================================================" << std::endl << std::endl;

    for (const auto& section : sections_) {
        ofs << "[SECTION] " << section.title << std::endl;
        ofs << "----------------------------------------------------------------" << std::endl;
        ofs << section.content << std::endl << std::endl;
    }

    ofs << "================================================================" << std::endl;
    ofs << "    REPORT COMPLETE" << std::endl;
    ofs << "================================================================" << std::endl;

    ofs.close();
    return true;
}

} // namespace Stellar
} // namespace TissLM
