**Designing Conformance Suite**

> This document outlines a design for a language conformance suite. It includes a proposal for a "GreenScore" metric, testing strategies using golden files and property tests, and a structured, multi-AI collaboration protocol for language development. The deliverables are designed to be minimal yet valuable, including an EBNF grammar, JSON AST examples, and parser skeletons for key languages.
# TissLang: an AI-built language, by design

TissLang is designed to be a language that promotes clear thinking and ecological computing. It is unique in its goal of being authored collaboratively by multiple AI systems from its inception. This document outlines the language specification and the protocol for its AI-driven co-creation.

---

## North star and constraints

- **Purpose:** TissLang is the intermediary between what people type and what they want—clarity over cleverness, minimal compute over brute force.
- **Ethos:** Edge-first, eco-aware, deterministic when it matters.
- **Guardrails:** Must run on Pi-class devices, avoid giant contexts, favor retrieval, and be legible to humans.

---

## Deliverables we’ll generate with AI

- **Spec v1.0:** Syntax, semantics, and eco directives.
- **Grammar:** EBNF and ABNF versions.
- **AST schema:** JSON shape with validation rules.
- **Interpreter contract:** How directives map to runtime knobs.
- **Reference parsers:** Rust and TypeScript, minimal and fast.
- **Conformance suite:** Golden inputs/outputs, error snapshots.
- **Style guide:** Error messages, comments, and examples.
- **Eco math:** A tiny, transparent GreenScore.

---

## Multi‑AI collaboration protocol

Use this lightweight ritual to get “three minds” to converge fast. You can paste these prompts into your other assistants.

1) Proposal round (Diverge)
- Prompt to run elsewhere:
  “Propose a compact, eco-first DSL called TissLang for controlling small LLM/RAG systems on Raspberry Pi. Include: goals, 10–12 directives, EBNF grammar, 5 examples, AST JSON for one example.”

2) Critique round (Converge)
- Prompt:
  “Critique this TissLang spec for ambiguity, parser feasibility, and eco alignment. Flag collisions, propose precise fixes, and rewrite the grammar to remove shift/reduce risks.”

3) Synthesis round (Decide)
- Prompt:
  “Synthesize the best pieces into a single spec. Output: final EBNF, directive table with defaults, validation rules, and 8 conformance tests.”

4) Implementation round (Build)
- Prompt:
  “Generate a minimal, dependency-light parser in Rust and TypeScript matching this EBNF, with error handling, AST builder, and visitor hooks. Include tests.”

---

## TissLang v1: compact spec (draft)

### Core ideas
- **Headers as directives**, then a simple body. Minimal punctuation, no indentation traps.
- **Deterministic defaults**, explicit budgets, explicit locality.
- **Everything serializes cleanly** to a small AST.

### Directives (v1 set)
- **@limits**: tokens, context
- **@compute.mode**: “sip” | “balanced” | “burst”
- **@retrieve**: top_k, overlap
- **@sources**: from, allow_new
- **@local.only**: true | false
- **@cache.prefer**: true | false
- **@quality**: style, citations, hedging
- **@seed**: integer
- **@stop**: list of strings
- **@output**: format “plain” | “bullet” | “json”
- **@plan**: steps integer (light agentic loop, bounded)
- **@time.budget_ms**: integer

### Examples

```tisslang
@local.only true
@compute.mode "sip"
@limits { tokens: 100, context: 320 }
@retrieve { top_k: 2, overlap: 64 }
@quality { style: "plain", citations: true }
@seed 42

intent: "summarize"
topic: "weekly research notes"
```

```tisslang
@local.only true
@limits { tokens: 120 }
@quality { style: "bullet", hedging: low }
@output "json"

intent: "rewrite"
tone: "concise and kind"
text: """
Please rewrite this for clarity and brevity...
"""
```

---

## Grammar (EBNF)

```ebnf
document     = { directive }, { statement } ;
directive    = "@" ident ( obj | bool | string | number ) EOL ;
statement    = field ":" value EOL ;

field        = ident ;
ident        = letter , { letter | digit | "_" | "." } ;

value        = string
             | number
             | bool
             | triple_string
             | list
             | obj ;

list         = "[" , [ value , { "," , value } ] , "]" ;
obj          = "{" , [ pair , { "," , pair } ] , "}" ;
pair         = ident , ":" , value ;

string       = '"' , { char - ('"' | '\n') | escape } , '"' ;
triple_string= '"""' , { char | EOL | escape } , '"""' ;

number       = ["-"] , digit , { digit } ;
bool         = "true" | "false" ;

letter       = 'A'..'Z' | 'a'..'z' ;
digit        = '0'..'9' ;
escape       = "\\" , ('"' | "\\" | "n" | "t") ;
EOL          = { " " | "\t" } , ("\n" | EOF) ;
```

