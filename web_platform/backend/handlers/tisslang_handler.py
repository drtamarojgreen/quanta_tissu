import json
import os
import sys

# Add project root to path for imports
BACKEND_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(BACKEND_DIR, '..', '..', '..'))
if PROJECT_ROOT not in sys.path:
    sys.path.append(PROJECT_ROOT)

try:
    from quanta_tissu.tisslm.parser.tisslang_parser import TissLangParser
    from quanta_tissu.tisslm.core.execution_engine import ExecutionEngine, ToolRegistry
    from quanta_tissu.tisslm.core.tools.builtins import RunCommandTool, WriteFileTool, ReadFileTool, AssertConditionTool
    TISSLANG_AVAILABLE = True
except ImportError:
    TISSLANG_AVAILABLE = False

def handle_tisslang(handler, path, data, command):
    if path == '/api/tisslang/run':
        run_tisslang(handler, data)
    else:
        return False
    return True

def run_tisslang(handler, data):
    if not TISSLANG_AVAILABLE:
        handler.send_response(501)
        handler.end_headers()
        handler.wfile.write(b'{"error": "TissLang modules not found"}')
        return

    script = data.get('script', '')
    try:
        parser = TissLangParser()
        ast = parser.parse(script)

        registry = ToolRegistry()
        registry.register("RUN", RunCommandTool(PROJECT_ROOT))
        registry.register("WRITE", WriteFileTool(PROJECT_ROOT))
        registry.register("READ", ReadFileTool(PROJECT_ROOT))
        registry.register("ASSERT", AssertConditionTool())

        engine = ExecutionEngine(registry)
        state = engine.execute(ast)

        # Convert execution log to JSON serializable
        serializable_log = []
        for entry in state.execution_log:
            serializable_log.append({
                'status': entry.get('status'),
                'duration_ms': entry.get('duration_ms'),
                'error': entry.get('error'),
                'command_type': entry.get('command', {}).get('type')
            })

        res = {
            'log': serializable_log,
            'halted': state.is_halted,
            'vars': state.variables
        }
        response = json.dumps(res).encode('utf-8')
        handler.send_response(200)
        handler.send_header('Content-Type', 'application/json')
        handler.end_headers()
        handler.wfile.write(response)
    except Exception as e:
        handler.send_response(500)
        handler.end_headers()
        handler.wfile.write(str(e).encode('utf-8'))
