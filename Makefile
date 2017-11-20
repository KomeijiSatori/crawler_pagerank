all: crawler pagerank

crawler: crawler.o bloom.o url_utils.o crawl_utils.o
	g++ -o crawler crawler.o bloom.o url_utils.o crawl_utils.o -levent

crawler.o: crawler.cpp
	g++ -c -g crawler.cpp -std=c++11

crawl_utils.o: crawl_utils.cpp crawl_utils.h
	g++ -c -g crawl_utils.cpp -std=c++11

bloom.o: bloom.cpp bloom.h
	g++ -c -g bloom.cpp

url_utils.o: url_utils.cpp url_utils.h
	g++ -c -g url_utils.cpp -std=c++11

pagerank: pagerank.o
	g++ -o pagerank pagerank.o

pagerank.o: pagerank.cpp
	g++ -c -g pagerank.cpp -std=c++11

clean :
	rm bloom.o url_utils.o crawler.o pagerank.o crawl_utils.o
