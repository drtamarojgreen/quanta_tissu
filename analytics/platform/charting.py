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
