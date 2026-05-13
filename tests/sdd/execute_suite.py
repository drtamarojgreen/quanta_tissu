import sys
import os

# Add the current directory to sys.path to allow imports from .
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from .base import Runner
from .viewer_shell import ViewerShell
from . import cards_batch_1
from . import cards_batch_2

def main():
    print("=== SDD Verification Suite Execution ===")

    runner = Runner()
    viewer = ViewerShell()

    # Load all cards
    all_cards = cards_batch_1.get_cards() + cards_batch_2.get_cards()

    print(f"Total Cards Loaded: {len(all_cards)}")

    for card in all_cards:
        # Reset viewer state slightly between cards if needed,
        # but SDD cards should ideally handle setup/teardown
        runner.run(card, viewer)

    summary = runner.get_summary()
    print("\n=== Execution Summary ===")
    print(f"Total Verifications: {summary['total']}")
    print(f"Passed: {summary['passed']}")
    print(f"Failed: {summary['failed']}")

    # Output results for documentation update
    with open("tests/sdd/execution_results.log", "w") as f:
        for name, status, result in runner.history:
            f.write(f"[{status}] {name} -> {result}\n")

if __name__ == "__main__":
    main()
