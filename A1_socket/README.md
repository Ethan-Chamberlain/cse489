# Socket Programming Assignment

## Overview
You are writing a TCP client/server pair in C. Sockets are the standard interface for
sending and receiving data over a network. Even though you will test on a single machine
(localhost), the same APIs are used for communication across different machines.

## Why localhost still uses real networking
Your client/server code works the same on a real network and on a single machine. Both are
valid:

- Two machines: client connects to the server's real IP address.
- One machine: client connects to 127.0.0.1 (localhost).

When you use 127.0.0.1, your OS still creates a real TCP connection. The same TCP/IP stack
handles connection setup, buffering, and delivery. The only difference is that the packets
loop back inside your machine instead of going out over a physical network.

Think of it like this (single machine):

```text
Client process                             OS TCP/IP stack                         Server process
--------------                             -----------------                       --------------
./client 127.0.0.1:12345  ->  [src=127.0.0.1:random_ephemeral -> dst=127.0.0.1:12345]  ->  ./server :12345
```

Even on one machine, the client uses its own ephemeral source port, the server listens on
its own port, and the OS routes the bytes through the full TCP/IP stack.

Everything is real networking: connection setup, buffering, retransmission, and the byte
stream semantics. The only difference is that the "wire" is inside your computer, so it is
fast and easy to debug.

If you later run the client on another machine and point to the server's IP, the code is
the same. Our automated tests are single-machine for convenience, but you are welcome to
test across two machines as well.

## Basic terminal concepts (very important)
You will use the terminal to run programs. Two important concepts:

1) stdin (standard input)
   - This is the input data your program reads.
   - By default, stdin comes from the keyboard.
   - When we use a pipe like:
```text
printf "Hello\n" | ./client 127.0.0.1 12345
```
     the output of printf becomes the stdin of ./client.

2) stdout (standard output)
   - This is the output your program writes.
   - By default, stdout is shown on the screen.
   - stdout can be redirected to a file, for example:
```text
./server 12345 > server_output.txt
```
   - Our server writes received bytes to stdout, so you can see what arrived or
     redirect it to a file for comparison.

## What you need to implement
You are given skeleton code with TODO comments. Your job is to complete the missing core
networking logic.

### Client requirements (client.c)
- Create a TCP socket.
- Parse server IP/port, build sockaddr_in, and connect.
- Read from stdin until EOF, sending data in chunks.
- Handle partial sends (send may send fewer bytes than requested).
- Handle EINTR for read/send properly.
- Close the socket and exit cleanly.

### Server requirements (server.c)
- Create a TCP listening socket.
- Set SO_REUSEADDR to allow quick restarts.
- Bind to INADDR_ANY and the given port.
- Listen with a small backlog (e.g., 5-10).
- Accept clients in a loop.
- For each client: read until EOF and write bytes to stdout.
- Handle partial writes and EINTR.
- Keep running unless terminated by an external signal.

## Exact I/O behavior required (part of the code requirements)
### Server output rules
- Output is raw bytes only. Do not add prefixes, tags, or extra newlines.
- When you receive bytes from a client into a buffer, write those same bytes to
  standard output using write(). This is a byte-for-byte pass-through.
- write(fd, buf, count) takes:
  - fd: the file descriptor to write to. Use STDOUT_FILENO for standard output.
  - buf: pointer to the buffer holding the bytes you received.
  - count: number of bytes to write from that buffer.
- Because write() can write fewer bytes than requested, you must loop until all
  bytes from the received chunk are written.
- If you want to save server output to a file, redirect stdout:
```text
./server 12345 > server_output.txt
```

### Client sending rules
- Read from standard input using read() (stdin is your keyboard or a pipe).
- read(fd, buf, count) takes:
  - fd: the file descriptor to read from. Use STDIN_FILENO for standard input.
  - buf: pointer to the buffer to fill with incoming bytes.
  - count: maximum number of bytes to read into that buffer.
