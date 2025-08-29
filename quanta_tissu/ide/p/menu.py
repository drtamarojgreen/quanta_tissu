import os
import re
import glob
from .buffer import BufferManager
from .search import Search
from .editor import Editor

class CommandExecutor:
    """
    Handles parsing and execution of commands for the IDE.
    """
    def __init__(self, tui, buffer_manager, search, editor):
        self.tui = tui
        self.buffer_manager = buffer_manager
        self.search = search
        self.editor = editor
        self.macros = {}
        self.is_recording_macro = False
        self.recording_macro_name = None
        self.recorded_macro = []

    def execute_command(self, command_string, from_macro=False):
        """
        Parses and executes a command.
        """
        if self.is_recording_macro and command_string != 'macro_end' and not from_macro:
            self.recorded_macro.append(command_string)

        try:
            parts = command_string.split()
            if not parts:
                return
            command = parts[0]
            args = parts[1:]

            if command == 'open' and len(args) == 1:
                if self.buffer_manager.load_file(args[0]):
                    self.tui.status_bar_text = f"Opened {args[0]}"
                else:
                    self.tui.status_bar_text = f"File not found: {args[0]}"
            elif command == 'save':
                filepath = args[0] if args else None
                if self.buffer_manager.save_file(filepath):
                    self.tui.status_bar_text = f"Saved to {self.buffer_manager.filepath}"
                else:
                    self.tui.status_bar_text = "No filename. Use :save <filepath>"
            elif command == 'format':
                self.buffer_manager.buffer = self.editor.format_buffer(self.buffer_manager.buffer)
                self.tui.status_bar_text = "Buffer formatted."
            elif command == 'block' and len(args) == 5 and args[2] == 'replace':
                start_regex, end_regex, _, search_regex, replacement = args
                full_buffer = "\n".join(self.buffer_manager.buffer)
                new_buffer, err = self.search.search_and_replace_in_block(full_buffer, start_regex, end_regex, search_regex, replacement)
                if err:
                    self.tui.status_bar_text = err
                else:
                    self.buffer_manager.buffer = new_buffer.split("\n")
                    self.tui.status_bar_text = "Block replace executed."
            elif command == 'macro_def' and len(args) == 1:
                if self.is_recording_macro:
                    self.tui.status_bar_text = "Cannot define a macro while recording another."
                    return
                self.recording_macro_name = args[0]
                self.is_recording_macro = True
                self.recorded_macro = []
                self.tui.status_bar_text = f"Recording macro '{self.recording_macro_name}'. End with :macro_end."
            elif command == 'macro_end':
                if self.is_recording_macro:
                    self.macros[self.recording_macro_name] = self.recorded_macro
                    self.tui.status_bar_text = f"Macro '{self.recording_macro_name}' saved."
                    self.is_recording_macro = False
                    self.recording_macro_name = None
                else:
                    self.tui.status_bar_text = "Not recording a macro."
            elif command == 'macro_run' and len(args) == 1:
                self._run_macro(args[0])
            elif command == 'bulksr' and len(args) == 4:
                directory, file_pattern, search_regex, replacement = args
                status, _, _ = self.search.bulk_search_and_replace(directory, file_pattern, search_regex, replacement)
                self.tui.status_bar_text = status
            elif command == 'bulkmacro' and len(args) == 3:
                macro_name, directory, file_pattern = args
                if macro_name not in self.macros:
                    self.tui.status_bar_text = f"Macro '{macro_name}' not found."
                    return

                pathname = os.path.join(directory, file_pattern)
                files_to_process = glob.glob(pathname, recursive=True)
                files_processed_count = 0

                original_buffer = self.buffer_manager.buffer[:]
                original_filepath = self.buffer_manager.filepath

                for filepath in files_to_process:
                    if os.path.isfile(filepath):
                        try:
                            self.buffer_manager.load_file(filepath)
                            self.tui.cursor_x = 0
                            self.tui.cursor_y = 0
                            self.tui.mode = "edit"

                            self._run_macro(macro_name)

                            self.buffer_manager.save_file()
                            files_processed_count += 1
                        except Exception as e:
                            self.tui.status_bar_text = f"Error on {filepath}: {e}"

                self.buffer_manager.buffer = original_buffer
                self.buffer_manager.filepath = original_filepath
                self.tui.status_bar_text = f"Executed '{macro_name}' on {files_processed_count} files."

            elif command.startswith('s') and len(command) > 1:
                try:
                    delimiter = command[1]
                    parts = command.split(delimiter)
                    if len(parts) < 3:
                        raise ValueError("Invalid format. Use :s/search/replace/g")

                    _, search_regex, replacement, *flags_part = parts
                    flags = "".join(flags_part)

                    current_line = self.buffer_manager.buffer[self.tui.cursor_y]

                    if 'g' in flags:
                        new_line, count = self.search.search_and_replace(current_line, search_regex, replacement)
                    else:
                        new_line, count = self.search.search_and_replace(current_line, search_regex, replacement, count=1)

                    if count > 0:
                        self.buffer_manager.buffer[self.tui.cursor_y] = new_line
                        self.tui.status_bar_text = f"{count} substitution(s) on 1 line"
                    else:
                        self.tui.status_bar_text = f"Pattern not found: {search_regex}"
                except (ValueError, re.error) as e:
                    self.tui.status_bar_text = f"Invalid substitute command: {e}"
            elif command in ['q', 'quit']:
                self.tui.running = False
            else:
                self.tui.status_bar_text = f"Unknown command: {command_string}"
        except Exception as e:
            self.tui.status_bar_text = f"Error executing command: {e}"

    def _run_macro(self, macro_name):
        if macro_name not in self.macros:
            self.tui.status_bar_text = f"Macro '{macro_name}' not found."
            return

        macro_commands = self.macros[macro_name]
        for command_string in macro_commands:
            self.execute_command(command_string, from_macro=True)
        self.tui.status_bar_text = f"Macro '{macro_name}' executed."
