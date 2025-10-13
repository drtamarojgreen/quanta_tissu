"""
Tests for the text-based charting tools.
"""

import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from analytics.platform import charting

def test_calculate_trend_line_simple():
    """
    Tests the calculate_trend_line with a simple, predictable dataset.
    """
    print("Testing calculate_trend_line...")
    # y = 2x + 1
    data = [1, 3, 5, 7, 9]
    m, b = charting.calculate_trend_line(data)

    # Using assert with a tolerance for floating point comparisons
    assert abs(m - 2.0) < 1e-9, f"Slope should be ~2.0, but was {m}"
    assert abs(b - 1.0) < 1e-9, f"Intercept should be ~1.0, but was {b}"
    print("OK: calculate_trend_line")

def test_plot_candlestick_chart_basic():
    """
    Tests the output of the candlestick chart with a simple dataset.
    """
    print("Testing plot_candlestick_chart (basic)...")
    ohlc_data = [
        {'open': 10, 'high': 15, 'low': 5, 'close': 12}, # Bullish
        {'open': 12, 'high': 14, 'low': 8, 'close': 9},  # Bearish
    ]
    # A simplified chart for testing purposes
    chart = charting.plot_candlestick_chart(ohlc_data, height=10)

    # We expect to see the body and wick characters in the output
    assert '┃' in chart, "Bullish candle body should be present"
    assert '█' in chart, "Bearish candle body should be present"
    assert '│' in chart, "Wick character should be present"

    # Check for a price label
    assert "15.00" in chart or "5.00" in chart, "Price labels should be present"
    print("OK: plot_candlestick_chart (basic)")

def test_plot_candlestick_with_trend_line():
    """
    Tests that the trend line character appears when show_trend_line is True.
    """
    print("Testing plot_candlestick_chart (with trend line)...")
    ohlc_data = [
        {'open': 10, 'high': 15, 'low': 5, 'close': 12, 'volume': 100},
        {'open': 12, 'high': 16, 'low': 11, 'close': 14, 'volume': 120},
        {'open': 14, 'high': 18, 'low': 13, 'close': 16, 'volume': 110},
    ]
    chart = charting.plot_candlestick_chart(ohlc_data, height=15, show_trend_line=True)

    assert '•' in chart, "Trend line character '•' should be present in the chart"
    print("OK: plot_candlestick_chart (with trend line)")

def test_plot_candlestick_with_volume():
    """
    Tests that the volume chart is rendered below the price chart.
    """
    print("Testing plot_candlestick_chart (with volume)...")
    ohlcv_data = [
        {'open': 10, 'high': 15, 'low': 5, 'close': 12, 'volume': 1000},
        {'open': 12, 'high': 14, 'low': 8, 'close': 9,  'volume': 2000},
    ]
    chart = charting.plot_candlestick_chart(ohlcv_data, height=10, volume_height=4)

    # Check for the separator between price and volume charts
    assert '┼' in chart, "Separator for volume chart should be present"

    # Check that the output string has more lines than the price chart height,
    # implying the volume chart was added.
    assert len(chart.split('\n')) > 10, "Chart should have more lines than just the price chart"

    # Check for volume labels
    assert "2000" in chart or "1000" in chart, "Volume labels should be present"
    print("OK: plot_candlestick_chart (with volume)")

if __name__ == "__main__":
    test_calculate_trend_line_simple()
    test_plot_candlestick_chart_basic()
    test_plot_candlestick_with_trend_line()
    test_plot_candlestick_with_volume()
    print("All charting tests passed!")