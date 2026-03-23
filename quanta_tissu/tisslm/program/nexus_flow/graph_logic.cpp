#include "graph_logic.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <array>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include "../db/tissdb_client.h"
#include "../../../../tissdb/json/json.h"

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#define popen _popen
#define pclose _pclose
#else
#include <unistd.h>
#include <termios.h>
#include <cstdio>
#endif

const double PERSPECTIVE_FOV = 128.0;

Point3D rotateY(const Point3D& p, double angle) {
    double cos_a = cos(angle), sin_a = sin(angle);
    return {p.x * cos_a + p.z * sin_a, p.y, -p.x * sin_a + p.z * cos_a};
}

Point2D project(const Point3D& p) {
    double scale = PERSPECTIVE_FOV / (PERSPECTIVE_FOV + p.z);
    return {static_cast<int>(p.x * scale) + (SCREEN_WIDTH / 2), static_cast<int>(p.y * scale) + (SCREEN_HEIGHT / 2)};
}

void CrossPlatformSleep(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

#ifndef _WIN32
struct termios old_settings;
void enableRawMode() {
    tcgetattr(STDIN_FILENO, &old_settings);
    struct termios raw = old_settings;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}
void disableRawMode() { tcsetattr(STDIN_FILENO, TCSANOW, &old_settings); }
int CrossPlatformKbhit() {
    struct timeval tv = { 0, 0 };
    fd_set fds; FD_ZERO(&fds); FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}
int CrossPlatformGetch() { return getchar(); }
#endif

GraphLogic::GraphLogic() {
    canvas.resize(SCREEN_HEIGHT, std::string(SCREEN_WIDTH, ' '));
    srand(time(NULL));
}

void GraphLogic::run() {
#ifndef _WIN32
    enableRawMode();
#endif
    while (true) {
        clearCanvas();
        canvas[4] = "  Nexus Flow"; canvas[5] = "  ----------";
        canvas[7] = "  1. Load Graphs from TissDB"; canvas[8] = "  2. Generate Graph from Prompt";
        canvas[10] = "  3. Exit"; canvas[12] = "  Enter your choice: ";
        renderCanvas();
        char choice = CrossPlatformGetch();
        if (choice == '1' || choice == '2') {
#ifndef _WIN32
            disableRawMode();
#endif
            if (choice == '1') runTissDBWorkflow();
            else runGenerationWorkflow();
#ifndef _WIN32
            enableRawMode();
#endif
        } else if (choice == '3') break;
    }
#ifndef _WIN32
    disableRawMode();
#endif
}

void GraphLogic::loadGraphsFromTissDB() {
    TissDB::TissDBClient client("127.0.0.1", 9876, "nexus_flow");
    try {
        auto docs = client.search_documents("graphs", "{}");
        for (const auto& doc : docs) {
            Graph g;
            for (const auto& elem : doc.elements) {
                if (elem.key == "data" && std::holds_alternative<std::string>(elem.value)) {
                    auto root = TissDB::Json::JsonValue::parse(std::get<std::string>(elem.value));
                    for (const auto& n_val : root.as_object().at("nodes").as_array()) {
                        auto n_obj = n_val.as_object();
                        g.nodes.push_back({(int)n_obj.at("id").as_number(), (int)n_obj.at("x").as_number(), (int)n_obj.at("y").as_number(), (int)n_obj.at("z").as_number(), (int)n_obj.at("size").as_number(), n_obj.at("label").as_string()});
                    }
                    for (const auto& e_val : root.as_object().at("edges").as_array()) {
                        auto e_obj = e_val.as_object();
                        g.edges.push_back({(int)e_obj.at("from").as_number(), (int)e_obj.at("to").as_number()});
                    }
                }
            }
            graphs.push_back(g);
        }
    } catch (...) { std::cerr << "Failed to load graphs from TissDB." << std::endl; }
}

void GraphLogic::runTissDBWorkflow() {
    graphs.clear(); loadGraphsFromTissDB();
    if (graphs.empty()) {
        clearCanvas(); canvas[SCREEN_HEIGHT / 2] = "  No graphs loaded. Is TissDB running and populated?";
        renderCanvas(); waitForSpacebar(); return;
    }
    for (const auto& graph : graphs) animateGraph(graph);
}

std::string executeCommand(const std::string& command) {
    std::array<char, 128> buffer; std::string result;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe)) result += buffer.data();
    pclose(pipe); return result;
}

void GraphLogic::runGenerationWorkflow() {
    std::string prompt = getUserPrompt();
    if (prompt.empty()) return;
    clearCanvas(); canvas[5] = "  Generating graph..."; renderCanvas();
    std::string escaped = prompt; size_t pos = 0;
    while ((pos = escaped.find('"', pos)) != std::string::npos) { escaped.replace(pos, 1, "\\\""); pos += 2; }
    try {
        std::string json_output = executeCommand("python ../pipelines/generate_graph_from_prompt.py \"" + escaped + "\"");
        auto root = TissDB::Json::JsonValue::parse(json_output);
        Graph g;
        for (const auto& n_val : root.as_object().at("nodes").as_array()) {
            auto n_obj = n_val.as_object();
            g.nodes.push_back({(int)n_obj.at("id").as_number(), rand() % 70 + 5, rand() % 20 + 2, rand() % 20 - 10, 3, n_obj.at("label").as_string()});
        }
        for (const auto& e_val : root.as_object().at("edges").as_array()) {
            auto e_obj = e_val.as_object();
            g.edges.push_back({(int)e_obj.at("from").as_number(), (int)e_obj.at("to").as_number()});
        }
        animateGraph(g);
    } catch (...) { waitForSpacebar(); }
}

