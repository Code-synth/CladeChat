FROM ubuntu:22.04

RUN apt-get update -y

RUN apt-get upgrade -y

RUN apt-get install -y unzip

RUN apt-get install -y nano

RUN apt-get install -y iproute2

RUN apt-get install -y ca-certificates curl

RUN apt-get install -y binutils g++ make

RUN apt-get install -y libssl-dev sqlite3 libsqlite3-dev

WORKDIR /
COPY server server
WORKDIR /server
RUN make
