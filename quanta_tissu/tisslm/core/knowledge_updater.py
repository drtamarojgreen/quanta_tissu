import threading
import time
import requests
import logging
from ..knowledge_base import KnowledgeBase

logger = logging.getLogger(__name__)

class RealTimeKnowledgeUpdater:
    def __init__(self, knowledge_base: KnowledgeBase, api_url: str, interval: int = 60):
        self.knowledge_base = knowledge_base
        self.api_url = api_url
        self.interval = interval
        self.is_running = False
        self.thread = None

    def _fetch_and_update(self):
        while self.is_running:
            try:
                response = requests.get(self.api_url)
                if response.status_code == 200:
                    new_documents = response.json()
                    for doc in new_documents:
                        self.knowledge_base.add_document(doc['text'], doc.get('metadata'))
                    logger.info(f"Updated knowledge base with {len(new_documents)} new documents.")
                else:
                    logger.warning(f"Failed to fetch new documents from {self.api_url}. Status code: {response.status_code}")
            except Exception as e:
                logger.error(f"Error fetching or updating knowledge base: {e}", exc_info=True)
            
            time.sleep(self.interval)

    def start(self):
        if not self.is_running:
            self.is_running = True
            self.thread = threading.Thread(target=self._fetch_and_update)
            self.thread.daemon = True
            self.thread.start()
            logger.info("Real-time knowledge updater started.")

    def stop(self):
        if self.is_running:
            self.is_running = False
            if self.thread:
                self.thread.join()
            logger.info("Real-time knowledge updater stopped.")
