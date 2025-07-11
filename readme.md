# kali_shell

A minimal yet feature-rich Unix-like shell written in C — inspired by Kali Linux workflows.  
It supports common shell features such as command execution, pipelines, redirection, aliases, and job control, all wrapped in a lightweight interactive shell environment.

---

## 🌟 Features

- ✅ **Command Execution** — Runs standard commands using `$PATH`.
- 🔄 **Pipes (`|`)** — Chain commands with output-to-input piping.
- 📂 **Redirection**
  - `>`: Redirect stdout to a file (overwrite)
  - `>>`: Append stdout to a file
  - `<`: Redirect stdin from a file
- 🧠 **Built-in Commands**
  - `cd`, `exit`, `help`, `alias`, `unalias`, `history`, `jobs`, `fg`, `bg`
- 📜 **Alias System**
  - Define aliases in `~/.kali_shellrc` with:  
    ```bash
    alias ll='ls -la'
    ```
- 🧠 **Command History**
  - Automatically saves history to `.kali_shell_history`
  - Integrated with GNU Readline
- ⚡ **Tab Completion**
  - Smart auto-completion for built-in commands and executables in `PATH`
- 🛠️ **Job Control**
  - Supports background tasks (`&`) and notifications when they complete
- 🎨 **Configurable Prompt**
  - Prompt rendering is customizable through internal config

---

## 📁 File Structure

kali_shell/
├── include/ # Header files (parser.h, executor.h, utils.h, etc.)
├── src/ # Source files
│ ├── main.c # Shell entry point
│ ├── parser.c # Input parsing and command struct creation
│ ├── executor.c # Handles execution logic, redirection, pipelines
│ ├── builtins.c # Implements built-in commands
│ ├── history.c # Read/write shell history
│ ├── config.c # Prompt configuration and aliases
│ ├── prompt.c # Custom prompt rendering
│ └── utils.c # Utility helpers like trim_whitespace
├── Makefile # Build script
└── README.md # Project overview


---

## 🚀 Build & Run

### 🧱 Requirements

- Linux or Unix-based system
- GCC compiler
- `libreadline-dev` installed

### 🔧 Build

```bash
make

▶️ Run

./kali_shell

🛠 Sample .kali_shellrc File

Place this file in your home directory (~/.kali_shellrc) to load custom aliases on startup:

alias ll='ls -la'
alias gs='git status'
alias ..='cd ..'

❓Example Usage

kali_shell> echo hello > out.txt
kali_shell> cat < out.txt | grep h
hello
kali_shell> alias hi='echo hello world'
kali_shell> hi
hello world

🧠 Why This Shell?

This project was created as a custom Linux terminal emulator focused on simplicity and extendability, especially for penetration testing workflows (e.g., quickly running recon commands).
It serves as a solid foundation for scripting, automation, or building more advanced interactive tools.
📜 License

This project is licensed under the MIT License.
🙌 Credits

    Developed using C11 and GNU Readline

    Inspired by Unix shell behavior and the Kali Linux environment
