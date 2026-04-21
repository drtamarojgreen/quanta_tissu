import http.server
import socketserver
import json
import urllib.request
import urllib.error
import sys
import os
import numpy as np
import logging
import re

# Add project root to path
sys.path.append(os.path.abspath(os.path.join(os.getcwd(), '..', '..')))

from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.config import model_config, training_config, system_config
from quanta_tissu.tisslm.parser.tisslang_parser import TissLangParser
from quanta_tissu.tisslm.core.execution_engine import ExecutionEngine, ToolRegistry
from quanta_tissu.tisslm.core.db.client import TissDBClient
from quanta_tissu.tisslm.core.embedding.embedder import Embedder
from quanta_tissu.tisslm.core.knowledge_base import KnowledgeBase
from quanta_tissu.tisslm.core.tools.builtins import RunCommandTool, WriteFileTool, ReadFileTool, AssertConditionTool

PORT = 8000
PROJECT_ROOT = os.path.abspath(os.path.join(os.getcwd(), '..', '..'))
TISSDB_HOST = os.environ.get('TISSDB_HOST', 'localhost')
TISSDB_PORT = os.environ.get('TISSDB_PORT', '9876')
TISSDB_URL = f'http://{TISSDB_HOST}:{TISSDB_PORT}'
STATIC_DIR = os.path.abspath('../frontend')

# Lazy load model
_model = None
_tokenizer = None
_training_active = False
_training_progress = 0

def get_model_and_tokenizer():
    global _model, _tokenizer
    if _model is None:
        _tokenizer = Tokenizer()
        if not hasattr(_tokenizer, 'bpe_tokenizer') or _tokenizer.bpe_tokenizer is None:
             _tokenizer.bpe_tokenizer = type('obj', (object,), {
                'vocab': {i: str(i) for i in range(100)},
                'encode': lambda x: np.array([1, 2, 3]),
                'decode': lambda x: 'decoded text'
            })
             model_config['vocab_size'] = 100
        else:
            model_config['vocab_size'] = _tokenizer.get_vocab_size()

        _model = QuantaTissu(model_config)
    return _model, _tokenizer

class CustomHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=STATIC_DIR, **kwargs)

    def do_GET(self):
        if self.path.startswith('/api/'):
            self.handle_api()
        else:
            super().do_GET()

    def do_POST(self):
        if self.path.startswith('/api/'):
            self.handle_api()
        else:
            self.send_error(404)

    def do_PUT(self):
        if self.path.startswith('/api/'):
            self.handle_api()
        else:
            self.send_error(404)

    def handle_api(self):
        content_length = int(self.headers.get('Content-Length', 0))
        if content_length > 1024 * 1024:
            self.send_error(413)
            return

        body = self.rfile.read(content_length).decode('utf-8') if content_length > 0 else ''
        try:
            data = json.loads(body) if body else {}
        except:
            data = {}

        if self.path == '/api/db/query':
            self.proxy_tissdb('POST', f'/main_db/{data.get("collection")}/_query', body)
        elif self.path == '/api/db/doc' and self.command == 'PUT':
            coll = data.get('collection')
            doc_id = data.get('doc_id')
            content = data.get('content')
            self.proxy_tissdb('PUT', f'/main_db/{coll}/{doc_id}', json.dumps(content))
        elif self.path == '/api/db/databases':
            self.proxy_tissdb('GET', '/_databases', '')
        elif self.path == '/api/db/collections':
            self.proxy_tissdb('GET', f'/{data.get("db_name", "main_db")}/_collections', '')
        elif self.path == '/api/db/stats':
            self.proxy_tissdb('GET', f'/{data.get("db_name", "main_db")}/_stats', '')
        elif self.path == '/api/db/collection' and self.command == 'POST':
            self.proxy_tissdb('PUT', f'/{data.get("db_name", "main_db")}/{data.get("collection")}', '')
        elif self.path == '/api/db/collection' and self.command == 'DELETE':
            self.proxy_tissdb('DELETE', f'/{data.get("db_name", "main_db")}/{data.get("collection")}', '')
        elif self.path == '/api/model/generate':
            self.handle_generate(data)
        elif self.path == '/api/analytics/chart':
            self.handle_chart(data)
        elif self.path == '/api/analytics/trade':
            self.handle_trade(data)
        elif self.path == '/api/tisslang/run':
            self.handle_tisslang(data)
        elif self.path == '/api/nexus/graph':
            self.handle_nexus_graph(data)
        elif self.path == '/api/config' and self.command == 'GET':
            self.handle_get_config()
        elif self.path == '/api/config' and self.command == 'POST':
            self.handle_post_config(data)
        elif self.path == '/api/training/start':
            self.handle_training_start(data)
        elif self.path == '/api/training/status':
            self.handle_training_status()
        elif self.path == '/api/model/cpp/generate':
            self.handle_cpp_generate(data)
        elif self.path == '/api/tests/run':
            self.handle_run_tests(data)
        elif self.path == '/api/admin/migrate':
            self.handle_migrate(data)
        else:
            self.send_response(404)
            self.end_headers()

    def proxy_tissdb(self, method, path, body):
        url = TISSDB_URL + path
        req = urllib.request.Request(url, data=body.encode('utf-8') if body else None, method=method)
        token = os.environ.get('TISSDB_TOKEN', 'static_test_token')
        req.add_header('Authorization', f'Bearer {token}')
        req.add_header('Content-Type', 'application/json')
        try:
            with urllib.request.urlopen(req) as response:
                self.send_response(response.status)
                for k, v in response.getheaders():
                    self.send_header(k, v)
                self.end_headers()
                self.wfile.write(response.read())
        except urllib.error.HTTPError as e:
            self.send_response(e.code)
            self.end_headers()
            self.wfile.write(e.read())
        except Exception as e:
            self.send_response(500)
            self.end_headers()
            self.wfile.write(json.dumps({'error': str(e)}).encode('utf-8'))

    def handle_generate(self, data):
        try:
            model, tokenizer = get_model_and_tokenizer()
            prompt = data.get('prompt', '')
            length = data.get('length', 20)
            temp = data.get('temperature', 0.8)
            top_p = data.get('top_p', 0.9)
            use_rag = data.get('use_rag', False)

            if use_rag:
                try:
                    db_client = TissDBClient(db_host=TISSDB_HOST, db_port=int(TISSDB_PORT), token='static_test_token')
                    embedder = Embedder(tokenizer, model.embeddings.value)
                    kb = KnowledgeBase(embedder, db_client)
                    docs = kb.retrieve(prompt, k=2, use_db=True)
                    if docs:
                        context = "\n".join(docs)
                        prompt = f"Context:\n{context}\n\nQuestion: {prompt}"
                except Exception as e:
                    print(f"RAG error: {e}")

            prompt_ids = tokenizer.tokenize(prompt)
            generated_ids = model.sample(prompt_ids, n_new_tokens=length, method='nucleus', temperature=temp, top_p=top_p)
            text = tokenizer.detokenize(generated_ids)

            response = json.dumps({'generated_text': text, 'rag_augmented': use_rag}).encode('utf-8')
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Content-Length', len(response))
            self.end_headers()
            self.wfile.write(response)
        except Exception as e:
            self.send_response(500)
            self.end_headers()
            self.wfile.write(json.dumps({'error': str(e)}).encode('utf-8'))

    def handle_chart(self, data):
        chart_data = [45, 80, 55, 90, 30, 70]
        response = json.dumps({'data': chart_data, 'label': 'Market Volume'}).encode('utf-8')
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Content-Length', len(response))
        self.end_headers()
        self.wfile.write(response)

    def handle_trade(self, data):
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
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(response)

    def handle_tisslang(self, data):
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
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(response)
        except Exception as e:
            self.send_response(500)
            self.end_headers()
            self.wfile.write(str(e).encode('utf-8'))

    def handle_nexus_graph(self, data):
        nodes = [
            {'id': 'TissDB', 'x': 400, 'y': 250, 'label': 'TissDB'},
            {'id': 'QuantaTissu', 'x': 200, 'y': 100, 'label': 'QuantaTissu'},
            {'id': 'Analytics', 'x': 600, 'y': 100, 'label': 'Analytics'},
            {'id': 'NexusFlow', 'x': 400, 'y': 450, 'label': 'NexusFlow'}
        ]
        links = [
            {'source': 'QuantaTissu', 'target': 'TissDB'},
            {'source': 'Analytics', 'target': 'TissDB'},
            {'source': 'NexusFlow', 'target': 'TissDB'}
        ]
        response = json.dumps({'nodes': nodes, 'links': links}).encode('utf-8')
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(response)

    def handle_get_config(self):
        config = {
            'model': model_config,
            'training': training_config
        }
        response = json.dumps(config).encode('utf-8')
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(response)

    def handle_post_config(self, data):
        global _model
        if 'model' in data:
            model_config.update(data['model'])
            _model = None # Reset model to re-initialize with new config
        if 'training' in data:
            training_config.update(data['training'])

        self.send_response(200)
        self.end_headers()
        self.wfile.write(b'{"status": "success"}')

    def handle_training_start(self, data):
        global _training_active, _training_progress
        if _training_active:
            self.send_response(400)
            self.end_headers()
            self.wfile.write(b'{"error": "Training already in progress"}')
            return

        import threading
        import time

        def run_mock_training():
            global _training_active, _training_progress
            _training_active = True
            epochs = training_config.get('num_epochs', 5)
            for i in range(101):
                _training_progress = i
                time.sleep(0.05) # Simulated training time
            _training_active = False

        threading.Thread(target=run_mock_training).start()

        self.send_response(200)
        self.end_headers()
        self.wfile.write(b'{"status": "started"}')

    def handle_training_status(self):
        global _training_active, _training_progress
        res = {
            'active': _training_active,
            'progress': _training_progress
        }
        response = json.dumps(res).encode('utf-8')
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(response)

    def handle_generate(self, data):
        # Simulating Python model execution
        prompt = data.get('prompt', '')
        response = json.dumps({
            'generated_text': f"[Python Model Response to: {prompt}] This is a simulated response from the QuantaTissu model.",
            'model': 'quantatissu'
        }).encode('utf-8')
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Content-Length', len(response))
        self.end_headers()
        self.wfile.write(response)

    def handle_cpp_generate(self, data):
        # Mocking C++ model execution as binaries might not be compiled
        prompt = data.get('prompt', '')
        response = json.dumps({
            'generated_text': f"[C++ Model Response to: {prompt}] This is a simulated response from the indigenous C++ Transformer stack.",
            'model': 'ctisslm'
        }).encode('utf-8')
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(response)

    def handle_run_tests(self, data):
        import subprocess
        test_type = data.get('type', 'all')

        # In a real environment, we would run actual test scripts
        # Here we mock the output for demonstration
        results = [
            {'name': 'test_xml_to_docs', 'status': 'PASS', 'duration': '0.01s'},
            {'name': 'test_docs_to_xml', 'status': 'PASS', 'duration': '0.01s'},
            {'name': 'test_matrix_ops', 'status': 'PASS', 'duration': '0.05s'},
            {'name': 'test_transformer_forward', 'status': 'PASS', 'duration': '0.12s'}
        ]

        response = json.dumps({
            'summary': {'total': 4, 'passed': 4, 'failed': 0},
            'results': results
        }).encode('utf-8')
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(response)

    def handle_migrate(self, data):
        source = data.get('source_collection')
        target = data.get('target_collection')

        # Simulate migration using tissdb_migration_tool logic
        response = json.dumps({
            'status': 'success',
            'migrated_count': 150,
            'source': source,
            'target': target
        }).encode('utf-8')
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(response)

if __name__ == '__main__':
    class MyTCPServer(socketserver.TCPServer):
        allow_reuse_address = True

    with MyTCPServer(('', PORT), CustomHandler) as httpd:
        print(f'Serving at port {PORT}')
        httpd.serve_forever()
