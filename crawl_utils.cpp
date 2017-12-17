#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include "bloom.h"
#include "url_utils.h"
#include "crawl_utils.h"
using namespace std;

char *host = NULL;
int port = 0;
map<string, int> urls;
const int MAX_CONNECTION = 100;

struct event_base *base = NULL;
struct evhttp_connection *connections[MAX_CONNECTION];
int connections_req_num[MAX_CONNECTION];
vector<pair<string, string> > url_conns;

int timeout_cnt = 0;


void deliver_request(const char *request_url)
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

	struct evhttp_request *request = evhttp_request_new(RemoteReadCallback, args);
	evhttp_add_header(evhttp_request_get_output_headers(request), "Host", host);
	evhttp_make_request(connections[min_ind], request, EVHTTP_REQ_GET, request_url);
}


void after_receive_response(struct arg_pack *args)
{
	connections_req_num[args->conn_id]--;
	delete args;
}


bool judge_close(void)
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


void RemoteReadCallback(struct evhttp_request *remote_rsp, void *arg)
{
	// if timeout
	if (remote_rsp == NULL)
	{
		timeout_cnt++;
		return;
	}

	int return_code = evhttp_request_get_response_code(remote_rsp);
	struct arg_pack *args = (struct arg_pack *)arg;

	if (return_code == 200)
	{
		string cur_url = string(evhttp_request_get_uri(remote_rsp));
		
		urls[cur_url] = urls.size();
		int cur_id = urls[cur_url];

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

		vector<string> res = get_page_urls(page_str, cur_url); 
		for (auto url:res)
		{
			if (!in_set(url))
			{
				set_in(url);
				deliver_request(url.c_str());
			}
			url_conns.push_back(pair<string, string>(cur_url, url));
		}
	}
	
	after_receive_response(args);
	
	if (judge_close())
	{
		event_base_loopexit(base, NULL);
	}
}


void RemoteConnectionCloseCallback(struct evhttp_connection *connection, void *arg)
{

}


void init_crawler(char *para_host, int para_port)
{
	host = para_host;
	port = para_port;
	const char *request_url = "/";
	base = event_base_new();
	for (int i = 0; i < MAX_CONNECTION; i++)
	{
		connections[i] = evhttp_connection_base_new(base, NULL, host, port);
		evhttp_connection_set_closecb(connections[i], RemoteConnectionCloseCallback, base);
		connections_req_num[i] = 0;
	}

	init_bloom();
	set_in(string(request_url));
	deliver_request(request_url);
}


// return the timeout count
int crawl(char *para_host, int para_port, char *para_file_name)
{
	init_crawler(para_host, para_port);
	event_base_dispatch(base);

	// eliminate non-200 urls	
	vector<pair<int, int> > url_num_conns;
	for (auto const &x : url_conns)
	{
		// if is 200 url connections
		if (urls.find(x.first) != urls.end() && urls.find(x.second) != urls.end())
		{
			url_num_conns.push_back(pair<int, int>(urls[x.first], urls[x.second]));
		}
	}

	// output the results
	ofstream out(para_file_name);
	string host_name = "http://news.sohu.com";
	for (auto const &x : urls)
	{
		out << x.second - 1 << " " << host_name + x.first << endl;
	}
	out << endl;
	for (auto const &x : url_num_conns)
	{
		out << x.first - 1 << " " << x.second - 1 << endl;
	}
	out.close();
	return timeout_cnt;
}
