"""
Tests for the trading pattern recognition functions.
"""

# Ensure the analytics module can be found
import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from analytics.platform import patterns

def test_bollinger_bands():
    """
    Tests the implementation of the bollinger_bands function.
    """
    print("Testing bollinger_bands...")
    # Provide enough data for the default period of 20
    data = list(range(100, 120))
    result = patterns.bollinger_bands(data)

    assert isinstance(result, dict), "bollinger_bands should return a dictionary"
    assert "upper_band" in result
    assert "middle_band" in result
    assert "lower_band" in result
    assert "signal" in result
    # With this data, the price should be within the bands
    assert result["signal"] == "IN_BANDS"
    print("OK: bollinger_bands")

def test_macd():
    """
    Tests the implementation of the moving_average_convergence_divergence function.
    """
    print("Testing moving_average_convergence_divergence (MACD)...")
    # Provide enough data for slow_period (26) + signal_period (9) = 35
    data = list(range(100, 135))
    result = patterns.moving_average_convergence_divergence(data)

    assert isinstance(result, dict), "MACD function should return a dictionary"
    assert "macd_line" in result
    assert "signal_line" in result
    assert "histogram" in result
    assert "signal" in result
    # With this linear data, we expect a neutral signal
    assert result["signal"] == "NEUTRAL"
    print("OK: moving_average_convergence_divergence")

def test_on_balance_volume():
    """
    Tests the implementation of the on_balance_volume function.
    """
    print("Testing on_balance_volume...")
    # Data with a clear uptrend
    data = [{'close': 100 + i, 'volume': 1000 + i*10} for i in range(10)]
    result = patterns.on_balance_volume(data)

    assert isinstance(result, str)
    assert result == "UPTREND"
    print("OK: on_balance_volume")

def test_get_pattern_signals_includes_all_patterns():
    """
    Tests that the get_pattern_signals function includes all patterns in its output.
    """
    print("Testing get_pattern_signals for inclusion of all patterns...")
    # Provide enough data for all patterns
    data = [{'open': 100+i, 'high': 102+i, 'low': 99+i, 'close': 101+i, 'volume': 1000} for i in range(60)]
    all_signals = patterns.get_pattern_signals(data)

    assert "bollinger_bands" in all_signals
    assert "macd" in all_signals
    assert "obv" in all_signals
    assert "moving_average_crossover" in all_signals
    assert "relative_strength_index" in all_signals
    assert "ichimoku_cloud" in all_signals
    assert "keltner_channels" in all_signals

    # Check the nested structure of the returned values
    assert "upper_band" in all_signals["bollinger_bands"]
    assert "macd_line" in all_signals["macd"]
    # With this data, OBV should be in an uptrend
    assert all_signals["obv"] == "UPTREND"
    print("OK: get_pattern_signals includes all patterns")
