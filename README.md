AT Modem Server
A C++17 server that listens on one or more serial devices (tty) or pseudo-terminals (pty) and responds to AT commands based on a dictionary file with a limited regular expression syntax.

This project demonstrates system programming in Linux, custom regex engine implementation, multi‑threading, and working with termios and poll.

Features
Listens on multiple tty devices simultaneously, each in its own thread.

Custom regex engine with support for . (any char), * (zero or more of any chars), and [...] character classes (ranges and negation).

Dictionary is loaded from a simple text file with expect=answer format.

Automatic addition of \r\n to responses if missing.

Proper termios raw mode setup and restoration on exit.

--pty mode creates a pseudo‑terminal pair and prints the slave path for easy testing without real hardware.

Unit tests for pattern matching and dictionary.

Cross‑platform POSIX (Linux, macOS, WSL).

Requirements
POSIX‑compatible OS (Linux, macOS, WSL)

C++17 compiler (GCC or Clang)

CMake ≥ 3.10 or GNU Make

libutil (for openpty), pthread

On Debian/Ubuntu, install build tools with:

sudo apt update
sudo apt install build-essential cmake
Project Structure
at-modem-server/
├── src/
│   ├── main.cpp
│   ├── server.h / .cpp
│   ├── serial_port.h / .cpp
│   ├── pattern.h / .cpp
│   ├── dictionary.h / .cpp
│   └── utils.h
├── tests/
│   └── test_main.cpp
├── dictionary.txt            # example dictionary
├── CMakeLists.txt
├── Makefile
├── .gitignore
└── README.md

Build
Using CMake
mkdir build && cd build
cmake ..
make
The executables at_server and tests will be placed inside build/.

Using Makefile
make
Run tests:

make test
Usage
Basic command

./at_server -d /dev/ttyUSB0 -f dictionary.txt
Multiple devices
./at_server -d /dev/ttyUSB0 -d /dev/ttyUSB1 -f dictionary.txt
Pseudo‑terminal mode
./at_server --pty -f dictionary.txt
The program will print the slave device path (e.g., /dev/pts/5). You can connect to it using picocom, minicom, socat, or simply cat and echo.

Example:

# Terminal 1
./at_server --pty -f dictionary.txt
# Output: PTY slave device: /dev/pts/5

# Terminal 2
echo -e "AT\r" > /dev/pts/5
cat /dev/pts/5   # to see responses
Command line options
Option	Description
-d, --device <tty>	Serial device to listen on (can be repeated)
-p, --pty	Create a pseudo‑terminal pair
-f, --dict <file>	Dictionary file (default: dictionary.txt)
-h, --help	Show help
Dictionary Format
Each non‑empty line that does not start with # has the form:

expect=answer
expect is a pattern with a limited regular expression syntax, answer is the response string.

Pattern syntax
. – matches any single character.

* – matches zero or more of any characters (greedy with backtracking). It is not a quantifier of the previous character; it stands for any sequence.

[...] – character class. Supports ranges ([a-z], [0-9]) and negation ([^...]).

All other characters (including +, ?, (, etc.) are treated as literals.

Pattern matching is anchored: the entire input command must match the pattern.

Answer escape sequences
In the answer part the following escapes are supported:

\r – carriage return

\n – line feed

\\ – backslash

\= – equals sign

If the answer does not end with \r\n or \n, the server automatically appends \r\n.

Example dictionary
# Basic commands
AT=OK
ATE[01]=OK
ATI=Modem Emulator v1.0
AT+COPS=+COPS: 0,0,"Test Operator"
AT+CPIN=+CPIN: READY
Server Behavior
The server reads bytes from the device, buffers them, and splits on \r or \n characters.

Each extracted command is looked up in the dictionary in order of pattern specificity (fewer * and longer literal parts first). The first matching entry is used.

If a match is found, the corresponding answer is sent back.

If no match is found, ERROR is sent (with \r\n).

Before sending, the server checks whether the answer ends with a newline; if not, it adds \r\n.

Limitations
No escaping of special characters inside expect. To match a literal . or *, use a character class: [.], [*].

In a character class, - is treated as range when between two characters. To include a literal -, place it at the start or end: [-a] or [a-].

A comma inside [...] is a regular character. Use [01] instead of [1,0].

The ] character inside a character class is not supported; a character class ends at the first ].

The implementation does not perform full Unicode normalization; it works with raw bytes and assumes valid UTF-8 (or EUC‑KR) input.

Testing
The tests executable contains unit tests for Pattern and Dictionary.

Run from the build directory:

./tests
Or with Make:
make test
Expected output:
Pattern tests passed
Dictionary tests passed

License
MIT License. See LICENSE file if present.
