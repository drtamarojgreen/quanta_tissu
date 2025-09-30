#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "../../../quanta_tissu/tisslm/compiler/parser.h"

// Helper function to print test names
void test_case(const std::string& name) {
    std::cout << "--- " << name << " ---" << std::endl;
}

void test_simple_valid_script() {
    test_case("Test Simple Valid Script");

    const std::string code = R"(
#TISS! Language=CPP

TASK "Create a simple C++ program"

STEP "Write the main file" {
    WRITE "main.cpp" <<CPP
#include <iostream>

int main() {
    std::cout << "Hello, Tiss!" << std::endl;
    return 0;
}
CPP
}

STEP "Compile the program" {
    RUN "g++ -o main main.cpp"
}

STEP "Run the program and check output" {
    RUN "./main > output.txt"
    ASSERT LAST_RUN.EXIT_CODE == 0
}

)";

    TissLang::Parser parser(code);
    std::unique_ptr<TissLang::Script> script = parser.parse();

    assert(script != nullptr);
    assert(script->global_language_hint == "CPP");
    assert(script->task_description == "Create a simple C++ program");
    assert(script->steps.size() == 3);

    // Check Step 1
    const auto& step1 = script->steps[0];
    assert(step1->description == "Write the main file");
    assert(step1->commands.size() == 1);
    const auto* write_cmd = dynamic_cast<TissLang::WriteCommand*>(step1->commands[0].get());
    assert(write_cmd != nullptr);
    assert(write_cmd->filepath == "main.cpp");
    assert(write_cmd->language_hint == "CPP");
    assert(write_cmd->content == "#include <iostream>\n\nint main() {\n    std::cout << \"Hello, Tiss!\" << std::endl;\n    return 0;\n}");

    // Check Step 2
    const auto& step2 = script->steps[1];
    assert(step2->description == "Compile the program");
    assert(step2->commands.size() == 1);
    const auto* run_cmd1 = dynamic_cast<TissLang::RunCommand*>(step2->commands[0].get());
    assert(run_cmd1 != nullptr);
    assert(run_cmd1->command == "g++ -o main main.cpp");

    // Check Step 3
    const auto& step3 = script->steps[2];
    assert(step3->description == "Run the program and check output");
    assert(step3->commands.size() == 2);
    const auto* run_cmd2 = dynamic_cast<TissLang::RunCommand*>(step3->commands[0].get());
    assert(run_cmd2 != nullptr);
    assert(run_cmd2->command == "./main > output.txt");
    const auto* assert_cmd = dynamic_cast<TissLang::AssertCommand*>(step3->commands[1].get());
    assert(assert_cmd != nullptr);
    assert(assert_cmd->condition == "LAST_RUN.EXIT_CODE == 0");

    std::cout << "Passed!" << std::endl;
}

void test_missing_task_declaration() {
    test_case("Test Missing TASK Declaration");

    const std::string code = R"(
STEP "This should fail" {
    RUN "echo hello"
}
)";

    bool exception_thrown = false;
    try {
        TissLang::Parser parser(code);
        parser.parse();
    } catch (const std::runtime_error& e) {
        exception_thrown = true;
        assert(std::string(e.what()) == "Expected TASK declaration.");
    }
    assert(exception_thrown);

    std::cout << "Passed!" << std::endl;
}

void test_invalid_step_block() {
    test_case("Test Invalid STEP Block (Missing Closing Brace)");

    const std::string code = R"(
TASK "Test invalid step"

STEP "A step that is not closed" {
    RUN "echo 'this will not end well'"
// No closing brace
)";

    bool exception_thrown = false;
    try {
        TissLang::Parser parser(code);
        parser.parse();
    } catch (const std::runtime_error& e) {
        exception_thrown = true;
        assert(std::string(e.what()) == "Expected '}' to end STEP block, but reached end of file.");
    }
    assert(exception_thrown);

    std::cout << "Passed!" << std::endl;
}

void test_unknown_command() {
    test_case("Test Unknown Command");

    const std::string code = R"(
TASK "Test unknown command"

STEP "A step with a weird command" {
    THINK "is this a valid command?"
}
)";

    bool exception_thrown = false;
    try {
        TissLang::Parser parser(code);
        parser.parse();
    } catch (const std::runtime_error& e) {
        exception_thrown = true;
        assert(std::string(e.what()) == "Unknown command: THINK \"is this a valid command?\"");
    }
    assert(exception_thrown);

    std::cout << "Passed!" << std::endl;
}

