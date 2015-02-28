/*
 * Some code idea from nginx version 0.5.
 * Author: Tianyi Wu
 *
 * This file use for parse the request line of HTTP Request.
 */
#include <cx_core.h>


#define cx_str4cmp(m, c0, c1, c2, c3)                                       \
             *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)


int
cx_http_parse_request_line(cx_http_request_t * r, cx_buf_t * b)
{

        u_char          ch,
                        *p,
                        *m;

        /*
         * the status for http parse
         */
        enum {
                s_start = 0,
                s_method,
                s_space_before_uri,
                s_http_version,
                s_check_uri,
                s_done
        } state;

        /*
         * v("start parse!");
         */
        state = 0;

        for (p = b->pos; p < b->last; p++) {
                ch = *p;        // current char.

                switch (state) {

                        /*
                         * just method: GET & POST
                         */
                case s_start:

                        r->request_start = p;

                        if (ch == CR || ch == LF) {
                                break;
                        }


                        /*
                         * check for method
                         */
                        if (ch < 'A' || ch > 'Z') {
                                return CX_HTTP_PARSE_INVALID_METHOD;
                        }


                        state = s_method;
                        break;


                case s_method:
                        /*
                         * find the space in the first line of the http header
                         */

                        /*
                         *  GET /xxxxxxx HTTP/1.1
                         *  so the method should be in range p - m
                         */

                        if (ch == ' ') {
                                r->method_end = p - 1;
                                m = r->request_start;
                        } else {
                                break;
                        }

                        switch (p - m) {

                        case 3:
                                if (cx_str4cmp(m, 'G', 'E', 'T', ' ')) {
                                        r->method = CX_HTTP_GET;
                                        break;
                                }

                        case 4:
                                if (cx_str4cmp(m, 'P', 'O', 'S', 'T')) {
                                        r->method = CX_HTTP_POST;
                                        break;
                                }
                        }

                        state = s_space_before_uri;
                        break;

                        if (ch < 'A' || ch > 'Z') {
                                return CX_HTTP_PARSE_INVALID_METHOD;
                        }

                        break;

                case s_space_before_uri:

                        /*
                         * To find which file in http request
                         */
                        if (ch == '/') {
                                r->uri_start = p;
                                state = s_check_uri;
                                break;
                        }


                        switch (ch) {
                        case ' ':
                                break;

                        default:
                                return CX_HTTP_PARSE_INVALID_REQUEST;

                        }

                        break;


                        /*
                         * check the URI and save it
                         */
                case s_check_uri:
                        /*
                         * v("check_uri") ;
                         */
                        switch (ch) {

                        case ' ':
                                r->uri_end = p;
                                state = s_http_version;
                                break;

                        case CR:
                                break;

                        case LF:
                                r->uri_end = p - 1;
                                state = s_done;
                                break;

                        default:
                                break;

                        }

                        break;

                case s_http_version:
                        /*
                         * v("s_http_v");
                         */
                        switch (ch) {

                        case CR:
                                break;

                        case LF:
                                r->first_end = p - 1;
                                state = s_done;
                                break;

                        }

                        break;


                case s_done:

                        return CX_OK;
                }


        }

        return CX_ERROR;
}


int
cx_poll_http_request_read(cx_buf_t * b, cx_connection_t * c)
{

        ssize_t         count;
        while (b->size < CX_KSIZE){

            count = read(c->sd, b->buf + b->size, CX_KSIZE - b->size);
            v("[cx_http_re_read]: reading");

            if (count == -1){
                    if (errno != EAGAIN) {
                            err(1, "error: read request error\n");
                            return CX_ERROR;
                    }else{

                            break;
                    }

            }
            
            b->size += count;

        }
        b->pos = b->buf;
        b->last = b->buf + b->size;
        return CX_OK;
}



int
cx_http_request_read(cx_buf_t * b, cx_connection_t * c)
{

        ssize_t         count;


        count = read(c->sd, b->buf, CX_KSIZE);
        v("[cx_http_re_read]: reading");
        /* fprintf(stderr, "[%d][READ] FD:%d\n", c->thread_id, c->sd); */
        if (count < 0) {
                err(1, "[%d]error: read request error [%d]",c->thread_id, c->sd);
                return CX_ERROR;
        }
        b->size = count;
        b->pos = b->buf;
        b->last = b->buf + b->size;
        return CX_OK;
}



void
cx_http_request_process(cx_http_request_t * p)
{
        v("http_reeq_proce");
        /*
         * check file first
         */
        char           *l;
        char            file_path[128];
        int             e;
        struct stat     f;

        bzero(file_path, 128);

        /*
         * copy the path
         */
        strncpy(file_path, p->connection->path, strlen(p->connection->path));
        l = file_path + strlen(p->connection->path);
        strncpy(l, (char *) p->uri_start, p->uri_end - p->uri_start);
        l += (p->uri_end - p->uri_start);
        *(++l) = '\0';

#ifdef DDEBUG
        /*
         * print file infomations
         */
        
        fprintf(stderr,"localtion:[%s]\n",p->connection->path);
        fprintf(stderr, "fileinfo: %s\n", file_path);
#endif

        e = check_file_path(file_path);
        switch (e) {

        case CX_FILE_OK:
                stat(file_path, &f);
                p->length = f.st_size;
                p->fd = open(file_path, O_RDONLY);
                p->fpos = 0;
                p->code = 200;

                break;
        case CX_FILE_NEXIST:
                /*
                 * 404
                 */
                p->code = 404;
                break;

        case CX_FILE_DIR:
                /*
                 * 403
                 */
                p->code = 403;
                break;

        case CX_FILE_NREAD:
                /*
                 * 403
                 */
                p->code = 403;
                break;

        case CX_FILE_ERR:
        default:
                /*
                 * 400
                 */
                p->code = 400;
                return;
        }
}



