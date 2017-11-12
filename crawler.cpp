#include <event.h>
#include "event2/http.h"

#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <vector>
#include <regex>
using namespace std;

map<string, int> urls;
map<string, int> all_urls;

char *host = NULL;
int port;
const int MAX_CONNECTION = 100;

struct event_base *base = NULL;
struct evhttp_connection *connections[MAX_CONNECTION];
int connections_req_num[MAX_CONNECTION];
vector<pair<int, int> >url_conns;


void RemoteReadCallback(struct evhttp_request* remote_rsp, void* arg);
struct arg_pack
{
	int conn_id;
	int source_url_id;
};


string cleaned_url(string url)
{
	// eliminate #
	size_t pos = url.find("#");
	if (pos != string::npos)
	{
		url = url.substr(0, pos);	
	}
	return url;
}


vector<string> get_page_urls(string page_str)
{
	vector<string> res;
	set<string> all_url;
	regex e("\"https?:\\/\\/news.sohu.com(.*?)\"");
	smatch m;
	while (regex_search(page_str, m, e))
	{
		if (m[1].length() > 0)
		{
			string url = cleaned_url(m[1]);
			all_url.insert(url);
		}
		page_str = m.suffix().str();
	}
	for (auto url:all_url)
	{
		res.push_back(url);
	}
	return res;
}


void deliver_request(const char *request_url, int source_url_id)
{
	int min_conn = connections_req_num[0];
	struct arg_pack *args = new struct arg_pack;
	
	int min_ind = 0;
	for (int i = 1; i < MAX_CONNECTION; i++)
	{
		if (connections_req_num[i] < min_conn)
		{
			min_conn = connections_req_num[i];
			min_ind = i;
		}
	}

	connections_req_num[min_ind]++;
	args->conn_id = min_ind;
	args->source_url_id = source_url_id;

	struct evhttp_request *request = evhttp_request_new(RemoteReadCallback, args);
    evhttp_add_header(evhttp_request_get_output_headers(request), "Host", host);
    evhttp_make_request(connections[min_ind], request, EVHTTP_REQ_GET, request_url);
}


void after_receive_response(struct arg_pack *args)
{
	connections_req_num[args->conn_id]--;
	delete args;
}


bool judge_close()
{
	for (int i = 0; i < MAX_CONNECTION; i++)
	{
		if (connections_req_num[i] > 0)
		{
			return false;
		}
	}
	return true;
}


void RemoteReadCallback(struct evhttp_request* remote_rsp, void* arg)
{
	int return_code = evhttp_request_get_response_code(remote_rsp);
	struct arg_pack *args = (struct arg_pack *)arg;

	if (return_code == 200)
	{
		string cur_url = string(evhttp_request_get_uri(remote_rsp));
		
		urls[cur_url] = urls.size();
		int cur_id = urls[cur_url];
		int source_id = args->source_url_id;

		url_conns.push_back(pair<int, int>(source_id, cur_id));
		cout << "size: " << urls.size() << endl;

		char buf[4096];
		struct evbuffer* evbuf = evhttp_request_get_input_buffer(remote_rsp);

		int n = 0;
		string page_str = "";
		while ((n = evbuffer_remove(evbuf, buf, sizeof(buf) - 1)) > 0)
		{
			buf[n] = '\0';
			page_str += buf;
		}

		vector<string> res = get_page_urls(page_str);  
		for (auto url:res)
		{
			if (all_urls.find(url) == all_urls.end())
			{
				all_urls[url] = all_urls.size();
				deliver_request(url.c_str(), cur_id);
			}
			else
			{
				if (urls.find(url) != urls.end())
				{
					int des_url_id = urls[url];
					url_conns.push_back(pair<int, int>(cur_id, des_url_id));
				}
			}
		}
	}
	
	after_receive_response(args);
	
	if (judge_close())
	{
    	event_base_loopexit(base, NULL);
	}
}


void RemoteConnectionCloseCallback(struct evhttp_connection* connection, void* arg)
{

}

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		cout << "usage: ./crawler ip port [url-file]" << endl;
		exit(1);
	}
	host = argv[1];
	port = stoi(string(argv[2]));
	char *file_name = argv[3];


	const char *request_url = "/";
	all_urls[request_url] = 0;

	base = event_base_new();
	for (int i = 0; i < MAX_CONNECTION; i++)
	{
		connections[i] = evhttp_connection_base_new(base, NULL, host, port);
		evhttp_connection_set_closecb(connections[i], RemoteConnectionCloseCallback, base);
		connections_req_num[i] = 0;
	}
	
    deliver_request(request_url, -1);
    event_base_dispatch(base);
	
	// delete the first url_conn of root url
	url_conns.erase(url_conns.begin());

	ofstream out(file_name);
	for (auto const& x : urls)
	{
		out << x.first << " " << x.second << endl;
	}
	out << endl;
	for (auto const &x : url_conns)
	{
		out << x.first << " " << x.second << endl;
	}
	out.close();

    return 0;
}

