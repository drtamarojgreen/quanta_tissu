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
