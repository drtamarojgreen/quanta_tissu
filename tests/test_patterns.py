"""
Tests for the trading pattern recognition functions.
"""

# Ensure the analytics module can be found
import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from analytics.platform import patterns

def test_bollinger_bands_placeholder():
    """
    Tests the placeholder implementation of the bollinger_bands function.
    """
    print("Testing bollinger_bands...")
    data = [100, 101, 102]
    result = patterns.bollinger_bands(data)

    assert isinstance(result, dict), "bollinger_bands should return a dictionary"
    assert "upper_band" in result
    assert "middle_band" in result
    assert "lower_band" in result
    assert "signal" in result
    assert result["signal"] == "PRICE_NEAR_UPPER"
    print("OK: bollinger_bands")

def test_macd_placeholder():
    """
    Tests the placeholder implementation of the moving_average_convergence_divergence function.
    """
    print("Testing moving_average_convergence_divergence (MACD)...")
    data = [100, 101, 102]
    result = patterns.moving_average_convergence_divergence(data)

    assert isinstance(result, dict), "MACD function should return a dictionary"
    assert "macd_line" in result
    assert "signal_line" in result
    assert "histogram" in result
    assert "signal" in result
    assert result["signal"] == "BULLISH_CROSSOVER"
    print("OK: moving_average_convergence_divergence")

def test_on_balance_volume_placeholder():
    """
    Tests the placeholder implementation of the on_balance_volume function.
    """
    print("Testing on_balance_volume...")
    # The placeholder doesn't actually use the data yet
    data = [{'close': 100, 'volume': 1000}, {'close': 102, 'volume': 1200}]
    result = patterns.on_balance_volume(data)

    assert isinstance(result, str)
    assert result == "UPTREND"
    print("OK: on_balance_volume")

def test_get_pattern_signals_includes_new_patterns():
    """
    Tests that the get_pattern_signals function includes the new patterns in its output.
    """
    print("Testing get_pattern_signals for inclusion of new patterns...")
    data = [{'close': 100, 'volume': 1000}] * 6 # Mock data for all patterns
    all_signals = patterns.get_pattern_signals(data)

    assert "bollinger_bands" in all_signals
    assert "macd" in all_signals
    assert "obv" in all_signals

    # Check the nested structure of the returned values
    assert "upper_band" in all_signals["bollinger_bands"]
    assert "macd_line" in all_signals["macd"]
    assert all_signals["obv"] == "UPTREND"
    print("OK: get_pattern_signals includes new patterns")
