#pragma once

#include <string>
#include <vector>
#include <memory>

namespace TissLang {

// Base class for all commands that can appear inside a STEP block.
// Using a base class allows for a polymorphic collection of commands.
class Command {
public:
    virtual ~Command() = default;
};

// Represents a WRITE command: WRITE "filepath" <<LANG ... LANG
class WriteCommand : public Command {
public:
    std::string filepath;
    std::string content;
    std::string language_hint; // Optional language hint (e.g., PYTHON)

    WriteCommand(std::string path, std::string cont, std::string lang = "")
        : filepath(std::move(path)), content(std::move(cont)), language_hint(std::move(lang)) {}
};

// Represents a RUN command: RUN "shell command"
class RunCommand : public Command {
public:
    std::string command;

    explicit RunCommand(std::string cmd) : command(std::move(cmd)) {}
};

// Represents an ASSERT command: ASSERT [condition]
class AssertCommand : public Command {
public:
    // For now, the condition is stored as a single string.
    // This could be parsed into a more structured representation in a future phase.
    std::string condition;

    explicit AssertCommand(std::string cond) : condition(std::move(cond)) {}
};

// Represents a STEP block, which has a description and contains a list of commands.
class Step {
public:
    std::string description;
    std::vector<std::unique_ptr<Command>> commands;

    explicit Step(std::string desc) : description(std::move(desc)) {}
};

// Represents the root of the AST for a TissLang script.
// It contains the overall task description and a sequence of steps.
class Script {
public:
    std::string task_description;
    std::string global_language_hint; // From the #TISS! pragma
    std::vector<std::unique_ptr<Step>> steps;

    explicit Script(std::string desc, std::string lang = "")
        : task_description(std::move(desc)), global_language_hint(std::move(lang)) {}
};

} // namespace TissLang
