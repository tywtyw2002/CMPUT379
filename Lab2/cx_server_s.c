#include <cx_config.h>
#include <cx_core.h>
#include <pthread.h>

#define CX_MAX_EVENTS  256
#define CX_NO_SOCK   -1

int done = 0;



void kill_server(int s)
{

        fprintf(stderr, "Server Shutdown now!\n");
        done = 1;
}


static void usage()
{

        extern char * __progname;
        fprintf(stderr,"usage: %s [port] [work_dir] [log_file]\n", __progname);
        exit(EXIT_FAILURE);

}




cx_slots_t* cx_find_empty_slot(cx_slots_t p[])
{
        int i;

        for (i=0; i< CX_MAX_EVENTS; i++) {
                if(p[i].sd == CX_NO_SOCK) {
                        return &p[i];
                }
        }

        return NULL;
}


cx_slots_t* cx_find_slot(int fd, cx_slots_t p[])
{

        int i;
        for (i=0; i < CX_MAX_EVENTS; i++) {
                if(p[i].sd == fd) {
                        return &p[i];
                }
        }

        return NULL;
}

int main(int argc, const char *argv[])
{

        char *ep;
        u_long xp;
        cx_socket_t s;
        struct sigaction ac;
        cx_slots_t *p;
        int i;
        int efd;
        struct epoll_event   event;
        struct epoll_event   *events;
        cx_slots_t      slots[CX_MAX_EVENTS];

        memset(&ac, 0, sizeof(struct sigaction));
        ac.sa_handler = kill_server;


        /* check the arguments */
        if (argc != 4)
                usage();

        xp = strtoul(argv[1], &ep, 10);

        if (*argv[1] == '\0' || *ep != '\0' ) {
                fprintf(stderr, "error: [%s] isnot a valid port number!\n", 
                                                                    argv[1]);
                exit(EXIT_FAILURE);
        }


        if (check_root_path((char *)argv[2]) != CX_OK) {
                exit(EXIT_FAILURE);
        }

        FILE *log_fp;

        log_fp = fopen(argv[3],"a+");
        if ( log_fp == NULL) {
                fprintf(stderr, "error: cannot open [%s] for log_file!\n",
                                                                    argv[3]);
                exit(EXIT_FAILURE);
        }

        v("CHECK ARGUMENT OK!");


        memset(&s, 0, sizeof(s));
        s.port = (u_short) xp;

        /* bind socket */
        cx_non_bind_socket(&s);
        v("CHECK BIND OK");

        /* create deamon */
#ifndef DEBUG
        if (daemon(1,0) == -1) {
                fprintf(stderr, "ERROR: daemon() failed!!!!!! \n");
                exit(EXIT_FAILURE);
        }
#endif


        /* bind term sign */
        sigaction(SIGTERM, &ac, NULL);
        sigaction(SIGINT,&ac, NULL);
        signal(SIGCHLD, SIG_IGN);

        /* empty all solt */
        for (i = 0; i< CX_MAX_EVENTS; i++) {
                bzero(&slots[i], sizeof(cx_slots_t));
                slots[i].sd = CX_NO_SOCK;
        }


        /* epoll */

        efd = epoll_create1(0);
        if (efd == -1) {
                err(1,"epoll failed!");
                exit(EXIT_FAILURE);
        }

        event.data.fd = s.sd;
        event.events = EPOLLIN;

        epoll_ctl(efd, EPOLL_CTL_ADD, s.sd, &event);

        events = malloc(CX_MAX_EVENTS * sizeof(event));
        bzero(events, CX_MAX_EVENTS * sizeof(event));

        while(!done) {

                v("wait for poll()");

                int n, i;

                n = epoll_wait(efd, events, CX_MAX_EVENTS, -1);

                fprintf(stderr, "N = %d\n", n);
                for ( i = 0; i < n; i++) {
                        if( ( events[i].events & EPOLLERR) ||
                            (events[i].events & EPOLLHUP)) {

                                err(1,"epoll in error!");
                                close(events[i].data.fd);
                                continue;

                        } else if (events[i].data.fd == s.sd) {

                                while (!done) {
                                        p = cx_find_empty_slot(slots);

                                        /* too many socket! */
                                        if (p == NULL) {
                                                struct sockaddr in_addr;
                                                socklen_t in_len;
                                                int infd;
                                                in_len = sizeof(in_addr);
                                                infd = accept(s.sd,
                                                            &in_addr, &in_len);
                                                close(infd);
                                                break;
                                        }

                                        p->c.len = sizeof(p->c.sa);
                                        p->c.sd = accept(s.sd, 
                                           (struct sockaddr *)&p->c.sa, 
                                                                &p->c.len);

                                        if (p->c.sd == -1) {
                                                if (errno == EAGAIN || 
                                                    errno == EWOULDBLOCK) {
                                                        break;
                                                } else {
                                                        err(1,"ACCEPT ERROR!");
                                                        break;
                                                }

                                        }

                                        p->sd = p->c.sd;
                                        p->c.state = CX_POLL_READ;
                                        cx_nonblocking_c(&p->c);
                                        fprintf(stderr, "%d->%p\n", p->sd, p);
                                        event.data.fd = p->sd;
                                        event.events = EPOLLIN | EPOLLET ;
                                        epoll_ctl (efd, EPOLL_CTL_ADD, 
                                                                p->sd, &event);
                                        fprintf(stderr, "REG FD[%d]\n",p->sd);
                                }
                                continue;
                        } else if (events[i].events & EPOLLIN){
                                p = cx_find_slot(events[i].data.fd, slots);

                                if (p == NULL) {
                                        continue;
                                }

                                p->c.path = argv[2];
                                p->c.log_fp = log_fp;

                                v("EPOLLIN, START HANDLE!");
                                cx_http_poll_handle(p); 

                                if (p->c.state == CX_POLL_WRITE){
                                        event.data.fd = p->sd;
                                        event.events =  EPOLLIN | EPOLLOUT;
                                
                                        epoll_ctl (efd, EPOLL_CTL_MOD, p->sd, 
                                                                    &event);
                                }else if(p->c.state == CX_POLL_DONE){
                                        epoll_ctl (efd, EPOLL_CTL_DEL, p->sd,
                                                                        NULL);
                                }
                                

                               
                                /* set pollout event */
                                // if (p->c.state == CX_POLL_WRITE){
                                //         v("EPOLLIN SET POLLOUT!");
                                //         event.data.fd = p->sd;
                                //         event.events = EPOLLOUT;
                                //         epoll_ctl (efd, EPOLL_CTL_ADD, 
                                //                             p->sd, &event);
                                // }
                                /* register the OUT */
                                continue;
                                
                        } else if (events[i].events & EPOLLOUT){
                                v("EPILLOUT HANDLE");
                                fprintf(stderr, "EPOLLOUT FD[%d]\n", 
                                                        events[i].data.fd );
                                p = cx_find_slot(events[i].data.fd, slots);

                                fprintf(stderr, "EPOLLOUT FD[%d][%d]\n", p->sd,
                                                                      p->c.sd);
                                if (p == NULL) {
                                        v("EPOLLOUT continue");
                                        continue;
                                }
                                if (p->c.state == CX_POLL_WRITE)
                                        cx_http_poll_handle(p);

                                if (p->c.state == CX_POLL_DONE){
                                        v("EPOLL CLEAN");
                                        epoll_ctl (efd, EPOLL_CTL_DEL, p->sd,
                                                                        NULL);

                                }
                                        
                        }

                }
        }

        free(events);
        close(s.sd);
        fclose(log_fp);
        return 0;
}

