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
vector<pair<string, string> > url_conns;

int timeout_cnt = 0;


void RemoteReadCallback(struct evhttp_request *remote_rsp, void *arg);
struct arg_pack
{
	int conn_id;
};


string cleaned_url(string url)
{
	// eliminate '\n' '\t' ' '
	int blank_pos;
	blank_pos = url.find("\n");
	if (blank_pos != string::npos)
	{
		url = url.substr(0, blank_pos);	
	}
	blank_pos = url.find("\t");
	if (blank_pos != string::npos)
	{
		url = url.substr(0, blank_pos);	
	}
	blank_pos = url.find(" ");
	if (blank_pos != string::npos)
	{
		url = url.substr(0, blank_pos);	
	}
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

	int state = 0;
	int start_ind = 0;
	int end_ind = 0;
	
	for (int i = 0; i < page_str.length(); i++)
	{
		char cur_ch = page_str[i];
		if (state == 0)
		{
			if (cur_ch == '<')
			{
				state = 1;
			}
		}
		else if (state == 1)
		{
			if (cur_ch == 'a')
			{
				state = 2;
			}
			else
			{
				state = 0;
			}
		}
		else if (state == 2)
		{
			if (cur_ch == 'h')
			{
				state = 3;
			}
			else if (cur_ch == '>')
			{
				state = 0;
			}
		}
		else if (state == 3)
		{
			if (cur_ch == 'r')
			{
				state = 4;
			}
			else if (cur_ch == '>')
			{
				state = 0;
			}
			else
			{
				state = 2;
			}
		}
		else if (state == 4)
		{
			if (cur_ch == 'e')
			{
				state = 5;
			}
			else if (cur_ch == '>')
			{
				state = 0;
			}
			else
			{
				state = 2;
			}
		}
		else if (state == 5)
		{
			if (cur_ch == 'f')
			{
				state = 6;
			}
			else if (cur_ch == '>')
			{
				state = 0;
			}
			else
			{
				state = 2;
			}
		}
		else if (state == 6)
		{
			if (cur_ch == '=')
			{
				state = 7;
			}
			else if (cur_ch == '>')
			{
				state = 0;
			}
			else
			{
				state = 2;
			}
		}
		else if (state == 7)
		{
			if (cur_ch == '"')
			{
				// start record website
				start_ind = i;
				state = 8;
			}
			else if (cur_ch == '>')
			{
				state = 0;
			}
			else
			{
				state = 2;
			}
		}
		else if (state == 8)
		{
			if (cur_ch == '"')
			{
				// end record website
				end_ind = i;
				string full_url = page_str.substr(start_ind + 1, end_ind - start_ind - 1);
				string host = "news.sohu.com";
				size_t found = full_url.find(host);
				if (found != string::npos)
				{
					string url = full_url.substr(found + host.length());

					all_url.insert(cleaned_url(url));
				}
				state = 0;
			}
		}
	}

	for (auto url:all_url)
	{
		res.push_back(url);
	}
	return res;
}


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

		vector<string> res = get_page_urls(page_str); 
		for (auto url:res)
		{
			if (all_urls.find(url) == all_urls.end())
			{
				all_urls[url] = all_urls.size();
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
	all_urls[request_url] = all_urls.size();

	base = event_base_new();
	for (int i = 0; i < MAX_CONNECTION; i++)
	{
		connections[i] = evhttp_connection_base_new(base, NULL, host, port);
		evhttp_connection_set_closecb(connections[i], RemoteConnectionCloseCallback, base);
		connections_req_num[i] = 0;
	}
	
	deliver_request(request_url);
	event_base_dispatch(base);

	cout << "crawl over" << endl;
	cout << "timeout count: " << timeout_cnt << endl;
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

	ofstream out(file_name);
	for (auto const &x : urls)
	{
		out << x.first << " " << x.second << endl;
	}
	out << endl;
	for (auto const &x : url_num_conns)
	{
		out << x.first << " " << x.second << endl;
	}
	out.close();

    return 0;
}

