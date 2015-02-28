#ifndef _CX_LOG_H_INC_
#define _CX_LOG_H_INC_

#include <cx_config.h>
#include <cx_core.h>

#define     CX_LOG_400      "400 Bad Request"
#define     CX_LOG_403      "403 Forbidden"
#define     CX_LOG_404      "404 Not Found"
#define     CX_LOG_200      "200 OK"
#define     CX_LOG_500      "500 Internal Server Error"


#define     CX_LOG_GET      "GET"
#define     CX_LOG_POST     "POST"

#define     CX_LOG_METHOD(r)    if(r->method == CX_HTTP_GET)    CX_LOG_GET \
                                else if(r->method == CX_HTTP_POST) CX_LOG_POST


void cx_log(cx_http_request_t *r);
#endif
