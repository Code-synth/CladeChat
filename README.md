
# Clade Chat

Clade Chat is a free and open-source Instant Messaging (IM) software that is available for Desktop OSes.

## Build

To compile Clade Chat, you need to have a C++ compiler.

You also need to have some libraries:

* Qt 6 (for client)
* SQLite3 (for server)
* OpenSSL

After downloading source code, you need to go to the directory and run `make` command. This will compile both client and server.

For compiling only one, switch to 'server' or 'client' directory and run `make` command.

**Warning!:** After compiling the source code, you also need to run `make cert` command to copy certificates to source directories.

To make server work, the server program creates database called 'server.db'. You can access to the database and add an account.

## License

Clade Chat is licensed under GNU General Public License v2.
