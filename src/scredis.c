#include "scredis.h"

#include <unistd.h>
#include <string.h>
redis_client* redis_connect(const char *host,const char *port)
{
	redis_client *client = malloc(sizeof(redis_client));
	client->fd = -1;
	struct addrinfo hints,*servinfo,*p;
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int s;
	int rv;
	if ((rv = getaddrinfo(host,port,&hints,&servinfo))!= 0)
		die("getaddrinfo:%s",gai_strerror(rv));
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((s = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
			continue;
		if (connect(s,p->ai_addr,p->ai_addrlen) == -1)
			die("connect:%s",strerror(errno)); //FIXME: don't die
		client->fd = s;
	}
	return client;

}

void free_client(redis_client *c) { free(c); }

redis_response *redis_command(redis_client *c, const char* fmt,...)
{
	char cmd[128]={0};
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(cmd,127,fmt,ap); // command will be truncated if longer than 127
	va_end(ap);
	//send cmd
	
	//get response

}
void free_response(redis_response *resp) { free(resp); }


/*about network */
static int send_cmd(int fd, const char *cmd,size_t len)
{
	int nsend = write(fd,cmd,len);
	if(nsend == -1)
		return -1;
	while (nsend < len)
	{
		int err = write(fd,cmd,len-nsend);
		if (err<0)
			return -1;
		nsend += err;
	}
	return nsend;
}
static redis_response* get_response(int fd)
{
	redis_response *resp = malloc(sizeof(redis_response));
	char *buf = malloc(1024);
	memset(buf,0,1024); //FIXME: magic number
	int nread = read(fd,buf,1023);
	if(nread == -1)
		return NULL;
	resp->len = nread;
	resp->data = buf;
	return resp;
}
