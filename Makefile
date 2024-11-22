all:
	g++ --std=c++11 -o serverM serverM.cpp
	g++ --std=c++11 -o serverA serverA.cpp
	g++ --std=c++11 -o serverB serverB.cpp
	g++ --std=c++11 -o client client.cpp

clean:
	rm serverA
	rm serverB
	rm serverM
	rm client