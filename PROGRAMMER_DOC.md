# UI

For the command-line user interface we will use [ncurses](http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/).

# Data Structures

## Buffers in memory

Managing the documents currently being edited will be done using a linked list of ropes (probably with [this implementation](https://github.com/josephg/librope)). Each entry of the list is a rope, each rope is a line in the document. To speed up traversing the list, we will store the pointer to the first line currently at the clients top of the window, and a pointer to each line that has a users cursor in it.

## Undo/Redo

TODO

## Metadata file format (for the server)

TODO

# Basic outline of operation

The server constantly listens for new connections from clients. When a new client connects it lists the available buffers for it. After the client selects which one it wants to edit, it waits for changes from clients to the buffers. When a change (cursor movement, insertion or deletion) is received, the server stores the change, and broadcasts it to all the other active clients. To avoid inconsistencies, each file has a version number, that is incremented each time a change is made. Every change sent by the client contains the version of the file, and if it isn't correct, the server will reject it.

## Client

* connect to server
* get list of buffers
* select buffer
* get and deserialize buffer
* start input loop (send input keys to server, and listen for events from server)

## Server

* wait for connections
* get connection, send list of files
* get file request
  * if file doesn't have a server process backing it yet, fork one for it
  * send tcp connection fd to the correct process, so it can use it [1](https://stackoverflow.com/questions/18936614/can-you-pass-a-tcp-connection-from-one-process-to-the-other) [2](https://sumitomohiko.wordpress.com/2015/09/24/file-descriptor-passing-with-sendmsg2-and-recvmsg2-over-unix-domain-socket/) [3](https://stackoverflow.com/questions/28003921/sending-file-descriptor-by-linux-socket/)
* in file specific server process
  * load buffer from file
  * start input loop
* in loop
  * wait for clients (from parent process, through unix socket), if one connects, serialize buffer and send it
  * wait for inputs from clients (through tcp sockets), broadcast events back to everyone else


# Modules

* Data backend: holds the currently edited buffer, takes commands like: load from file, move cursor, insert char, delete char
* UI: responsible for displaying the text, cursors, etc.
* Network: responsible for synchronizing state with server/client

## Network

### Login protocol

1. establish secure channel...
2. C -> S : username + password hash
3. S -> C : OK/FAIL + file list
4. C -> S : file id
5. S -> C : buffer with ids

### Input event

| file id | file version | user id | event id (cursor move, insert etc.) | event data |

cursor id = user id
