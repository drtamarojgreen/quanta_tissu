"""
A native Python text-based charting tool.

This tool generates simple ASCII art charts from numerical data.
It has no external dependencies and adheres to the no-compile/no-test protocol.
"""

def plot_ascii_chart(data: list, label: str = "Data", width: int = 50):
    """
    Generates and prints a simple horizontal ASCII bar chart.

    Args:
        data: A list of numerical data points to plot.
        label: A label for the y-axis of the chart.
        width: The maximum width of the chart in characters.
    """
    if not data:
        print(f"No data provided for {label}.")
        return

    print(f"--- Chart: {label} ---")

    min_val = min(data)
    max_val = max(data)
    val_range = max_val - min_val

    # Avoid division by zero if all data points are the same
    if val_range == 0:
        for i, val in enumerate(data):
            bar = '*' * (width // 2)
            print(f"{i:03d} |{bar} ({val})")
        return

    # Scale and print each bar
    for i, val in enumerate(data):
        # Scale the value to the desired width
        scaled_val = int(((val - min_val) / val_range) * width)
        bar = '*' * scaled_val
        print(f"{i:03d} |{bar} ({val})")

    print(f"--- End Chart ---")

def calculate_trend_line(data_points: list) -> (float, float):
    """
    Calculates the slope and intercept of a trend line using linear regression.

    Args:
        data_points: A list of numerical data points.

    Returns:
        A tuple containing the slope (m) and intercept (b) of the trend line.
    """
    n = len(data_points)
    if n < 2:
        return 0, sum(data_points) / n if n == 1 else 0

    sum_x = sum(range(n))
    sum_y = sum(data_points)
    sum_xy = sum(i * y for i, y in enumerate(data_points))
    sum_x2 = sum(i**2 for i in range(n))

    denominator = n * sum_x2 - sum_x**2
    if denominator == 0:
        return 0, sum_y / n # Cannot calculate slope, return a horizontal line

    m = (n * sum_xy - sum_x * sum_y) / denominator
    b = (sum_y - m * sum_x) / n
    return m, b


def plot_candlestick_chart(data: list, height: int = 20, volume_height: int = 5, show_trend_line: bool = False) -> str:
    """
    Generates a text-based candlestick chart with volume and an optional trend line.

    Args:
        data: A list of dicts, each with 'open', 'high', 'low', 'close', 'volume'.
        height: The height of the price chart in characters.
        volume_height: The height of the volume chart in characters.
        show_trend_line: If True, calculates and overlays a trend line.

    Returns:
        A string representing the ASCII candlestick chart.
    """
    if not data:
        return "No data provided for candlestick chart."

    # --- Price Chart Section ---
    lows = [d['low'] for d in data]
    highs = [d['high'] for d in data]
    min_price = min(lows)
    max_price = max(highs)
    price_range = max_price - min_price
    width = len(data)

    if price_range == 0:
        # Avoid division by zero, but still draw a simple line
        price_range = 1

    price_grid = [[' ' for _ in range(width)] for _ in range(height)]
    y_axis_width = 12

    def scale_price(price):
        return int(((price - min_price) / price_range) * (height - 1))

    # Draw candles
    for i, d in enumerate(data):
        s_open, s_high = scale_price(d['open']), scale_price(d['high'])
        s_low, s_close = scale_price(d['low']), scale_price(d['close'])
        body_char = '┃' if d['close'] >= d['open'] else '█'
        body_start, body_end = min(s_open, s_close), max(s_open, s_close)

        for y in range(s_low, s_high + 1):
            if 0 <= y < height:
                price_grid[y][i] = '│' if y < body_start or y > body_end else body_char

    # Draw trend line
    if show_trend_line:
        close_prices = [d['close'] for d in data]
        m, b = calculate_trend_line(close_prices)
        for x in range(width):
            trend_price = m * x + b
            if min_price <= trend_price <= max_price:
                y = scale_price(trend_price)
                if 0 <= y < height:
                    # Overlay trend line character, even on top of wicks
                    price_grid[y][x] = '•'

    # Format price chart with axis
    price_chart_rows = []
    price_step = price_range / (height - 1) if height > 1 else price_range
    for y in range(height):
        price = min_price + (y * price_step)
        label = f"{price:10.2f} ┤"
        price_chart_rows.append(label + "".join(price_grid[y]))

    price_chart_rows.reverse()
    final_chart_str = "\n".join(price_chart_rows)

    # --- Volume Chart Section ---
    if volume_height > 0 and all('volume' in d for d in data):
        volumes = [d['volume'] for d in data]
        max_volume = max(volumes) if volumes else 0
        vol_grid = [[' ' for _ in range(width)] for _ in range(volume_height)]

        for x, vol in enumerate(volumes):
            scaled_vol = int((vol / max_volume) * (volume_height)) if max_volume > 0 else 0
            for y in range(scaled_vol):
                if y < volume_height:
                    vol_grid[y][x] = '█'

        separator = (' ' * (y_axis_width - 1)) + '┼' + ('─' * width)
        final_chart_str += f"\n{separator}\n"

        volume_chart_rows = []
        vol_step = max_volume / volume_height if volume_height > 0 else max_volume
        for y in range(volume_height):
            vol = (y + 1) * vol_step
            label = f"{vol:10.0f} ┤"
            volume_chart_rows.append(label + "".join(vol_grid[y]))

        volume_chart_rows.reverse()
        final_chart_str += "\n".join(volume_chart_rows)

    return final_chart_str


def example_usage():
    """
    An example of how to use the plotting function.
    This function is for demonstration only and is not called by the main application.
    """
    print("Plotting example stock prices:")
    stock_prices = [100, 102, 105, 103, 108, 115, 112, 118, 120, 115, 110]
    plot_ascii_chart(stock_prices, label="Stock XYZ", width=60)

    print("\nPlotting example RSI values:")
    rsi_values = [30, 25, 40, 55, 72, 78, 65, 50]
    plot_ascii_chart(rsi_values, label="RSI", width=40)
