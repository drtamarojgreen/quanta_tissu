import os

# This class will be used to simulate exceptions thrown by the Tissu Sinew connector.
class TissuException(Exception):
    pass

class TissuConnectionException(TissuException):
    pass

class TissuQueryException(TissuException):
    pass

def register_steps(runner):

    # Helper to reset the context for a new scenario
    def reset_context(context):
        context.sim_state = {
            "server_running": False,
            "client_configured": False,
            "client": None,
            "session": None,
            "last_error": None,
            "last_result": None,
            "transaction": None,
            "log": []
        }

    @runner.step(r'a TissDB server is running')
    def server_is_running(context):
        reset_context(context)
        context.sim_state["server_running"] = True
        print("SIM: TissDB server is running.")

    @runner.step(r'no TissDB server is running')
    def server_is_not_running(context):
        reset_context(context)
        context.sim_state["server_running"] = False
        print("SIM: TissDB server is NOT running.")

    @runner.step(r'a C\+\+ application configures a TissuClient with the correct host and port')
    def configure_client(context):
        context.sim_state["client_configured"] = True
        print("SIM: TissuClient configured.")

    @runner.step(r'the application creates a TissuClient')
    def create_client(context):
        if context.sim_state["server_running"]:
            context.sim_state["client"] = {"status": "connected"}
            print("SIM: TissuClient created successfully.")
        else:
            # This step does not throw, the next one asserts the failure.
            context.sim_state["client"] = None
            print("SIM: TissuClient creation failed (as expected).")

    @runner.step(r'the application attempts to create a TissuClient')
    def attempt_create_client(context):
        # Simulate the try-catch block
        try:
            if not context.sim_state["server_running"]:
                raise TissuConnectionException("Cannot connect to server")
            context.sim_state["client"] = {"status": "connected"}
        except TissuConnectionException as e:
            context.sim_state["last_error"] = e
        print("SIM: Application attempted to create TissuClient.")

    @runner.step(r'the TissuClient should be created successfully')
    def client_created_successfully(context):
        assert context.sim_state["client"] is not None
        print("SIM: Verified TissuClient was created.")

    @runner.step(r'the client should be able to get a session from the connection pool')
    def get_session(context):
        if context.sim_state["client"]:
            context.sim_state["session"] = {"status": "active"}
            print("SIM: Session acquired from pool.")
        else:
            raise Exception("Cannot get session without a client")

    @runner.step(r'the client creation should fail with a TissuConnectionException')
    def client_creation_fails(context):
        assert isinstance(context.sim_state["last_error"], TissuConnectionException)
        print("SIM: Verified client creation failed with TissuConnectionException.")

    @runner.step(r'an application has a valid TissuSession')
    def has_valid_session(context):
        reset_context(context)
        context.sim_state["server_running"] = True
        context.sim_state["client"] = {"status": "connected"}
        context.sim_state["session"] = {"status": "active"}
        print("SIM: Application has a valid session.")

    @runner.step(r'the application runs the query "(.*)"')
    def run_simple_query(context, query):
        if query == "PING":
            context.sim_state["last_result"] = "PONG"
        else:
            context.sim_state["last_result"] = "OK"
        print(f"SIM: Ran query '{query}'.")

    @runner.step(r'it should receive a TissuResult')
    def receive_result(context):
        assert context.sim_state["last_result"] is not None
        print("SIM: Verified a result was received.")

    @runner.step(r'the result should contain the string "(.*)"')
    def result_contains_string(context, text):
        assert text in context.sim_state["last_result"]
        print(f"SIM: Verified result contains '{text}'.")

    @runner.step(r'the application runs the query "(.*)" with parameters:')
    def run_param_query(context, query, table):
        # table is a list of strings like ['| name   | John Doe |', '| age    | 30       |']
        params = {}
        for row in table:
            parts = [p.strip() for p in row.strip('|').split('|')]
            params[parts[0]] = parts[1]

        context.sim_state["last_query"] = {"query": query, "params": params}
        if "INSERT INTO users" in query:
            context.sim_state["log"].append(f"User '{params['name']}' inserted.")
        print(f"SIM: Ran parameterized query '{query}' with params {params}.")

    @runner.step(r'the query should execute successfully')
    def query_executes_successfully(context):
        # This is a conceptual step, success is implicit if no error was thrown.
        assert context.sim_state["last_error"] is None
        print("SIM: Query executed successfully.")

    @runner.step(r'a log should indicate that the user "(.*)" was inserted')
    def log_indicates_insertion(context, user_name):
        log_entry = f"User '{user_name}' inserted."
        assert log_entry in context.sim_state["log"]
        print(f"SIM: Verified log contains insertion message for '{user_name}'.")

    @runner.step(r'the application begins a transaction')
    def begin_transaction(context):
        context.sim_state["transaction"] = {"status": "active"}
        print("SIM: Transaction started.")

    @runner.step(r'commits the transaction')
    def commit_transaction(context):
        context.sim_state["transaction"]["status"] = "committed"
        print("SIM: Transaction committed.")

    @runner.step(r'the transaction should be committed successfully')
    def transaction_committed(context):
        assert context.sim_state["transaction"]["status"] == "committed"
        print("SIM: Verified transaction was committed.")

    @runner.step(r'runs a query that causes a TissuQueryException')
    def run_failing_query(context):
        try:
            raise TissuQueryException("Invalid syntax in query")
        except TissuQueryException as e:
            context.sim_state["last_error"] = e
            # In a real scenario, the session would handle this. We simulate the rollback.
            if context.sim_state["transaction"]:
                context.sim_state["transaction"]["status"] = "rolled back"
        print("SIM: Ran a query that caused a TissuQueryException.")

    @runner.step(r'the transaction should be automatically rolled back')
    def transaction_rolled_back(context):
        assert context.sim_state["transaction"]["status"] == "rolled back"
        print("SIM: Verified transaction was rolled back.")
