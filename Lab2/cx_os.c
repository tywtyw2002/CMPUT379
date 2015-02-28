#include <cx_core.h>

/*static void cx_daemon(){

       pid_t pid = fork();

    if (pid < 0 ){
       fprintf(stderr, "error: failed in fork\n");
       exit(EXIT_FAILURE);
    }

    if (pid > 0){
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0){

        fprintf(stderr, "error: failed in sertsid\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    if (( pid = fork() ) < 0) {
        fprintf(stderr, "error: failed in fork\n");
        exit(EXIT_FAILURE);
    }

    umask(0);

    stdin = fopen(NULL,"r");
    stderr = fopen(NULL, "w+");
    stdout = fopen(NULL,"w+");

    chdir("/");

}
*/

/* Check directory */
int check_root_path(char *p)
{

        struct stat s;
        bzero(&s, sizeof(s));

        int err = stat(p, &s);

        if (err == EACCES) {
                fprintf(stderr, "The directory [%s] is denied for read!\n",p);
                return CX_ERROR;
        }

        if (err == ENOENT) {
                fprintf(stderr, "The directory [%s] does not exist!\n",p);
                return CX_ERROR;
        }

        if (!S_ISDIR( s.st_mode)) {
                fprintf(stderr, "The path [%s] is not a firectory!", p);
                return CX_ERROR;
        }

        return CX_OK;
}




int check_file_path(char *p)
{

#ifdef DDEBUG
        fprintf(stderr, "filepath:==%s==\n",p);
#endif

        struct stat s;
        bzero(&s, sizeof(s));
        stat(p, &s);
        if (errno == EACCES)
                return CX_FILE_NREAD;

        if (errno == ENOENT)
                return CX_FILE_NEXIST;

        if (S_ISDIR(s.st_mode))
                return CX_FILE_DIR;

        if (S_ISREG(s.st_mode))
                return CX_FILE_OK;
        else
                return CX_FILE_ERR;


}


int cx_nonblocking_c(cx_connection_t *c)
{
        int n;
        n = 1;
        return ioctl(c->sd, FIONBIO, &n);

}

/* quick set nonblock IO */
int cx_nonblocking(cx_socket_t *s)
{
        int n;

        n = 1;

        return ioctl(s->sd, FIONBIO, &n);

}

/* quick set block IO */
int cx_blocking(cx_socket_t *s)
{

        int n;
        n = 1;
        return ioctl(s->sd, FIONBIO, &n);

}

/*
 * print info when flag DEBUG is set
 *
 *  USAGE: gcc -DDEBUG
 */
void v(char *p)
{
#ifdef DEBUG
        fprintf(stderr,"%s\n",p);
#endif
}


/* bindsocket on http server */
void cx_bind_socket(cx_socket_t *s)
{
        int e;

        /*memset(&s->sock, 0, sizeof(s->sock));*/
        s->sock.sin_family = AF_INET;
        s->sock.sin_addr.s_addr = INADDR_ANY;
        s->sock.sin_port = htons(s->port);

        s->sd = socket(AF_INET, SOCK_STREAM, 0);
        if (s->sd < 0) {
                err(1, "ERROR:Cannot open socket!\n");
                exit(EXIT_FAILURE);
        }

        if(bind(s->sd, (struct sockaddr *) &s->sock, sizeof(s->sock)) == -1) {
                err(1,"bind error");
                exit(EXIT_FAILURE);
        }

        e = listen(s->sd, CX_MAX_FD);
        if (e < 0) {
                fprintf(stderr,"ERROR:CANNOT LISTEN CODE:%d",errno);
                exit(EXIT_FAILURE);
        }

        printf("Server up and listening for connections on port %u\n",
                                                                     s->port);

}


void cx_non_bind_socket(cx_socket_t  *s)
{
        int e;
        int opt = 1;
        /*memset(&s->sock, 0, sizeof(s->sock));*/
        s->sock.sin_family = AF_INET;
        s->sock.sin_addr.s_addr = INADDR_ANY;
        s->sock.sin_port = htons(s->port);

        s->sd = socket(AF_INET, SOCK_STREAM, 0);
        if (s->sd < 0) {
                err(1, "ERROR:Cannot open socket!\n");
                exit(EXIT_FAILURE);
        }

        setsockopt(s->sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        cx_nonblocking(s);

        if(bind(s->sd, (struct sockaddr *) &s->sock, sizeof(s->sock)) == -1) {
                err(1,"bind error");
                exit(EXIT_FAILURE);
        }

        e = listen(s->sd, CX_MAX_FD);
        if (e < 0) {
                fprintf(stderr,"ERROR:CANNOT LISTEN CODE:%d",errno);
                exit(EXIT_FAILURE);
        }

        printf("Server up and listening for connections on port %u\n", 
                                                                    s->port);

}