void
cx_http_request_write(cx_http_request_t * r)
{

        v("r write!");
        switch (r->code) {

        case 200:
                v("200");
                cx_http_write(r, CX_HEADER_200, sizeof(CX_HEADER_200));
                cx_http_request_sendbody(r);
                break;

        default:
        case 400:
                v("400");
                cx_http_write(r, CX_HEADER_400, sizeof(CX_HEADER_400));
                break;

        case 403:
                v("403");
                cx_http_write(r, CX_HEADER_403, sizeof(CX_HEADER_403));
                break;

        case 404:
                v("404");
                cx_http_write(r, CX_HEADER_404, sizeof(CX_HEADER_404));
                break;

        }


}

void
cx_http_request_sendbody(cx_http_request_t * r)
{


        off_t           s = r->fpos;

        v("HTTP_SENDBODY!");
        /*
        fprintf(stderr, "[%d]FILE SIZE: %d, fpos: %d, RM: %d\n", 
                                             r->connection->sd, r->length,
                                             r->fpos, r->length - r->fpos);
                                             */
        while (r->fpos != r->length) {

                s = sendfile(r->connection->sd, r->fd, &s, r->length -r->fpos);
                /*
                fprintf(stderr, "[%d]S: %d, fpos: %d, RM: %d\n", 
                                                 r->connection->sd, s,
                                                 r->fpos, r->length - r->fpos);
                                                 */
                if (s == -1) {
                        if (errno != EAGAIN) {
                                err(1, "error: sendbody \n");
                                return;
                        } else {
                                if(r->connection->state != 0){
                                    v("set CX_POLL_WRITE");
                                    r->connection->state = CX_POLL_WRITE;
                                    return;
                                }
                                v("ERR"); 
                        }
                }
                r->fpos += s;

        }
        if (r->connection->state == CX_POLL_WRITE){
                v("SEND FILE SET DONE");
                r->connection->state = CX_POLL_DONE;
        }
}



void
cx_http_write(cx_http_request_t * r, char *buf, int n)
{
        v("entry http_write!");
        int             e;

        
        while (r->offset != n) {

                e = write(r->connection->sd, buf + r->offset, n - r->offset);
                if (e == -1) {
                        if (e != EAGAIN) {
                                err(1, "[%d]error: cx_http_write write [%d]",
                                r->connection->thread_id,  r->connection->sd);
                                return;
                        }

                } else {
                        r->offset += e;
                }
        
        }

        if(r->connection->state != 0 && r->code != 200){
                r->connection->state = CX_POLL_DONE;
        }
}





/* http handle */
void
cx_http_handle(cx_connection_t * c)
{

        cx_http_request_t r;
        cx_buf_t        b;
        int             e;

        memset(&r, 0, sizeof(cx_http_request_t));
        memset(&b, 0, sizeof(cx_buf_t));

        r.connection = c;

        v("[Handle] Parse for reading ");
        /*
         * read requesttst
         */
        if (cx_http_request_read(&b, r.connection) != CX_OK) {
                v("[Handle] read error!");
                //return;
        }
        v("[Handle] finish read");

        /*
         * prase http header
         */
        if ((e = cx_http_parse_request_line(&r, &b)) != CX_OK) {
                r.code = 400;
                v("[Handle] http parse request line 400!");
        } else {
                v("[Handle] start http request process!");
                cx_http_request_process(&r);
        }


        /* write handle and send file */
        cx_http_request_write(&r);
        cx_log(&r);
        if(r.code == 200){
                close(r.fd);
        }
        close(r.connection->sd);
        v("[Handle] FINISHED!");

}

void 
cx_cleanup(cx_slots_t *p)
{

        v("CLEAN UP");
        bzero(p, sizeof(cx_slots_t));
        p->sd = -1;
}




/*handle for poll */
void
cx_http_poll_handle(cx_slots_t *p)
{

        int             e;

        if (p->c.state == CX_POLL_READ){

                cx_http_request_t *r = malloc( sizeof(cx_http_request_t ));
                cx_buf_t        *b = malloc(sizeof(cx_buf_t));

                memset(r, 0, sizeof(cx_http_request_t));
                memset(b, 0, sizeof(cx_buf_t));

                
                r->connection = &p->c;
                p->r = r;

                v("[Handle] Parse for reading ");
                /*
                 * read requesttst
                 */
                if (cx_http_request_read(b, r->connection) != CX_OK) {
                        v("[Handle] read error!");
                        cx_cleanup(p);
                        close(p->r->connection->sd);
                        return;
                }
                v("[Handle] finish read");

                /*
                 * prase http header
                 */
                if ((e = cx_http_parse_request_line(r, b)) != CX_OK) {
                        r->code = 400;
                        v("[Handle] http parse request line 400!");
                } else {
                        v("[Handle] start http request process!");
                        cx_http_request_process(r);
                }
                /* write handle and send file */
                p->c.state = CX_POLL_WRITE;
                cx_http_request_write(r);

        } else if(p->c.state == CX_POLL_WRITE){
                v("continue send");
                /*fprintf(stderr, "HANDLE FD[%d]\n", p->r->connection->sd);*/
                cx_http_request_sendbody(p->r);

        }

        fprintf(stderr, "[%d]\n", p->c.state);

        if(p->c.state == CX_POLL_DONE){
                v("STATUS DONE");
                cx_log(p->r);
                if (p->r->code == 200){
                        close(p->r->fd);
                }
                close(p->r->connection->sd);
                v("[Handle] FINISHED!");
                cx_cleanup(p);

        }

}