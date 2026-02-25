#include "stellar_meta_analyst.h"
#include "stellar_visualizer.h"
#include <fstream>
#include <stdexcept>
#include <cmath>
#include <algorithm>

namespace TissLM {
namespace Stellar {

SourceMetrics StellarMetaAnalyst::analyze_source(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) throw std::runtime_error("MetaAnalyst: Open failed for " + path);
    SourceMetrics m;
    m.line_count = 0; m.char_count = 0;
    std::string line;
    float wsum = 0;
    while (std::getline(f, line)) {
        m.line_count++; m.char_count += line.length();
        for (char c : line) m.char_distribution[c]++;
        float lc = (float)line.length() * 0.1f;
        if (line.find('{') != std::string::npos) lc += 4.0f;
        wsum += lc;
    }
    m.complexity_index = m.line_count > 0 ? wsum / m.line_count : 0;
    return m;
}

ModelMetrics StellarMetaAnalyst::analyze_model(std::shared_ptr<TissLM::Core::Model> model) {
    ModelMetrics m;
    auto params = model->get_parameters();
    m.parameter_count = 0;
    for (const auto& p : params) m.parameter_count += p->value().rows() * p->value().cols();
    m.layer_count = params.size() / 2;
    m.param_density = (float)m.parameter_count / (m.layer_count + 1);
    return m;
}

ModelMetrics StellarMetaAnalyst::analyze_indigenous_model(const Model& model) {
    ModelMetrics m;
    m.parameter_count = model.get_parameter_count();
    m.layer_count = model.get_layer_count();
    m.param_density = m.layer_count > 0 ? (float)m.parameter_count / m.layer_count : (float)m.parameter_count;
    return m;
}

EthicsMetrics StellarMetaAnalyst::audit_ethics(const std::string& path) {
    std::ifstream f(path);
    EthicsMetrics m = {0, 0, 0};
    if (!f.is_open()) return m;

    std::string line;
    size_t lines = 0;
    size_t errors = 0;
    size_t comments = 0;
    size_t alignments = 0;
    size_t defensive_checks = 0;

    while (std::getline(f, line)) {
        lines++;
        // Error handling density
        if (line.find("try") != std::string::npos || line.find("catch") != std::string::npos ||
            line.find("throw") != std::string::npos || line.find("error") != std::string::npos ||
            line.find("runtime_error") != std::string::npos || line.find("invalid_argument") != std::string::npos) errors++;

        // Defensive coding density
        if (line.find("if (") != std::string::npos && (line.find("nullptr") != std::string::npos || line.find("empty()") != std::string::npos || line.find("== 0") != std::string::npos)) defensive_checks++;

        // Explainability density
        if (line.find("//") != std::string::npos || line.find("/*") != std::string::npos ||
            line.find("* ") != std::string::npos || line.find("@brief") != std::string::npos) comments++;

        // Principle alignment
        if (line.find("Small is kind") != std::string::npos || line.find("Compute lightly") != std::string::npos ||
            line.find("Stellar") != std::string::npos || line.find("Indig") != std::string::npos ||
            line.find("Harmony") != std::string::npos || line.find("Empathy") != std::string::npos) alignments++;
    }

    if (lines > 0) {
        m.graceful_degradation_score = (float)(errors + defensive_checks) / lines * 100.0f;
        m.explainability_score = (float)comments / lines * 100.0f;
        m.principle_alignment = (float)alignments / lines * 1000.0f;
        if (alignments > 0 && m.principle_alignment < 5.0f) m.principle_alignment = 5.0f; // Minimum score if keywords present
    }
    return m;
}

std::vector<Point3D> StellarMetaAnalyst::extract_3d_points(const SourceMetrics& m) {
    std::vector<Point3D> pts;
    for (const auto& [c, count] : m.char_distribution) {
        if (c >= 32 && c <= 126) {
            Point3D p; p.x = (float)c; p.y = (float)count; p.z = std::log10((float)count + 1.0f) * 15.0f;
            pts.push_back(p);
        }
    }
    return pts;
}

} // namespace Stellar
} // namespace TissLM
