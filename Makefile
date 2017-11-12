all: crawler pagerank

crawler: crawler.o
	g++ -o crawler crawler.o -levent

crawler.o: crawler.cpp
	g++ -c -g crawler.cpp -std=c++11

pagerank: pagerank.o
	g++ -o pagerank pagerank.o

pagerank.o: pagerank.cpp
	g++ -c -g pagerank.cpp -std=c++11

clean :
	rm crawler.o pagerank.o
