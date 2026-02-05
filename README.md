
# Clade Chat

Clade Chat is a free and open-source Instant Messaging (IM) software that is available for Desktop OSes.

## Build

To compile Clade Chat, you need to have a C++ compiler.

You also need to have some libraries:

* Qt 5 (for client)
* SQLite3 (for server)
* OpenSSL

After downloading source code, you need to go to the directory and run `make` command. This will compile both client and server.

For compiling client, you need to have CMake (minimum 3.5) installed. Compiling server only needs `make` program.

To make server work, the server program creates database called 'server.db'. You can access to the database and add an account.

## License

Clade Chat is licensed under GNU General Public License v2.
