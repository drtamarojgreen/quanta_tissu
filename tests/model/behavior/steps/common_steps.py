import os
import shutil

# Helper function for cleaning up test directories
def cleanup_test_dir(test_dir):
    if os.path.exists(test_dir):
        # Manually remove directory and its contents
        for root, dirs, files in os.walk(test_dir, topdown=False):
            for name in files:
                os.remove(os.path.join(root, name))
            for name in dirs:
                os.rmdir(os.path.join(root, name))
        os.rmdir(test_dir)