std::string GraphLogic::getUserPrompt() {
    clearCanvas(); canvas[5] = "  Enter prompt: "; renderCanvas();
#ifdef _WIN32
    COORD coord = {16, 5}; SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
    printf("\033[6;17H"); fflush(stdout);
#endif
    std::string p; std::getline(std::cin, p); return p;
}

void GraphLogic::initializeGraphs() {
    // Legacy static initialization removed in favor of loadGraphsFromTissDB
}

void GraphLogic::clearCanvas() {
#ifdef _WIN32
    for (int i = 0; i < SCREEN_HEIGHT; ++i) canvas[i] = std::string(SCREEN_WIDTH, ' ');
#else
    printf("\033[2J\033[H");
#endif
}

void GraphLogic::renderCanvas() {
#ifdef _WIN32
    COORD coord = {0, 0}; SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
    printf("\033[H");
#endif
    for (const auto& line : canvas) std::cout << line << std::endl;
    std::cout << std::flush;
}

struct ProjectedNode { int id; Point2D pos; double z; int size; std::string label; };

void GraphLogic::drawGraph(const Graph& graph, double angle) {
    std::vector<ProjectedNode> projected;
    for (const auto& node : graph.nodes) {
        Point3D p = {(double)node.x - SCREEN_WIDTH / 2, (double)node.y - SCREEN_HEIGHT / 2, (double)node.z};
        Point3D rot = rotateY(p, angle); Point2D proj = project(rot);
        projected.push_back({node.id, proj, rot.z, node.size, node.label});
    }
    std::sort(projected.begin(), projected.end(), [](const ProjectedNode& a, const ProjectedNode& b) { return a.z < b.z; });
    for (const auto& edge : graph.edges) {
        auto it1 = std::find_if(projected.begin(), projected.end(), [&](const ProjectedNode& n) { return n.id == edge.node1_id; });
        auto it2 = std::find_if(projected.begin(), projected.end(), [&](const ProjectedNode& n) { return n.id == edge.node2_id; });
        if (it1 != projected.end() && it2 != projected.end()) drawLine(it1->pos.x, it1->pos.y, it2->pos.x, it2->pos.y);
    }
    for (const auto& p : projected) {
        double scale = PERSPECTIVE_FOV / (PERSPECTIVE_FOV + p.z);
        int sz = std::max(1, (int)(p.size * scale * 0.5));
        drawNode(p.pos.x, p.pos.y, sz, p.z < 0 ? 'o' : '@');
        drawLabel(p.pos.x, p.pos.y, sz, p.label);
    }
}

void GraphLogic::animateGraph(const Graph& graph) {
    double angle = 0.0;
#ifndef _WIN32
    enableRawMode();
#endif
    while (!CrossPlatformKbhit()) {
        clearCanvas(); drawGraph(graph, angle); renderCanvas();
        angle += 0.05; CrossPlatformSleep(30);
    }
    CrossPlatformGetch();
#ifndef _WIN32
    disableRawMode();
#endif
}

void GraphLogic::drawNode(int x, int y, int size, char c) {
    for (int i = 0; i < size; ++i) for (int j = 0; j < size; ++j) {
        int px = x + j - size / 2, py = y + i - size / 2;
        if (py >= 0 && py < SCREEN_HEIGHT && px >= 0 && px < SCREEN_WIDTH) canvas[py][px] = c;
    }
}

void GraphLogic::drawLine(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1, dy = y2 - y1, steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0) return;
    float x_inc = (float)dx / steps, y_inc = (float)dy / steps, x = x1, y = y1;
    for (int i = 0; i <= steps; ++i) {
        int px = (int)round(x), py = (int)round(y);
        if (py >= 0 && py < SCREEN_HEIGHT && px >= 0 && px < SCREEN_WIDTH) canvas[py][px] = (dx * dy > 0 ? '\\' : '/');
        x += x_inc; y += y_inc;
    }
}

void GraphLogic::drawLabel(int x, int y, int size, const std::string& text) {
    int start_x = x + size + 2, label_y = y + size / 2;
    if (label_y >= 0 && label_y < SCREEN_HEIGHT) {
        for (size_t i = 0; i < text.length(); ++i) {
            int px = start_x + i;
            if (px >= 0 && px < SCREEN_WIDTH) canvas[label_y][px] = text[i];
            else break;
        }
    }
}

void GraphLogic::waitForSpacebar() {
#ifndef _WIN32
    enableRawMode();
#endif
    while (CrossPlatformGetch() != ' ');
#ifndef _WIN32
    disableRawMode();
#endif
}
