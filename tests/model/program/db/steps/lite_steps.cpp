#include "../bdd_framework.h"
#include "../../../../../quanta_tissu/tisslm/program/db/tissdb_lite_client.h"
#include "../../../../../tissdb/json/json.h"

using namespace TissDB;
using namespace TissDB::BDD;

static std::unique_ptr<TissDBLiteClient> g_lite_client;

STEP(R"bdd(a TissDBLite server is running on "(.*)" at port (\d+))bdd") {
    g_lite_client = std::make_unique<TissDBLiteClient>(matches[1], std::stoi(matches[2]));
}

STEP(R"bdd(I send a TissDBLite command "(.*)")bdd") {
    Json::JsonObject obj;
    obj["action"] = std::string(matches[1]);
    std::string res = g_lite_client->sendCommand(Json::JsonValue(obj).serialize());
    context.set("last_lite_res", res);
}

STEP(R"bdd(the command should succeed)bdd") {
    std::string res = context.get<std::string>("last_lite_res");
    if (res.find("success") == std::string::npos) throw std::runtime_error("Lite cmd failed");
}
