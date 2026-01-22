build domain.crt domain.key:
	cp domain.crt server
	cp domain.key server
	cd server && make
	mkdir -p client/build
	cp domain.crt client/build
	cp domain.key client/build
	cd client && cd build && cmake .. && make
	cp client/*.png client/build

clean:
	-rm server/domain.crt
	-rm server/domain.key
	-cd server && make clean
	-cd client && rm -rf build
