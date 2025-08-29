# Quanta_Tissu Unified Spec — Educational README

This README provides an educational overview of a modular, token-driven **quanta_tissu** system. It covers architecture, configuration, message contracts, Python implementation, and integration paths toward C++.

---

## Overview

**Objective:** Create a system that can wrap or replace `tisslm`, decide per token how deep to process, and remain fully configurable.

**Architecture:**
- **Core loop:** Token-by-token orchestration with per-step policy, approximation control, and pluggable components.
- **Modes:**
  - **Wrap mode:** Intercept logits/tokens from `tisslm`, apply corrections, accept/modify/reroll.
  - **Standalone mode:** Request logits from a base LM module (local or remote) and emit tokens.
- **Approximation layers:** Nearest-neighbor reuse and stack levels to reduce heavy passes on every token.
- **Hot-config:** Profiles, per-request overrides, and live reload without code changes.
- **Contracts:** Language-agnostic message types designed for future C++ parity.

## Configuration

**YAML Global/Profiles, JSON Per-request overrides.**

**Example YAML:**
```yaml
version: 4
modules:
  tisslm: {mode: "wrap", endpoint: "ipc://tisslm", timeout_ms: 60}
  base_lm: {mode: "standalone", endpoint: "ipc://cerebra", timeout_ms: 80}
  backward_cnn: {endpoint: "ipc://backcnn", timeout_ms: 120}
  curvature: {endpoint: "ipc://curv", timeout_ms: 80}
  memora: {endpoint: "ipc://memora", timeout_ms: 120}

pipelines:
  chat_ll:
    forward: {curvature_mode: "laplace_diag", topk: 32, value_head: false}
    backward: {enabled: false}
  draft_hp:
    forward: {curvature_mode: "newton_guidance", topk: 64, value_head: true}
    backward:
      enabled: true
      cnn: {conv: "adjacency", hops: 2, propose_edits: true, emit_bias: true}

approximation:
  stride: {enabled: true, N: 4}
  entropy_trigger: {enabled: true, threshold: 2.5}
  role_aware: {enabled: true}
  nearest_neighbor:
    enabled: true
    space: "graph"
    max_hops: 2
    similarity_threshold: 0.85

stack_levels:
  default: 1
  max_level: 3
  escalation:
    entropy: 2.5
    risk: 2

policies:
  - name: "fast_chat"
    when: 'task == "chat" && latency_ms <= 250 && risk <= 1'
    use_pipeline: "chat_ll"
    knobs: {fsm_enforce: "soft", safety_viewer: "lite"}
  - name: "high_precision"
    when: 'task in ["report","spec"] || risk >= 2'
    use_pipeline: "draft_hp"
    knobs: {fsm_enforce: "hard", safety_viewer: "full"}

resources:
  timeouts: {total_ms: 2500, forward_step_ms: 120, backward_pass_ms: 400}
  parallelism: {beams: 4, path_diversity: 3}

routing:
  fsm: {source: "memora"}

telemetry:
  metrics: ["latency","coverage","redundancy","edits_applied","entropy"]
  trace_sampling: {default: 0.05, high_risk: 1.0}
```

**Per-request JSON override example:**
```json
{
  "context": {"task": "report", "latency_ms": 600, "risk": 2},
  "overrides": {"pipelines.draft_hp.forward.topk": 48, "stack_levels.default": 2}
}
```

## Message Contracts

Define once in Python; generate C++ later.

- **Headers:** request_id, session_id, profile, config_version, deadline_ms, tags.
- **TokenScores:** step, topk[{token_id, logit}], mask_bits.
- **BiasField:** span_indices, delta_logits[topk], uncertainty[topk].
- **GraphUpdate:** nodes[], edges[], fsm_state_id, coverage.
- **CurvatureStats:** mode, diag_variance[topk], damping_lambda, probes_used.
- **PolicyDecision:** name, reasons[], pipeline, knobs.
- **ErrorReport:** component, code, detail, retryable.

**Proto-style example:**
```proto
message TokenScores { uint32 step=1; repeated TokenLogit topk=2; bytes mask=3; }
message TokenLogit { uint32 token_id=1; float logit=2; }
message BiasField { repeated uint32 span_idx=1; repeated TokenLogit delta=2; repeated float uncertainty=3; }
message CurvatureStats { string mode=1; repeated float diag_var=2; float lambda=3; uint32 probes=4; }
```

## Python Implementation Skeleton

**Interfaces:**
```python
# interfaces.py
from abc import ABC, abstractmethod
from typing import Dict, Any, Tuple, List

class PolicyEngine(ABC):
    @abstractmethod
    def decide(self, context: Dict[str, Any], overrides: Dict[str, Any]) -> Dict[str, Any]: ...

class FSMProvider(ABC):
    @abstractmethod
    def compile(self, plan_graph: Dict[str, Any]) -> Any: ...
    @abstractmethod
    def mask(self, fsm: Any) -> List[int]: ...
    @abstractmethod
    def advance(self, fsm: Any, token_id: int) -> Any: ...

class BackwardCNN(ABC):
    @abstractmethod
    def analyze(self, draft: List[int], graph: Dict[str, Any], ctx: Dict[str, Any]) -> Tuple[Dict[int,float], Dict[int,float]]: ...

class CurvatureEstimator(ABC):
    @abstractmethod
    def adjust(self, logits: Dict[int,float], mode: str, ctx: Dict[str, Any]) -> Dict[int,float]: ...

class BaseLM(ABC):
    @abstractmethod
    def next_logits(self, prefix_ids: List[int], ctx: Dict[str, Any]) -> Dict[int,float]: ...

class TissLMWrapper(ABC):
    @abstractmethod
    def intercept(self, ctx: Dict[str, Any]) -> Dict[int,float]: ...
```

**Approximation & stack-level policy:**
- Escalate to Level 3 if token entropy ≥ threshold.
- Increase level if context risk ≥ threshold.
- Force full pass at discourse or graph node boundaries.
- Nearest-neighbor reuse in graph or embedding space.
- Cache and evict prefix, bias, and uncertainty data as needed.

## Integration & C++ Migration

- **Wrap tisslm:** Intercept per-step logits/tokens, apply masks, bias, curvature, accept or re-roll.
- **Replace tisslm:** Use `base_lm.next_logits` with the same token loop.
- **C++ port:** Mirror Python config, proto contracts, and ABCs; swap Python calls with gRPC/ZeroMQ; maintain NNCache and approximation triggers to preserve behavior.

