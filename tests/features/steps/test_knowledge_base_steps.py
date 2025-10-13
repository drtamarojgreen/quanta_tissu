import os
import numpy as np
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.config import model_config
from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.knowledge_base import KnowledgeBase
from quanta_tissu.tisslm.core.embedding.embedder import Embedder
from quanta_tissu.tisslm.core.db.client import TissDBClient

def register_steps(runner):
    @runner.step(r'^(?:Given|And) a knowledge base with a model and tokenizer$')
    def knowledge_base_context(context):
        np.random.seed(42)
        script_dir = os.path.dirname(os.path.abspath(__file__))
        project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
        tokenizer_prefix = os.path.join(project_root, "test_tokenizer", "test_tokenizer")
        tokenizer = Tokenizer(tokenizer_prefix=tokenizer_prefix)
        model_config['vocab_size'] = 8000
        model = QuantaTissu(model_config)

        # Create the correct dependencies for the KnowledgeBase
        embedder = Embedder(tokenizer, model.embeddings.value)
        # The BDD tests run the server on port 9876.
        db_client = TissDBClient(db_host='127.0.0.1', db_port=9876, token='static_test_token')

        context['model'] = model
        context['tokenizer'] = tokenizer
        context['knowledge_base'] = KnowledgeBase(embedder, db_client)

    @runner.step(r'^(?:When|And) I add the document "(.*)"$')
    def add_document(context, document_text):
        context['knowledge_base'].add_document(document_text)

    @runner.step(r'^(?:When|And) I retrieve documents for the query "(.*)"$')
    def retrieve_documents(context, query_text):
        context['last_query'] = query_text # Save for feedback step
        retrieved_docs, _ = context['knowledge_base'].retrieve(query_text)
        context['retrieved_docs'] = retrieved_docs

    @runner.step(r'^Then the retrieved documents should contain "(.*)"$')
    def check_retrieved_documents(context, expected_document):
        assert expected_document in context['retrieved_docs'], f"Expected to retrieve '{expected_document}', but got {context['retrieved_docs']}"

    @runner.step(r'^(?:When|And) I add feedback with score (\d+) and text "(.*)" for the retrieved documents$')
    def add_feedback(context, score, feedback_text):
        context['knowledge_base'].add_feedback(
            query=context.get('last_query', 'test'),
            retrieved_docs=context['retrieved_docs'],
            feedback_score=int(score),
            feedback_text=feedback_text
        )

    @runner.step(r'^Then the knowledge base stats should show (\d+) feedback entry$')
    def check_feedback_entry(context, expected_count):
        stats = context['knowledge_base'].get_knowledge_stats()
        assert stats.get('feedback_entries', 0) == int(expected_count)

    @runner.step(r'^(?:When|And) I self-update from interaction with query "(.*)" generated response "(.*)" and user correction "(.*)"$')
    def self_update_with_correction(context, query, generated_response, user_correction):
        context['knowledge_base'].self_update_from_interaction(query, generated_response, user_correction)

    @runner.step(r'^Then the knowledge base should contain "(.*)"$')
    def check_knowledge_base_content(context, expected_content):
        retrieved_docs, _ = context['knowledge_base'].retrieve(expected_content)
        assert any(expected_content in doc for doc in retrieved_docs), f"Did not find '{expected_content}' in knowledge base after self-update."

    @runner.step(r'^(?:When|And) I self-update from interaction with query "(.*)" generated response "(.*)" and no user correction$')
    def self_update_no_correction(context, query, generated_response):
        context['knowledge_base'].self_update_from_interaction(query, generated_response)

    @runner.step(r'^Then the knowledge base stats should show (\d+) total documents$')
    def check_total_documents(context, expected_count):
        stats = context['knowledge_base'].get_knowledge_stats()
        assert stats.get('total_docs', 0) == int(expected_count)
