import http.server
import socketserver
import json
import sys
import os
import mimetypes
import traceback

# Add project root to path
BACKEND_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(BACKEND_DIR, '..', '..'))
if PROJECT_ROOT not in sys.path:
    sys.path.append(PROJECT_ROOT)

# Individual handler imports with granular error handling
def safe_import(module_path, func_name):
    try:
        module = __import__(module_path, fromlist=[func_name])
        return getattr(module, func_name)
    except Exception as e:
        print(f"Warning: Could not import {func_name} from {module_path}: {e}")
        return lambda *args: False

handle_db = safe_import('web_platform.backend.handlers.db_handler', 'handle_db')
handle_model = safe_import('web_platform.backend.handlers.model_handler', 'handle_model')
handle_analytics = safe_import('web_platform.backend.handlers.analytics_handler', 'handle_analytics')
handle_tisslang = safe_import('web_platform.backend.handlers.tisslang_handler', 'handle_tisslang')
handle_nexus = safe_import('web_platform.backend.handlers.nexus_handler', 'handle_nexus')
handle_admin = safe_import('web_platform.backend.handlers.admin_handler', 'handle_admin')
handle_tasks = safe_import('web_platform.backend.handlers.task_handler', 'handle_tasks')

PORT = 8000
STATIC_DIR = os.path.abspath(os.path.join(BACKEND_DIR, '..', 'frontend'))

class CustomHandler(http.server.BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        sys.stdout.write("%s - - [%s] %s\n" %
                         (self.address_string(),
                          self.log_date_time_string(),
                          format % args))

    def do_GET(self):
        if self.path.startswith('/api/'):
            self.handle_api()
            return
        self.serve_static()

    def do_HEAD(self):
        if self.path.startswith('/api/'):
            self.send_response(200)
            self.end_headers()
            return
        self.serve_static(only_headers=True)

    def serve_static(self, only_headers=False):
        path_clean = self.path.split('?')[0].split('#')[0]
        if path_clean == '/' or path_clean == '':
            requested = 'index.html'
        else:
            requested = path_clean.lstrip('/')

        full_path = os.path.join(STATIC_DIR, requested)
        if os.path.isdir(full_path):
            full_path = os.path.join(full_path, 'index.html')
        if not os.path.isfile(full_path):
            full_path = os.path.join(STATIC_DIR, 'index.html')

        if os.path.isfile(full_path):
            content_type, _ = mimetypes.guess_type(full_path)
            if not content_type:
                content_type = 'application/octet-stream'

            try:
                with open(full_path, 'rb') as f:
                    content = f.read()
                    self.send_response(200)
                    self.send_header("Content-type", content_type)
                    self.send_header("Content-length", len(content))
                    self.send_header("Access-Control-Allow-Origin", "*")
                    self.end_headers()
                    if not only_headers:
                        self.wfile.write(content)
            except Exception as e:
                self.send_error(500, str(e))
        else:
            self.send_error(404, f"File not found: {requested}")

    def do_POST(self):
        if self.path.startswith('/api/'):
            self.handle_api()
        else:
            self.send_error(405)

    def do_PUT(self):
        if self.path.startswith('/api/'):
            self.handle_api()
        else:
            self.send_error(405)

    def do_DELETE(self):
        if self.path.startswith('/api/'):
            self.handle_api()
        else:
            self.send_error(405)

    def handle_api(self):
        content_length = int(self.headers.get('Content-Length', 0))
        if content_length > 10 * 1024 * 1024:
            self.send_error(413)
            return

        body = self.rfile.read(content_length).decode('utf-8') if content_length > 0 else ''
        try:
            data = json.loads(body) if body else {}
        except json.JSONDecodeError:
            self.send_error(400)
            return

        handlers = [handle_db, handle_model, handle_analytics, handle_tisslang, handle_nexus, handle_admin, handle_tasks]
        handled = False
        for handler_func in handlers:
            try:
                if handler_func(self, self.path, data, self.command):
                    handled = True
                    break
            except Exception as e:
                traceback.print_exc()
                self.send_response(500)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({'error': str(e)}).encode('utf-8'))
                return

        if not handled:
            self.send_response(404)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({'error': f"Endpoint {self.path} not found"}).encode('utf-8'))

if __name__ == '__main__':
    class MyTCPServer(socketserver.TCPServer):
        allow_reuse_address = True

    with MyTCPServer(('0.0.0.0', PORT), CustomHandler) as httpd:
        print(f'QuantaTissu Platform serving at http://127.0.0.1:{PORT}')
        print(f'Static directory: {STATIC_DIR}')
        httpd.serve_forever()
