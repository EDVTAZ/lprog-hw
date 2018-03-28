# Linux Programming Homework: Google Docs concurrent editing

## Short description

This project aims to imitate the way several people can view and edit the same document concurrently.

## User Documentation

The program can be set to run in client or server mode. In client mode the user has to specify the address of the server it wants to connect to. If the server allows more than one file to be edited, the file also has to be selected from the list. In server mode the file (or list of files or directory) that the server will make available for the clients.

```
ce --client-mode <SERVER_IP> [FILE_NAME]
ce --server-mode <FILES or DIRECTORIES>
```

As a first version, the length of the document will be limited in size (in order to avoid having to deal with scrolling and other difficulties that arise when writing a text editor). If I have the time I will later remove some of these limitations, or use an already existing editor through and API (e.g. [neovim](https://neovim.io/doc/user/api.html)).

## Technical documentation

### Communication

* use TCP, so we don't have to worry about lost packets
* messages:
    * request synchronization for ``file``, with current ``hash`` (this is the first message in a connection, but it may be later repeated to ensure the two sides are in sync): ``hash`` is the hash of the version of the file that the client has (if the client doesn't have the file, send all 0 hash) (client -> server)
    * response to synchronization: (server -> client)
        * if hash matched: ``OK`` ``hash``
        * if hash was incorrect: ``WRONG`` and send the whole file
    * character at ``position`` deleted in ``file`` (both way)
    * insert ``character`` at ``position`` in ``file`` (meaning the character originally at ``position`` is now at ``position+1``) (both way)
    * sign off message (client -> server)
