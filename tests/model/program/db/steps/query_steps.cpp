#include "../bdd_framework.h"
#include "../db_test_actions.h"

using namespace TissDB;
using namespace TissDB::BDD;

extern std::unique_ptr<DatabaseTestActions> g_actions;

STEP(R"bdd(I execute the TissQL query "(.*)" on "(.*)")bdd") {
    std::string res = g_actions->execute_query(matches[2], matches[1]);
    context.set("last_query_res", res);
}

STEP(R"bdd(the query result should contain "(.*)")bdd") {
    std::string res = context.get<std::string>("last_query_res");
    if (res.find(matches[1].str()) == std::string::npos) throw std::runtime_error("Result mismatch");
}

STEP(R"bdd(I query "(.*)" with "(.*)")bdd") {
    std::string res = g_actions->execute_query(matches[1], matches[2]);
    context.set("last_query_res", res);
}

STEP(R"bdd(the query results should contain "(.*)")bdd") {
    std::string res = context.get<std::string>("last_query_res");
    if (res.find(matches[1].str()) == std::string::npos) throw std::runtime_error("Result mismatch");
}
