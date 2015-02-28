#include <cx_config.h>
#include <cx_core.h>
#include <pthread.h>

#define  CX_MAX_THREAD     32
#define  CX_MAX_QUEUE_SIZE    1000

int done = 0;
pthread_mutex_t p_mutex;
pthread_cond_t p_cond;

cx_request_list_t   queue[1];

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

void *cx_pthread_handle(void *id)
{
        int thread_id = *(int *) id;
        cx_request_node_t *node;

        /* fprintf(stderr, "[%d] Thread Start.....\n",thread_id );
        */

        while (!done){

                pthread_mutex_lock(&p_mutex);

                /*check size again */
                if (queue->size == 0){
                    pthread_mutex_unlock(&p_mutex);
                    usleep(1000);
                    continue;
                }
                

                /*
                fprintf(stderr, "[%d] Thread process.....QUEUE[%d]\n",
                                                        thread_id,queue->size);
                */
                queue->size--;
                if(queue -> size == 0){
                    node = queue->head;
                    /*queue->head = NULL;*/
                }else{
                    node = queue->head;
                    queue->head = node->next;
                }
                

                pthread_mutex_unlock(&p_mutex);
                node->c.thread_id = thread_id;
                cx_http_handle(&node->c);
                free(node);
                /* fprintf(stderr, "[%d] Thread Done.....\n",thread_id ); */

        }
        return 0; 
}


int main(int argc, const char *argv[])
{

        char *ep;
        u_long p;
        cx_socket_t s;
        int i;
        int thread_id[CX_MAX_THREAD];
        pthread_t thread[CX_MAX_THREAD];
        cx_connection_t  *c;
        // cx_request_list_t   queue[1];
        struct sigaction ac;
        memset(&ac, 0, sizeof(struct sigaction));
        ac.sa_handler = kill_server;
        pthread_cond_init(&p_cond, NULL);
        memset(queue, 0, sizeof(queue));

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



        /*start thread */
        for (i=0; i< CX_MAX_THREAD; i++){

            thread_id[i] = i;
            pthread_create(&thread[i], NULL, (void *) cx_pthread_handle,
                                                     (void *) &thread_id[i]);
        }


        /* start loop */
        while(!done) {

                /*check the queue size */
                if (queue->size > CX_MAX_QUEUE_SIZE){
                    usleep(100);
                    continue;
                }

                cx_request_node_t   *node = malloc( sizeof(cx_request_node_t));

                /* client handle */
                memset(node,0, sizeof(cx_request_node_t));
                c = &node->c;
                c->len = sizeof(c->sa);
                c->sd = accept(s.sd, (struct sockaddr *)&c->sa, &c->len);
                fprintf(stderr, "FD:[%d]\n", c->sd );
                if (c->sd == -1 && errno != 4) {
                        err(1, "accpet failed!");
                } else if(errno == 4) {
                        break;
                }

                c->path = argv[2];
                c->log_fp = log_fp;

                /* add node to queue */
                pthread_mutex_lock(&p_mutex);
                
                if (queue->size == 0){
                        queue->tail = node;
                        queue->head = node;
                }else{
                        queue->tail->next = node;
                        queue->tail = node;
                }
                queue->size++;    

                pthread_mutex_unlock(&p_mutex);

        }


        close(s.sd);
        fclose(log_fp);
        return 0;
}

