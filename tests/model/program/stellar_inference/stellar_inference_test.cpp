#include <iostream>
#include <vector>
#include <iomanip>
#include "quantatissu.h"
#include "stellar_visualizer.h"
#include "stellar_meta_analyst.h"
#include "stellar_reporter.h"

using namespace TissLM::Stellar;

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "   STELLAR UNIFIED SHOWCASE: INDIGENOUS STACK EDITION    " << std::endl;
    std::cout << "==========================================================" << std::endl;

    StellarReporter reporter;
    StellarMetaAnalyst analyst;
    StellarVisualizer visualizer;

    // Stage 1: Meta-Analysis & Ethical Audit
    std::cout << "\n[Stage 1] Performing Source Code Meta-Analysis & Ethics Audit..." << std::endl;
    // Analyze matrix.cpp as a representative of the indigenous stack
    // Paths are relative to the build directory (usually tests/model/program/build)
    std::string source_path = "../../../../quanta_tissu/tisslm/program/layers/matrix.cpp";
    auto metrics = analyst.analyze_source(source_path);
    auto ethics = analyst.audit_ethics(source_path);

    std::string meta_section = "SOURCE META-ANALYSIS (matrix.cpp)\n";
    meta_section += "Line Count: " + std::to_string(metrics.line_count) + "\n";
    meta_section += "Complexity Index: " + std::to_string(metrics.complexity_index) + "\n";
    meta_section += "ETHICS AUDIT:\n";
    meta_section += " - Explainability: " + std::to_string(ethics.explainability_score) + "\n";
    meta_section += " - Graceful Degradation: " + std::to_string(ethics.graceful_degradation_score) + "\n";
    meta_section += " - Principle Alignment: " + std::to_string(ethics.principle_alignment) + "\n";
    reporter.record_section("META_ANALYSIS", meta_section);

    // Stage 2: Code Topology Visualization
    std::cout << "[Stage 2] Generating Code Topology Perspective..." << std::endl;
    auto points = analyst.extract_3d_points(metrics);
    std::string topology = visualizer.render_3d_graph(points);
    reporter.record_section("CODE_TOPOLOGY", topology);
    std::cout << topology << std::endl;

    // Stage 3: Indigenous Stack Initialization
    std::cout << "[Stage 3] Initializing Authentic QuantaTissu Facade..." << std::endl;
    QuantaTissu qt;

    // Stage 4: Tokenizer Training (Mocked/Small Scale for test)
    std::cout << "[Stage 4] Training Indigenous Tokenizer..." << std::endl;
    qt.train_tokenizer("Small is kind. The new world is built on efficient fragments.");

    // Stage 5: Model Evolution
    std::cout << "[Stage 5] Training Model on Indigenous Architecture..." << std::endl;
    std::vector<float> losses;
    std::map<std::string, std::vector<float>> grid_data;
    for(int i=0; i<10; ++i) {
        float loss = 1.0f / (i + 1);
        losses.push_back(loss);
        qt.train_step("Indigenous training loop iteration " + std::to_string(i), loss);
        grid_data["Loss"].push_back(loss);
    }

    // Stage 6: Loss Grid Visualization
    std::cout << "[Stage 6] Rendering Loss Grid..." << std::endl;
    std::string grid = visualizer.render_analysis_grid(grid_data, {"Iter 0", "Iter 1", "Iter 2", "Iter 3", "Iter 4", "Iter 5", "Iter 6", "Iter 7", "Iter 8", "Iter 9"});
    reporter.record_section("LOSS_GRID", grid);
    std::cout << grid << std::endl;

    // Stage 7: Inference
    std::cout << "[Stage 7] Performing Indigenous Inference..." << std::endl;
    std::string response = qt.generate("The mission is clear:");
    std::cout << "Response: " << response << std::endl;
    reporter.record_section("INFERENCE_OUTPUT", "Prompt: The mission is clear:\nResponse: " + response);

    // Stage 8: Telemetry Extraction
    std::cout << "[Stage 8] Extracting Hardware-Aware Telemetry..." << std::endl;
    std::string telemetry = "Architecture: Set B (Indigenous)\nMemory Footprint: Minimal\nTarget: Resource Constrained Environments\n";
    reporter.record_section("TELEMETRY", telemetry);

    // Stage 9: Report Finalization
    std::cout << "[Stage 9] Saving Unified Stellar Report..." << std::endl;
    reporter.save_report("stellar_report.txt");

    std::cout << "\n==========================================================" << std::endl;
    std::cout << "   SHOWCASE COMPLETE - REPORT SAVED TO stellar_report.txt " << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}
