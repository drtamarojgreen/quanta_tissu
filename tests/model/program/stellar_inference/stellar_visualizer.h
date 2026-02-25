#ifndef TISSLM_STELLAR_VISUALIZER_H
#define TISSLM_STELLAR_VISUALIZER_H

/**
 * @file stellar_visualizer.h
 * @brief Advanced ASCII visualization module for the Stellar package.
 */

#include <vector>
#include <string>
#include <map>

namespace TissLM {
namespace Stellar {

struct Point3D {
    float x, y, z;
    std::string label;
};

/**
 * @class StellarVisualizer
 * @brief Handles rendering of 3D graphs and analysis grids using ASCII characters.
 */
class StellarVisualizer {
public:
    /**
     * @brief Renders a 3D-perspective ASCII graph.
     * @param points Vector of 3D points to plot.
     * @param width Width of the ASCII canvas.
     * @param height Height of the ASCII canvas.
     * @return Rendered ASCII string.
     */
    static std::string render_3d_graph(const std::vector<Point3D>& points, int width = 80, int height = 40);

    /**
     * @brief Renders a data analysis grid chart.
     * @param grid Map of row labels to vectors of column values.
     * @param col_labels Labels for the columns.
     * @return Rendered ASCII string.
     */
    static std::string render_analysis_grid(const std::map<std::string, std::vector<float>>& grid, const std::vector<std::string>& col_labels);
};

} // namespace Stellar
} // namespace TissLM

#endif // TISSLM_STELLAR_VISUALIZER_H
