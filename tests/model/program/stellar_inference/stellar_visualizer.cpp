#include "stellar_visualizer.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace TissLM {
namespace Stellar {

std::string StellarVisualizer::render_3d_graph(const std::vector<Point3D>& points, int width, int height) {
    std::vector<std::string> canvas(height, std::string(width, ' '));

    if (points.empty()) return "No data for 3D graph.";

    float min_x = points[0].x, max_x = points[0].x;
    float min_y = points[0].y, max_y = points[0].y;
    float min_z = points[0].z, max_z = points[0].z;

    for (const auto& p : points) {
        min_x = std::min(min_x, p.x); max_x = std::max(max_x, p.x);
        min_y = std::min(min_y, p.y); max_y = std::max(max_y, p.y);
        min_z = std::min(min_z, p.z); max_z = std::max(max_z, p.z);
    }

    auto project = [&](const Point3D& p) {
        // Simple perspective projection with depth offset
        float norm_x = (max_x > min_x) ? (p.x - min_x) / (max_x - min_x) : 0.5f;
        float norm_y = (max_y > min_y) ? (p.y - min_y) / (max_y - min_y) : 0.5f;
        float norm_z = (max_z > min_z) ? (p.z - min_z) / (max_z - min_z) : 0.5f;

        // Shift X and Y based on Z to give 3D depth impression
        float px = norm_x * (width * 0.6f) + norm_z * (width * 0.3f);
        float py = norm_y * (height * 0.6f) + norm_z * (height * 0.3f);

        return std::make_pair((int)px, (int)py);
    };

    const std::string shading = ".:!*#@";

    for (const auto& p : points) {
        auto [ix, iy] = project(p);
        if (ix >= 0 && ix < width && iy >= 0 && iy < height) {
            float norm_z = (max_z > min_z) ? (p.z - min_z) / (max_z - min_z) : 0.5f;
            int shade_idx = (int)(norm_z * (shading.size() - 1));
            // Standard ASCII coordinates: Y increases downwards.
            // We'll flip Y for graph representation.
            canvas[height - 1 - iy][ix] = shading[shade_idx];
        }
    }

    std::stringstream ss;
    ss << "+" << std::string(width, '-') << "+" << std::endl;
    for (const auto& line : canvas) {
        ss << "|" << line << "|" << std::endl;
    }
    ss << "+" << std::string(width, '-') << "+" << std::endl;
    ss << "  (3D Perspective: Shading denotes Z-Depth [" << min_z << " to " << max_z << "])" << std::endl;

    return ss.str();
}

std::string StellarVisualizer::render_analysis_grid(const std::map<std::string, std::vector<float>>& grid, const std::vector<std::string>& col_labels) {
    std::stringstream ss;

    // Header
    ss << std::left << std::setw(20) << "METRIC";
    for (const auto& label : col_labels) {
        ss << "| " << std::setw(12) << label;
    }
    ss << std::endl;
    ss << std::string(20 + col_labels.size() * 15, '-') << std::endl;

    for (const auto& [row_name, values] : grid) {
        ss << std::left << std::setw(20) << row_name;
        for (float v : values) {
            ss << "| " << std::fixed << std::setprecision(4) << std::setw(12) << v;
        }
        ss << std::endl;
    }

    return ss.str();
}

} // namespace Stellar
} // namespace TissLM
