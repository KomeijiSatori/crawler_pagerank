#include <event.h>
#include <event2/http.h>


struct arg_pack
{
	int conn_id;
};


void deliver_request(const char *request_url);
void after_receive_response(struct arg_pack *args);
bool judge_close(void);
void RemoteReadCallback(struct evhttp_request *remote_rsp, void *arg);
void RemoteConnectionCloseCallback(struct evhttp_connection *connection, void *arg);
void init_crawler(char *host, int port);
int crawl(char *host, int port, char *file_name);

