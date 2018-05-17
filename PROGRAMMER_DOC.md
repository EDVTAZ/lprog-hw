# Programmers Documentation

## Division of tasks

The original idea was to divide the development into client side and server side. However this didn't prove to be feasible, since a lot of components are used on both server and client (e.g. both sides use buffer.h to represent a file in memory, and both use msg.h to communicate through the network). The final division ended up so:

* Gnandt Balazs: plaintext network communication (msg.h among others), server side management of files and users
* Szekely Gabor: data backend (buffer.h), client user interface, tunneling network communication through an ssl tunnel with socat, server multithreading to allow more than one file to be edited at the same time

## Dependencies and external resources used

* [parson](https://github.com/kgabis/parson) a lightweight JSON library in c: used for buffer serialization and network communication
* [librope](https://github.com/josephg/librope) a rope data structure implementation: the rope data structure allows fast insertion and deletion in strings
* [ncurses](https://www.gnu.org/software/ncurses/) for the user interface
* [socat](https://linux.die.net/man/1/socat): a multipurpose relay, used to tunnel network communication in ssl

## Basic operation overview

### Client

When the client is launched, it connects to the hardcoded (ip:port) remote server. After logging in and selecting a file (there are more possible actions here, which are detailed in the user documentation) the server replies with the port number of the worker that will be serving the requested file. Now the client can connect to said worker, log in and start editing.

### Server

When the server is launched it will listen on a predefined port. Let's call this process the `main server`. When a (logged in) client requests a file the server forks a process that will listen on a somewhat random port. We will call this process the `worker`. The `main server` will then send the port number of the appropriate `worker` to the client, so it can connect to it.

## Details of the components

### buffer

* A `buffer` is the representation of a file in our programs memory. It is meant to be used by telling it what a specific cursor did. For example `cursor` `A` moved left, or inserted a character. To find out what exact operations a `buffer` takes take a look at `buffer.h`.
* The `buffer` has three different structure types:
  * A `line` represents a single line of the file, and for now it has a maximum length (although one can get around that with a simple trick). A `line` has a unique id (does not equal the number of the line!) and a pointer to the previous and next `line`. This way the `buffer` is a linked list of `line`. The characters of the `line` are stored in `rope` structure. To make it easier to handle scrolling and updating the screen, it is also stored whether a `line` is currently on the screen of the user and how it is positioned relatively to the `cursor` of the user (above, bellow, or it is the line that the `cursor` is in).
  * A `cursor` represents a user that is editing the file. Just as a line it has a unique id. Id `0` has an additional meaning, as it is always the id of the `cursor` that represents the local user. A pointer to the `buffer` and the `line` of the `cursor` is also stored, just as the position of the `cursor` in said `line`.
  * In the `buffer` structure (among others) the pointers to the first and last `line` of the file are stored and the pointers to the active cursors in an array. For ease of displaying, the pointers to the `ui` and the `line` currently at the top and bottom of the screen are also stored. When needed, the `ui_update` function will be called.
* The `buffer` can be initialized in `server_mode`, so no user interface will be created for it. When a new client connects, the server serializes the whole `buffer` and sends the whole thing over to it. Then it clones it's own `cursor` with the new clients' id and broadcasts to the other clients to create the appropriate cursor as well.

### ui

This is currently a pretty simple solution; when the update function is called, it iterates over all the lines that are on screen, and prints them. Then it repaints the letters that have peer cursors on them and lastly places the cursor to the location of the local users position. Currently some additional debug information is also displayed.

### ssl tunneling

A socat relay is started both on client and server side. On client side the relay listens on localhost and connects to the server side over the public network. On server side it's the opposite, meaning it listens for connections on the public interface and connects to the server instance that is listening on localhost. This does allow any other program to also use the ssl tunnel at least on the client side. The socat relays use `ssl`/`ssl-listen` on the public network and `tcp4`/`tcp4-listen` on the localhost. The relays are started by calling `fork` and `exec`.

### msg / network proto (gb)
TBD
### utilities (gb)
TBD