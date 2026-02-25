#include "stellar_visualizer.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace TissLM {
namespace Stellar {

void StellarVisualizer::draw_line(std::vector<std::string>& canvas, std::vector<float>& z_buffer,
                                  int x0, int y0, float z0, int x1, int y1, float z1, char c) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    int height = canvas.size();
    int width = canvas[0].size();

    while (true) {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
            // Linear interpolation for Z
            float t = (dx > dy) ? (float)(x0 - x0) / dx : (float)(y0 - y0) / dy; // Simplified
            float z = z0; // Should interpolate but for ASCII depth is discrete enough

            if (z >= z_buffer[y0 * width + x0]) {
                canvas[y0][x0] = c;
                z_buffer[y0 * width + x0] = z;
            }
        }

        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

std::string StellarVisualizer::render_3d_graph(const std::vector<Point3D>& points, int width, int height) {
    std::vector<std::string> canvas(height, std::string(width, ' '));
    std::vector<float> z_buffer(width * height, -1e9f);

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
        float norm_x = (max_x > min_x) ? (p.x - min_x) / (max_x - min_x) : 0.5f;
        float norm_y = (max_y > min_y) ? (p.y - min_y) / (max_y - min_y) : 0.5f;
        float norm_z = (max_z > min_z) ? (p.z - min_z) / (max_z - min_z) : 0.5f;

        float px = norm_x * (width * 0.6f) + norm_z * (width * 0.2f);
        float py = norm_y * (height * 0.6f) + norm_z * (height * 0.2f);

        return std::make_pair((int)px, (int)py);
    };

    const std::string shading = ".:!*#@";

    for (const auto& p : points) {
        auto [ix, iy] = project(p);
        int cy = height - 1 - iy;
        if (ix >= 0 && ix < width && cy >= 0 && cy < height) {
            float norm_z = (max_z > min_z) ? (p.z - min_z) / (max_z - min_z) : 0.5f;
            int shade_idx = (int)(norm_z * (shading.size() - 1));

            if (p.z >= z_buffer[cy * width + ix]) {
                canvas[cy][ix] = shading[shade_idx];
                z_buffer[cy * width + ix] = p.z;
            }

            if (!p.label.empty()) {
                int label_y = cy + 1;
                int label_x = ix - (int)p.label.length() / 2;
                if (label_y < height) {
                    for (size_t k = 0; k < p.label.length(); ++k) {
                        int lx = label_x + k;
                        if (lx >= 0 && lx < width) {
                            // Check if we are overwriting a background point or empty space
                            if (canvas[label_y][lx] == ' ' || shading.find(canvas[label_y][lx]) != std::string::npos) {
                                canvas[label_y][lx] = p.label[k];
                                z_buffer[label_y * width + lx] = p.z; // Label shares depth of node
                            }
                        }
                    }
                }
            }
        }
    }

    std::stringstream ss;
    ss << "+" << std::string(width, '-') << "+" << std::endl;
    for (const auto& line : canvas) ss << "|" << line << "|" << std::endl;
    ss << "+" << std::string(width, '-') << "+" << std::endl;
    return ss.str();
}

std::string StellarVisualizer::render_network_graph(const std::vector<Node>& nodes, const std::vector<Edge>& edges, int width, int height) {
    std::vector<std::string> canvas(height, std::string(width, ' '));
    std::vector<float> z_buffer(width * height, -1e9f);

    if (nodes.empty()) return "No nodes to render.";

    float min_x = nodes[0].x, max_x = nodes[0].x;
    float min_y = nodes[0].y, max_y = nodes[0].y;
    float min_z = nodes[0].z, max_z = nodes[0].z;

    for (const auto& n : nodes) {
        min_x = std::min(min_x, n.x); max_x = std::max(max_x, n.x);
        min_y = std::min(min_y, n.y); max_y = std::max(max_y, n.y);
        min_z = std::min(min_z, n.z); max_z = std::max(max_z, n.z);
    }

    auto project = [&](const Node& n) {
        float norm_x = (max_x > min_x) ? (n.x - min_x) / (max_x - min_x) : 0.5f;
        float norm_y = (max_y > min_y) ? (n.y - min_y) / (max_y - min_y) : 0.5f;
        float norm_z = (max_z > min_z) ? (n.z - min_z) / (max_z - min_z) : 0.5f;

        // Leave more room for labels (padding)
        int px = (int)(norm_x * (width * 0.7f) + norm_z * (width * 0.15f) + width * 0.05f);
        int py = (int)(norm_y * (height * 0.7f) + norm_z * (height * 0.15f) + height * 0.05f);
        return std::make_pair(px, py);
    };

    std::map<size_t, std::pair<int, int>> node_positions;

    // Draw Edges first (behind nodes)
    for (const auto& edge : edges) {
        Node from_node, to_node;
        bool found_from = false, found_to = false;
        for (const auto& n : nodes) {
            if (n.id == edge.from) { from_node = n; found_from = true; }
            if (n.id == edge.to) { to_node = n; found_to = true; }
        }
        if (found_from && found_to) {
            auto [x0, y0] = project(from_node);
            auto [x1, y1] = project(to_node);
            draw_line(canvas, z_buffer, x0, height - 1 - y0, std::min(from_node.z, to_node.z) - 0.1f,
                      x1, height - 1 - y1, std::min(from_node.z, to_node.z) - 0.1f, edge.style);
        }
    }

    // Draw Nodes and Labels
    for (const auto& n : nodes) {
        auto [ix, iy] = project(n);
        int cy = height - 1 - iy;
        if (ix >= 0 && ix < width && cy >= 0 && cy < height) {
            // Node glyph
            if (n.z >= z_buffer[cy * width + ix]) {
                canvas[cy][ix] = 'O'; // Node marker
                z_buffer[cy * width + ix] = n.z;
            }

            // Label
            if (!n.label.empty()) {
                int label_y = cy + 1;
                int label_x = ix - (int)n.label.length() / 2;
                if (label_y < height) {
                    for (size_t k = 0; k < n.label.length(); ++k) {
                        int lx = label_x + k;
                        if (lx >= 0 && lx < width) {
                            if (n.z >= z_buffer[label_y * width + lx]) {
                                canvas[label_y][lx] = n.label[k];
                                z_buffer[label_y * width + lx] = n.z;
                            }
                        }
                    }
                }
            }
        }
    }

    std::stringstream ss;
    ss << "+" << std::string(width, '-') << "+" << std::endl;
    for (const auto& line : canvas) ss << "|" << line << "|" << std::endl;
    ss << "+" << std::string(width, '-') << "+" << std::endl;
    return ss.str();
}

std::string StellarVisualizer::render_analysis_grid(const std::map<std::string, std::vector<float>>& grid, const std::vector<std::string>& col_labels) {
    std::stringstream ss;
    ss << std::left << std::setw(20) << "METRIC";
    for (const auto& label : col_labels) ss << "| " << std::setw(12) << label;
    ss << std::endl << std::string(20 + col_labels.size() * 15, '-') << std::endl;
    for (const auto& [row_name, values] : grid) {
        ss << std::left << std::setw(20) << row_name;
        for (float v : values) ss << "| " << std::fixed << std::setprecision(4) << std::setw(12) << v;
        ss << std::endl;
    }
    return ss.str();
}

} // namespace Stellar
} // namespace TissLM
