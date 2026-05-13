import curses
import sys
import os
import requests
import json
import logging

# API Configuration
API_BASE = os.environ.get("API_BASE", "http://localhost:8000/api")

class QuantaPrisma:
    def __init__(self, stdscr):
        self.stdscr = stdscr
        self.running = True
        self.menu_items = ["DB", "Model", "TissLang", "Tasks", "Training", "Quit"]
        self.current_menu_idx = 0
        self.side_panel_width = 25
        self.logs = []
        self.max_logs = 10

        # Command state
        self.module_states = {
            "DB": {"sub": "status", "collection": "tiss_generations", "query": '{"query": "SELECT * FROM tiss_generations"}'},
            "Model": {"prompt": "Hello", "length": 50, "temp": 0.7, "rag": False},
            "TissLang": {"script": "RUN 'ls'"},
            "Tasks": {"sub": "list", "id": ""},
            "Training": {"sub": "status"}
        }

        self._init_colors()
        self.stdscr.nodelay(False)
        self.stdscr.keypad(True)
        curses.curs_set(0)

    def _init_colors(self):
        curses.start_color()
        curses.use_default_colors()
        # Greenhouse theme colors
        curses.init_pair(1, curses.COLOR_GREEN, -1)     # Accent / Header
        curses.init_pair(2, curses.COLOR_BLACK, curses.COLOR_GREEN) # Selection
        curses.init_pair(3, curses.COLOR_WHITE, -1)     # Default text
        curses.init_pair(4, curses.COLOR_CYAN, -1)      # Info
        curses.init_pair(5, curses.COLOR_RED, -1)       # Error

    def add_log(self, message, is_error=False):
        # Format message if it's a dict
        if isinstance(message, dict):
            message = json.dumps(message)

        lines = str(message).split('\n')
        for line in lines:
            self.logs.append((line, is_error))

        while len(self.logs) > self.max_logs:
            self.logs.pop(0)

    def draw_side_panel(self):
        height, width = self.stdscr.getmaxyx()

        # Header
        header = " QUANTA PRISMA "
        self.stdscr.addstr(1, 2, header.center(self.side_panel_width - 4), curses.color_pair(1) | curses.A_BOLD | curses.A_REVERSE)

        for idx, item in enumerate(self.menu_items):
            x = 2
            y = 4 + idx
            if idx == self.current_menu_idx:
                self.stdscr.addstr(y, x, f" > {item} ".ljust(self.side_panel_width - 4), curses.color_pair(2))
            else:
                self.stdscr.addstr(y, x, f"   {item} ", curses.color_pair(3))

        # Draw vertical separator
        for y in range(height):
            self.stdscr.addch(y, self.side_panel_width, curses.ACS_VLINE)

    def get_input(self, prompt):
        height, width = self.stdscr.getmaxyx()
        self.stdscr.addstr(height - 1, 0, f"{prompt}: ".ljust(width), curses.color_pair(4))
        curses.echo()
        curses.curs_set(1)
        input_str = self.stdscr.getstr(height - 1, len(prompt) + 2).decode('utf-8')
        curses.noecho()
        curses.curs_set(0)
        return input_str

    def draw_main_content(self):
        height, width = self.stdscr.getmaxyx()
        current_selection = self.menu_items[self.current_menu_idx]
        if current_selection == "Quit": return

        state = self.module_states[current_selection]

        # Title
        title = f" {current_selection} MODULE "
        self.stdscr.addstr(1, self.side_panel_width + 2, title, curses.color_pair(1) | curses.A_BOLD)

        # Configuration Display
        y = 3
        x = self.side_panel_width + 2
        self.stdscr.addstr(y, x, "Current Configuration:", curses.A_UNDERLINE)
        y += 1
        for key, value in state.items():
            self.stdscr.addstr(y, x, f"{key}: {value}")
            y += 1

        y += 1
        self.stdscr.addstr(y, x, "Controls: [E]xecute  [C]onfigure", curses.color_pair(1))

        # Logs area at the bottom
        log_start_y = height - self.max_logs - 2
        self.stdscr.addstr(log_start_y - 1, self.side_panel_width + 2, "--- OUTPUT LOGS ---", curses.color_pair(4))
        for i, (log, is_error) in enumerate(self.logs):
            color = curses.color_pair(5) if is_error else curses.color_pair(3)
            self.stdscr.addstr(log_start_y + i, self.side_panel_width + 2, str(log)[:width - self.side_panel_width - 4], color)

    def configure_module(self):
        module = self.menu_items[self.current_menu_idx]
        if module == "Quit": return

        state = self.module_states[module]
        for key in state.keys():
            new_val = self.get_input(f"Enter {key} (current: {state[key]})")
            if new_val:
                if isinstance(state[key], int):
                    try:
                        state[key] = int(new_val)
                    except ValueError:
                        self.add_log(f"Invalid integer for {key}: {new_val}", is_error=True)
                elif isinstance(state[key], float):
                    try:
                        state[key] = float(new_val)
                    except ValueError:
                        self.add_log(f"Invalid float for {key}: {new_val}", is_error=True)
                elif isinstance(state[key], bool):
                    state[key] = new_val.lower() in ("true", "yes", "1", "t")
                else:
                    state[key] = new_val

    def handle_input(self):
        key = self.stdscr.getch()
        if key == curses.KEY_UP:
            self.current_menu_idx = (self.current_menu_idx - 1) % len(self.menu_items)
        elif key == curses.KEY_DOWN:
            self.current_menu_idx = (self.current_menu_idx + 1) % len(self.menu_items)
        elif key in (ord('e'), ord('E'), curses.KEY_ENTER, 10, 13):
            if self.menu_items[self.current_menu_idx] == "Quit":
                self.running = False
            else:
                self.execute_current_module()
        elif key in (ord('c'), ord('C')):
            self.configure_module()
        elif key == ord('q'):
            self.running = False

    def execute_current_module(self):
        module = self.menu_items[self.current_menu_idx]
        state = self.module_states[module]
        self.add_log(f"Executing {module}...")

        try:
            if module == "DB":
                if state["sub"] == "status":
                    resp = requests.get(f"{API_BASE}/db/databases")
                else:
                    payload = {"collection": state["collection"], "query": state["query"]}
                    resp = requests.post(f"{API_BASE}/db/query", json=payload)
            elif module == "Model":
                payload = {"prompt": state["prompt"], "length": state["length"], "temperature": state["temp"], "use_rag": state["rag"]}
                resp = requests.post(f"{API_BASE}/model/generate", json=payload)
            elif module == "TissLang":
                resp = requests.post(f"{API_BASE}/tisslang/run", json={"script": state["script"]})
            elif module == "Tasks":
                if state["sub"] == "list":
                    resp = requests.get(f"{API_BASE}/tasks")
                else:
                    resp = requests.get(f"{API_BASE}/tasks/{state['id']}")
            elif module == "Training":
                if state["sub"] == "start":
                    resp = requests.post(f"{API_BASE}/training/start", json={})
                else:
                    resp = requests.get(f"{API_BASE}/training/status")
            else:
                return

            if resp.status_code >= 400:
                self.add_log(f"Error {resp.status_code}: {resp.text}", is_error=True)
            else:
                self.add_log(resp.json())
        except Exception as e:
            self.add_log(f"Request failed: {e}", is_error=True)

    def main_loop(self):
        while self.running:
            try:
                self.stdscr.clear()
                self.draw_side_panel()
                self.draw_main_content()
                self.stdscr.refresh()
                self.handle_input()
            except Exception as e:
                self.add_log(f"Critical Error: {e}", is_error=True)

def main():
    curses.wrapper(lambda stdscr: QuantaPrisma(stdscr).main_loop())

if __name__ == "__main__":
    main()
