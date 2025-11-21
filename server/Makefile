CPP=g++
CPPFLAGS=-std=c++98
SQLITELIB=-L/usr/local/lib -lsqlite3
SQLITEINC=-I/usr/local/include
SSL=-lssl -lcrypto

build: server.c
	${CPP} ${CPPFLAGS} server.c ${SQLITELIB} ${SQLITEINC} ${SSL} -o server

clean:
	-rm server
