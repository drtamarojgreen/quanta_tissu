import time
import os

class ViewerShell:
    """Mock Image Viewer SUT for SDD verification."""
    def __init__(self):
        self.is_running = False
        self.active_streams = 0
        self.current_image = None
        self.image_directory = None
        self.zoom_level = 1.0
        self.rotation = 0
        self.is_fullscreen = False
        self.theme = "light"
        self.memory_usage = 100 # Mock MB
        self.thread_pool_active = False
        self.gpu_accelerated = True
        self.config = {"interval": 5, "slideshow": False}
        self.history = []
        self.clipboard = None
        self.cache_files = []

    def launch(self, streams=1, source=None):
        self.is_running = True
        self.active_streams = streams
        self.image_directory = source
        self.history.append(f"Launched with {streams} streams")
        return True

    def shutdown(self):
        self.is_running = False
        self.active_streams = 0
        self.thread_pool_active = False
        self.history.append("Shutdown")
        return True

    def load_image(self, path):
        if not os.path.exists(path) and not path.startswith("mock://") and not ":" in path:
             raise FileNotFoundError(f"Image {path} not found")
        if path.endswith(".exe"):
             self.history.append(f"Rejected unsupported extension: {path}")
             return False
        if "corrupt" in path:
             self.history.append(f"Rejected corrupted file: {path}")
             return False
        self.current_image = path
        self.history.append(f"Loaded image: {path}")
        return True

    def toggle_fullscreen(self):
        self.is_fullscreen = not self.is_fullscreen
        return self.is_fullscreen

    def set_zoom(self, level):
        if level < 0:
            return False
        self.zoom_level = level
        return True

    def apply_effect(self, effect):
        self.history.append(f"Applied effect: {effect}")
        return True

    def get_stats(self):
        return {
            "fps": 60,
            "memory": self.memory_usage,
            "threads": 4 if self.thread_pool_active else 1,
            "gpu": self.gpu_accelerated
        }

    def simulate_stress(self, count):
        self.history.append(f"Simulating stress with {count} images")
        return True

    def trigger_action(self, action, payload=None):
        self.history.append(f"Action triggered: {action} with {payload}")
        return True
