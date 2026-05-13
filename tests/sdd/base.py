import time
from .restrictions import enforce_empirical_output, architectural_guardrail

class Fact:
    """A verified truth about the system environment or state."""
    def __init__(self, key, value, source="Execution"):
        self.key = key
        self.value = value
        self.source = source
        self.timestamp = time.time()

    def __repr__(self):
        return f"Fact({self.key}={self.value}, source={self.source})"

class Card:
    """A minimal executable unit that performs a single action and produces a Fact."""
    def __init__(self, name):
        self.name = name

    def execute(self, context):
        raise NotImplementedError("Subclasses must implement execute")

class Runner:
    """Executes Cards against a context and maintains a Fact repository."""
    def __init__(self):
        self.facts = {}
        self.history = []

    def run(self, card, context):
        print(f"Runner: Executing Card '{card.name}'...")
        try:
            # Enforcement of structural validation before behavior
            if not hasattr(card, 'execute'):
                print(f"Runner: ERROR - Card {card.name} is missing 'execute' method.")
                return None

            fact = card.execute(context)
            if fact and isinstance(fact, Fact):
                self.facts[fact.key] = fact
                self.history.append((card.name, "SUCCESS", fact))
                print(f"Runner: SUCCESS - Produced {fact}")
                return fact
            else:
                self.history.append((card.name, "FAILURE", "No Fact produced"))
                print(f"Runner: FAILURE - Card '{card.name}' did not produce a valid Fact.")
                return None
        except Exception as e:
            self.history.append((card.name, "ERROR", str(e)))
            print(f"Runner: ERROR during '{card.name}': {e}")
            return None

    def get_summary(self):
        return {
            "total": len(self.history),
            "passed": len([h for h in self.history if h[1] == "SUCCESS"]),
            "failed": len([h for h in self.history if h[1] != "SUCCESS"])
        }
