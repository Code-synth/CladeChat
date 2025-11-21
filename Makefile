build:
	cd server && make
	cd client && mkdir -p build && cd build && cmake .. && make

cert: domain.crt domain.key
	cp domain.crt server
	cp domain.crt client
	cp domain.key server
	cp domain.key client

clean:
	-rm server/domain.crt
	-rm client/domain.crt
	-rm server/domain.key
	-rm client/domain.key
	-rm client/chat
	-cd server && make clean
	-cd client && rm -rf build
