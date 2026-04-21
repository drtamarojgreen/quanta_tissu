import unittest
import json
import urllib.request
import threading
import time
import os
import sys

# Add backend to path
sys.path.append(os.path.dirname(__file__))
from server import CustomHandler

class MockServer(threading.Thread):
    def __init__(self, port):
        super().__init__()
        self.port = port
        from socketserver import TCPServer
        TCPServer.allow_reuse_address = True
        self.httpd = TCPServer(('', port), CustomHandler)

    def run(self):
        self.httpd.serve_forever()

    def shutdown(self):
        self.httpd.shutdown()
        self.httpd.server_close()

class TestBackend(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.port = 8001
        cls.server = MockServer(cls.port)
        cls.server.start()
        cls.url = f"http://localhost:{cls.port}"
        time.sleep(1)

    @classmethod
    def tearDownClass(cls):
        cls.server.shutdown()
        cls.server.join()

    def test_config_get(self):
        req = urllib.request.Request(f"{self.url}/api/config")
        with urllib.request.urlopen(req) as res:
            data = json.loads(res.read().decode())
            self.assertIn('model', data)
            self.assertIn('training', data)

    def test_config_post(self):
        payload = json.dumps({'model': {'n_layer': 10}}).encode()
        req = urllib.request.Request(f"{self.url}/api/config", data=payload, method='POST')
        with urllib.request.urlopen(req) as res:
            self.assertEqual(res.status, 200)

        # Verify update
        with urllib.request.urlopen(f"{self.url}/api/config") as res:
            data = json.loads(res.read().decode())
            self.assertEqual(data['model']['n_layer'], 10)

    def test_training_workflow(self):
        # Start training
        req = urllib.request.Request(f"{self.url}/api/training/start", method='POST')
        with urllib.request.urlopen(req) as res:
            self.assertEqual(res.status, 200)

        # Check status
        with urllib.request.urlopen(f"{self.url}/api/training/status") as res:
            data = json.loads(res.read().decode())
            self.assertTrue(data['active'] or data['progress'] == 100)

    def test_tests_run(self):
        payload = json.dumps({'type': 'unit'}).encode()
        req = urllib.request.Request(f"{self.url}/api/tests/run", data=payload, method='POST')
        with urllib.request.urlopen(req) as res:
            data = json.loads(res.read().decode())
            self.assertEqual(data['summary']['total'], 4)

    def test_migrate(self):
        payload = json.dumps({'source_collection': 'a', 'target_collection': 'b'}).encode()
        req = urllib.request.Request(f"{self.url}/api/admin/migrate", data=payload, method='POST')
        with urllib.request.urlopen(req) as res:
            data = json.loads(res.read().decode())
            self.assertEqual(data['status'], 'success')
            self.assertEqual(data['migrated_count'], 150)

    def test_cpp_generate(self):
        payload = json.dumps({'prompt': 'test'}).encode()
        req = urllib.request.Request(f"{self.url}/api/model/cpp/generate", data=payload, method='POST')
        with urllib.request.urlopen(req) as res:
            data = json.loads(res.read().decode())
            self.assertIn('C++ Model Response', data['generated_text'])

    def test_tisslang_run(self):
        script = 'TASK "Test"\nSTEP "Step" {\n  ASSERT 1 == 1\n}'
        payload = json.dumps({'script': script}).encode()
        req = urllib.request.Request(f"{self.url}/api/tisslang/run", data=payload, method='POST')
        with urllib.request.urlopen(req) as res:
            data = json.loads(res.read().decode())
            self.assertIn('log', data)

    def test_analytics_chart(self):
        req = urllib.request.Request(f"{self.url}/api/analytics/chart", method='POST')
        with urllib.request.urlopen(req) as res:
            data = json.loads(res.read().decode())
            self.assertEqual(len(data['data']), 6)

    def test_nexus_graph(self):
        req = urllib.request.Request(f"{self.url}/api/nexus/graph", method='POST')
        with urllib.request.urlopen(req) as res:
            data = json.loads(res.read().decode())
            self.assertGreater(len(data['nodes']), 0)

if __name__ == '__main__':
    unittest.main()
