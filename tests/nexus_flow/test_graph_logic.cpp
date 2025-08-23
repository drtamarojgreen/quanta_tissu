#include "../db/test_framework.h"
#include "../../quanta_tissu/nexus_flow/graph_logic.h"

// Note: The TEST_CASE macro is defined in ../db/test_framework.h
// It registers a test function that will be run by a test runner.

TEST_CASE(NexusFlow_Graph_InitialState) {
    Graph g;
    ASSERT_EQ(g.nodes.size(), 0);
    ASSERT_EQ(g.edges.size(), 0);
}

TEST_CASE(NexusFlow_Graph_AddNode) {
    Graph g;
    Node n = {1, 10, 20, 30, 5, "NodeA"};
    g.nodes.push_back(n);

    ASSERT_EQ(g.nodes.size(), 1);
    ASSERT_EQ(g.nodes[0].id, 1);
    ASSERT_EQ(g.nodes[0].x, 10);
    ASSERT_EQ(g.nodes[0].y, 20);
    ASSERT_EQ(g.nodes[0].z, 30);
    ASSERT_EQ(g.nodes[0].size, 5);
    ASSERT_EQ(g.nodes[0].label, "NodeA");
}

TEST_CASE(NexusFlow_Graph_AddEdge) {
    Graph g;
    Node n1 = {1, 0, 0, 0, 5, "Node1"};
    Node n2 = {2, 0, 0, 0, 5, "Node2"};
    g.nodes.push_back(n1);
    g.nodes.push_back(n2);

    Edge e = {1, 2};
    g.edges.push_back(e);

    ASSERT_EQ(g.edges.size(), 1);
    ASSERT_EQ(g.edges[0].node1_id, 1);
    ASSERT_EQ(g.edges[0].node2_id, 2);
}

TEST_CASE(NexusFlow_Graph_MultipleNodesAndEdges) {
    Graph g;
    g.nodes.push_back({1, 0, 0, 0, 5, "N1"});
    g.nodes.push_back({2, 0, 0, 0, 5, "N2"});
    g.nodes.push_back({3, 0, 0, 0, 5, "N3"});

    g.edges.push_back({1, 2});
    g.edges.push_back({2, 3});

    ASSERT_EQ(g.nodes.size(), 3);
    ASSERT_EQ(g.edges.size(), 2);
}

// --- Tests for 3D Math Utilities ---

// The necessary structs (Point3D, Point2D) and function declarations
// (rotateY, project) are now included via graph_logic.h.

// Helper to compare floating point numbers with a tolerance.
void ASSERT_POINT_EQ(const Point3D& expected, const Point3D& actual, double tolerance = 1e-6) {
    ASSERT_TRUE(std::abs(expected.x - actual.x) < tolerance);
    ASSERT_TRUE(std::abs(expected.y - actual.y) < tolerance);
    ASSERT_TRUE(std::abs(expected.z - actual.z) < tolerance);
}

TEST_CASE(NexusFlow_Math_RotateY) {
    const double PI = 3.1415926535;
    Point3D p = {10, 0, 0};

    // Rotate 90 degrees
    Point3D p90 = rotateY(p, PI / 2.0);
    ASSERT_POINT_EQ({0, 0, -10}, p90);

    // Rotate 180 degrees
    Point3D p180 = rotateY(p, PI);
    ASSERT_POINT_EQ({-10, 0, 0}, p180);

    // Rotate -90 degrees (or 270)
    Point3D p270 = rotateY(p, -PI / 2.0);
    ASSERT_POINT_EQ({0, 0, 10}, p270);
}

TEST_CASE(NexusFlow_Math_Project) {
    // A point directly in front, should not be scaled and centered.
    Point3D p1 = {0, 0, 0};
    Point2D proj1 = project(p1);
    ASSERT_EQ(40, proj1.x); // SCREEN_WIDTH / 2
    ASSERT_EQ(12, proj1.y); // SCREEN_HEIGHT / 2

    // A point farther away, should be scaled down.
    // The z value must be > -PERSPECTIVE_FOV to be visible.
    Point3D p2 = {40, 20, 128}; // z = PERSPECTIVE_FOV
    Point2D proj2 = project(p2);
    // scale_factor = 128 / (128 + 128) = 0.5
    // x = (40 * 0.5) + 40 = 60
    // y = (20 * 0.5) + 12 = 22
    ASSERT_EQ(60, proj2.x);
    ASSERT_EQ(22, proj2.y);
}
