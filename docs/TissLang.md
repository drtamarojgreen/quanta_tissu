# TissLang 🦰⚙️

**TissLang** is a behavioral pseudo-language for communicating with Large Language Models (LLMs) using a compact, expressive syntax. It was created to simplify and streamline repetitive prompting tasks and bring back the power of concise command-line-like interaction in a world full of verbose chat input.

## �� Why TissLang?

LLMs have become powerful engines of productivity — but with that power comes verbosity. Writing long-form sentences to perform simple tasks wastes time and bores even the most patient human. TissLang is designed to:

- Reduce prompt fatigue
- Formalize behavioral interaction patterns with LLMs
- Encode intent compactly and consistently
- Bring back the efficiency of `C:\>` to the age of AI

## 📦 File Format

TissLang files use the `.tiss` extension and begin with a shebang-style marker:

```tiss
#TISS!
```

## 🧠 Syntax Overview

TissLang supports:

### 1. 👋 Greetings

Recognizes natural human greetings like:

```tiss
GM CG! U today, how are?
hello there
```

These are identified and interpreted as part of the behavioral context.

---

### 2. 🏷️ Metadata

Set metadata like the programming language:

```tiss
Language=Py
```

---

### 3. 📥 Queries

Perform specific programming or research requests:

```tiss
Query=Read text file into a listENDQuery
```

---

### 4. 🧮 Conditionals

Use **W{}T{}E{}** blocks to encode logic (When → Then → Else):

```tiss
W{Language=Py}T{Generate Fibonacci code}E{Ask for supported language}
```

This reads:  
**When** Language is Py, **Then** generate Fibonacci code, **Else** ask for supported language.

---

## 🔎 Example

```tiss
#TISS!
GM CG! U today, how are?
Language=Py
Query=Create dictionary from two listsENDQuery
W{Language=Py}T{Generate sample code}E{Notify of invalid language}
```

## ⚙️ Parser

The current Python parser supports:
- `#TISS!` shebang
- Metadata blocks
- Query blocks
- Conditional logic with W{}T{}E{}
- Human-like greetings

Future plans:
- Arithmetic and data structures
- Loops and iteration
- Nested logic
- Symbolic tokens

## 🧩 Philosophy

TissLang is not a programming language.
It is not a markup language.

It is a **command dialect** — the missing link between the cold precision of CLI and the adaptive brilliance of LLMs.

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
