# SHELL-IN-C
This project is a simple shell program implemented in C. It reads user input, tokenizes commands, executes external programs, and supports built-in commands like ct (change directory) and where (print current directory).

# FEATURES
  - Reads user input with linenoise for command-line history support.
  - Parses and tokenizes input into arguments.
  - Supports various built in commads 

# Built-in Commands
  - ct <directory>: CHANGE TO directory (same as cd).
  - where: prints path to current working directory (same as pwd)

# USAGE

To run the program use following commands:
```
gcc ./lib/linenoise.c shell.c
```

Make sure to include linenoise.c and linenoise.h libraries in lib folder (included in the repo)
