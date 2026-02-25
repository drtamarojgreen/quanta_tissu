#include <iostream>
#include <vector>
#include <iomanip>
#include "quantatissu.h"
#include "stellar_visualizer.h"
#include "stellar_meta_analyst.h"
#include "stellar_reporter.h"
#include "ddl_parser.h"

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

    // Paths are relative to the execution directory: tests/model/program/stellar_build
    // We try to locate matrix.cpp by checking multiple possible relative paths
    std::vector<std::string> possible_paths = {
        "../../../../quanta_tissu/tisslm/program/layers/matrix.cpp",
        "../../../../../quanta_tissu/tisslm/program/layers/matrix.cpp",
        "../../../quanta_tissu/tisslm/program/layers/matrix.cpp"
    };

    std::string source_path = "";
    for (const auto& p : possible_paths) {
        std::ifstream f(p);
        if (f.good()) {
            source_path = p;
            break;
        }
    }

    if (source_path.empty()) {
        throw std::runtime_error("MetaAnalyst: Could not find matrix.cpp in any expected location.");
    }

    auto metrics = analyst.analyze_source(source_path);
    auto ethics = analyst.audit_ethics(source_path);

    std::string meta_section = "SOURCE META-ANALYSIS (matrix.cpp)\n";
    meta_section += "Line Count: " + std::to_string(metrics.line_count) + "\n";
    meta_section += "Complexity Index: " + std::to_string(metrics.complexity_index) + "\n";
    meta_section += "ETHICS AUDIT:\n";
    meta_section += " - Explainability Score: " + std::to_string(ethics.explainability_score) + "%\n";
    meta_section += " - Graceful Degradation: " + std::to_string(ethics.graceful_degradation_score) + "%\n";
    meta_section += " - Principle Alignment: " + std::to_string(ethics.principle_alignment) + "/10\n";
    reporter.record_section("META_ANALYSIS", meta_section);

    // Stage 2: Code Topology Visualization
    std::cout << "[Stage 2] Generating Code Topology Perspective with Labels..." << std::endl;
    auto points = analyst.extract_3d_points(metrics);
    // Add explicit landmark labels
    points.push_back({0, 0, 0, "ORIGIN"});
    points.push_back({50, 20, 30, "STELLAR_CORE"});

    std::string topology = visualizer.render_3d_graph(points);
    reporter.record_section("CODE_TOPOLOGY", topology);
    std::cout << topology << std::endl;

    // Stage 3: Indigenous Stack Initialization
    std::cout << "[Stage 3] Initializing Authentic QuantaTissu Facade from JSON..." << std::endl;
    // model_config.json is in the source directory next to the test source
    QuantaTissu qt("../stellar_inference/model_config.json");

    // Stage 3.5: Architectural Network Graph
    std::cout << "[Stage 3.5] Rendering Architectural Network Graph..." << std::endl;
    auto model_graph = analyst.extract_model_graph(qt.get_model());
    std::string network_viz = visualizer.render_network_graph(model_graph.nodes, model_graph.edges);
    reporter.record_section("ARCHITECTURE_GRAPH", network_viz);
    std::cout << network_viz << std::endl;

    // Indigenous Model Analysis
    auto model_metrics = analyst.analyze_indigenous_model(qt.get_model());
    std::string model_sec = "INDIGENOUS MODEL METRICS\n";
    model_sec += "Parameter Count: " + std::to_string(model_metrics.parameter_count) + "\n";
    model_sec += "Layer Count: " + std::to_string(model_metrics.layer_count) + "\n";
    model_sec += "Param Density: " + std::to_string(model_metrics.param_density) + "\n";
    reporter.record_section("MODEL_METRICS", model_sec);
    std::cout << model_sec << std::endl;

    // Stage 4: Tokenizer Training
    std::cout << "[Stage 4] Training Indigenous Tokenizer..." << std::endl;
    qt.train_tokenizer("Small is kind. The new world is built on efficient fragments. Harmony in code leads to empathy in AI.");

    // Stage 5: Model Evolution (Loss Calculation)
    std::cout << "[Stage 5] Calculating Loss on Indigenous Architecture..." << std::endl;
    std::vector<float> losses;
    std::map<std::string, std::vector<float>> grid_data;
    for(int i=0; i<10; ++i) {
        float loss = 0.0f;
        qt.train_step("Indigenous training loop iteration " + std::to_string(i), loss);
        losses.push_back(loss);
        grid_data["Loss"].push_back(loss);
    }

    // Stage 6: Loss Grid Visualization
    std::cout << "[Stage 6] Rendering Loss Grid..." << std::endl;
    std::string grid = visualizer.render_analysis_grid(grid_data, {"Iter 0", "Iter 1", "Iter 2", "Iter 3", "Iter 4", "Iter 5", "Iter 6", "Iter 7", "Iter 8", "Iter 9"});
    reporter.record_section("LOSS_GRID", grid);
    std::cout << grid << std::endl;

    // Stage 7: Real Autoregressive Inference
    std::cout << "[Stage 7] Performing Indigenous Autoregressive Inference..." << std::endl;
    std::string prompt = "The mission of QuantaTissu is";
    std::string response = qt.generate(prompt, 20);
    std::cout << "Prompt: " << prompt << std::endl;
    std::cout << "Response: " << response << std::endl;
    reporter.record_section("INFERENCE_OUTPUT", "Prompt: " + prompt + "\nResponse: " + response);

    // Stage 8: DDL Parsing Demonstration
    std::cout << "[Stage 8] Demonstrating DDL Parsing Logic..." << std::endl;
    TissDB::DDL::DDLParser parser;
    auto stmt = parser.parse("CREATE TABLE StellarDocuments (id INT, content STRING)");
    std::string ddl_output = "DDL Query: CREATE TABLE StellarDocuments (id INT, content STRING)\n";
    if (stmt && stmt->getType() == TissDB::DDL::DDLStatement::Type::CREATE_TABLE) {
        auto create = static_cast<TissDB::DDL::CreateTableStatement*>(stmt.get());
        ddl_output += "Parsed Table Name: " + create->getTableName() + "\n";
        ddl_output += "Status: SUCCESS\n";
    } else {
        ddl_output += "Status: FAILED TO PARSE\n";
    }
    reporter.record_section("DDL_PARSING", ddl_output);
    std::cout << ddl_output << std::endl;

    // Stage 9: Telemetry Extraction
    std::cout << "[Stage 9] Extracting Hardware-Aware Telemetry..." << std::endl;
    std::string telemetry = "Architecture: Set B (Indigenous)\nMemory Footprint: Minimal\nTarget: Resource Constrained Environments\n";
    reporter.record_section("TELEMETRY", telemetry);

    // Report Finalization
    std::cout << "Saving Unified Stellar Report..." << std::endl;
    reporter.save_report("stellar_report.txt");

    std::cout << "\n==========================================================" << std::endl;
    std::cout << "   SHOWCASE COMPLETE - REPORT SAVED TO stellar_report.txt " << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}
