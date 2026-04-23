import json

def handle_analytics(handler, path, data, command):
    if path == '/api/analytics/chart':
        handle_chart(handler, data)
    elif path == '/api/analytics/trade':
        handle_trade(handler, data)
    else:
        return False
    return True

def handle_chart(handler, data):
    chart_data = [45, 80, 55, 90, 30, 70]
    response = json.dumps({'data': chart_data, 'label': 'Market Volume'}).encode('utf-8')
    handler.send_response(200)
    handler.send_header('Content-Type', 'application/json')
    handler.send_header('Content-Length', len(response))
    handler.end_headers()
    handler.wfile.write(response)

def handle_trade(handler, data):
    symbol = data.get('symbol', 'AAPL')
    res = {
        'symbol': symbol,
        'signal': 'BUY',
        'market_data': {'price': 150.25, 'volume': 54000},
        'historical_data': [
            {'open': 140, 'high': 145, 'low': 138, 'close': 142, 'volume': 10000},
            {'open': 142, 'high': 148, 'low': 141, 'close': 147, 'volume': 12000},
            {'open': 147, 'high': 152, 'low': 145, 'close': 150, 'volume': 15000},
        ]
    }
    response = json.dumps(res).encode('utf-8')
    handler.send_response(200)
    handler.send_header('Content-Type', 'application/json')
    handler.end_headers()
    handler.wfile.write(response)
