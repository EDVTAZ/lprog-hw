# Linux Programming Homework: Google Docs concurrent editing

## Short description

This project aims to imitate the way several people can view and edit the same document concurrently with Google Docs.

## Usage (client)

After starting the client you will be asked if you have an account. If you answer `y` you can supply your credentials, if you answer `n` you can type the username and password that you would like to register with. Next you will be asked what file you want to edit; type the number to start editing, type a negative number to delete the corresponding file and type 0 to create a new file. Once you choose a file curses mode will start and you can edit your file. Move around with the arrow keys, and type letters to insert. Press `ESC` to quit.

## Deploying the server

* Install `ncurses` and `socat` with your preferred method (source/binaries/package manager)
* `parson` is included in the source
* Clone `librope` from [here](https://github.com/EDVTAZ/librope/commits/master) (or from the original repository if it gets updated), I forked it so I could fix some compiler warnings. Hopefully I didn't mess anything up :)
    * run `make` and copy `librope.a` to `/lib`
* Modify the `SERVER_NAME` define in editor.c to match your servers name
* Generate a certificate for the server. You can do this with [this script](https://gist.github.com/EDVTAZ/3b7a98787331ccafb4ad6402a11d008e) if you don't like using OpenSSL's syntax (replace localhost with the appropriate hostname)
```
./snc.sh -g --cert b --CN "localhost"
```
* Distribute the public value (`.crt`) with the clients, so they can verify the authenticity of the server, but keep the `.key` secret!
* Build the server with `make server` and the client with `make editor`
* Start the server