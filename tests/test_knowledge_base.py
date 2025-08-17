import sys
import os
import numpy as np

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from quanta_tissu.tisslm.knowledge_base import KnowledgeBase
from quanta_tissu.tisslm.tokenizer import tokenize as real_tokenize
from tests.test_utils import assert_equal, assert_true

# --- Test-specific vocabulary and tokenizer ---
# We create a more extensive, controlled vocabulary and tokenizer for these tests
# to make similarity testing reliable and independent of the main config.
test_vocab = {
    "the": 0, "sky": 1, "is": 2, "blue": 3, "sun": 4, "bright": 5, "what": 6,
    "color": 7, "cats": 8, "are": 9, "great": 10, "pets": 11, "dogs": 12,
    "loyal": 13, "companions": 14, "parrots": 15, "can": 16, "talk": 17,
    "a": 18, "known": 19, "document": 20, "<unk>": 21
}
vocab_size = len(test_vocab)

class MockTokenizer:
    def tokenize(self, text):
        """A tokenizer that uses the test-specific vocabulary."""
        tokens = []
        for word in text.lower().split():
            tokens.append(test_vocab.get(word, test_vocab["<unk>"]))
        return np.array(tokens)

def create_mock_kb():
    """Helper function to create a KnowledgeBase with deterministic embeddings."""
    # Create a deterministic embedding matrix where embeddings for related
    # words are intentionally made similar.
    d_model = 8
    mock_embeddings = np.zeros((vocab_size, d_model))

    # Group 1: Sky/Blue/Color
    mock_embeddings[test_vocab["sky"]]    = np.array([0.9, 0.1, 0, 0, 0, 0, 0, 0])
    mock_embeddings[test_vocab["blue"]]   = np.array([0.8, 0.2, 0, 0, 0, 0, 0, 0])
    mock_embeddings[test_vocab["color"]]  = np.array([0.7, 0.3, 0, 0, 0, 0, 0, 0])

    # Group 2: Sun/Bright
    mock_embeddings[test_vocab["sun"]]    = np.array([0, 0, 0.9, 0.1, 0, 0, 0, 0])
    mock_embeddings[test_vocab["bright"]] = np.array([0, 0, 0.8, 0.2, 0, 0, 0, 0])

    # Group 3: Pets
    mock_embeddings[test_vocab["cats"]]   = np.array([0, 0, 0, 0, 0.9, 0.1, 0, 0])
    mock_embeddings[test_vocab["dogs"]]   = np.array([0, 0, 0, 0, 0.8, 0.2, 0, 0])
    mock_embeddings[test_vocab["pets"]]   = np.array([0, 0, 0, 0, 0.7, 0.3, 0, 0])
    mock_embeddings[test_vocab["loyal"]]  = np.array([0, 0, 0, 0, 0.6, 0.4, 0, 0])
    mock_embeddings[test_vocab["companions"]] = np.array([0, 0, 0, 0, 0.5, 0.5, 0, 0])

    # Group 4: Parrots
    mock_embeddings[test_vocab["parrots"]] = np.array([0, 0, 0, 0, 0, 0, 0.9, 0.1])
    mock_embeddings[test_vocab["talk"]]    = np.array([0, 0, 0, 0, 0, 0, 0.8, 0.2])


    # Normalize embeddings to have unit length for clean cosine similarity
    for i in range(vocab_size):
        norm = np.linalg.norm(mock_embeddings[i])
        if norm > 0:
            mock_embeddings[i] /= norm
    
    # Wrap the mock embeddings in a Parameter-like object
    class MockEmbeddings:
        def __init__(self, value):
            self.value = value

    # Use the test-specific tokenizer
    kb = KnowledgeBase(MockEmbeddings(mock_embeddings), MockTokenizer())
    return kb

def test_add_document():
    """Tests that a document is successfully added to the knowledge base."""
    kb = create_mock_kb()
    doc_text = "the sky is blue"
    kb.add_document(doc_text)

    assert_equal(len(kb.documents), 1, "KB should have one document")
    assert_equal(kb.documents[0], doc_text, "Document text should match")
    assert_equal(len(kb.doc_embeddings), 1, "KB should have one embedding")
    assert_equal(kb.doc_embeddings[0].shape, (8,), "Embedding should have correct dimension")

def test_retrieve_document():
    """Tests that the correct document is retrieved for a query."""
    kb = create_mock_kb()
    doc1 = "the sky is blue"
    doc2 = "the sun is bright"
    kb.add_document(doc1)
    kb.add_document(doc2)

    # Query that is very similar to doc1
    query = "what color is the sky"
    retrieved_docs = kb.retrieve(query, k=1)

    assert_equal(len(retrieved_docs), 1, "Should retrieve one document")
    assert_equal(retrieved_docs[0], doc1, "Should retrieve the most relevant document")

def test_retrieve_from_empty_kb():
    """Tests that retrieving from an empty knowledge base returns an empty list."""
    kb = create_mock_kb()
    retrieved_docs = kb.retrieve("any query", k=1)
    assert_equal(retrieved_docs, [], "Retrieving from empty KB should yield empty list")

def test_retrieve_with_empty_query():
    """Tests retrieval with a query that results in no tokens."""
    kb = create_mock_kb()
    kb.add_document("a known document")

    # This query will result in an empty token list
    query = ""
    retrieved_docs = kb.retrieve(query, k=1)

    # The KB should return an empty list for an empty query
    assert_equal(len(retrieved_docs), 0, "Should not retrieve documents for empty query")

def test_add_and_retrieve_multiple():
    """Tests adding multiple documents and retrieving the top 2."""
    kb = create_mock_kb()
    doc1 = "cats are great pets"
    doc2 = "dogs are loyal companions"
    doc3 = "parrots can talk"
    kb.add_document(doc1)
    kb.add_document(doc2)
    kb.add_document(doc3)

    query = "loyal pets companions" # Should be closer to dogs and cats
    retrieved_docs = kb.retrieve(query, k=2)

    assert_equal(len(retrieved_docs), 2, "Should retrieve two documents")
    # The order matters, most relevant should be first
    assert_equal(retrieved_docs[0], doc2, "First retrieved doc should be about dogs")
    # The second one is likely to be about cats
    assert_true(retrieved_docs[1] in [doc1, doc3], "Second retrieved doc should be one of the others")
