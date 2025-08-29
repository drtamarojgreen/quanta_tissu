"""
Core trading engine for the algorithmic trading platform.
"""

from .db_connector import TissDBConnector
from .lm_interface import TissLMInterface
from . import charting

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

            # --- Begin Charting Integration ---
            # NOTE: The following is a demonstration of the charting module.
            # In a real-world scenario, the historical data would be fetched
            # from the db_connector instead of being hardcoded.
            print("\n--- Generating Historical Data Chart ---")
            historical_data = [
                {'open': 100, 'high': 105, 'low': 98, 'close': 102, 'volume': 10000},
                {'open': 102, 'high': 108, 'low': 101, 'close': 107, 'volume': 12000},
                {'open': 107, 'high': 112, 'low': 105, 'close': 110, 'volume': 15000},
                {'open': 110, 'high': 115, 'low': 108, 'close': 112, 'volume': 13000},
                {'open': 112, 'high': 118, 'low': 111, 'close': 115, 'volume': 16000},
                {'open': 115, 'high': 120, 'low': 114, 'close': 118, 'volume': 17000},
                {'open': 118, 'high': 122, 'low': 117, 'close': 120, 'volume': 14000},
                {'open': 120, 'high': 125, 'low': 119, 'close': 123, 'volume': 18000},
                {'open': 123, 'high': 128, 'low': 122, 'close': 126, 'volume': 20000},
                {'open': 126, 'high': 130, 'low': 125, 'close': 129, 'volume': 19000},
            ]
            chart_output = charting.plot_candlestick_chart(
                historical_data,
                height=15,
                volume_height=4,
                show_trend_line=True
            )
            print(chart_output)
            print("--- End Chart ---\n")
            # --- End Charting Integration ---

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
