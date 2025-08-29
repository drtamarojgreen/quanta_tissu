import os

class BufferManager:
    """
    Manages the text buffer and file I/O for the IDE.
    """
    def __init__(self, filepath=None):
        self.filepath = filepath
        self.buffer = [""]
        if filepath:
            self.load_file(filepath)

    def load_file(self, filepath):
        """
        Loads a file into the buffer.
        Returns True on success, False on failure.
        """
        if os.path.exists(filepath):
            with open(filepath, 'r', encoding='utf-8') as f:
                self.buffer = [line.rstrip('\n') for line in f.readlines()]
            self.filepath = filepath
            return True
        return False

    def save_file(self, filepath=None):
        """
        Saves the buffer to a file.
        If no filepath is provided, it uses the current filepath.
        Returns True on success, False on failure.
        """
        path_to_save = filepath if filepath else self.filepath
        if not path_to_save:
            return False

        try:
            with open(path_to_save, 'w', encoding='utf-8') as f:
                for line in self.buffer:
                    f.write(line + '\n')
            self.filepath = path_to_save
            return True
        except IOError:
            return False

    def new_file(self):
        """
        Resets the buffer for a new file.
        """
        self.filepath = None
        self.buffer = [""]