Notes:
- No indentation-significance.
- Directives must precede statements.
- Unknown directives are errors (v1).

---

## AST schema (JSON)

```json
{
  "directives": {
    "local.only": true,
    "compute.mode": "sip",
    "limits": { "tokens": 100, "context": 320 },
    "retrieve": { "top_k": 2, "overlap": 64 },
    "quality": { "style": "plain", "citations": true },
    "seed": 42
  },
  "intent": "summarize",
  "topic": "weekly research notes",
  "_meta": { "version": "1.0.0", "source_hash": "..." }
}
```

### Validation Rules
-   `tokens` ∈ [16, 512]; `context` ∈ [128, 1024].
-   `top_k` ∈ [0, 5]; `overlap` ∈ [0, 200].
-   If `@output "json"`, response must be valid minified JSON, no prose.
-   If `@local.only true`, no remote calls; hard fail if unmet.


## Execution Model (Interpreter Contract)

Map directives to runtime knobs:

- **@compute.mode**
  - sip: temperature=0.1, top_p=0.9, threads=ceil(cores/2)
  - balanced: temperature=0.5, top_p=0.95
  - burst: temperature=0.8, top_p=0.98, allow plan steps
- **@limits**
  - tokens → max_new_tokens; context → max_context
- **@retrieve**
  - top_k → vector search; overlap → chunk windowing
- **@quality**
  - style: “plain” | “bullet” | “json”; citations → enforce RAG source inclusion
- **@time.budget_ms**
  - Preempt if exceeded; suggest smaller tokens/context

---

## GreenScore v0.1

A tiny, transparent score to nudge eco behavior:
\[
\text{GreenScore} = 100 - \left( a \cdot \frac{\text{ctx}}{C_{\max}} + b \cdot \frac{\text{tok}}{T_{\max}} + c \cdot \frac{\text{top\_k}}{K_{\max}} \right) \times 100
\]
-   Defaults: \(a=0.5, b=0.4, c=0.1, C_{\max}=1024, T_{\max}=512, K_{\max}=5\).
-   Clamp to [0, 100]. Show one-line tip to improve.


## Reference parser scaffolds

### Rust (no_std-ready core)

```rust
pub struct Ast {
    pub directives: serde_json::Value,
    pub fields: serde_json::Value,
}

pub fn parse(input: &str) -> Result<Ast, ParseError> {
    // 1) tokenize: @, identifiers, numbers, strings, braces, brackets
    // 2) parse directives until first non-@ line
    // 3) parse field: value pairs
    // 4) build JSON AST; validate ranges
    unimplemented!()
}

#[derive(Debug)]
pub struct ParseError {
    pub line: usize,
    pub col: usize,
    pub message: String,
}
```

### TypeScript (Node or Deno)

```ts
export interface AST {
  directives: Record<string, any>;
  fields: Record<string, any>;
  _meta?: { version: string; source_hash?: string };
}

export function parse(src: string): AST {
  // tokenize lines; handle triple-quoted blocks; JSON-ish objects/lists
  // throw with {line, col, hint} on error
  return { directives: {}, fields: {} };
}
```

---

## Conformance tests (goldens)

- Parse success:
  - minimal: only intent
  - full directives set (as above)
  - list/object nesting
- Parse failures:
  - unknown directive
  - invalid range (tokens=0)
  - unterminated triple string
- Interpreter:
  - compute.mode=sip sets temperature=0.1
  - output=json yields valid JSON only
  - local.only=true blocks remote

Each test includes input.tiss, ast.json, and err.txt (for negative cases).

---

## Error style (kind, precise)

- Format: “line:col error: message. hint: …”
- Examples:
  - “7:14 error: unknown directive ‘@speed.mode’. hint: did you mean ‘@compute.mode’?”
  - “3:10 error: tokens=2048 exceeds limit (≤512). hint: try @limits { tokens: 120 }”

---

## Roadmap to “AI-built” reality (6 weeks)

- Week 1: Run Proposal/Critique/Synthesis rounds across assistants; lock EBNF and directive table.
- Week 2: Generate Rust + TS parsers; wire validation; add error copy; build conformance suite.
- Week 3: Implement interpreter mapping and GreenScore; integrate with Pi runtime.
- Week 4: Dogfood on Pi: summarize/rewrite/Q&A templates; measure and tune defaults.
- Week 5: Write spec doc and playground; add auto-fix hints (“tighten intent”).
- Week 6: Brand polish; publish spec, parsers, tests; invite “First‑Pi issues”.

---

## Conclusion

This document provides the foundational specification for TissLang v1 and a structured protocol for its continued development through multi-AI collaboration. The next step is to execute the "Proposal" round to generate the initial, complete language specification. The ultimate goal is to create a language that is not only powerful and efficient but also respects user resources and the environment.

The project's guiding principles can be summarized by taglines such as "Think clearly. Compute lightly." or "Small is kind."
