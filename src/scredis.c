#include "scredis.h"

#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
static void pack_cmd(const char *cmd,size_t cmdsize, char *packed,size_t *packsize);
/*about network */
static int send_cmd(int fd, const char *cmd,size_t len)
{
	char packedcmd[1024] = {0};
	size_t size = 0;
	pack_cmd(cmd,len,packedcmd,&size);
	int nsend = write(fd,packedcmd,size);
	if(nsend == -1)
		return nsend;
	while (nsend < len)
	{
		int err = write(fd,packedcmd,size-nsend);
		if (err<0)
			return err;
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
		//die("getaddrinfo:%s",gai_strerror(rv));
		return NULL;
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((s = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
			continue;
		if (connect(s,p->ai_addr,p->ai_addrlen) == -1)
			//die("connect:%s",strerror(errno)); //FIXME: don't die
			continue;
		client->fd = s;
	}
	return client;

}

void free_client(redis_client *c) { if(c->fd > 0) close(c->fd) ;free(c); }

/*about command */
static void pack_cmd(const char *cmd,size_t cmdsize, char *packed,size_t *packsize)
{
	const char *delim = "\r\n";
	char packedcmd[1024]={0};
	int cur = 0;
	int i = 0,argc = 0,per = 0;
	int prelen = 0;
	while(i < cmdsize) {
		while(cmd[i] && isspace(cmd[i])){++i;} // skip spaces
		prelen = i;
		per = 0;
		while((cmd[i] != '\0') && !isspace(cmd[i++]))++per;
		sprintf(packedcmd+cur,"%s$%d\r\n%s",delim,per,strndup(cmd+prelen,i-prelen));
		cur += strlen(packedcmd+cur);
		++ argc;
	}
	sprintf(packedcmd+cur,"%s",delim);
	char num[10]={0};
	snprintf(num,9,"%d",argc);
	//char packed[2048]={0};
	packed[0]='*';
	strcat(packed,num);
	strcat(packed,packedcmd);
	*packsize = strlen(packed);
}
redis_response *redis_command(redis_client *c, const char* fmt,...)
{
	char cmd[128]={0};
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(cmd,127,fmt,ap); // command will be truncated if longer than 127
	va_end(ap);
	//send cmd
	int len = strlen(cmd);
	if(send_cmd(c->fd,cmd,len)<0)
		return NULL;
	//get response
	return (redis_response *)get_response(c->fd);

}
void free_response(redis_response *resp) { free(resp); }


