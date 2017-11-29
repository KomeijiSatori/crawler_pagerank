#include <iostream>

#include "crawl_utils.h"
using namespace std;

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		cout << "usage: ./crawler ip port [url-file]" << endl;
		exit(1);
	}
	char *host = argv[1];
	int port = stoi(string(argv[2]));
	char *file_name = argv[3];
	int timeout_cnt = crawl(host, port, file_name);
	
	cout << "crawl over" << endl;
	cout << "timeout count: " << timeout_cnt << endl;
	return 0;
}

