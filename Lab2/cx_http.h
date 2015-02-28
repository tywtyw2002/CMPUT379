/*
 * this file for http functions using
 */

#ifndef _CX_HTTP_H_INC_
#define _CX_HTTP_H_INC_
#include <cx_config.h>
#include <cx_core.h>

#define CX_HTTP_PARSE_INVALID_METHOD        0x1000
#define CX_HTTP_PARSE_INVALID_REQUEST       0x1001



#define CX_HTTP_GET     0x11
#define CX_HTTP_POST    0x22
#define CX_KSIZE        4024

#define CX_HEADER_404  "HTTP/1.1 404 Not Found\r\nServer: CosHiM/1.0\r\n"\
    "Content-Type: text/html\r\nConnection: Close\r\n\r\n<h1>Not found</h1>"

#define CX_HEADER_400 "HTTP/1.1 400 Bad Request\r\nServer: CosHiM/1.0\r\n" \
    "Content-Type: text/html\r\nConnection: Close\r\n\r\n<h1>Bad request</h1>"

#define CX_HEADER_403  "HTTP/1.1 403 Forbidden\r\nServer: CosHiM/1.0\r\n"\
    "Content-Type: text/html\r\nConnection: Close\r\n\r\n<h1>Forbidden</h1>"

#define CX_HEADER_200 "HTTP/1.1 200 OK\r\nServer: CosHiM/1.0\r\n"\
    "Content-Type: text/html\r\nConnection: Close\r\n\r\n"


struct cx_socket_s {
        struct sockaddr_in sock;
        u_short port;
        int     sd;
        int     max;
};

struct cx_buf_s {
        u_char      *pos; /* current pos */
        u_char      *last; /* the end of buff */

        u_char      buf[CX_KSIZE];

        size_t      size;
        size_t      read; /*the left to read/write*/

};


struct cx_connection_s {
        int sd;   /* The socket for this conn */
        int state;
        struct sockaddr_in sa;
        socklen_t len;

        const char  *path;
        FILE  *log_fp;
        int     thread_id;

};

typedef struct cx_connection_s cx_connection_t;


struct cx_http_request_s {

        cx_connection_t     *connection;


        uint32_t            state;
        uint32_t            method;
        uint32_t            code;
        uint32_t            length;
        uint32_t            fd;
        uint32_t            offset;


        off_t               fpos;

        u_char              *request_start;
        u_char              *method_end;
        u_char              *uri_start;
        u_char              *uri_end;
        u_char              *schema_start;
        u_char              *scheme_end;
        u_char              *first_end;
        u_char              *pos;

};

typedef struct cx_http_request_s cx_http_request_t;

struct cx_slots_s {
        cx_http_request_t    *r;
        cx_connection_t      c;
        int                  sd;

};

struct cx_request_node_s {

        cx_connection_t c;
        struct cx_request_node_s *next;

};

typedef struct cx_request_node_s cx_request_node_t;

struct cx_request_list_s
{
        cx_request_node_t  *head;
        cx_request_node_t  *tail;
        int                 size;    
};

typedef struct cx_buf_s cx_buf_t;

typedef struct cx_socket_s cx_socket_t;
typedef struct cx_slots_s cx_slots_t;
typedef struct cx_request_list_s cx_request_list_t;


int cx_http_parse_request_line(cx_http_request_t *r, cx_buf_t *b);
int cx_http_request_read(cx_buf_t *b, cx_connection_t *c);
void cx_http_request_process( cx_http_request_t *p);
void cx_http_request_write(cx_http_request_t *r );
void cx_http_request_sendbody(cx_http_request_t *r);
void cx_http_write(cx_http_request_t *r, char *buf, int n);
void cx_http_handle(cx_connection_t *c);
void cx_http_poll_handle(cx_slots_t *p);
#endif
