#ifndef CX_OS_H_INC_
#define CX_OS_H_INC_
#include <cx_config.h>
#include <cx_core.h>


#define CX_FILE_OK       0
#define CX_FILE_DIR     -1
#define CX_FILE_NEXIST  -2
#define CX_FILE_NREAD   -3
#define CX_FILE_ERR     -5




int check_root_path(char *p);
int check_file_path(char *p);
void cx_bind_socket(cx_socket_t *s);
int cx_nonblocking(cx_socket_t *s);
int cx_blocking(cx_socket_t *s);
int cx_nonblocking_c(cx_connection_t *c);
void cx_non_bind_socket(cx_socket_t *s);
void v(char *p);
#endif
