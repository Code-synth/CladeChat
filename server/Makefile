CPP=g++
CPPFLAGS=-std=c++98
SQLITELIB=-L/usr/local/lib -lsqlite3
SQLITEINC=-I/usr/local/include
SSL=-lssl -lcrypto
UUID=-luuid

build: server.c
	${CPP} ${CPPFLAGS} server.c ${SQLITELIB} ${SQLITEINC} ${SSL} ${UUID} -o server

clean:
	-rm server
