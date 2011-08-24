all=libscredis.so
all:$(all)
libscredis.so:scredis.h scredis.c
	gcc -Wall -g -fPIC -shared -o libscredis.so scredis.c
clean:
	rm $(all)
