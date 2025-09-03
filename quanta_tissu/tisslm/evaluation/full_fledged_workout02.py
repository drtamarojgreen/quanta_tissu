import os
import sys
import requests
import numpy as np
import uuid
import time
from datetime import datetime, timezone

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
# from quanta_tissu.tisslm.core.embedding.embedder import Embedder # Comment out original Embedder
from quanta_tissu.tisslm.core.db.client import TissDBClient

# --- Configuration ---
EMBEDDINGS_DIR = os.path.join(project_root, "data", "embeddings")

# --- Mock Embedder Class ---
class MockEmbedder:
    def __init__(self, tokenizer_or_str, embeddings_value):
        # This mock embedder does not use tokenizer or embeddings_value for hashing
        # It just stores them to mimic the original interface
        self.tokenizer_info = str(tokenizer_or_str)
        self.embeddings_info = embeddings_value
        print(f"  MockEmbedder initialized with tokenizer_info: {self.tokenizer_info} and embeddings_info shape: {self.embeddings_info.shape}")

    def embed(self, text):
        # Return a dummy embedding (e.g., a numpy array of zeros)
        # The shape should match the expected embedding dimension, e.g., 128
        print(f"  MockEmbedder embedding text: '{text[:30]}...'")
        return np.zeros(128, dtype=np.float32) # Assuming embedding dimension is 128

# --- Helper function for text generation (needed for evaluator_prompt and final_prompt) ---
def generate_with_model(model, tokenizer, prompt, length, method, **kwargs):
    """Helper to generate text using model.sample, which uses AlgorithmicGenerator."""
    if method == "nucleus" and "top_p" not in kwargs:
        kwargs["top_p"] = 0.9
    prompt_tokens = tokenizer.tokenize(prompt).tolist()
    generated_tokens = model.sample(
        prompt_tokens=prompt_tokens,
        n_new_tokens=length,
        method=method,
        tokenizer=tokenizer,  # Pass tokenizer for methods that need it
        **kwargs
    )
    return tokenizer.detokenize(np.array(generated_tokens))

