#include <cx_config.h>
#include <cx_core.h>

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




int main(int argc, const char *argv[])
{

        char *ep;
        u_long p;
        cx_socket_t s;
        pid_t pid;
        struct sigaction ac;
        memset(&ac, 0, sizeof(struct sigaction));
        ac.sa_handler = kill_server;


        /* check the arguments */
        if (argc != 4)
                usage();

        p = strtoul(argv[1], &ep, 10);

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
        s.port = (u_short) p;

        /* bind socket */
        cx_bind_socket(&s);
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
        /* start loop */
        while(!done) {

                /* client handle */
                cx_connection_t     c;
                memset(&c,0, sizeof(cx_connection_t));
                c.len = sizeof(c.sa);
                c.sd = accept(s.sd, (struct sockaddr *)&c.sa, &c.len);
                if (c.sd == -1 && errno != 4) {
                        err(1, "accpet failed!");
                } else if(errno == 4) {
                        break;
                }

                pid = fork();
                if (pid == 0) {
                        fprintf(stderr, "fork:[%d]\n", getpid());
                        c.path = argv[2];
                        c.log_fp = log_fp;
                        cx_http_handle(&c);
                        exit(0);
                        v("should not show!");
                }
                close(c.sd);
        }


        close(s.sd);
        fclose(log_fp);
        return 0;
}
