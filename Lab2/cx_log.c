#include <cx_core.h>


void cx_log(cx_http_request_t *r)
{
        v("cx_log entry");
        time_t      rawtime;
        struct tm * timeinfo;
        char buffer[80];
        char *ip;
        char header[80];
        memset(&header, 0, 80);
        
        time(&rawtime);
        timeinfo = gmtime(&rawtime);
        /*Tue 05 Nov 2013 19:18:34 GMT */
        strftime (buffer, 80, "%a %d %b %Y %T %Z",timeinfo);

        ip = inet_ntoa(r->connection->sa.sin_addr);

        strncpy(header, (char *) r->request_start, 
                                r->first_end - r->request_start);

        switch (r->code) {

        case 200:
                fprintf(r->connection->log_fp, "%s\t%s\t%s\t%s %d/%d\n",
                                                        buffer,ip, header,
                                                CX_LOG_200, (int) r->fpos,
                                                                r->length );
                break;

        case 400:
                fprintf(r->connection->log_fp, "%s\t%s\t%s\t%s\n",
                                                buffer,ip, header ,CX_LOG_400);
                break;

        case 403:
                fprintf(r->connection->log_fp, "%s\t%s\t%s\t%s\n",
                                                buffer,ip, header ,CX_LOG_403);
                break;

        case 404:
                fprintf(r->connection->log_fp, "%s\t%s\t%s\t%s\n",
                                               buffer, ip, header, CX_LOG_404);
                break;

        }

        fflush(r->connection->log_fp);

}
