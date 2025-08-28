import sys
import os
import msvcrt
import shlex
from .highlighter import TissLangHighlighter
from .editor import Editor

class TissLangTUI:
    def __init__(self):
        self.buffer = [""]
        self.cursor_x = 0
        self.cursor_y = 0
        self.running = True
        self.highlighter = TissLangHighlighter()
        self.editor = Editor()
        self.status_bar_text = ""
        self.prompt_input = ""
        self.mode = "edit" # 'edit', 'prompt', or 'command'
        self.prompt_callback = None
        self.macros = {}
        self.macro_recording_name = None
        self.recorded_macro = []

    def get_char(self):
        return msvcrt.getch().decode('utf-8', errors='ignore')

    def clear_screen(self):
        os.system('cls' if os.name == 'nt' else 'clear')

    def draw(self):
        self.clear_screen()
        # Draw buffer
        for i, line in enumerate(self.buffer):
            highlighted_line = self.highlighter.highlight(line)
            sys.stdout.write(highlighted_line + '\n')

        # Draw status bar
        sys.stdout.write('\033[s') # save cursor
        try:
            height, width = os.get_terminal_size()
            sys.stdout.write(f'\033[{height};1H') # move to bottom left
            if self.mode == 'command':
                status = ":" + self.prompt_input
            else:
                status = self.status_bar_text + self.prompt_input
            sys.stdout.write(status.ljust(width))
        except OSError:
            # This can happen if not run in a real terminal
            pass
        sys.stdout.write('\033[u') # restore cursor

        # Move cursor to the correct position
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
        elif char == '\r': # Enter key
            self.buffer.insert(self.cursor_y + 1, self.buffer[self.cursor_y][self.cursor_x:])
            self.buffer[self.cursor_y] = self.buffer[self.cursor_y][:self.cursor_x]
            self.cursor_y += 1
            self.cursor_x = 0
        elif ord(char) == 8: # Backspace
            if self.cursor_x > 0:
                self.buffer[self.cursor_y] = self.buffer[self.cursor_y][:self.cursor_x - 1] + self.buffer[self.cursor_y][self.cursor_x:]
                self.cursor_x -= 1
            elif self.cursor_y > 0:
                self.cursor_x = len(self.buffer[self.cursor_y - 1])
                self.buffer[self.cursor_y - 1] += self.buffer.pop(self.cursor_y)
                self.cursor_y -= 1
        elif char in ('\x00', '\xe0'): # Special keys (like arrows)
            char = msvcrt.getch().decode('utf-8', errors='ignore')
            if char == 'H': self.cursor_y = max(0, self.cursor_y - 1)
            elif char == 'P': self.cursor_y = min(len(self.buffer) - 1, self.cursor_y + 1)
            elif char == 'K': self.cursor_x = max(0, self.cursor_x - 1)
            elif char == 'M': self.cursor_x = min(len(self.buffer[self.cursor_y]), self.cursor_x + 1)
            self.cursor_x = min(self.cursor_x, len(self.buffer[self.cursor_y]))
        else:
            self.buffer[self.cursor_y] = self.buffer[self.cursor_y][:self.cursor_x] + char + self.buffer[self.cursor_y][self.cursor_x:]
            self.cursor_x += 1

    def handle_prompt_input(self, char):
        if char == '\r': # Enter
            if self.prompt_callback:
                self.prompt_callback(self.prompt_input)
            self.mode = "edit"
            self.prompt_input = ""
            self.status_bar_text = ""
            self.prompt_callback = None
        elif ord(char) == 8: # Backspace
            self.prompt_input = self.prompt_input[:-1]
        elif ord(char) == 27: # Escape
            self.mode = "edit"
            self.prompt_input = ""
            self.status_bar_text = ""
        else:
            self.prompt_input += char

    def handle_command_input(self, char):
        if char == '\r': # Enter
            self.execute_command(self.prompt_input)
            self.mode = "edit"
            self.prompt_input = ""
        elif ord(char) == 8: # Backspace
            self.prompt_input = self.prompt_input[:-1]
        elif ord(char) == 27: # Escape
            self.mode = "edit"
            self.prompt_input = ""
        else:
            self.prompt_input += char

    def handle_input(self, char):
        if self.macro_recording_name and self.mode == 'edit':
            self.recorded_macro.append(char)

        if self.mode == 'edit':
            self.handle_edit_input(char)
        elif self.mode == 'prompt':
            self.handle_prompt_input(char)
        elif self.mode == 'command':
            self.handle_command_input(char)

    def execute_command(self, command_string):
        try:
            parts = shlex.split(command_string)
            if not parts:
                return
            command = parts[0]
            args = parts[1:]

            if command == 'block' and len(args) == 5 and args[2] == 'replace':
                start_regex, end_regex, _, search_regex, replacement = args
                full_buffer = "\n".join(self.buffer)
                new_buffer, err = self.editor.search_and_replace_in_block(full_buffer, start_regex, end_regex, search_regex, replacement)
                if err:
                    self.status_bar_text = err
                else:
                    self.buffer = new_buffer.split("\n")
                    self.status_bar_text = f"Block replace executed."
            elif command == 'macro_def' and len(args) == 1:
                self.macro_recording_name = args[0]
                self.recorded_macro = []
                self.status_bar_text = f"Recording macro '{self.macro_recording_name}'... Press :macro_end to stop."
            elif command == 'macro_end' and self.macro_recording_name:
                self.macros[self.macro_recording_name] = self.recorded_macro
                self.status_bar_text = f"Macro '{self.macro_recording_name}' saved."
                self.macro_recording_name = None
                self.recorded_macro = []
            elif command == 'macro_run' and len(args) == 1:
                macro_name = args[0]
                if macro_name in self.macros:
                    for recorded_char in self.macros[macro_name]:
                        self.handle_input(recorded_char)
                    self.status_bar_text = f"Macro '{macro_name}' executed."
                else:
                    self.status_bar_text = f"Macro '{macro_name}' not found."
            else:
                self.status_bar_text = f"Unknown command: {command_string}"
        except Exception as e:
            self.status_bar_text = f"Error executing command: {e}"


    def start_search_and_replace(self):
        self.mode = 'prompt'
        self.status_bar_text = "Search: "
        self.prompt_callback = self.on_search_input

    def on_search_input(self, search_term):
        self.mode = 'prompt'
        self.status_bar_text = f"Replace '{search_term}' with: "
        self.prompt_callback = lambda replace_term: self.do_replace(search_term, replace_term)

    def do_replace(self, search_term, replace_term):
        full_buffer = "\n".join(self.buffer)
        new_buffer = self.editor.search_and_replace(full_buffer, search_term, replace_term)
        self.buffer = new_buffer.split("\n")
        self.status_bar_text = f"Replaced all occurrences of '{search_term}' with '{replace_term}'"


    def run(self):
        self.clear_screen()
        while self.running:
            self.draw()
            char = self.get_char()
            self.handle_input(char)
        self.clear_screen()

def main():
    tui = TissLangTUI()
    tui.run()

if __name__ == "__main__":
    main()

