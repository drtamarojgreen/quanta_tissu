#include "../bdd_framework.h"
#include "../db_test_actions.h"

using namespace TissDB;
using namespace TissDB::BDD;

extern std::unique_ptr<DatabaseTestActions> g_actions;

STEP(R"bdd(I begin a transaction)bdd") {
    int64_t tid = g_actions->begin_transaction();
    context.set("tid", tid);
}

STEP(R"bdd(I commit the transaction)bdd") {
    g_actions->commit_transaction(context.get<int64_t>("tid"));
}

STEP(R"bdd(I rollback the transaction)bdd") {
    g_actions->rollback_transaction(context.get<int64_t>("tid"));
}
