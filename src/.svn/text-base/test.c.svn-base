#include "scredis.h"
#include <string.h>
int main(int argc,char *argv[])
{
	redis_client *c = redis_connect("10.210.210.146","8001");
	if(!c || c->fd < 0)die("connect error");
	redis_response *resp = redis_command(c,"set key abc");
	if(resp)
		free_response(resp);
	resp = redis_command(c,"get key");
	if(resp)
	{
		printf("%s\n",resp->data);
	}
	free_response(resp);
	free_client(c);
	return 0;
}
