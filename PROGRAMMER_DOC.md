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

# Modules

* Data backend: holds the currently edited buffer, takes commands like: load from file, move cursor, insert char, delete char
* UI: responsible for displaying the text, cursors, etc.
* Network: responsible for synchronizing state with server/client
