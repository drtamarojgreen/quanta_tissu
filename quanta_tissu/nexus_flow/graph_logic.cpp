#include "graph_logic.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <sstream>   // Required for building JSON query strings
#include <stdexcept> // Required for std::runtime_error
#include <cstdio>    // For _popen, _pclose
#include <array>     // For std::array
#include <cstdlib>   // For rand, srand
#include <ctime>     // For time
#include <algorithm> // For std::sort
#include <windows.h> // For Windows-specific console functions
#include <conio.h>   // For _getch()

// --- 3D Math Utilities ---

struct Point3D {
    double x, y, z;
};

struct Point2D {
    int x, y;
};

const double PERSPECTIVE_FOV = 128.0; // Field of view for projection. Larger values mean less distortion.

/**
 * @brief Rotates a 3D point around the Y-axis.
 * @param p The point to rotate.
 * @param angle The rotation angle in radians.
 * @return The rotated point.
 */
Point3D rotateY(const Point3D& p, double angle) {
    double cos_a = cos(angle);
    double sin_a = sin(angle);
    double new_x = p.x * cos_a + p.z * sin_a;
    double new_z = -p.x * sin_a + p.z * cos_a;
    return {new_x, p.y, new_z};
}

/**
 * @brief Projects a 3D point to 2D screen coordinates using perspective projection.
 * @param p The 3D point.
 * @return The projected 2D point.
 */
Point2D project(const Point3D& p) {
    double scale_factor = PERSPECTIVE_FOV / (PERSPECTIVE_FOV + p.z);
    int screen_x = static_cast<int>(p.x * scale_factor) + (SCREEN_WIDTH / 2);
    int screen_y = static_cast<int>(p.y * scale_factor) + (SCREEN_HEIGHT / 2);
    return {screen_x, screen_y};
}

// --- End 3D Math Utilities ---

/**
 * @brief Constructor for GraphLogic class.
 * Initializes the canvas.
 */
GraphLogic::GraphLogic() {
    canvas.resize(SCREEN_HEIGHT, std::string(SCREEN_WIDTH, ' '));
    srand(time(NULL)); // Seed random number generator for node positioning
}

/**
 * @brief The main execution loop of the application.
 *
 * Displays a menu and allows the user to choose a workflow.
 */
void GraphLogic::run() {
    while (true) {
        clearCanvas();
        canvas[4] = "  Nexus Flow";
        canvas[5] = "  ----------";
        canvas[7] = "  1. Load Graphs from TissDB";
        canvas[8] = "  2. Generate Graph from Prompt";
        canvas[10] = "  3. Exit";
        canvas[12] = "  Enter your choice: ";
        renderCanvas();

        char choice = _getch();

        if (choice == '1') {
            runTissDBWorkflow();
        } else if (choice == '2') {
            runGenerationWorkflow();
        } else if (choice == '3') {
            break; // Exit loop
        }
    }
}

/**
 * @brief Executes the workflow for loading and displaying graphs from TissDB.
 */
void GraphLogic::runTissDBWorkflow() {
    graphs.clear(); // Clear previous data
    loadGraphsFromTissDB();

    if (graphs.empty()) {
        clearCanvas();
        canvas[SCREEN_HEIGHT / 2] = std::string(2, ' ') + "No graphs loaded. Is TissDB running and populated?";
        renderCanvas();
        waitForSpacebar();
        return;
    }

    for (const auto& graph : graphs) {
        animateGraph(graph);
    }
}

// Helper function to execute a command and capture its output
std::string executeCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    _pclose(pipe);
    return result;
}

/**
 * @brief Executes the workflow for generating a graph from a user prompt.
 */
void GraphLogic::runGenerationWorkflow() {
    std::string prompt = getUserPrompt();
    if (prompt.empty()) {
        return; // User entered nothing
    }

    clearCanvas();
    canvas[5] = "  Generating graph from prompt...";
    canvas[6] = "  Please wait, this may take a moment...";
    renderCanvas();

    // Construct the command to execute the Python script
    std::string script_path = "../tisslm/generate_graph_from_prompt.py";
    std::string escaped_prompt = prompt;
    size_t pos = 0;
    while ((pos = escaped_prompt.find('"', pos)) != std::string::npos) {
        escaped_prompt.replace(pos, 1, "\\\"");
        pos += 2;
    }
    std::stringstream command;
    command << "python " << script_path << " \"" << escaped_prompt << "\"";

    try {
        std::string json_output = executeCommand(command.str());
        Json parsed_json = Json::parse(json_output);

        // Check for an error message from the Python script
        if (parsed_json.type() == Json::Type::OBJECT && parsed_json.as_object().count("error")) {
            clearCanvas();
            canvas[5] = "  An error occurred during graph generation:";
            std::string error_details = parsed_json["message"].as_string();
            canvas[7] = "  " + error_details.substr(0, SCREEN_WIDTH - 4);
            renderCanvas();
            waitForSpacebar();
            return;
        }

        // Populate the graph object from the parsed JSON
        Graph g;
        const Json& nodes_json = parsed_json["nodes"];
        for (const auto& node_json : nodes_json.as_array()) {
            Node n;
            n.id = node_json["id"].as_integer();
            n.label = node_json["label"].as_string();
            // Assign random positions for visualization
            n.x = (rand() % (SCREEN_WIDTH - 15)) + 5;
            n.y = (rand() % (SCREEN_HEIGHT - 5)) + 2;
            n.z = (rand() % 20) - 10; // Assign random Z for 3D effect
            n.size = 3;
            g.nodes.push_back(n);
        }

        const Json& edges_json = parsed_json["edges"];
        for (const auto& edge_json : edges_json.as_array()) {
            Edge e;
            e.node1_id = edge_json["from"].as_integer();
            e.node2_id = edge_json["to"].as_integer();
            g.edges.push_back(e);
        }

        // Render the newly generated graph
        animateGraph(g);

    } catch (const std::exception& e) {
        clearCanvas();
        canvas[5] = "  An error occurred:";
        canvas[7] = std::string("  ") + e.what();
        renderCanvas();
        waitForSpacebar();
    }
}

