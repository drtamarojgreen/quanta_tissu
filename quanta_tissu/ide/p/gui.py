import sys
import os
import msvcrt
from .highlighter import TissLangHighlighter
from .buffer import BufferManager
from .search import Search
from .menu import CommandExecutor
from .editor import Editor

class TissLangTUI:
    def __init__(self, filepath=None):
        self.buffer_manager = BufferManager(filepath)
        self.search = Search()
        self.editor = Editor()
        # Pass self to CommandExecutor so it can modify TUI state
        self.command_executor = CommandExecutor(self, self.buffer_manager, self.search, self.editor)

        self.cursor_x = 0
        self.cursor_y = 0
        self.running = True
        self.highlighter = TissLangHighlighter()
        self.status_bar_text = ""
        self.prompt_input = ""
        self.mode = "edit"
        self.prompt_callback = None

    def get_char(self):
        return msvcrt.getch().decode('utf-8', errors='ignore')

    def clear_screen(self):
        os.system('cls' if os.name == 'nt' else 'clear')

    def draw(self):
        self.clear_screen()
        for i, line in enumerate(self.buffer_manager.buffer):
            highlighted_line = self.highlighter.highlight(line)
            sys.stdout.write(highlighted_line + '\n')

        sys.stdout.write('\033[s')
        try:
            height, width = os.get_terminal_size()
            sys.stdout.write(f'\033[{height};1H')
            if self.mode == 'command':
                status = ":" + self.prompt_input
            else:
                status = self.status_bar_text + self.prompt_input
            sys.stdout.write(status.ljust(width))
        except OSError:
            pass
        sys.stdout.write('\033[u')

        # Use buffer_manager.buffer to get line length
        self.cursor_x = min(self.cursor_x, len(self.buffer_manager.buffer[self.cursor_y]))
        sys.stdout.write(f'\033[{self.cursor_y + 1};{self.cursor_x + 1}H')
        sys.stdout.flush()

    def handle_edit_input(self, char):
        if ord(char) == 3: # Ctrl+C
            self.running = False
        elif ord(char) == 6: # Ctrl+F
            self.start_search_and_replace()
        elif char == ':':
            self.mode = 'command'
            self.prompt_input = ""
        elif char == '\r':
            line = self.buffer_manager.buffer[self.cursor_y]
            self.buffer_manager.buffer.insert(self.cursor_y + 1, line[self.cursor_x:])
            self.buffer_manager.buffer[self.cursor_y] = line[:self.cursor_x]
            self.cursor_y += 1
            self.cursor_x = 0
        elif ord(char) == 8: # Backspace
            if self.cursor_x > 0:
                line = self.buffer_manager.buffer[self.cursor_y]
                self.buffer_manager.buffer[self.cursor_y] = line[:self.cursor_x - 1] + line[self.cursor_x:]
                self.cursor_x -= 1
            elif self.cursor_y > 0:
                self.cursor_x = len(self.buffer_manager.buffer[self.cursor_y - 1])
                self.buffer_manager.buffer[self.cursor_y - 1] += self.buffer_manager.buffer.pop(self.cursor_y)
                self.cursor_y -= 1
        elif char in ('\x00', '\xe0'):
            char = msvcrt.getch().decode('utf-8', errors='ignore')
            if char == 'H': self.cursor_y = max(0, self.cursor_y - 1)
            elif char == 'P': self.cursor_y = min(len(self.buffer_manager.buffer) - 1, self.cursor_y + 1)
            elif char == 'K': self.cursor_x = max(0, self.cursor_x - 1)
            elif char == 'M': self.cursor_x = min(len(self.buffer_manager.buffer[self.cursor_y]), self.cursor_x + 1)
        else:
            line = self.buffer_manager.buffer[self.cursor_y]
            self.buffer_manager.buffer[self.cursor_y] = line[:self.cursor_x] + char + line[self.cursor_x:]
            self.cursor_x += 1

    def handle_prompt_input(self, char):
        if char == '\r':
            if self.prompt_callback:
                self.prompt_callback(self.prompt_input)
            self.mode = "edit"
            self.prompt_input = ""
            self.status_bar_text = ""
            self.prompt_callback = None
        elif ord(char) == 8:
            self.prompt_input = self.prompt_input[:-1]
        elif ord(char) == 27:
            self.mode = "edit"
            self.prompt_input = ""
            self.status_bar_text = ""
        else:
            self.prompt_input += char

    def handle_command_input(self, char):
        if char == '\r':
            self.command_executor.execute_command(self.prompt_input)
            self.mode = "edit"
            self.prompt_input = ""
        elif ord(char) == 8:
            self.prompt_input = self.prompt_input[:-1]
        elif ord(char) == 27:
            self.mode = "edit"
            self.prompt_input = ""
        else:
            self.prompt_input += char

    def handle_input(self, char):
        if self.command_executor.is_recording_macro and self.mode == 'edit':
             # This logic will be moved to CommandExecutor
            pass

        if self.mode == 'edit':
            self.handle_edit_input(char)
        elif self.mode == 'prompt':
            self.handle_prompt_input(char)
        elif self.mode == 'command':
            self.handle_command_input(char)

    def start_search_and_replace(self):
        self.mode = 'prompt'
        self.status_bar_text = "Search: "
        self.prompt_callback = self.on_search_input

    def on_search_input(self, search_term):
        self.mode = 'prompt'
        self.status_bar_text = f"Replace '{search_term}' with: "
        self.prompt_callback = lambda replace_term: self.do_replace(search_term, replace_term)

    def do_replace(self, search_term, replace_term):
        full_buffer = "\n".join(self.buffer_manager.buffer)
        new_buffer = self.search.search_and_replace(full_buffer, search_term, replace_term)
        self.buffer_manager.buffer = new_buffer.split("\n")
        self.status_bar_text = f"Replaced all occurrences of '{search_term}'"

    def run(self):
        self.clear_screen()
        while self.running:
            self.draw()
            char = self.get_char()
            self.handle_input(char)
        self.clear_screen()

def main():
    filepath = sys.argv[1] if len(sys.argv) > 1 else None
    tui = TissLangTUI(filepath)
    tui.run()

if __name__ == "__main__":
    main()
