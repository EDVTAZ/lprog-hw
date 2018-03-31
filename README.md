# Linux Programming Homework: Google Docs concurrent editing

## Short description

This project aims to imitate the way several people can view and edit the same document concurrently with Google Docs.

## Features and general usage

### The Server

The server stores the files, manages changes made to them and notifies online clients of these changes. 

### The Client

When starting a client, the user has to specify the IP address of the server (s)he wants to use. After a connection has been made, the client lists the available files on the server and the user can choose one to edit, or upload a new one. While more then one users are editing the same document, changes made by the other user can be seen real-time, but changes made above the cursor shouldn't push the lines down so as not to annoy the user. Undoing changes should undo only changes made by a specific user (and leave changes made by other users at the same time); this might be a bit difficult to implement.


