all: sspd sspgame

sspd: server.o
		$(CXX) -L./ -Wall -o sspd server.o

sspgame: client.o
		$(CXX) -L./ -Wall -o sspgame client.o

clean:
		rm *.o sspd sspgame