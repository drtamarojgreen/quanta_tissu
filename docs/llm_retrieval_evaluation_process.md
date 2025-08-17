# LLM-Based Retrieval Evaluation Process

This document outlines a proposed workflow for enhancing the standard Retrieval-Augmented Generation (RAG) process with an LLM-powered evaluation step. This approach shifts the burden of source verification from the data ingestion phase to the query-time execution phase, using the LLM itself as a fact-checker.

## Core Concept: Retrieve-Evaluate-Generate

The standard RAG model follows a two-step "Retrieve-Generate" process. This proposal introduces a third step in the middle, creating a "Retrieve-Evaluate-Generate" workflow.

1.  **Step 1: Retrieval:** As with standard RAG, the system takes a user's query and retrieves a set of relevant documents (a "local corpus") from the `TissDB` knowledge base.

2.  **Step 2: LLM-Powered Evaluation:** This is the new, critical step. The retrieved documents are not immediately used. Instead, they are passed to the `TissLM` model with a specialized "Evaluator" prompt. The goal is to have the LLM critique the retrieved information.

3.  **Step 3: Final Answer Generation:** The "verified" or "sanitized" context that results from the evaluation step is then combined with the user's original query. This final, augmented prompt is sent to `TissLM` to generate a high-quality, trusted answer for the user.

## The "Evaluator" Prompt Template

The success of the evaluation step depends on a carefully crafted prompt. The prompt must instruct the LLM to act as a critical, unbiased fact-checker.

**Example Prompt Template:**

> "You are a meticulous and impartial fact-checking AI. Your task is to analyze a set of retrieved documents in the context of a user's query. Your goal is to identify and extract only the information that is factually accurate, directly relevant to the query, and free from speculation or bias.
>
> **User Query:** `{user_query}`
>
> **Retrieved Documents:**
> ---
> `{retrieved_documents}`
> ---
>
> Based on your general knowledge, please perform the following actions:
> 1.  **Fact-Check:** Scrutinize each statement in the retrieved documents. Compare them against well-established knowledge.
> 2.  **Filter:** Discard any information that is speculative, contains marketing language, expresses strong bias, or contradicts known facts.
> 3.  **Extract:** Synthesize the remaining, verified facts into a concise, neutral summary.
>
> **Output only the extracted, verified facts.**"

## Trade-Offs and Important Considerations

This LLM-centric evaluation model is a powerful but complex approach with significant trade-offs compared to a standard RAG system with a curated data source.

*   **Increased Latency & Cost:** This workflow requires two separate calls to the `TissLM` for every user query (one for evaluation, one for final generation). This will significantly increase the time it takes to get an answer and will consume more computational resources.

*   **Reliance on the LLM's Internal Knowledge:** The entire process hinges on the assumption that the LLM's internal, pre-trained knowledge is a reliable baseline for "truth." This is a major risk, as all LLMs have inherent biases, knowledge gaps, and can be prone to their own forms of hallucination. The evaluation is only as good as the evaluator.

*   **System Complexity:** A three-step pipeline is more complex to build, monitor, and debug than a standard two-step RAG pipeline. Prompt engineering for the evaluator step becomes a critical and ongoing task.

*   **Potential for Information Loss:** A strict evaluator prompt might discard nuanced or newly emerged information that is actually correct but has not yet become part of the LLM's "well-established knowledge."

This approach should be considered experimental and requires careful implementation and continuous monitoring to be effective.
