import tkinter as tk
from tkinter import filedialog, messagebox, simpledialog
import re

class TissLangIDE:
    def __init__(self, root):
        self.root = root
        self.root.title("TissLang IDE")
        self.root.geometry("800x600")

        self.text_widget = tk.Text(self.root, wrap="word", undo=True, font=("Courier New", 10))
        self.text_widget.pack(expand=True, fill="both")

        self.create_menu()
        self.create_tags()
        self.bind_events()

        self.filepath = None

    def create_tags(self):
        self.text_widget.tag_configure("keyword", foreground="#CC7A00")
        self.text_widget.tag_configure("string", foreground="#008000")
        self.text_widget.tag_configure("comment", foreground="#808080")
        self.text_widget.tag_configure("heredoc", foreground="#008000", font=("Courier New", 10, "italic"))
        self.text_widget.tag_configure("directive", foreground="#9B59B6")

    def bind_events(self):
        self.text_widget.bind("<<Modified>>", self.on_text_change)

    def on_text_change(self, event=None):
        self.highlight_syntax()
        self.text_widget.edit_modified(False)

    def highlight_syntax(self):
        self.text_widget.mark_set("range_start", "1.0")

        # Define TissLang patterns
        patterns = {
            'keyword': r'\b(TASK|STEP|WRITE|RUN|ASSERT|READ|PROMPT_AGENT|SET_BUDGET|REQUEST_REVIEW|PARALLEL|CHOOSE|ESTIMATE_COST|AS|INTO)\b',
            'string': r'"[^"]*"',
            'comment': r'#.*$',
            'directive': r'@[a-zA-Z_]+',
            'heredoc_start': r'<<(\w+)',
        }

        # Remove all tags first
        for tag in self.text_widget.tag_names():
            if tag != "sel": # Don't remove selection tag
                self.text_widget.tag_remove(tag, "1.0", "end")

        content = self.text_widget.get("1.0", "end-1c")

        # Apply keyword, string, and comment tags
        for token_type, pattern in patterns.items():
            for match in re.finditer(pattern, content):
                start, end = match.span()
                self.text_widget.tag_add(token_type, f"1.0+{start}c", f"1.0+{end}c")

        # Special handling for heredoc content
        in_heredoc = False
        heredoc_delimiter = None
        for i, line in enumerate(content.splitlines()):
            if in_heredoc:
                if line.strip() == heredoc_delimiter:
                    in_heredoc = False
                    heredoc_delimiter = None
                else:
                    self.text_widget.tag_add("heredoc", f"{i+1}.0", f"{i+1}.end")
            else:
                match = re.search(r'<<(\w+)', line)
                if match:
                    in_heredoc = True
                    heredoc_delimiter = match.group(1)

    def create_menu(self):
        menu_bar = tk.Menu(self.root)
        self.root.config(menu=menu_bar)

        file_menu = tk.Menu(menu_bar, tearoff=0)
        menu_bar.add_cascade(label="File", menu=file_menu)
        file_menu.add_command(label="Open", command=self.open_file)
        file_menu.add_command(label="Save", command=self.save_file)
        file_menu.add_command(label="Save As...", command=self.save_file_as)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.exit_editor)

    def open_file(self):
        filepath = filedialog.askopenfilename(
            filetypes=[("TissLang Files", "*.tiss"), ("All Files", "*.*")]
        )
        if not filepath:
            return
        self.text_widget.delete(1.0, tk.END)
        with open(filepath, "r", encoding="utf-8") as f:
            self.text_widget.insert(tk.END, f.read())
        self.filepath = filepath
        self.root.title(f"TissLang IDE - {self.filepath}")
        self.on_text_change()

    def save_file(self):
        if self.filepath:
            try:
                with open(self.filepath, "w", encoding="utf-8") as f:
                    f.write(self.text_widget.get(1.0, tk.END))
                self.root.title(f"TissLang IDE - {self.filepath}")
            except Exception as e:
                messagebox.showerror("Save Error", f"Could not save file:\n{e}")
        else:
            self.save_file_as()

    def save_file_as(self):
        filepath = filedialog.asksaveasfilename(
            defaultextension=".tiss",
            filetypes=[("TissLang Files", "*.tiss"), ("All Files", "*.*")],
        )
        if not filepath:
            return
        self.filepath = filepath
        self.save_file()

    def exit_editor(self):
        if messagebox.askokcancel("Quit", "Do you really want to quit?"):
            self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    ide = TissLangIDE(root)
    root.mainloop()
