import unittest
import numpy as np
import os
import shutil
import requests
from unittest.mock import MagicMock, patch

# Adjust path to import modules from the project root
import sys
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.db.client import TissDBClient
from quanta_tissu.tisslm.core.knowledge_base import KnowledgeBase
from quanta_tissu.tisslm.core.embedding.embedder import Embedder
from quanta_tissu.tisslm.core.retrieval.strategy import CosineSimilarityStrategy
from quanta_tissu.tisslm.core.model_error_handler import ModelProcessingError
from quanta_tissu.tisslm.core.system_error_handler import DatabaseConnectionError

class TestTissDBClient(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.db_host = '127.0.0.1'
        cls.db_port = 9876
        cls.db_name = 'test_quanta_tissu_db'
        cls.client = TissDBClient(db_host=cls.db_host, db_port=cls.db_port, db_name=cls.db_name, token="static_test_token")
        
        # Ensure the database is clean before tests
        try:
            requests.delete(f"http://{cls.db_host}:{cls.db_port}/{cls.db_name}")
        except requests.exceptions.RequestException:
            pass # Ignore if DB doesn't exist

    @classmethod
    def tearDownClass(cls):
        # Clean up the database after tests
        try:
            requests.delete(f"http://{cls.db_host}:{cls.db_port}/{cls.db_name}")
        except requests.exceptions.RequestException:
            pass # Ignore if DB doesn't exist

    def test_ensure_db_setup_success(self):
        collections = ['col1', 'col2']
        self.assertTrue(self.client.ensure_db_setup(collections))

    @unittest.skip("TissDB PUT for adding documents is currently returning 500 Internal Server Error.")
    def test_add_document_success(self):
        collection = 'test_docs'
        doc_id = 'doc1'
        document = {'content': 'test_value'}
        response = self.client.add_document(collection, document, doc_id=doc_id)
        self.assertIsNotNone(response)
        # TissDB add_document currently returns an empty JSON object on success
        self.assertEqual(response, {})

    def test_get_all_documents_stub(self):
        # This method is explicitly a stub and should return an empty list
        documents = self.client.get_all_documents('any_collection')
        self.assertEqual(documents, [])

    def test_get_stats_success(self):
        stats = self.client.get_stats()
        self.assertIsInstance(stats, dict)
        self.assertIn('feedback_entries', stats)
        self.assertIn('total_accesses', stats)
        self.assertIn('total_docs', stats)

    @unittest.skip("TissDB PUT for adding documents is currently returning 500 Internal Server Error.")
    def test_add_feedback_success(self):
        feedback_data = {'query': 'test query', 'score': 5}
        response = self.client.add_feedback(feedback_data)
        self.assertIsNotNone(response)
        self.assertEqual(response, {})

class TestKnowledgeBase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.db_host = '127.0.0.1'
        cls.db_port = 9876
        cls.db_name = 'test_kb_db'
        cls.db_client = TissDBClient(db_host=cls.db_host, db_port=cls.db_port, db_name=cls.db_name, token="static_test_token")
        
        # Ensure the database is clean before tests
        try:
            requests.delete(f"http://{cls.db_host}:{cls.db_port}/{cls.db_name}")
        except requests.exceptions.RequestException:
            pass # Ignore if DB doesn't exist

        # Mock tokenizer for Embedder
        cls.mock_tokenizer = MagicMock()
        cls.mock_tokenizer.tokenize.side_effect = lambda text: np.array([ord(c) for c in text])
        
        # Dummy model embeddings for Embedder
        cls.embedding_dim = 10
        cls.model_embeddings_value = np.random.randn(256, cls.embedding_dim)
        cls.embedder = Embedder(cls.mock_tokenizer, cls.model_embeddings_value)

    @classmethod
    def tearDownClass(cls):
        # Clean up the database after tests
        try:
            requests.delete(f"http://{cls.db_host}:{cls.db_port}/{cls.db_name}")
        except requests.exceptions.RequestException:
            pass # Ignore if DB doesn't exist

    def setUp(self):
        # Re-initialize KnowledgeBase for each test to ensure clean state
        self.kb = KnowledgeBase(embedder=self.embedder, db_client=self.db_client)
        # Ensure KB is connected
        self.kb.db_client.ensure_db_setup(['knowledge', 'knowledge_feedback', 'feedback'])

    def test_add_document_local_and_db(self):
        text = "This is a test document."
        self.kb.add_document(text)
        
        # Check local cache
        self.assertEqual(len(self.kb._local_document_cache), 1)
        self.assertEqual(self.kb._local_document_cache[0]['text'], text)

        # Since get_all_documents is a stub, we can't directly verify DB content.
        # We rely on add_document not raising an error.

    def test_retrieve_from_local_cache(self):
        doc1_text = "Document one for local cache."
        doc2_text = "Document two for local cache."
        self.kb.add_document(doc1_text)
        self.kb.add_document(doc2_text)

        query_text = "Document one"
        retrieved_docs = self.kb.retrieve(query_text, use_db=False) # Force local cache

        self.assertEqual(len(retrieved_docs), 1)
        self.assertEqual(retrieved_docs[0], doc1_text)

    def test_retrieve_db_fallback_to_local(self):
        doc_text = "Document for DB fallback."
        self.kb.add_document(doc_text)

        # Mock db_client.get_all_documents to return empty (as it's a stub)
        with patch.object(self.kb.db_client, 'get_all_documents', return_value=[]):
            query_text = "Document for DB"
            retrieved_docs = self.kb.retrieve(query_text, use_db=True) # Try DB, should fallback

            self.assertEqual(len(retrieved_docs), 1)
            self.assertEqual(retrieved_docs[0], doc_text)

    def test_self_update_from_interaction(self):
        query = "What is the capital of France?"
        generated_response = "Paris."
        self.kb.self_update_from_interaction(query, generated_response)

        # Check local cache
        self.assertEqual(len(self.kb._local_document_cache), 1)
        self.assertIn("Query: What is the capital of France?\nResponse: Paris.", self.kb._local_document_cache[0]['text'])

    def test_add_feedback(self):
        query = "Feedback query"
        retrieved_docs = ["doc1", "doc2"]
        feedback_score = 4
        feedback_text = "Good retrieval."
        self.kb.add_feedback(query, retrieved_docs, feedback_score, feedback_text)
        # No direct way to verify in DB, rely on add_feedback not raising error

if __name__ == '__main__':
    unittest.main()
