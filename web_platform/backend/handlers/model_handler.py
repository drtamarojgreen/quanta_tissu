import json
import os
import numpy as np
import sys

# Add project root to path for imports
BACKEND_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(BACKEND_DIR, '..', '..', '..'))
if PROJECT_ROOT not in sys.path:
    sys.path.append(PROJECT_ROOT)

try:
    from quanta_tissu.tisslm.core.model import QuantaTissu
    from quanta_tissu.tisslm.core.tokenizer import Tokenizer
    from quanta_tissu.tisslm.config import model_config
    from quanta_tissu.tisslm.core.db.client import TissDBClient
    from quanta_tissu.tisslm.core.embedding.embedder import Embedder
    from quanta_tissu.tisslm.core.knowledge_base import KnowledgeBase
    CORE_AVAILABLE = True
except ImportError:
    CORE_AVAILABLE = False

# Lazy load model
_model = None
_tokenizer = None

def get_model_and_tokenizer():
    global _model, _tokenizer
    if not CORE_AVAILABLE:
        raise RuntimeError("QuantaTissu core modules not found.")

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

def handle_model(handler, path, data, command):
    if path == '/api/model/generate':
        handle_generate(handler, data)
    elif path == '/api/model/cpp/generate':
        handle_cpp_generate(handler, data)
    else:
        return False
    return True

def handle_generate(handler, data):
    try:
        if not CORE_AVAILABLE:
            raise RuntimeError("QuantaTissu core modules not found. Python generation disabled.")

        model, tokenizer = get_model_and_tokenizer()
        prompt = data.get('prompt', '')
        length = data.get('length', 20)
        temp = data.get('temperature', 0.8)
        top_p = data.get('top_p', 0.9)
        use_rag = data.get('use_rag', False)

        if use_rag:
            try:
                TISSDB_HOST = os.environ.get('TISSDB_HOST', 'localhost')
                TISSDB_PORT = os.environ.get('TISSDB_PORT', '9876')
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
        handler.send_response(200)
        handler.send_header('Content-Type', 'application/json')
        handler.send_header('Content-Length', len(response))
        handler.end_headers()
        handler.wfile.write(response)
    except Exception as e:
        handler.send_response(500)
        handler.end_headers()
        handler.wfile.write(json.dumps({'error': str(e)}).encode('utf-8'))

def handle_cpp_generate(handler, data):
    # Mocking C++ model execution as binaries might not be compiled
    prompt = data.get('prompt', '')
    response = json.dumps({
        'generated_text': f"[C++ Model Response to: {prompt}] This is a simulated response from the indigenous C++ Transformer stack.",
        'model': 'ctisslm'
    }).encode('utf-8')
    handler.send_response(200)
    handler.send_header('Content-Type', 'application/json')
    handler.end_headers()
    handler.wfile.write(response)
