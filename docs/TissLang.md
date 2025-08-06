# TissLang 🦰⚙️

**TissLang** is a behavioral pseudo-language for communicating with Large Language Models (LLMs) using a compact, expressive syntax. It was created to simplify and streamline repetitive prompting tasks and bring back the power of concise command-line-like interaction in a world full of verbose chat input.

## �� Why TissLang?

LLMs have become powerful engines of productivity — but with that power comes verbosity. Writing long-form sentences to perform simple tasks wastes time and bores even the most patient human. TissLang is designed to:

- Reduce prompt fatigue
- Formalize behavioral interaction patterns with LLMs
- Encode intent compactly and consistently
- Bring back the efficiency of `C:\>` to the age of AI

## 🚀 Vision and Roadmap

TissLang is currently in its initial, behavioral phase, focused on simple, direct commands. However, the vision for TissLang is to evolve it into a powerful, declarative language for orchestrating complex agentic workflows.

The development plan is divided into several phases:
1.  **Core Language & Parser**: Establish the fundamental syntax (the current version).
2.  **Agent Integration**: Connect TissLang to agent tools for file I/O, shell execution, and validation.
3.  **Advanced Features**: Introduce variables, control flow, and sub-tasks.
4.  **Ecological Awareness**: Integrate cost analysis and "green" planning.
5.  **Human-in-the-Loop**: Add commands for seamless human-agent collaboration.

For a detailed breakdown of the future of TissLang, see the [TissLang Development Plan](TissLang_plan.md).

## 📦 File Format

TissLang files use the `.tiss` extension and begin with a shebang-style marker:

```tiss
#TISS!
```

This marker identifies the file as a TissLang script.

## 🧠 Syntax Overview

TissLang's syntax is line-based and designed for simplicity. It currently supports the following constructs:

### 1. 👋 Greetings

The parser recognizes informal, human-like greetings. Any line containing common greeting words (e.g., "hi," "hello," "gm") is classified as a `greeting`. This allows for a more natural, conversational start to a script.

```tiss
GM CG! U today, how are?
hello there, let's get to work
```

These are identified and interpreted as part of the behavioral context.

---

### 2. 🏷️ Metadata

Metadata is specified in a `Key=Value` format. It is used to set the context for subsequent commands. The most common use case is defining the programming language for a task.

```tiss
Language=Py
Task=Implement a new feature
```

The parser stores this information in a metadata dictionary.

---

### 3. 📥 Queries

Queries are specific, self-contained requests. They must start with `Query=` and end with `ENDQuery`. This block structure ensures that multi-word queries are captured as a single command.

```tiss
Query=Read a text file into a list of stringsENDQuery
Query=How to connect to a PostgreSQL database in Python?ENDQuery
```

---

### 4. 🧮 Conditionals

Conditional logic is encoded using a `W{...}T{...}E{...}` block, which stands for **When-Then-Else**. This provides a simple way to introduce branching logic into a script.

```tiss
W{Language=Py}T{Generate Fibonacci code}E{Ask for a supported language}
```

This reads:  
**When** the condition `Language=Py` is true (based on metadata), **Then** execute the "Generate Fibonacci code" task, **Else** execute the "Ask for a supported language" task. The `E{...}` (Else) block is optional.

---

## 🔎 Practical Examples

Here are a few examples of how TissLang can be used to script common tasks.

### Example 1: Simple Code Generation

This script greets the model, sets the language context, and asks for a specific piece of code.

```tiss
#TISS!
hello
Language=Python
Query=Generate a function that calculates the factorial of a numberENDQuery
```

### Example 2: Conditional Logic

This script uses a conditional block to generate code based on the language context.

```tiss
#TISS!
Language=JavaScript
W{Language=Python}T{Generate a class for a doubly linked list}E{Notify that only Python is supported for this task}
```

## ⚙️ Parser and Future Development

The current Python parser (`tisslang_parser.py`) handles all the features described above. It serves as the foundation for the more advanced execution engine planned for future phases.

While the current parser is simple, the long-term goal is to build it out to support the full, agentic version of TissLang, including:
-   **Structured Commands**: `READ`, `WRITE`, `RUN`, and `ASSERT` for agentic workflows.
-   **Advanced Logic**: Variables, loops, and error handling (`TRY/CATCH`).
-   **Self-Generation**: Enabling the QuantaTissu agent to write its own `.tiss` plans.

## 🧩 Philosophy

TissLang is not a programming language. It is not a markup language.

It is a **command dialect** — a specialized syntax designed to be the missing link between the cold, unambiguous precision of a command-line interface (CLI) and the fluid, adaptive brilliance of Large Language Models. It provides just enough structure to ensure intent is preserved, while remaining lightweight and easy for both humans and AIs to read and write.

## 🧪 Run It

The parser can be executed in Python:

```bash
python tisslang_parser.py
```

Example output:
```python
{'type': 'greeting', 'content': 'GM CG! U today, how are?'}
{'type': 'query', 'content': 'Read text file into a list'}
{'type': 'conditional', 'condition': 'Language=Py', 'true_branch': 'Generate Fibonacci code', 'else_branch': 'Ask for supported language'}
```

---

## 📚 License

TissLang is an experimental language. Use, extend, and remix freely.

---

## 🧠 Created By

[QuantaTissu](https://github.com/drtamarojgreen/quanta_tissu) — a custom agentic AI framework for building code-native intelligence.
