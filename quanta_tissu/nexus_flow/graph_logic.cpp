#include "graph_logic.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <windows.h> // For Windows-specific console functions
#include <conio.h>   // For _getch()

/**
 * @brief Constructor for GraphLogic class.
 * Initializes the canvas and loads the sample graph data.
 */
GraphLogic::GraphLogic() {
    canvas.resize(SCREEN_HEIGHT, std::string(SCREEN_WIDTH, ' '));
    initializeGraphs();
}

/**
 * @brief The main execution loop of the application.
 *
 * It cycles through the predefined graphs, rendering each one and waiting for
 * the user to press the spacebar to continue.
 */
void GraphLogic::run() {
    for (size_t i = 0; i < graphs.size(); ++i) {
        clearCanvas();
        drawGraph(graphs[i]);
        renderCanvas();
        waitForSpacebar();
    }
}

/**
 * @brief Initializes the three sample graphs (4, 8, and 16 nodes).
 *
 * This function populates the `graphs` vector with predefined node positions,
 * sizes, labels, and the edges connecting them. The data is designed to
 * demonstrate occlusion and various node sizes.
 */
void GraphLogic::initializeGraphs() {
    // Cognitive Behavioral Therapy related labels
    std::vector<std::string> cbt_labels = {
        "Challenge negative thoughts", "Cognitive-Behavioral Therapy", "Practice self-compassion",
        "Develop coping strategies", "Mindfulness and relaxation", "Break harmful patterns",
        "A holistic approach", "Build resilience", "Emotional regulation",
        "Seek professional help", "It's okay to not be okay", "Your feelings are valid",
        "Set healthy boundaries", "A journey of self-discovery", "Nurture your well-being",
        "Bloom into your better self"
    };

    // Graph 1: 4 Nodes
    Graph g1;
    g1.nodes = {
        {1, 10, 5, 5, cbt_labels[0]},
        {2, 30, 15, 3, cbt_labels[1]},
        {3, 50, 8, 5, cbt_labels[2]},
        {4, 25, 2, 1, cbt_labels[3]}
    };
    g1.edges = {{1, 2}, {1, 3}, {2, 3}, {2, 4}};
    graphs.push_back(g1);

    // Graph 2: 8 Nodes (demonstrates occlusion)
    Graph g2;
    g2.nodes = {
        {1, 5, 3, 5, cbt_labels[4]},
        {2, 20, 10, 3, cbt_labels[5]},
        {3, 18, 9, 1, cbt_labels[6]}, // Occluded by node 2
        {4, 40, 5, 5, cbt_labels[7]},
        {5, 60, 18, 3, cbt_labels[8]},
        {6, 70, 2, 1, cbt_labels[9]},
        {7, 35, 20, 3, cbt_labels[10]},
        {8, 5, 20, 5, cbt_labels[11]}
    };
    g2.edges = {{1, 2}, {1, 8}, {2, 4}, {3, 4}, {4, 5}, {5, 7}, {6, 7}, {7, 8}};
    graphs.push_back(g2);

    // Graph 3: 16 Nodes
    Graph g3;
    g3.nodes = {
        {1, 5, 2, 5, cbt_labels[12]}, {2, 15, 10, 3, cbt_labels[13]},
        {3, 25, 5, 1, cbt_labels[14]}, {4, 35, 12, 3, cbt_labels[15]},
        {5, 45, 3, 5, cbt_labels[0]}, {6, 55, 15, 1, cbt_labels[1]},
        {7, 65, 8, 3, cbt_labels[2]}, {8, 75, 20, 5, cbt_labels[3]},
        {9, 8, 22, 1, cbt_labels[4]}, {10, 20, 18, 3, cbt_labels[5]},
        {11, 30, 23, 5, cbt_labels[6]}, {12, 40, 17, 1, cbt_labels[7]},
        {13, 50, 21, 3, cbt_labels[8]}, {14, 60, 14, 5, cbt_labels[9]},
        {15, 70, 19, 1, cbt_labels[10]}, {16, 5, 15, 3, cbt_labels[11]}
    };
    g3.edges = {
        {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7}, {7, 8}, {8, 9},
        {9, 10}, {10, 11}, {11, 12}, {12, 13}, {13, 14}, {14, 15}, {15, 16}, {16, 1},
        {1, 10}, {2, 9}, {3, 12}, {4, 14} // Some cross connections
    };
    graphs.push_back(g3);
}

/**
 * @brief Clears the canvas by filling it with space characters.
 */
void GraphLogic::clearCanvas() {
    for (int i = 0; i < SCREEN_HEIGHT; ++i) {
        canvas[i] = std::string(SCREEN_WIDTH, ' ');
    }
}