- For each chunk read, send exactly those bytes to the server (no formatting).
- Like write(), send()/write() can send fewer bytes than requested, so you must
  loop until the entire chunk is transmitted.
- If you want to feed a file as input, you can redirect or use cat:
```text
./client 127.0.0.1 12345 < input.txt
cat input.txt | ./client 127.0.0.1 12345
```

## How to build (step-by-step)
1) Open a terminal.
2) Change into the student directory:
```text
cd student
```
3) Build both programs:
```text
make
```

You should now have two executables:
  ./client
  ./server

## How to test (manual)
1) Open Terminal A and start the server:
```text
./server 12345
```
   Keep this terminal open; the server keeps running.

2) Open Terminal B and send a message with the client:
```text
printf "Hello\n" | ./client 127.0.0.1 12345
```

3) Look at Terminal A. You should see:
```text
Hello
```

Notes:
- 127.0.0.1 means "this same machine."
- 12345 is the server's listening port.
- The client does NOT use 12345 as its own source port; the OS assigns a temporary
  client port automatically. The connection is identified by:
    client_ip:client_port -> server_ip:server_port
  So both sides do NOT use the same port.

## Automated test script (detailed)
This script launches the server, runs a client with specific input, captures the
server's stdout, and compares it to the expected bytes.

### How to run the tests
From the student directory:
```text
./test_client_server.sh 12345
```

If the port is already in use, choose another port (10000-60000).

### How each test works
All tests follow the same pattern:
1) Generate an input file (the exact bytes the client should send).
2) Start the server and redirect its stdout to a temporary file.
3) Pipe the input file into the client (client connects, sends, exits).
4) Stop the server and compare the server's stdout file to the input file
   byte-for-byte.

Tests included:
1) Short text message
   - Input: a short line of text.
   - Goal: basic end-to-end correctness for small data.
2) Long alphanumeric text payload
   - Input: 100000 bytes of [A-Za-z0-9].
   - Goal: large text transfer with chunked reads/sends.
3) Long binary payload
   - Input: 65536 bytes from /dev/urandom.
   - Goal: verify you handle arbitrary binary bytes (not just text).
4) Sequential short messages from separate clients
   - Input: five short lines sent by five separate client connections.
   - Goal: confirm the server accepts multiple clients in sequence and outputs
     the concatenated bytes in order.
5) Concurrent clients sending the same message
   - Input: the same short line sent by multiple clients at the same time.
   - Goal: confirm the server handles the connection queue correctly and that
     the final output is the concatenation of all client messages.

### Output comparison and debugging
The script compares expected input bytes to server stdout bytes. Any extra
stdout output (debug prints, status messages) will cause test failures because it
changes the byte stream. If you need debugging during development, consider
printing to stderr instead.

## Reference tutorial
A solid, practical introduction to socket programming:
- https://beej.us/guide/bgnet/html/

## Files in this folder
- client.c: student skeleton with TODOs
- server.c: student skeleton with TODOs
- Makefile: build client/server
- test_client_server.sh: test harness with detailed output

## Submission instructions
Submit a single zip that contains:
1) The entire program folder (this folder).
2) A separate report file placed outside the program folder.

Directory layout example:
```text
submission_zip/
  student/                    (this folder, all source files inside)
    client.c
    server.c
    Makefile
    test_client_server.sh
    README.md
  REPORT.pdf                  (report is outside the program folder)
```

### Report requirements
Your report should include:
- Student information: full name, student ID, and email.
- Design summary: describe how you implemented the required features (socket setup,
  connect/accept loop, chunked read/write, partial send/write handling, EINTR handling).
- Testing: list the tests you ran and summarize the results.
- Include a screenshot of test output showing the results.
- If you used commands different from our test script, list the exact commands and
  the environment (OS/version if relevant).

## Tips
- Read/write in loops; network calls can be partial.
- Check return values carefully and print errors with perror.
- Keep output exactly as received; do not add extra formatting.