void test_write_command_parsing() {
    test_case("Test WRITE Command Parsing");

    const std::string code = R"(
TASK "Test write command"

STEP "Write a file" {
    WRITE "test.txt" <<TXT
Hello, this is a test.
This is the second line.
TXT
}
)";

    TissLang::Parser parser(code);
    std::unique_ptr<TissLang::Script> script = parser.parse();

    assert(script != nullptr);
    assert(script->steps.size() == 1);
    const auto& step = script->steps[0];
    assert(step->commands.size() == 1);
    const auto* write_cmd = dynamic_cast<TissLang::WriteCommand*>(step->commands[0].get());
    assert(write_cmd != nullptr);
    assert(write_cmd->filepath == "test.txt");
    assert(write_cmd->language_hint == "TXT");
    assert(write_cmd->content == "Hello, this is a test.\nThis is the second line.");

    std::cout << "Passed!" << std::endl;
}

void test_setup_command() {
    test_case("Test SETUP Command with VAR and LOG");

    const std::string code = R"(
TASK "Test setup command"

SETUP "Initialize environment" {
    VAR my_var = "test_value"
    LOG "Variable set to: {my_var}"
}
)";

    TissLang::Parser parser(code);
    std::unique_ptr<TissLang::Script> script = parser.parse();

    assert(script != nullptr);
    assert(script->setup_block != nullptr);
    assert(script->setup_block->description == "Initialize environment");
    assert(script->setup_block->commands.size() == 2);

    const auto* var_cmd = dynamic_cast<TissLang::VarCommand*>(script->setup_block->commands[0].get());
    assert(var_cmd != nullptr);
    assert(var_cmd->variable_name == "my_var");
    assert(var_cmd->value == "test_value");

    const auto* log_cmd = dynamic_cast<TissLang::LogCommand*>(script->setup_block->commands[1].get());
    assert(log_cmd != nullptr);
    assert(log_cmd->message == "Variable set to: {my_var}");

    std::cout << "Passed!" << std::endl;
}

void test_if_else_block() {
    test_case("Test IF-ELSE Block");

    const std::string code = R"(
TASK "Test if-else"

STEP "Conditional execution" {
    IF condition == true
        LOG "Condition is true"
    ELSE
        LOG "Condition is false"
    ENDIF
}
)";

    TissLang::Parser parser(code);
    std::unique_ptr<TissLang::Script> script = parser.parse();

    assert(script != nullptr);
    assert(script->steps.size() == 1);
    const auto& step = script->steps[0];
    assert(step->commands.size() == 1);

    const auto* if_cmd = dynamic_cast<TissLang::IfCommand*>(step->commands[0].get());
    assert(if_cmd != nullptr);
    assert(if_cmd->condition == "condition == true");
    assert(if_cmd->then_commands.size() == 1);
    assert(if_cmd->else_commands.size() == 1);

    const auto* then_log = dynamic_cast<TissLang::LogCommand*>(if_cmd->then_commands[0].get());
    assert(then_log != nullptr);
    assert(then_log->message == "Condition is true");

    const auto* else_log = dynamic_cast<TissLang::LogCommand*>(if_cmd->else_commands[0].get());
    assert(else_log != nullptr);
    assert(else_log->message == "Condition is false");

    std::cout << "Passed!" << std::endl;
}

void test_prompt_agent_command() {
    test_case("Test PROMPT_AGENT Command");

    const std::string code = R"(
TASK "Test prompt agent"

STEP "Get summary from agent" {
    PROMPT_AGENT "Summarize this text" -> summary_var
}
)";

    TissLang::Parser parser(code);
    std::unique_ptr<TissLang::Script> script = parser.parse();

    assert(script != nullptr);
    assert(script->steps.size() == 1);
    const auto& step = script->steps[0];
    assert(step->commands.size() == 1);

    const auto* prompt_cmd = dynamic_cast<TissLang::PromptAgentCommand*>(step->commands[0].get());
    assert(prompt_cmd != nullptr);
    assert(prompt_cmd->prompt_text == "Summarize this text");
    assert(prompt_cmd->variable_name == "summary_var");

    std::cout << "Passed!" << std::endl;
}

void run_parser_tests() {
    test_simple_valid_script();
    test_missing_task_declaration();
    test_invalid_step_block();
    test_unknown_command();
    test_write_command_parsing();
    test_setup_command();
    test_if_else_block();
    test_prompt_agent_command();
    // More tests will be added here
}
