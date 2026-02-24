#ifndef TISSLM_STELLAR_REPORTER_H
#define TISSLM_STELLAR_REPORTER_H

/**
 * @file stellar_reporter.h
 * @brief Report generation module for the Stellar package.
 */

#include <string>
#include <vector>
#include <fstream>

namespace TissLM {
namespace Stellar {

/**
 * @class StellarReporter
 * @brief Orchestrates report generation for Stellar activities.
 */
class StellarReporter {
public:
    /**
     * @brief Records a section of the report.
     * @param title Title of the section.
     * @param content Content of the section (can include ASCII graphs).
     */
    void record_section(const std::string& title, const std::string& content);

    /**
     * @brief Saves the accumulated report to a file.
     * @param filepath Destination file path.
     * @return true if successful, false otherwise.
     */
    bool save_report(const std::string& filepath);

private:
    struct Section {
        std::string title;
        std::string content;
    };
    std::vector<Section> sections_;
};

} // namespace Stellar
} // namespace TissLM

#endif // TISSLM_STELLAR_REPORTER_H
