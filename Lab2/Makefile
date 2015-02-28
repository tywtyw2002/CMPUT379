CC=gcc
CFLAGS=-c -Wall 
LD=-lpthread
LIBS=./


all: server_f server_p server_s

server_p: cx_server_p.o cx_http.o cx_os.o cx_log.o
	$(CC)  -o server_p $^ $(LD)

server_s: cx_server_s.o cx_http.o cx_os.o cx_log.o
	$(CC)  -o server_s $^ 

server_f: cx_server_f.o cx_http.o cx_os.o  cx_log.o
	$(CC)  -o server_f $^
%.o: %.c 
	$(CC) $(CFLAGS) $(LD) -I$(LIBS) -c -o $@ $<


clean:
	rm -rf *.o server_f server_s server_p