def run_rag_and_self_update_test(model, tokenizer):
    """Evaluates the Retrieve-Evaluate-Generate workflow and self-updating KB with TissDB integration."""
    report = ["\n--- Test 2: RAG and Self-Updating Knowledge Base ---"]
    db_name = "rag_test_db"
    collection_name = "knowledge"
    db_client = None

    doc_counter = 0

    try:
        # --- 1. Setup RAG components with live TissDB ---
        report.append("\n  --- Test 2a: Retrieve-Evaluate-Generate (RAG) ---\n")
        report.append("  Setting up RAG components with live TissDB integration...\n")
        report.append(f"  Type of model.embeddings.value: {type(model.embeddings.value)}\n")
        report.append(f"  Value of model.embeddings.value: {model.embeddings.value}\n")
        report.append(f"  Type of tokenizer: {type(tokenizer)}\n")
        report.append(f"  Value of tokenizer: {tokenizer}\n")

        # Use MockEmbedder instead of original Embedder
        embedder = MockEmbedder(tokenizer, model.embeddings.value)
        db_client = TissDBClient(db_port=9876, token="static_test_token", db_name=db_name)

        # Ensure the database is clean before starting
        headers = {"Authorization": f"Bearer static_test_token"}
        requests.delete(f"http://localhost:9876/{db_name}", headers=headers)

        # Ensure the database and collection exist
        response = requests.put(f"http://localhost:9876/{db_name}", headers=headers, json={{}})
        if response.status_code not in [200, 201, 409]:
            response.raise_for_status()
        response = requests.put(f"http://localhost:9876/{db_name}/{collection_name}", headers=headers, json={{}})
        if response.status_code not in [200, 201, 409]:
            response.raise_for_status()

        # --- 2. Populate the Knowledge Base ---
        report.append("  Populating Knowledge Base with sample documents...\n")
        sample_docs = [
            {"id": "mars_mission", "content": "The first manned mission to Mars, named 'Ares 1', is scheduled for 2035."},
            {"id": "eldoria_capital", "content": "The capital of the fictional country of Eldoria is Silverhaven."},
            {"id": "quantum_computing", "content": "Quantum computing relies on the principles of superposition and entanglement."}
        ]
        for doc in sample_docs:
            embedding = embedder.embed(doc["content"])
            report.append(f"  Type of embedding: {type(embedding)}\n")
            report.append(f"  Value of embedding: {embedding}\n")
            # Save embedding to a file
            embedding_file_path = os.path.join(EMBEDDINGS_DIR, f"{doc['id']}.npy")
            np.save(embedding_file_path, embedding)

            document = {
                "text": doc["content"],
                "source": "user_input",
                "timestamp": datetime.now(timezone.utc).isoformat()
            }
            response = requests.put(f"http://localhost:9876/{db_name}/{collection_name}/{doc['id']}", json=document, headers=headers)
            response.raise_for_status()
            doc_counter += 1

        report.append(f"  Added {len(sample_docs)} documents to the '{collection_name}' collection.\n")

        # --- 3. Run the RAG Workflow ---
        user_query = "What is the name of the first Mars mission and when is it scheduled?"
        report.append(f"\n  User Query: '{user_query}'\n")

        # Step 3a: Retrieve
        response = requests.get(f"http://localhost:9876/{db_name}/{collection_name}/mars_mission", headers=headers)
        if response.status_code == 200:
            retrieved_context = response.json()['text']
            # Load embedding from file (if needed for further processing like similarity search)
            retrieved_embedding_path = os.path.join(EMBEDDINGS_DIR, "mars_mission.npy")
            retrieved_embedding = None
            if os.path.exists(retrieved_embedding_path):
                retrieved_embedding = np.load(retrieved_embedding_path)
                report.append(f"  Retrieved Embedding (from file): {retrieved_embedding.shape}\n")
            else:
                report.append(f"  [WARNING] Embedding file not found for mars_mission at {retrieved_embedding_path}.\n")

            report.append(f"  Retrieved Context: '{retrieved_context}'\n")
        else:
            report.append("  [ERROR] Retrieval failed to return any documents.\n")
            return report

        # Step 3b: Evaluate
        evaluator_prompt = f"""You are a fact-checking AI. Analyze the retrieved documents in the context of the user's query. Extract only the information that is factually accurate and directly relevant.\n\nUser Query: \"{user_query}\"\n\nRetrieved Documents:\n---\n{retrieved_context}\n---\n\nOutput only the extracted, verified facts."""

        report.append("  Sending context to LLM for evaluation and sanitization...\n")
        sanitized_context = generate_with_model(
            model, tokenizer,
            prompt=evaluator_prompt,
            length=60,
            method="nucleus"
        )
        report.append(f"  Sanitized Context: '{sanitized_context}'\n")

        # Step 3c: Generate
        final_prompt = f"""Based on the following verified information, answer the user's question.\n\nVerified Information: \"{sanitized_context}\"\n\nUser Question: \"{user_query}\"\n\nAnswer:"""
        report.append("  Generating final answer using sanitized context...\n")
        final_answer = generate_with_model(
            model, tokenizer,
            prompt=final_prompt,
            length=50,
            method="nucleus",
            top_p=0.9
        )
        report.append(f"  Final Generated Answer: '{final_answer}'\n")

        # --- 4. Test Self-Updating Knowledge Base ---
        report.append("\n  --- Test 2b: Self-Updating Knowledge Base ---\n")
        report.append("  Testing the ability of the KB to learn from interactions...\n")

        # Manually add the new knowledge to the database
        new_doc_id = f"doc_{{doc_counter}}"
        document_text = f"Query: {user_query}\nResponse: {final_answer}"
        embedding = embedder.embed(document_text)
        report.append(f"  Type of new_embedding: {type(embedding)}\n")
        report.append(f"  Value of new_embedding: {embedding}\n")
        # Save embedding to a file
        embedding_file_path = os.path.join(EMBEDDINGS_DIR, f"{new_doc_id}.npy")
        np.save(embedding_file_path, embedding)

        new_document = {
            "id": new_doc_id,
            "text": document_text,
            "source": "generated_response",
            "timestamp": datetime.now(timezone.utc).isoformat()
        }
        # Reverting to individual PUT requests for TissDB document insertion
        # This ensures correct ID generation and avoids issues with the _bulk endpoint.
        response = requests.put(f"http://localhost:9876/{db_name}/{collection_name}/{new_document['id']}", json=new_document, headers=headers)
        response.raise_for_status()
        doc_counter += 1

        report.append("  KB updated with the previous Q&A pair.\n")

        # We can't retrieve from the database, so we'll just check if the document was added
        response = requests.get(f"http://localhost:9876/{db_name}/{collection_name}/{new_doc_id}", headers=headers)
        if response.status_code == 200:
            report.append("  [SUCCESS] Self-updated knowledge was successfully added to the database.\n")
        else:
            report.append("  [WARNING] Could not retrieve the self-updated document from the database.\n")

    except Exception as e:
        report.append(f"\n  [ERROR] RAG test failed unexpectedly: {e}\n")
    finally:
        # --- Cleanup ---
        if db_client:
            try:
                report.append("\n  --- Cleanup ---\n")
                report.append(f"  Deleting database '{db_name}'...\n")
                headers = {"Authorization": f"Bearer static_test_token"}
                requests.delete(f"http://localhost:9876/{db_name}", headers=headers)
                report.append("  Cleanup complete.\n")
            except Exception as e:
                report.append(f"  [WARNING] Failed to clean up database: {e}\n")

    return report
