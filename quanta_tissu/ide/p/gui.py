import sys
import os
import curses
import json
from .highlighter import TissLangHighlighter, COLORS
from .buffer import BufferManager
from .search import Search
from .menu import CommandExecutor
from .editor import Editor
from .linter import TissLangLinter

class TissLangTUI:
    SESSION_FILE = os.path.expanduser("~/.tiss_ide_session.json")

    def __init__(self, stdscr, filepath=None):
        self.stdscr = stdscr

        self.cursor_x = 0
        self.cursor_y = 0

        # Load session or use filepath argument
        initial_filepath = filepath
        if not initial_filepath:
            self._load_session()
            initial_filepath = self.buffer_manager.filepath if hasattr(self, 'buffer_manager') else None

        self.buffer_manager = BufferManager(initial_filepath)
        self.search = Search()
        self.editor = Editor()
        self.command_executor = CommandExecutor(self, self.buffer_manager, self.search, self.editor)
        self.linter = TissLangLinter()
        self.linter_errors = {}

        self.running = True
        self.highlighter = TissLangHighlighter()
        self.status_bar_text = ""
        self.prompt_input = ""
        self.mode = "edit"
        self.prompt_callback = None
        self.clipboard = []
        self._init_curses()
        self.run_linter()

    def _save_session(self):
        session_data = {
            'filepath': self.buffer_manager.filepath,
            'cursor_x': self.cursor_x,
            'cursor_y': self.cursor_y,
        }
        with open(self.SESSION_FILE, 'w') as f:
            json.dump(session_data, f)

    def _load_session(self):
        if os.path.exists(self.SESSION_FILE):
            with open(self.SESSION_FILE, 'r') as f:
                try:
                    session_data = json.load(f)
                    self.buffer_manager = BufferManager(session_data.get('filepath'))
                    self.cursor_x = session_data.get('cursor_x', 0)
                    self.cursor_y = session_data.get('cursor_y', 0)
                except (json.JSONDecodeError, KeyError):
                    # Handle corrupted or invalid session file
                    self.buffer_manager = BufferManager()


    def _init_curses(self):
        curses.curs_set(1)
        self.stdscr.nodelay(False)
        curses.start_color()
        curses.use_default_colors()

        color_map = {
            "keyword": curses.COLOR_YELLOW,
            "string": curses.COLOR_GREEN,
            "comment": curses.COLOR_WHITE, # Using white for grey
            "directive": curses.COLOR_MAGENTA,
            "heredoc": curses.COLOR_GREEN,
            "error": curses.COLOR_RED,
            "special_var": curses.COLOR_CYAN,
            "operator": curses.COLOR_RED,
            "default": -1, # default terminal color
            "line_number": curses.COLOR_WHITE,
        }

        for name, (code, _) in COLORS.items():
            if name == "indent":
                curses.init_pair(code, curses.COLOR_BLACK, curses.COLOR_WHITE)
            elif name in color_map:
                curses.init_pair(code, color_map[name], -1) # -1 for default background

    def run_linter(self):
        self.linter_errors = self.linter.lint(self.buffer_manager.buffer)

    def draw(self):
        self.stdscr.clear()
        height, width = self.stdscr.getmaxyx()

        screen_y = 0
        for i, line in enumerate(self.buffer_manager.buffer):
            if screen_y >= height - 1:
                break

            # Draw the line content
            highlighted_tokens = self.highlighter.highlight(line)
            x = 0
            for token_type, token_string in highlighted_tokens:
                color_code = self.highlighter.get_color_code(token_type)
                color_pair = curses.color_pair(color_code)
                self.stdscr.addstr(screen_y, x, token_string, color_pair)
                x += len(token_string)

            screen_y += 1

            # Draw linter errors for this line
            line_num = i + 1
            if line_num in self.linter_errors:
                if screen_y >= height -1:
                    break
                error_color = curses.color_pair(COLORS['error'][0])
                for error in self.linter_errors[line_num]:
                    self.stdscr.addstr(screen_y, 0, f"└─> {error}", error_color)
                    screen_y += 1


        # Draw status bar
        if self.mode == 'command':
            status = ":" + self.prompt_input
        else:
            status = self.status_bar_text + self.prompt_input
        self.stdscr.addstr(height - 1, 0, status.ljust(width), curses.A_REVERSE)

        # Move cursor
        # This needs to be adjusted for linter messages
        screen_cursor_y = self.cursor_y
        for i in range(self.cursor_y):
            line_num = i + 1
            if line_num in self.linter_errors:
                screen_cursor_y += len(self.linter_errors[line_num])

        self.cursor_x = min(self.cursor_x, len(self.buffer_manager.buffer[self.cursor_y]))
        self.stdscr.move(screen_cursor_y, self.cursor_x)
        self.stdscr.refresh()

    def handle_input(self):
        key = self.stdscr.getch()

        if self.mode == 'edit':
            self.handle_edit_input(key)
        elif self.mode == 'prompt':
            self.handle_prompt_input(key)
        elif self.mode == 'command':
            self.handle_command_input(key)

    def handle_edit_input(self, key):
        if key == 17: # Ctrl+Q
            self.running = False
        elif key == 3: # Ctrl+C
            if self.buffer_manager.buffer:
                self.clipboard = [self.buffer_manager.buffer[self.cursor_y]]
                self.status_bar_text = "Line copied."
        elif key == 24: # Ctrl+X
            if self.buffer_manager.buffer:
                self.clipboard = [self.buffer_manager.buffer.pop(self.cursor_y)]
                self.status_bar_text = "Line cut."
        elif key == 22: # Ctrl+V
            if self.clipboard:
                for line in reversed(self.clipboard):
                    self.buffer_manager.buffer.insert(self.cursor_y, line)
                self.status_bar_text = "Pasted."
        elif key == 6: # Ctrl+F
            self.start_search_and_replace()
        elif key == ord(':'):
            self.mode = 'command'
            self.prompt_input = ""
        elif key in (curses.KEY_ENTER, 10, 13):
            line = self.buffer_manager.buffer[self.cursor_y]
            self.buffer_manager.buffer.insert(self.cursor_y + 1, line[self.cursor_x:])
            self.buffer_manager.buffer[self.cursor_y] = line[:self.cursor_x]
            self.cursor_y += 1
            self.cursor_x = 0
        elif key in (curses.KEY_BACKSPACE, 8, 127):
            if self.cursor_x > 0:
                line = self.buffer_manager.buffer[self.cursor_y]
                self.buffer_manager.buffer[self.cursor_y] = line[:self.cursor_x - 1] + line[self.cursor_x:]
                self.cursor_x -= 1
            elif self.cursor_y > 0:
                self.cursor_x = len(self.buffer_manager.buffer[self.cursor_y - 1])
                self.buffer_manager.buffer[self.cursor_y - 1] += self.buffer_manager.buffer.pop(self.cursor_y)
                self.cursor_y -= 1
        elif key == curses.KEY_UP:
            self.cursor_y = max(0, self.cursor_y - 1)
        elif key == curses.KEY_DOWN:
            self.cursor_y = min(len(self.buffer_manager.buffer) - 1, self.cursor_y + 1)
        elif key == curses.KEY_LEFT:
            self.cursor_x = max(0, self.cursor_x - 1)
        elif key == curses.KEY_RIGHT:
            self.cursor_x = min(len(self.buffer_manager.buffer[self.cursor_y]), self.cursor_x + 1)
        elif 32 <= key <= 126:
            char = chr(key)
            line = self.buffer_manager.buffer[self.cursor_y]
            self.buffer_manager.buffer[self.cursor_y] = line[:self.cursor_x] + char + line[self.cursor_x:]
            self.cursor_x += 1

    def handle_prompt_input(self, key):
        if key in (curses.KEY_ENTER, 10, 13):
            if self.prompt_callback:
                self.prompt_callback(self.prompt_input)
            self.mode = "edit"
            self.prompt_input = ""
            self.status_bar_text = ""
            self.prompt_callback = None
        elif key in (curses.KEY_BACKSPACE, 8, 127):
            self.prompt_input = self.prompt_input[:-1]
        elif key == 27: # ESC
            self.mode = "edit"
            self.prompt_input = ""
            self.status_bar_text = ""
        elif 32 <= key <= 126:
            self.prompt_input += chr(key)

    def handle_command_input(self, key):
        if key in (curses.KEY_ENTER, 10, 13):
            self.command_executor.execute_command(self.prompt_input)
            self.mode = "edit"
            self.prompt_input = ""
        elif key in (curses.KEY_BACKSPACE, 8, 127):
            self.prompt_input = self.prompt_input[:-1]
        elif key == 27: # ESC
            self.mode = "edit"
            self.prompt_input = ""
        elif 32 <= key <= 126:
            self.prompt_input += chr(key)

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
        new_buffer, _ = self.search.search_and_replace(full_buffer, search_term, replace_term)
        self.buffer_manager.buffer = new_buffer.split("\n")
        self.status_bar_text = f"Replaced all occurrences of '{search_term}'"

    def main_loop(self):
        while self.running:
            self.draw()
            self.handle_input()

def main_wrapper(stdscr):
    filepath = sys.argv[1] if len(sys.argv) > 1 else None
    tui = TissLangTUI(stdscr, filepath)
    try:
        tui.main_loop()
    finally:
        tui._save_session()

def main():
    curses.wrapper(main_wrapper)

if __name__ == "__main__":
    main()
