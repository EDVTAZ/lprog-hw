# Linux Programming Homework: Google Docs concurrent editing

## Short description

This project aims to imitate the way several people can view and edit the same document concurrently with Google Docs.

## Features and general usage

### The Server

The server stores the files, manages changes made to them and notifies online clients of these changes. 

### The Client

When starting a client, the user can specify the IP address of the server (s)he wants to use. After a connection has been made, the user has to log in (username-password). If successful, the client lists the available files on the server and the user can choose one to edit, or upload a new one. While more than one user is editing the same document, changes made by other users can be seen real-time, but changes made above the cursor shouldn't push the lines down so as not to annoy the user. 

### Possible further improvements

* Undo: Unduing changes should undo only changes made by a specific user (and leave changes made by other users at the same time); this might be a bit difficult to implement.

## Dependencies

* msgpack



