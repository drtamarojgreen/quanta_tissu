#include "graph_logic.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <sstream> // Required for building JSON query strings
#include <windows.h> // For Windows-specific console functions
#include <conio.h>   // For _getch()

/**
 * @brief Constructor for GraphLogic class.
 * Initializes the canvas and loads the graph data from the TissDB server.
 */
GraphLogic::GraphLogic() {
    canvas.resize(SCREEN_HEIGHT, std::string(SCREEN_WIDTH, ' '));
    loadGraphsFromTissDB();
}

/**
 * @brief The main execution loop of the application.
 *
 * It cycles through the predefined graphs, rendering each one and waiting for
 * the user to press the spacebar to continue.
 */
void GraphLogic::run() {
    if (graphs.empty()) {
        std::cerr << "No graphs were loaded. Please ensure TissDB is running and populated." << std::endl;
        return;
    }
    for (size_t i = 0; i < graphs.size(); ++i) {
        clearCanvas();
        drawGraph(graphs[i]);
        renderCanvas();
        waitForSpacebar();
    }
}

/**
 * @brief Loads graph data from the TissDB server.
 *
 * This function connects to a TissDB instance, queries for graph data,
 * parses the JSON response, and populates the `graphs` vector.
 */
void GraphLogic::loadGraphsFromTissDB() {
    TissDB::Test::HttpClient client("localhost", 8080);

    for (int i = 1; i <= 3; ++i) {
        // Form the JSON query to select the graph document
        std::stringstream query_stream;
        query_stream << "{\"query\": \"SELECT * WHERE graph_id = " << i << "\"}";
        std::string query_json = query_stream.str();

        // Send the query to the TissDB server
        TissDB::Test::HttpResponse response = client.post("/_query", query_json, "application/json");

        if (response.status_code != 200) {
            std::cerr << "Error fetching graph " << i << ": HTTP " << response.status_code << std::endl;
            continue;
        }

        try {
            // Parse the JSON response body
            Json parsed_json = Json::parse(response.body);

            // The response should be an array containing a single graph document
            if (parsed_json.type() != Json::Type::ARRAY || parsed_json.as_array().empty()) {
                std::cerr << "Warning: No data returned for graph " << i << std::endl;
                continue;
            }
            const Json& graph_doc = parsed_json.as_array()[0];

            Graph g;

            // Extract nodes
            const Json& nodes_json = graph_doc["nodes"];
            for (const auto& node_json : nodes_json.as_array()) {
                Node n;
                n.id = node_json["id"].as_integer();
                n.x = node_json["x"].as_integer();
                n.y = node_json["y"].as_integer();
                n.size = node_json["size"].as_integer();
                n.label = node_json["label"].as_string();
                g.nodes.push_back(n);
            }

            // Extract edges
            const Json& edges_json = graph_doc["edges"];
            for (const auto& edge_json : edges_json.as_array()) {
                Edge e;
                e.node1_id = edge_json["from"].as_integer();
                e.node2_id = edge_json["to"].as_integer();
                g.edges.push_back(e);
            }

            graphs.push_back(g);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing JSON for graph " << i << ": " << e.what() << std::endl;
        }
    }
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