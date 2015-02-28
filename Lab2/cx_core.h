#ifndef CX_CORE_H_INC_
#define CX_CORE_H_INC_

//#include <cx_http_request.h>
#include <cx_http.h>
#include <cx_os.h>
#include <cx_log.h>


#define LF      (u_char) 10
#define CR      (u_char) 13
#define CRLF    "\x0d\x0a"


#define CX_OK       0
#define CX_ERROR    -1

#define CX_MAX_FD    1000
#define CX_POLL_READ       1
#define CX_POLL_WRITE 	   2
#define CX_POLL_DONE 	   3
#endif


