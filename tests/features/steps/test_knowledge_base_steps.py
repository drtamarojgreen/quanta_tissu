import numpy as np
from quanta_tissu.tisslm.model import QuantaTissu
from quanta_tissu.tisslm.config import model_config
from quanta_tissu.tisslm.tokenizer import Tokenizer
from quanta_tissu.tisslm.knowledge_base import KnowledgeBase

def register_steps(runner):
    @runner.step(r'^(?:Given|And) a knowledge base with a model and tokenizer$')
    def knowledge_base_context(context):
        np.random.seed(42)
        tokenizer = Tokenizer()
        model_config['vocab_size'] = tokenizer.get_vocab_size()
        model = QuantaTissu(model_config)
        context['model'] = model # Store model for other steps
        context['tokenizer'] = tokenizer # Store tokenizer for other steps
        context['knowledge_base'] = KnowledgeBase(model.embeddings, tokenizer)

    @runner.step(r'^(?:When|And) I add the document "(.*)"$')
    def add_document(context, document_text):
        context['knowledge_base'].add_document(document_text)

    @runner.step(r'^(?:When|And) I retrieve documents for the query "(.*)"$')
    def retrieve_documents(context, query_text):
        context['retrieved_docs'] = context['knowledge_base'].retrieve(query_text)

    @runner.step(r'^Then the retrieved documents should contain "(.*)"$')
    def check_retrieved_documents(context, expected_document):
        assert expected_document in context['retrieved_docs']
        return "Test passed!"

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
        return "Test passed!"

    @runner.step(r'^(?:When|And) I self-update from interaction with query "(.*)" generated response "(.*)" and user correction "(.*)"$')
    def self_update_with_correction(context, query, generated_response, user_correction):
        context['knowledge_base'].self_update_from_interaction(query, generated_response, user_correction)

    @runner.step(r'^Then the knowledge base should contain "(.*)"$')
    def check_knowledge_base_content(context, expected_content):
        # We retrieve the document to check if it's in the KB
        query = expected_content.split("Correct Answer:")[0].replace("Query:", "").strip()
        retrieved = context['knowledge_base'].retrieve(query, k=5)
        assert any(expected_content in doc for doc in retrieved)

    @runner.step(r'^(?:When|And) I self-update from interaction with query "(.*)" generated response "(.*)" and no user correction$')
    def self_update_no_correction(context, query, generated_response):
        context['knowledge_base'].self_update_from_interaction(query, generated_response)

    @runner.step(r'^Then the knowledge base stats should show (\d+) total documents$')
    def check_total_documents(context, expected_count):
        stats = context['knowledge_base'].get_knowledge_stats()
        assert stats.get('total_docs', 0) == int(expected_count)
        return "Test passed!"

