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


def test_add_document_with_metadata():
    """Tests that custom metadata is correctly added to a document."""
    kb = create_mock_kb()
    doc_text = "parrots can talk"
    metadata = {"source": "test_suite", "author": "Jules"}
    kb.add_document(doc_text, metadata=metadata)

    assert_equal(len(kb.doc_metadata), 1, "Should have one metadata entry")
    # Check that custom metadata was added
    assert_equal(kb.doc_metadata[0]['source'], "test_suite", "Metadata 'source' should be updated")
    assert_equal(kb.doc_metadata[0]['author'], "Jules", "Metadata should contain new keys")
    # Check that default metadata is still present
    assert_true('timestamp' in kb.doc_metadata[0], "Default metadata 'timestamp' should exist")


def test_retrieval_updates_access_count():
    """Tests that retrieving a document increments its access_count."""
    kb = create_mock_kb()
    doc_text = "the sun is bright"
    kb.add_document(doc_text)

    assert_equal(kb.doc_metadata[0]['access_count'], 0, "Initial access count should be 0")

    # Retrieve the document
    kb.retrieve("what color is the sun", k=1)

    assert_equal(kb.doc_metadata[0]['access_count'], 1, "Access count should be 1 after one retrieval")

    # Retrieve it again
    kb.retrieve("is the sun bright", k=1)

    assert_equal(kb.doc_metadata[0]['access_count'], 2, "Access count should be 2 after two retrievals")


def test_add_feedback():
    """Tests that feedback is recorded in the feedback_history."""
    kb = create_mock_kb()
    query = "what are cats"
    retrieved_docs = ["cats are great pets"]
    feedback_score = 5
    feedback_text = "Very relevant"

    kb.add_feedback(query, retrieved_docs, feedback_score, feedback_text)

    assert_equal(len(kb.feedback_history), 1, "Should have one feedback entry")
    feedback_entry = kb.feedback_history[0]
    assert_equal(feedback_entry['query'], query, "Feedback query should match")
    assert_equal(feedback_entry['score'], feedback_score, "Feedback score should match")
    assert_equal(feedback_entry['text'], feedback_text, "Feedback text should match")


def test_feedback_updates_relevance_score():
    """Tests that providing feedback adjusts a document's relevance_score."""
    kb = create_mock_kb()
    doc_text = "cats are great pets"
    kb.add_document(doc_text)

    initial_score = kb.doc_metadata[0]['relevance_score']
    assert_equal(initial_score, 1.0, "Initial relevance score should be 1.0")

    # Give positive feedback (5/5)
    kb.add_feedback("what are cats", [doc_text], 5)

    # After positive feedback, the score should not change much from 1.0, maybe slightly
    # as per the EMA formula, but since it's already max, it might stay same or dip slightly
    # depending on the formula: new_score = (1 - alpha) * current_score + alpha * (feedback_score / 5.0)
    # new_score = 0.9 * 1.0 + 0.1 * 1.0 = 1.0. So it should be 1.0
    assert_equal(kb.doc_metadata[0]['relevance_score'], 1.0, "Relevance score should be 1.0 after high feedback")

    # Give negative feedback (1/5)
    kb.add_feedback("what are dogs", [doc_text], 1)

    # After negative feedback, the score should decrease
    # new_score = 0.9 * 1.0 + 0.1 * (1/5) = 0.9 + 0.02 = 0.92
    assert_true(kb.doc_metadata[0]['relevance_score'] < 1.0, "Relevance score should decrease after low feedback")
    assert_equal(round(kb.doc_metadata[0]['relevance_score'], 2), 0.92, "Relevance score should be ~0.92")


def test_self_update_with_user_correction():
    """Tests that a new document is added when a user_correction is provided."""
    kb = create_mock_kb()
    query = "what is the sun"
    generated_response = "the sun is blue" # An incorrect response
    user_correction = "the sun is bright"

    initial_doc_count = len(kb.documents)
    kb.self_update_from_interaction(query, generated_response, user_correction)

    assert_equal(len(kb.documents), initial_doc_count + 1, "Should add one new document")
    new_doc_text = kb.documents[-1]
    new_doc_metadata = kb.doc_metadata[-1]

    assert_true(user_correction in new_doc_text, "New document should contain the correction")
    assert_equal(new_doc_metadata['source'], 'self_correction', "Source should be 'self_correction'")


def test_self_update_without_correction():
    """Tests that a new document is added from the model's own output."""
    kb = create_mock_kb()
    query = "what are dogs"
    generated_response = "dogs are loyal companions"

    initial_doc_count = len(kb.documents)
    kb.self_update_from_interaction(query, generated_response)

    assert_equal(len(kb.documents), initial_doc_count + 1, "Should add one new document")
    new_doc_metadata = kb.doc_metadata[-1]
    assert_equal(new_doc_metadata['source'], 'self_generated', "Source should be 'self_generated'")


def test_get_knowledge_stats_empty():
    """Tests that stats are correct for an empty knowledge base."""
    kb = create_mock_kb()
    stats = kb.get_knowledge_stats()
    assert_equal(stats['total_docs'], 0, "Should have 0 total docs")


def test_get_knowledge_stats_populated():
    """Tests that stats are correct for a populated knowledge base."""
    kb = create_mock_kb()
    kb.add_document("the sky is blue", metadata={'source': 'manual'})
    kb.add_document("the sun is bright", metadata={'source': 'manual'})
    kb.add_document("cats are great pets", metadata={'source': 'user_input'})

    # Access one document
    kb.retrieve("what is the sky")

    stats = kb.get_knowledge_stats()

    assert_equal(stats['total_docs'], 3, "Should have 3 total docs")
    assert_equal(stats['total_accesses'], 1, "Should have 1 total access")
    assert_equal(stats['sources']['manual'], 2, "Should have 2 'manual' sources")
    assert_equal(stats['sources']['user_input'], 1, "Should have 1 'user_input' source")
    assert_true(isinstance(stats['avg_relevance_score'], float), "Avg relevance score should be a float")


def test_retrieve_k_greater_than_docs():
    """Tests that retrieving with k > num_docs returns all documents."""
    kb = create_mock_kb()
    kb.add_document("cats are great pets")
    kb.add_document("dogs are loyal companions")

    # Request more documents than exist, with a meaningful query
    retrieved_docs = kb.retrieve("great loyal pets", k=5)

    assert_equal(len(retrieved_docs), 2, "Should return all available documents")


def test_retrieve_query_with_unknown_words():
    """Tests retrieval with a query containing only out-of-vocabulary words."""
    kb = create_mock_kb()
    kb.add_document("a known document")

    # These words are not in the test_vocab
    query = "zombie xylophone"
    retrieved_docs = kb.retrieve(query, k=1)

    assert_equal(len(retrieved_docs), 0, "Should not retrieve documents for OOV query")
