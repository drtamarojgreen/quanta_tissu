#ifndef GRAPH_LOGIC_H
#define GRAPH_LOGIC_H

#include <vector>
#include <string>

// Define screen dimensions
const int SCREEN_WIDTH = 80;
const int SCREEN_HEIGHT = 25;

// --- 3D Math Utilities ---
// Moved here so they can be shared with test files.
struct Point3D {
    double x, y, z;
};

struct Point2D {
    int x, y;
};
// --- End 3D Math Utilities ---

// Forward declarations for math functions so they can be tested.
Point3D rotateY(const Point3D& p, double angle);
Point2D project(const Point3D& p);

/**
 * @struct Node
 * @brief Represents a single node in the graph.
 *
 * Stores the node's ID, position (x, y), size, and a text label.
 */
struct Node {
    int id;
    int x, y, z;
    int size;
    std::string label;
};

/**
 * @struct Edge
 * @brief Represents an edge connecting two nodes.
 *
 * Stores the IDs of the two nodes it connects.
 */
struct Edge {
    int node1_id;
    int node2_id;
};

/**
 * @struct Graph
 * @brief Represents a graph, containing a collection of nodes and edges.
 */
struct Graph {
    std::vector<Node> nodes;
    std::vector<Edge> edges;
};

/**
 * @class GraphLogic
 * @brief Manages the application logic for graph creation, rendering, and user interaction.
 *
 * This class handles the main application loop, initializes graph data,
 * and draws the graphs to the console.
 */
class GraphLogic {
public:
    // Constructor
    GraphLogic();

    // Main application loop
    void run();

private:
    // The screen buffer
    std::vector<std::string> canvas;

    // Storage for the three graph versions
    std::vector<Graph> graphs;

    // Workflows
    void runTissDBWorkflow();
    void runGenerationWorkflow();

    // Helper methods
    std::string getUserPrompt();
    void loadGraphsFromTissDB();

    // Drawing functions
    void clearCanvas();
    void renderCanvas();
    void drawGraph(const Graph& graph, double angle);
    void drawNode(int x, int y, int size, char c);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawLabel(int x, int y, int nodeSize, const std::string& text);
    void animateGraph(const Graph& graph);

    // User input
    void waitForSpacebar();

    // Graph initialization
    void initializeGraphs();
};

#endif // GRAPH_LOGIC_H