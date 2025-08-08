# TissLang Parsing Challenges

This document outlines the key challenges and complexities involved in building and maintaining a robust parser for TissLang. Understanding these challenges is crucial for ensuring the parser is correct, scalable, and provides a good developer experience.

---

## 1. Stateful Parsing and Context Management

TissLang is not a context-free language. The validity of a command often depends on the commands that came before it. The parser must operate as a state machine.

- **Challenge**: The parser must track its current state (e.g., `IDLE`, `IN_STEP`, `IN_WRITE`). A `RUN` command is only valid inside a `STEP` or `SETUP` block, and a `STEP` block cannot be nested inside another `STEP`.
- **Implication**: A simple line-by-line pattern match is insufficient. The parser needs to manage a state variable and transition between states based on block-opening (`{`) and block-closing (`}`) tokens. Failure to manage state correctly can lead to mis-parsing valid scripts or accepting invalid ones.

## 2. Heredoc Parsing for the `WRITE` Command

The `WRITE` command uses a heredoc syntax (`<<DELIMITER ... DELIMITER`) to handle multi-line string content. This requires a special parsing mode.

- **Challenge**: When the parser encounters a `WRITE` command, it must:
    1.  Extract the heredoc delimiter (e.g., `PYTHON`).
    2.  Switch to a "content consumption" mode.
    3.  Read every subsequent line as raw string content, *without* attempting to parse them as TissLang commands.
    4.  Switch back to normal parsing mode only after encountering a line that exactly matches the stored delimiter.
- **Implication**: This is a common source of bugs. The parser must handle cases where the delimiter is never found (resulting in an unclosed block error) and ensure that the delimiter matching is exact.

## 3. Nested Structures and Recursion

TissLang's structure is inherently nested. A `TASK` contains `STEP`s, which in turn contain a list of commands. Future features like `IF/ELSE` will introduce further nesting.

- **Challenge**: The parser must correctly handle nested blocks of commands. While the current specification doesn't allow `STEP`s inside `STEP`s, the parser's design must be robust enough to handle the nested command lists within these blocks.
- **Implication**: A common approach is to use a recursive descent strategy, where the parser calls itself to handle the contents of a block. This requires careful management of the input stream (lines of code) and line numbers to ensure error messages remain accurate.

## 4. Rich and Actionable Error Reporting

A good parser must provide excellent error messages. For a language like TissLang, this means reporting not just *that* an error occurred, but *why* it's an error in the current context.

- **Challenge**: The parser needs to maintain an accurate line number throughout the parsing process. When an error is found, it should provide a message like: `Error on line 42: Unexpected 'STEP' command. 'STEP' blocks cannot be nested.` This is more helpful than a generic `Syntax Error`.
- **Implication**: This requires careful propagation of line numbers and context. If the parser consumes multiple lines at once (e.g., for a `WRITE` block), it must correctly report the starting line of the construct that caused the error.

## 5. Scalability for Future Language Features

The TissLang plan includes advanced features like `IF/ELSE`, `PROMPT_AGENT`, and `PAUSE`. The parser's architecture must be flexible enough to accommodate these additions without a complete rewrite.

- **Challenge**: `IF/ELSE` introduces branching and conditional blocks. `PROMPT_AGENT` might have its own multi-line syntax. The initial parser design should not be so rigid that adding new keywords or block structures is difficult.
- **Implication**: Using a pattern-based dispatch mechanism, where the parser tries to match a line against a list of known command patterns, is a good strategy. This allows new commands to be added by simply defining a new regex and a handler function.

## 6. Parsing Complex Expressions

Currently, the `ASSERT` command's condition is treated as a simple string. However, the plan implies more complex logic (`LAST_RUN.EXIT_CODE == 0`).

- **Challenge**: As the language evolves, conditions within `ASSERT` or `IF` statements could become complex expressions involving variables, operators (`==`, `!=`, `CONTAINS`), and boolean logic (`AND`, `OR`). Parsing a full expression language is significantly more complex than parsing the current command-based syntax.
- **Implication**: This would require a dedicated expression parser, potentially using classic parsing techniques like shunting-yard or Pratt parsing to build an expression tree. This is the most significant long-term challenge for the parser's evolution.

## 7. Security and Command Injection

The `RUN` command is extremely powerful and presents a significant security risk if not handled carefully.

- **Challenge**: The parser itself cannot distinguish between a safe command (`RUN "ls -l"`) and a malicious one (`RUN "rm -rf /"`). While parsing is about syntactic correctness, the language design must consider the semantic danger of its constructs.
- **Implication**: This challenge transcends the parser and touches the execution engine. The language specification and its tools (parser, linter, executor) must work together. The parser may need to identify potentially dangerous patterns, and the execution engine *must* run commands in a tightly controlled sandbox. The parser's output (the AST) should make it easy for downstream tools to apply security policies.

## 8. Handling Mixed Modalities (Code and Natural Language)

The planned `PROMPT_AGENT` command introduces natural language queries directly into the structured script.

- **Challenge**: The parser must correctly identify and extract the natural language prompt as a single string literal, even if it contains characters that might otherwise be significant (like quotes or braces). It needs to know where the structured command ends and the unstructured prompt begins and ends.
- **Implication**: This requires a very robust string-parsing mechanism. The parser must be careful not to misinterpret parts of the natural language prompt as TissLang syntax. This is similar to the heredoc challenge but applies to single-line commands.

## 9. Ambiguity in Grammar

As new control flow structures like `IF/ELSE` are added, the grammar can become ambiguous without careful design.

