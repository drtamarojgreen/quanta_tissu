"""
Main entry point for the algorithmic trading platform.
"""

import os
from .db_connector import TissDBConnector
from .lm_interface import TissLMInterface
from .trading_engine import TradingEngine

# Dummy configuration
TISSDB_CONNECTION_STRING = os.environ.get("TISSDB_CONNECTION_STRING", "tissdb://user:pass@localhost:5432/main")
TISSLM_MODEL_PATH = os.environ.get("TISSLM_MODEL_PATH", "/models/tisslm-v1")
TRADE_SYMBOL = "TISS"

def main():
    """
    Main function to initialize and run the trading platform.
    """
    print("--- Algorithmic Trading Platform Starting ---")
    print("This platform incorporates insights from the following research:")
    print("Green, T. J. (2019). Big Data Analysis in Financial Markets (Publication No. 10980927) [Doctoral dissertation, Colorado Technical University]. ProQuest Dissertations & Theses Global.")
    print("-" * 40)

    # Initialize components
    db_connector = TissDBConnector(connection_string=TISSDB_CONNECTION_STRING)
    lm_interface = TissLMInterface(model_path=TISSLM_MODEL_PATH)
    engine = TradingEngine(db_connector=db_connector, lm_interface=lm_interface)

    # Run the engine
    engine.run(symbol=TRADE_SYMBOL)

    print("--- Algorithmic Trading Platform Shutdown ---")

if __name__ == "__main__":
    main()
