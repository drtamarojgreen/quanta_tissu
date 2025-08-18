#!/bin/bash

# This script populates TissDB with the graph data for the nexus_flow application.
# It assumes the TissDB server is running on localhost:8080.
# It posts three documents, one for each graph, to the database.

echo "Populating TissDB with graph data..."
echo ""

# --- Graph 1: 4 Nodes ---
echo "Posting Graph 1..."
curl -X POST http://localhost:8080/ \
-H "Content-Type: application/json" \
-d @- <<'EOF'
{
  "graph_id": 1,
  "nodes": [
    {"id": 1, "x": 10, "y": 5, "size": 5, "label": "Challenge negative thoughts"},
    {"id": 2, "x": 30, "y": 15, "size": 3, "label": "Cognitive-Behavioral Therapy"},
    {"id": 3, "x": 50, "y": 8, "size": 5, "label": "Practice self-compassion"},
    {"id": 4, "x": 25, "y": 2, "size": 1, "label": "Develop coping strategies"}
  ],
  "edges": [
    {"from": 1, "to": 2},
    {"from": 1, "to": 3},
    {"from": 2, "to": 3},
    {"from": 2, "to": 4}
  ]
}
EOF
echo ""
echo ""

# --- Graph 2: 8 Nodes ---
echo "Posting Graph 2..."
curl -X POST http://localhost:8080/ \
-H "Content-Type: application/json" \
-d @- <<'EOF'
{
  "graph_id": 2,
  "nodes": [
    {"id": 1, "x": 5, "y": 3, "size": 5, "label": "Mindfulness and relaxation"},
    {"id": 2, "x": 20, "y": 10, "size": 3, "label": "Break harmful patterns"},
    {"id": 3, "x": 18, "y": 9, "size": 1, "label": "A holistic approach"},
    {"id": 4, "x": 40, "y": 5, "size": 5, "label": "Build resilience"},
    {"id": 5, "x": 60, "y": 18, "size": 3, "label": "Emotional regulation"},
    {"id": 6, "x": 70, "y": 2, "size": 1, "label": "Seek professional help"},
    {"id": 7, "x": 35, "y": 20, "size": 3, "label": "It's okay to not be okay"},
    {"id": 8, "x": 5, "y": 20, "size": 5, "label": "Your feelings are valid"}
  ],
  "edges": [
    {"from": 1, "to": 2}, {"from": 1, "to": 8}, {"from": 2, "to": 4},
    {"from": 3, "to": 4}, {"from": 4, "to": 5}, {"from": 5, "to": 7},
    {"from": 6, "to": 7}, {"from": 7, "to": 8}
  ]
}
EOF
echo ""
echo ""

# --- Graph 3: 16 Nodes ---
echo "Posting Graph 3..."
curl -X POST http://localhost:8080/ \
-H "Content-Type: application/json" \
-d @- <<'EOF'
{
    "graph_id": 3,
    "nodes": [
        {"id": 1, "x": 5, "y": 2, "size": 5, "label": "Set healthy boundaries"},
        {"id": 2, "x": 15, "y": 10, "size": 3, "label": "A journey of self-discovery"},
        {"id": 3, "x": 25, "y": 5, "size": 1, "label": "Nurture your well-being"},
        {"id": 4, "x": 35, "y": 12, "size": 3, "label": "Bloom into your better self"},
        {"id": 5, "x": 45, "y": 3, "size": 5, "label": "Challenge negative thoughts"},
        {"id": 6, "x": 55, "y": 15, "size": 1, "label": "Cognitive-Behavioral Therapy"},
        {"id": 7, "x": 65, "y": 8, "size": 3, "label": "Practice self-compassion"},
        {"id": 8, "x": 75, "y": 20, "size": 5, "label": "Develop coping strategies"},
        {"id": 9, "x": 8, "y": 22, "size": 1, "label": "Mindfulness and relaxation"},
        {"id": 10, "x": 20, "y": 18, "size": 3, "label": "Break harmful patterns"},
        {"id": 11, "x": 30, "y": 23, "size": 5, "label": "A holistic approach"},
        {"id": 12, "x": 40, "y": 17, "size": 1, "label": "Build resilience"},
        {"id": 13, "x": 50, "y": 21, "size": 3, "label": "Emotional regulation"},
        {"id": 14, "x": 60, "y": 14, "size": 5, "label": "Seek professional help"},
        {"id": 15, "x": 70, "y": 19, "size": 1, "label": "It's okay to not be okay"},
        {"id": 16, "x": 5, "y": 15, "size": 3, "label": "Your feelings are valid"}
    ],
    "edges": [
        {"from": 1, "to": 2}, {"from": 2, "to": 3}, {"from": 3, "to": 4}, {"from": 4, "to": 5},
        {"from": 5, "to": 6}, {"from": 6, "to": 7}, {"from": 7, "to": 8}, {"from": 8, "to": 9},
        {"from": 9, "to": 10}, {"from": 10, "to": 11}, {"from": 11, "to": 12}, {"from": 12, "to": 13},
        {"from": 13, "to": 14}, {"from": 14, "to": 15}, {"from": 15, "to": 16}, {"from": 16, "to": 1},
        {"from": 1, "to": 10}, {"from": 2, "to": 9}, {"from": 3, "to": 12}, {"from": 4, "to": 14}
    ]
}
EOF
echo ""
echo ""

echo "TissDB population complete."