/**
 * @brief Prompts the user to enter a text string and returns it.
 *
 * @return The string entered by the user.
 */
std::string GraphLogic::getUserPrompt() {
    clearCanvas();
    canvas[5] = "  Enter a prompt to generate a graph (e.g., 'a simple solar system'):";
    renderCanvas();

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;

    // Show cursor
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    // Move cursor to input position
    COORD coord = {2, 7}; // Column 2, Row 7
    SetConsoleCursorPosition(hConsole, coord);

    std::string prompt;
    std::getline(std::cin, prompt);

    // Hide cursor again
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    return prompt;
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
            n.z = (rand() % 20) - 10; // Assign random Z for 3D effect
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

// A struct to hold the projected 2D coordinates and depth of a node
struct ProjectedNode {
    int id;
    Point2D pos;
    double z;
    int original_size;
    std::string label;
};

/**
 * @brief Draws a complete graph onto the canvas using 3D projection.
 * @param graph The graph to be drawn.
 * @param angle The angle to rotate the graph around the Y-axis.
 */
void GraphLogic::drawGraph(const Graph& graph, double angle) {
    // 1. Rotate and project all nodes
    std::vector<ProjectedNode> projected_nodes;
    for (const auto& node : graph.nodes) {
        // Center the graph around the origin for rotation
        Point3D p = {
            static_cast<double>(node.x - SCREEN_WIDTH / 2),
            static_cast<double>(node.y - SCREEN_HEIGHT / 2),
            static_cast<double>(node.z)
        };
        Point3D rotated_p = rotateY(p, angle);
        Point2D projected_p = project(rotated_p);
        projected_nodes.push_back({node.id, projected_p, rotated_p.z, node.size, node.label});
    }

    // 2. Depth sort the nodes (farther nodes first)
    std::sort(projected_nodes.begin(), projected_nodes.end(), [](const ProjectedNode& a, const ProjectedNode& b) {
        return a.z < b.z;
    });

    // 3. Draw Edges
    for (const auto& edge : graph.edges) {
        auto it1 = std::find_if(projected_nodes.begin(), projected_nodes.end(), [id = edge.node1_id](const ProjectedNode& n){ return n.id == id; });
        auto it2 = std::find_if(projected_nodes.begin(), projected_nodes.end(), [id = edge.node2_id](const ProjectedNode& n){ return n.id == id; });

        if (it1 != projected_nodes.end() && it2 != projected_nodes.end()) {
            drawLine(it1->pos.x, it1->pos.y, it2->pos.x, it2->pos.y);
        }
    }

    // 4. Draw Nodes and Labels (in sorted order)
    for (const auto& p_node : projected_nodes) {
        double scale_factor = PERSPECTIVE_FOV / (PERSPECTIVE_FOV + p_node.z);
        int new_size = std::max(1, static_cast<int>(p_node.original_size * scale_factor * 0.5));

        char node_char;
        if (p_node.z < -PERSPECTIVE_FOV / 2.0) node_char = '.';
        else if (p_node.z < 0) node_char = 'o';
        else node_char = '@';

        drawNode(p_node.pos.x, p_node.pos.y, new_size, node_char);
        drawLabel(p_node.pos.x, p_node.pos.y, new_size, p_node.label);
    }
}

/**
 * @brief Runs an animation loop to draw a rotating 3D graph.
 * @param graph The graph to animate.
 */
void GraphLogic::animateGraph(const Graph& graph) {
    double angle = 0.0;
    while (!_kbhit()) { // Loop until a key is pressed
        clearCanvas();
        drawGraph(graph, angle);

        std::string help_text = "Rotating... Press any key to continue.";
        for(size_t i = 0; i < help_text.length(); ++i) {
            if (i < SCREEN_WIDTH) {
                canvas[SCREEN_HEIGHT - 1][i] = help_text[i];
            }
        }

        renderCanvas();

        angle += 0.05;
        if (angle > 6.28318) { // 2 * PI
            angle -= 6.28318;
        }

        Sleep(30); // ~33 FPS
    }
    _getch(); // Consume the key press to exit
}

/**
 * @brief Draws a single node on the canvas at a given position.
 * @param x The screen x-coordinate.
 * @param y The screen y-coordinate.
 * @param size The size of the node square.
 * @param c The character to draw the node with.
 */
void GraphLogic::drawNode(int x, int y, int size, char c) {
    int half_size = size / 2;
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int px = x + j - half_size;
            int py = y + i - half_size;
            if (py >= 0 && py < SCREEN_HEIGHT && px >= 0 && px < SCREEN_WIDTH) {
                canvas[py][px] = c;
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