- **Challenge**: Consider a nested `IF` statement: `IF condition1 { IF condition2 { ... } ELSE { ... } }`. This is a classic "dangling else" problem. Does the `ELSE` belong to `condition1` or `condition2`? While the use of curly braces `{}` in TissLang mitigates this specific example, other ambiguities can arise.
- **Implication**: The language grammar must be defined formally (e.g., using EBNF) to be unambiguous. The parser must correctly associate blocks with their corresponding statements. This becomes more complex if `ELSE IF` constructs are introduced.

## 10. Variable Scoping and Symbol Table Management

The `READ "file" AS my_var` command introduces variables into the script's context.

- **Challenge**: While the parser's primary job is to create the AST, a more advanced parser or a subsequent semantic analysis pass needs to manage a symbol table. This involves tracking where variables are defined and where they are used. What happens if the same variable name is redefined? Is it allowed?
- **Implication**: For the language to be robust, the toolchain needs to detect errors like using a variable before it's defined. This logic often lives just beyond the parser in a "semantic analyzer," but the parser is responsible for providing the raw information (identifying declarations and uses) to make this analysis possible.

## 11. Tooling and Developer Experience (DX)

The design of a language directly impacts the quality of tools that can be built for it. Features like Language Server Protocol (LSP) support for autocompletion, linting, and go-to-definition rely on the parser's ability to produce a rich and accurate AST.

- **Challenge**: A parser designed only for execution may discard information crucial for good tooling, such as comments, whitespace, and the exact position of tokens. For example, to build an auto-formatter, the parser must preserve the original structure of the code.
- **Implication**: A "lossless" parser that creates a Concrete Syntax Tree (CST) before the AST might be necessary for advanced tooling. This adds complexity but enables a much richer developer experience, which is critical for language adoption.

## 12. Cross-Platform Compatibility of `RUN`

The `RUN` command executes shell commands, but shell syntax is not universal. A script written for a Linux `bash` environment may fail on Windows `PowerShell` or `cmd`.

- **Challenge**: The parser treats the `RUN` command's content as an opaque string. It has no awareness of the underlying operating system or shell. This can lead to non-portable TissLang scripts.
- **Implication**: While this is primarily an execution-time problem, the language design might need to evolve to address it. This could involve adding syntax to specify the target shell (`RUN --shell=bash "..."`) or introducing higher-level, platform-agnostic commands (e.g., `COPY_FILE "src" "dest"`) that the execution engine translates into the appropriate native command.

## 13. Advanced Error Recovery

A simple parser will stop at the first error it encounters. A more user-friendly parser will attempt to recover from an error and continue parsing to report multiple issues in a single pass.

- **Challenge**: Implementing error recovery is notoriously difficult. After encountering a syntax error, the parser must intelligently decide which tokens to discard and where to resume parsing without getting stuck in an infinite loop of cascaded, phantom errors.
- **Implication**: This requires a more sophisticated parsing strategy than simple line-by-line regex matching. The parser needs to understand expected "synchronization points" in the grammar (e.g., the start of a new `STEP` or a closing brace `}`) where it can confidently resume.

## 14. Macro and Templating Systems

As TissLang grows, users may want to define their own reusable commands or templates to reduce boilerplate.

- **Challenge**: A macro or templating system introduces a new phase to the compilation process. The parser would need to handle a pre-processing step where macros are identified and expanded into standard TissLang syntax *before* the main parsing begins.
- **Implication**: This adds significant complexity. The parser must manage two levels of syntax (the macro definition language and the core language) and provide clear error messages that can trace a problem back to the original macro definition, not just the expanded code.

## 15. Language Versioning and Backward Compatibility

As TissLang evolves, new keywords will be added and old behaviors may change. A script written today might not be valid for a parser built two years from now.

- **Challenge**: The parser must be able to handle scripts written for different versions of the language. This could require a version pragma at the top of a script (e.g., `#TISS_VERSION >= 1.2`), forcing the parser to enable or disable features based on the declared version.
- **Implication**: This adds a layer of conditional logic to the entire parsing process. The parser can no longer be a single, monolithic set of rules; it must become a version-aware system, which complicates maintenance and testing.

## 16. Asynchronous Operations

Some commands, particularly `RUN` or future `PROMPT_AGENT` calls, can be long-running. A purely synchronous execution model can be inefficient. The language might need to support asynchronous execution.

- **Challenge**: Introducing `async` capabilities requires new syntax (e.g., `RUN --async "..." AS job1`) and a way to await results (`AWAIT job1`). The parser must recognize these new keywords and structures.
- **Implication**: This fundamentally changes the execution model from a simple linear AST walk to a more complex state machine that can track running jobs. The parser's output must be rich enough for the execution engine to manage this complexity.

## 17. Introduction of a Type System

As variables (`READ ... AS my_var`) and expressions become more common, the need for a type system will emerge to catch errors before execution.

- **Challenge**: The parser would need to be extended to recognize type annotations (`READ "data.json" AS my_data: Map`). This moves the parser's responsibility beyond just recognizing keywords into the realm of type checking and semantic analysis.
- **Implication**: This is a major leap in complexity. It likely requires a separate semantic analysis phase after initial parsing to build a symbol table with type information and validate that variables are used correctly.

## 18. Concurrency and Parallel Execution

To improve performance, users may want to run independent `STEP`s or commands concurrently.

- **Challenge**: The language would need syntax to define parallel blocks of execution (e.g., `PARALLEL { STEP "A" { ... } STEP "B" { ... } }`). The parser must correctly identify these blocks and structure the AST in a way that the execution engine can understand.
- **Implication**: This dramatically complicates the execution engine's state management. If parallel steps can modify the same files or variables, the system must handle race conditions and ensure deterministic outcomes, which is a significant engineering challenge. The parser's role is to provide a clear and unambiguous representation of the user's intent for parallel execution.