/**
 * @brief Renders the canvas to the console.
 *
 * Uses Windows-specific functions to move the cursor to the top-left
 * to prevent flickering during redraw.
 */
void GraphLogic::renderCanvas() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = {0, 0};
    SetConsoleCursorPosition(hConsole, coord);
    for (int i = 0; i < SCREEN_HEIGHT; ++i) {
        std::cout << canvas[i] << std::endl;
    }
}

/**
 * @brief Draws a complete graph onto the canvas.
 *
 * The drawing order is edges, then nodes, then labels to handle occlusion.
 * @param graph The graph to be drawn.
 */
void GraphLogic::drawGraph(const Graph& graph) {
    // Draw edges first
    for (const auto& edge : graph.edges) {
        const Node* n1 = nullptr;
        const Node* n2 = nullptr;
        for (const auto& node : graph.nodes) {
            if (node.id == edge.node1_id) n1 = &node;
            if (node.id == edge.node2_id) n2 = &node;
        }
        if (n1 && n2) {
            drawLine(n1->x + n1->size / 2, n1->y + n1->size / 2, n2->x + n2->size / 2, n2->y + n2->size / 2);
        }
    }

    // Draw nodes
    for (const auto& node : graph.nodes) {
        drawNode(node);
    }

    // Draw labels last to appear on top
    for (const auto& node : graph.nodes) {
        drawLabel(node.x, node.y, node.size, node.label);
    }
}

/**
 * @brief Draws a single node on the canvas.
 *
 * A node is represented as a square of 'x' characters.
 * @param node The node to draw.
 */
void GraphLogic::drawNode(const Node& node) {
    for (int i = 0; i < node.size; ++i) {
        for (int j = 0; j < node.size; ++j) {
            int px = node.x + j;
            int py = node.y + i;
            if (py >= 0 && py < SCREEN_HEIGHT && px >= 0 && px < SCREEN_WIDTH) {
                canvas[py][px] = 'x';
            }
        }
    }
}

/**
 * @brief Draws a line between two points using a simplified DDA algorithm.
 *
 * Uses '/' and '\' characters to represent the line.
 * @param x1 The x-coordinate of the starting point.
 * @param y1 The y-coordinate of the starting point.
 * @param x2 The x-coordinate of the ending point.
 * @param y2 The y-coordinate of the ending point.
 */
void GraphLogic::drawLine(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0) return;

    float x_inc = static_cast<float>(dx) / steps;
    float y_inc = static_cast<float>(dy) / steps;

    float x = x1;
    float y = y1;

    for (int i = 0; i <= steps; ++i) {
        int px = static_cast<int>(round(x));
        int py = static_cast<int>(round(y));

        if (py >= 0 && py < SCREEN_HEIGHT && px >= 0 && px < SCREEN_WIDTH) {
            // Determine character based on slope
            if ((dx > 0 && dy > 0) || (dx < 0 && dy < 0)) {
                canvas[py][px] = '\\';
            } else {
                canvas[py][px] = '/';
            }
        }
        x += x_inc;
        y += y_inc;
    }
}

/**
 * @brief Draws a text label to the right of a node.
 *
 * The horizontal offset depends on the node's size. The label is truncated
 * if it extends beyond the screen width.
 * @param x The x-coordinate of the node's top-left corner.
 * @param y The y-coordinate of the node's top-left corner.
 * @param nodeSize The size of the node (1, 3, or 5).
 * @param text The label text to draw.
 */
void GraphLogic::drawLabel(int x, int y, int nodeSize, const std::string& text) {
    int label_x_offset;
    switch (nodeSize) {
        case 1: label_x_offset = 2; break;
        case 3: label_x_offset = 3; break;
        case 5: label_x_offset = 4; break;
        default: label_x_offset = 2; break;
    }

    int start_x = x + nodeSize + label_x_offset;
    int label_y = y + nodeSize / 2;

    if (label_y >= 0 && label_y < SCREEN_HEIGHT) {
        for (size_t i = 0; i < text.length(); ++i) {
            int px = start_x + i;
            if (px >= 0 && px < SCREEN_WIDTH) {
                canvas[label_y][px] = text[i];
            } else {
                break; // Truncate if off-screen
            }
        }
    }
}

/**
 * @brief Pauses execution until the user presses the spacebar.
 */
void GraphLogic::waitForSpacebar() {
    std::cout << "\nPress spacebar to continue..." << std::flush;
    while (true) {
        if (_kbhit()) {
            int ch = _getch();
            if (ch == ' ') {
                break;
            }
        }
    }
}