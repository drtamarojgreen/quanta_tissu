# Proposed TissLM Architecture: Configurable Attention Block
**File:** `docs/tisslm_core_diagram6_proposed.md`

**Note on the Proposed Architecture:**

This design replaces the static Multi-Head Attention block with a `ConfigurableAttention` block. At initialization, this block can be configured to operate in one of three modes, as per the user's requirement, providing full flexibility over the attention mechanism and its memory strategy.

---

### Configurable Attention Block

```
q_in, k_in, v_in
|
v
+--------------------------------------------------------------------------+
| CONFIGURABLE ATTENTION BLOCK                                             |
|--------------------------------------------------------------------------|
|                                                                          |
| Mode selected at initialization:                                         |
| [ ] Standard Multi-Head [X] Multi-Query [ ] Multi-Head Latent            |
|                                                                          |
|                                                                          |
| +-----------------------+                                                |
| | Q, K, V Projections |                                                |
| +-----------------------+                                                |
| |           |           |                                                |
| v           v           v                                                |
| q_projected k_projected v_projected                                    |
|                                                                          |
|                                                                          |
| /--------------------(IF Mode == Standard Multi-Head)--------------------
| |                                                                        |
| | q -> H heads, k -> H heads, v -> H heads                               |
| | Standard KV Cache (stores H heads for K, V)                            |
| | Each q_head attends to its corresponding k_head/v_head.                |
| |                                                                        |
| \----------------------------------------------------------------------/
|                                                                          |
| /--------------------(IF Mode == Multi-Query)----------------------------
| |                                                                        |
| | q -> H heads                                                           |
| | k -> 1 head (shared)                                                   |
| | v -> 1 head (shared)                                                   |
| | KV Cache stores only 1 head for K, V (Memory efficient)                |
| | All H q_heads attend to the SAME shared k_head/v_head.                 |
| |                                                                        |
| \----------------------------------------------------------------------/
|                                                                          |
| /--------------------(IF Mode == Multi-Head Latent)----------------------
| |                                                                        |
| | q -> H heads                                                           |
| | k -> H heads, v -> H heads                                             |
| |                                                                        |
| | +------------------------------------------------------+               |
| | | Past K/V heads -> LATENT COMPRESSOR (e.g., RNN/Conv) |               |
| | +------------------------------------------------------+               |
| |                       |                                                |
| |                       v                                                |
| |      latent_k (fixed size), latent_v (fixed size)                      |
| |                                                                        |
| | All H q_heads attend to the SAME latent_k/latent_v representation.     |
| |                                                                        |
| \----------------------------------------------------------------------/
|                                                                          |
|                                                                          |
|                                     |                                    |
|                                     v                                    |
| +----------------------------------------------------------------+       |
| |      Merge Heads (if necessary) & Final Projection (W_o)       |       |
| +----------------------------------------------------------------+       |
|                                     |                                    |
|                                     v                                    |
|                                   output                                 |
|                                                                          |
+--------------------------------------------------------------------------+
```