"""
Core trading engine for the algorithmic trading platform.
"""

from .db_connector import TissDBConnector
from .lm_interface import TissLMInterface

class TradingEngine:
    """
    The core trading engine that orchestrates data fetching, signal generation,
    and order execution.
    """

    def __init__(self, db_connector: TissDBConnector, lm_interface: TissLMInterface):
        """
        Initializes the trading engine.

        Args:
            db_connector: An instance of TissDBConnector.
            lm_interface: An instance of TissLMInterface.
        """
        self._db_connector = db_connector
        self._lm_interface = lm_interface
        print("TradingEngine initialized.")

    def run(self, symbol: str):
        """
        Runs the trading loop for a given symbol.

        Args:
            symbol: The stock symbol to trade.
        """
        print(f"Starting trading engine for symbol: {symbol}")

        try:
            # Connect to services
            self._db_connector.connect()
            self._lm_interface.load_model()

            # Main loop
            market_data = self._db_connector.fetch_market_data(symbol)
            signal = self._lm_interface.get_trading_signal(market_data)

            self.execute_trade(signal, market_data)

        except Exception as e:
            print(f"An error occurred: {e}")
        finally:
            self._db_connector.disconnect()
            print("Trading engine stopped.")

    def execute_trade(self, signal: str, market_data: dict):
        """
        Executes a trade based on the signal.

        Args:
            signal: The trading signal ('BUY', 'SELL', 'HOLD').
            market_data: The market data used for the signal.
        """
        print(f"Received signal: {signal} for {market_data['symbol']}")
        if signal == "BUY":
            print(f"Executing BUY order for {market_data['symbol']} at {market_data['price']}")
        elif signal == "SELL":
            print(f"Executing SELL order for {market_data['symbol']} at {market_data['price']}")
        else: # HOLD
            print(f"Holding position for {market_data['symbol']}")
        # Placeholder for actual order execution logic
        print("Trade execution complete.")
