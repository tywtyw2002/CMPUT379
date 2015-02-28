#include "saucer.h"
#include "ascii.h"

struct system_info sys_info; /* save for the current level game info */
player_info player[1];
int exit_flag = 1;  /* check gameover */
int fire_flag = 0;  /* launch rocket */
int game_flag = 1;  /* levelup check */
int end_flag = 1;   /* end this game */
int *rkt;           /* the rocket size */
/*saucer_node *node_matrix[MAXROW];*/



/*
 * Set the current level game infomation and config.
 */
void set_level_info()
{
        int r;
        sys_info.saucer_miss = 0;
        sys_info.saucer_destory = 0;

        switch (sys_info.level) {

        case 1:
                sys_info.level_up_hit = LEVEL_1_HIT;
                sys_info.rocket_left = LEVEL_1_R_MAX;
                sys_info.rocket_max = LEVEL_1_R_MAX;
                sys_info.life = LEVEL_1_LIFE;
                sys_info.saucer_speed = LEVEL_1_S_SPEED;
                sys_info.saucer_num   = LEVEL_1_S_NUM;
                sys_info.saucer_limit = LEVEL_1_S_LIM;
                break;

        case 2:
                sys_info.level_up_hit = LEVEL_2_HIT;
                sys_info.rocket_left = LEVEL_2_R_MAX;
                sys_info.rocket_max = LEVEL_2_R_MAX;
                sys_info.life = LEVEL_2_LIFE;
                sys_info.saucer_speed = LEVEL_2_S_SPEED;
                sys_info.saucer_num   = LEVEL_2_S_NUM;
                sys_info.saucer_limit = LEVEL_2_S_LIM;
                break;

        case 3:
                sys_info.level_up_hit = LEVEL_3_HIT;
                sys_info.rocket_left = LEVEL_3_R_MAX;
                sys_info.rocket_max = LEVEL_3_R_MAX;
                sys_info.life = LEVEL_3_LIFE;
                sys_info.saucer_speed = LEVEL_3_S_SPEED;
                sys_info.saucer_num   = LEVEL_3_S_NUM;
                sys_info.saucer_limit = LEVEL_3_S_LIM;

                break;


        case 4:
                sys_info.level_up_hit = LEVEL_4_HIT;
                sys_info.rocket_left = LEVEL_4_R_MAX;
                sys_info.rocket_max = LEVEL_4_R_MAX;
                sys_info.life = LEVEL_4_LIFE;
                sys_info.saucer_speed = LEVEL_4_S_SPEED;
                sys_info.saucer_num   = LEVEL_4_S_NUM;
                sys_info.saucer_limit = LEVEL_4_S_LIM;

                break;

        /* If level great than 4, go here, just change the hit range and rocket
         * number
         */
        default:
                if (sys_info.pass) {
                        r = rand();
                        sys_info.level_up_hit = sys_info.level_up_hit +  r % 5;
                        sys_info.rocket_left = sys_info.rocket_left + r % 5;
                        sys_info.rocket_max = sys_info.rocket_left;
                        sys_info.life = LEVEL_N_LIFE;
                }
                sys_info.saucer_speed = LEVEL_N_S_SPEED;
                sys_info.saucer_num   = LEVEL_N_S_NUM;
                sys_info.saucer_limit = LEVEL_N_S_LIM;

        }


}



/* To init a new saucer */
saucer_node * build_saucer()
{
        int r;
        saucer_node *node = malloc(sizeof(saucer_node));
        bzero(node, sizeof(saucer_node));

        r = rand();
        node->y = r % MAXROW;
        node->live = 1;
        node->x = 0;
        node->speed = S_SPEED - (r % 8) * sys_info.saucer_speed;
        pthread_mutex_lock( &sx);
        move(node->y, node->x);
        addstr(SCHAR);
        refresh();
        pthread_mutex_unlock(&sx);

        return node;
}

/* add saucer node to saucer list */
void add_saucer_node(saucer_list *l, saucer_node *n)
{
        if (l->size == 0) {
                l->size = 1;
                l->head = n;
                l->tail = n;
        } else {
                l->size++;
                l->tail->next = n;
                n->prev = l->tail;
                l->tail = n;
        }
}

/* remove saucer node to saucer list */
void remove_saucer_node(saucer_list *l, saucer_node *n)
{

        if(l->size == 1) {
                l->size = 0;
                l->head = NULL;
                l->tail = NULL;
        } else {
                l->size--;
                if (l->head == n) {
                        l->head = n->next;
                        l->head->prev = NULL;
                } else if(l->tail == n) {
                        l->tail = n->prev;
                        l->tail->next = NULL;
                } else {
                        n->prev->next = n->next;
                        n->next->prev = n->prev;
                }
        }
        free(n);
}


/* Manager all saucer nodes */
void *saucerManager(void * list)
{
        saucer_node *s;
        saucer_list *saucer = (saucer_list *) list;
        int r;

        while(exit_flag && game_flag) {
                usleep(TUNIT);

                //add saucer
                if (saucer->size < sys_info.saucer_limit) {
                        //roll to decided to add or not.
                        r = rand();
                        if(r % 100 > 95) {
                                s = build_saucer();
                                add_saucer_node(saucer, s);
                                pthread_create(&s->thd, NULL, saucerThread, s);
                                r = rand();
                                if (sys_info.saucer_limit <= sys_info.saucer_num) {
                                        sys_info.saucer_limit ++;
                                }
                        }
                }

                //check node status
                if (saucer->size == 0)
                        continue;


                for (s = saucer->head; s != NULL; s = s->next) {
                        //check the saucer is alive or not.
                        if (s->live == 0) {
                                //miss
                                pthread_mutex_lock(&sysx);
                                sys_info.saucer_miss++;
                                pthread_mutex_unlock(&sysx);

                                //remove this node;
                                pthread_mutex_lock(&sx);
                                remove_saucer_node(saucer, s);
                                mvprintw(LINES-1, 0, "MISS: %d ", sys_info.saucer_miss);
                                refresh();
                                pthread_mutex_unlock(&sx);
                        }

                }

        }

        //clear thread when exit.
        while(saucer->size) {
                s = saucer->head;
                pthread_cancel(s->thd);

                saucer->size--;
                s = s->next;
                free(saucer->head);

                saucer->head = s;
        }

        pthread_exit(NULL);
}

/* add rocket node to rocket list */
void add_rocket_node(rocket_list *l, rocket_node *n)
{
        if (l->size == 0) {
                l->size = 1;
                l->head = n;
                l->tail = n;
        } else {
                l->size++;
                l->tail->next = n;
                n->prev = l->tail;
                l->tail = n;
        }
}

/* remove rocket node to rocket list */
void remove_rocket_node(rocket_list *l, rocket_node *n)
{

        if(l->size == 1) {
                l->size = 0;
                l->head = NULL;
                l->tail = NULL;
        } else {
                l->size--;
                if (l->head == n) {
                        l->head = n->next;
                        l->head->prev = NULL;
                } else if(l->tail == n) {
                        l->tail = n->prev;
                        l->tail->next = NULL;
                } else {
                        n->prev->next = n->next;
                        n->next->prev = n->prev;
                }
        }
        free(n);
}

/* Manager the rocket thread(node) */
void * rocketManager(void * list)
{
        int i;
        struct arg_list *arg = (struct arg_list *)list ;
        rocket_node *r;
        rocket_list *rocket = arg->r;
        saucer_list *saucer = arg->s;
        saucer_node *s;

        rkt = &rocket->size;  /* point the rocket to global varirate.*/
        while(exit_flag && game_flag) {

                usleep(TUNIT);

                //lanch rocket
                if(fire_flag) {
                        if(sys_info.rocket_left > 0) {

                                r = build_rocket();
                                add_rocket_node(rocket, r);

                                pthread_create(&r->thd, NULL, rocketThread, r);
                                fire_flag = 0;
                        }
                }

                //check the rocket size
                if (rocket->size == 0) {
                        continue;
                }

                pthread_mutex_lock(&rx);
                /* check the rocket is miss or hit */
                for (r = rocket->head; r != NULL; r = r->next) {
                        
                        if (r->live == 0) {
                                remove_rocket_node(rocket, r);
                                continue;
                        }

                        if(saucer->size == 0) {
                                continue;
                        }

                        pthread_mutex_lock(&sx);
                        /* check hit */
                        for(s = saucer->head; s != NULL; s = s->next) {
                                if( (r->y == s->y) && (( r->x >= s->x ) && (r->x <= s->x + 5))) {
                                        //destored saucer.
                                        s->live = -1;
                                        r->live = 0;
                                        move(s->y, s->x);
                                        addstr("      ");
                                        refresh();
                                        //cancle saucer thread;
                                        pthread_cancel(s->thd);
                                        pthread_cancel(r->thd);

                                        //update info
                                        pthread_mutex_lock(&sysx);
                                        sys_info.saucer_destory++;
                                        //update score
                                        i = sys_info.level < 5 ? sys_info.level : 5;
                                        sys_info.score += i;
                                        pthread_mutex_unlock(&sysx);

                                        //delete node
                                        remove_saucer_node(saucer, s);
                                }
                        }
                        pthread_mutex_unlock(&sx);

                }
                pthread_mutex_unlock(&rx);

        }

        //claer thead when exit;
        while( rocket->size) {
                r = rocket->head;
                pthread_cancel(r->thd);

                rocket->size--;
                r = r->next;
                free(rocket->head);

                rocket->head = r;
        }

        pthread_exit(NULL);
}

/* THE rocket thread, maintinue the rocket move */
void *rocketThread(void * rocket)
{
        rocket_node *r = (rocket_node *) rocket;

        while(r->live) {
                usleep(r->speed);
                pthread_mutex_lock(&sx);
                move( r->y, r->x);
                addch(' ');
                r->y--;
                if(r->y >= 0) {
                        move(r->y, r->x);
                        addch('^');
                }
                refresh();
                pthread_mutex_unlock(&sx);
                if(r->y < 0)
                        r->live = 0;
        }

        pthread_exit(NULL);
}

/* how to draw saucer to the scree */
void draw_saucer(int x, int y)
{
        int s;
        move(y, x);
        addch(' ');
        x++;
        s = sys_info.max_x - x;
        move(y, x);
        /* check the rocket to the right of the screen */
        switch(s) {

        case 0:
                break;

        case 1:
                addch('<');
                break;

        case 2:
                addstr("<-");
                break;

        case 3:
                addstr("<--");
                break;

        case 4:
                addstr("<---");
                break;

        default:
                addstr(SCHAR);
                break;
        }
        refresh();
}

/* THE saucer thread, maintinue the rocket move */
void *saucerThread(void * saucer)
{
        saucer_node *s = (saucer_node *) saucer;
        /*soucer_node *n; */
        while(s->live) {
                usleep(s->speed);
                pthread_mutex_lock(&sx);
                draw_saucer(s->x, s->y);
                s->x++;
                pthread_mutex_unlock(&sx);
                if(s->x >= sys_info.max_x) {
                        s->live = 0;
                }

        }

        pthread_exit(NULL);
}

/* build the new rocket node */
rocket_node *build_rocket()
{

        rocket_node *node = malloc(sizeof(rocket_node));
        bzero(node, sizeof(rocket_node));
        node->live = 1;
        node->speed = R_SPEED;
        node->x = player->x;
        node->y = sys_info.max_y - 3;
        pthread_mutex_lock(&sysx);
        sys_info.rocket_left--;
        pthread_mutex_unlock(&sysx);
        //Draw first rocket
        pthread_mutex_lock(&sx);
        move(node->y, node->x);
        addch('^');
        refresh();
        pthread_mutex_unlock(&sx);
        return node;

}

/* maintinue the game status, check the gmae over or level up 
 * and draw the bottom powerline.
 */
void *statusManager()
{
        int c, v;

        while(exit_flag && game_flag) {
                usleep(TUNIT);
                pthread_mutex_lock(&sx);

                /* draw the power line */
                v = sys_info.life - sys_info.saucer_miss;
                c = v > 3 ? 3 : 4 ;
                attron(COLOR_PAIR(c));
                mvprintw(LINES-1, 0, "LIFE: %d ", v);
                attroff(COLOR_PAIR(c));

                attron(COLOR_PAIR(2));
                printw(" HIT: %d ", sys_info.saucer_destory);
                attroff(COLOR_PAIR(2));

                attron(COLOR_PAIR(8));
                printw(" LEVEL: %d ", sys_info.level);
                attroff(COLOR_PAIR(8));

                attron(COLOR_PAIR(9));
                printw(" Help(H) ");
                attroff(COLOR_PAIR(9));

                attron(COLOR_PAIR(10));
                printw(" Quit(Q) ");
                attroff(COLOR_PAIR(10));

                attron(COLOR_PAIR(8));
                mvprintw(sys_info.max_y - 1, sys_info.max_x - 28,
                         " SCORE: %5d ", sys_info.score);
                attroff(COLOR_PAIR(8));


                c = sys_info.rocket_left > 5 ? 6 : 4;
                move(sys_info.max_y - 1, sys_info.max_x - 14 );
                /* the A_BINK not work well just remove it */
                if (sys_info.rocket_left == 0) {
                        attron(COLOR_PAIR(c) );
                        printw(" ROCKET: %2d/%2d", sys_info.rocket_left,
                               sys_info.rocket_max);
                        attroff(COLOR_PAIR(c) );
                } else {
                        attron(COLOR_PAIR(c));
                        printw(" ROCKET: %2d/%2d", sys_info.rocket_left,
                               sys_info.rocket_max);
                        attroff(COLOR_PAIR(c));
                }


                refresh();
                pthread_mutex_unlock(&sx);

                pthread_mutex_lock(&sysx);

                /* decided win(level up) */
                if (sys_info.saucer_destory >= sys_info.level_up_hit) {
                        //stop all thread.
                        exit_flag = 0;
                        pthread_mutex_lock(&sx);
                        show_levelup();
                        sys_info.level++;
                        sys_info.pass = 1;
                        pthread_mutex_unlock(&sx);

                }

                /* decided loss(game over) */
                if(sys_info.saucer_miss >= sys_info.life ||
                    ( *rkt == 0 && sys_info.rocket_left == 0 )) {
                        //stop all thread.
                        game_flag = 0;
                        pthread_mutex_lock(&sx);
                        show_gameover_windows();
                        sys_info.pass = 0;
                        pthread_mutex_unlock(&sx);
                }


                pthread_mutex_unlock(&sysx);
        }
        pthread_exit(NULL);
}


/* init the color pair when terminal donot support 256 color */
void setup_black_pair()
{
        /* init the colorpair when terminal only support 8 colors*/
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_BLACK, COLOR_WHITE);
        init_pair(3, COLOR_WHITE, COLOR_BLUE);
        init_pair(4, COLOR_WHITE, COLOR_RED);
        init_pair(5, COLOR_BLACK, COLOR_RED);
        init_pair(6, COLOR_WHITE, COLOR_CYAN);
        init_pair(7, COLOR_WHITE, COLOR_GREEN);
        init_pair(8, COLOR_WHITE, COLOR_MAGENTA);
        init_pair(9, COLOR_BLACK, COLOR_YELLOW);
        init_pair(10, COLOR_BLACK, COLOR_GREEN);
        init_pair(11, COLOR_BLACK, COLOR_BLACK);

}


/*init the color pair when terminal support 256 color.*/
void setup_color_pair()
{
        /*init the color pair when terminal support 256 color.*/
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_BLACK, COLOR_WHITE);
        init_pair(3, COLOR_WHITE, COLOR_BLUE);
        init_pair(4, COLOR_WHITE + 8, COLOR_RED);
        init_pair(5, COLOR_BLACK, COLOR_RED);
        init_pair(6, COLOR_WHITE, COLOR_CYAN);
        init_pair(7, COLOR_WHITE + 8, COLOR_GREEN);
        init_pair(8, COLOR_WHITE + 8, COLOR_MAGENTA);
        init_pair(9, COLOR_BLACK, COLOR_YELLOW);
        init_pair(10, COLOR_BLACK, COLOR_GREEN);
        init_pair(11, COLOR_BLACK, COLOR_BLACK + 8);

}

/* if terminal support color change, just change it */

void change_color()
{
        /*init the color to beautiful color. */
        init_color( 0, 160, 160, 160);
        init_color( 8, 239, 239, 239);
        init_color( 1, 870, 411, 317);
        init_color( 9, 772, 415, 278);
        init_color( 2, 737, 854, 333);
        init_color( 10, 615, 749, 376);
        init_color( 3, 886, 647, 392);
        init_color( 11, 925, 541, 145);
        init_color( 4, 129, 529, 964);
        init_color( 12, 329, 584, 862);
        init_color( 5, 529, 360, 552);
        init_color( 13, 894, 121, 400);
        init_color( 6, 262, 564, 694);
        init_color( 14, 152, 423, 760);
        init_color( 7, 823, 823, 823);
        init_color( 15, 1000, 1000, 1000);

}

/* fix the xterm */
void init_term()
{

        char *s;
        s = getenv("TERM");
        if ( strcmp("xterm", s) == 0) {
                setenv("TERM", "xterm-256color", 1);
                /*show_msg("set 256color");*/
                /*fprintf(stderr, "set 256 color, %s\n", getenv("TERM"));*/
        }

}

/* show the help information */
void show_help()
{
        /*print the help info for this game.*/
        WINDOW *w;
        w = newwin(19, 50 , (sys_info.max_y - 19 )/2, (sys_info.max_x - 50)/2);
        wbkgd(w, COLOR_PAIR(2));
        box(w, 0, 0);
        wattron(w, COLOR_PAIR(10));
        mvwprintw(w, 0, 17, " CONTROL INFO ");
        wattroff(w, COLOR_PAIR(10));
        /*wattron(w, COLOR_PAIR(2));*/
        mvwprintw(w, 2, 9, "SPACE    : Launch Rocket.");
        mvwprintw(w, 3, 9, ",        : Move ship to left.");
        mvwprintw(w, 4, 9, ".        : Move ship to right.");
        mvwprintw(w, 5, 9, "Q        : Exit the game.");
        mvwprintw(w, 6, 9, "H        : Show this help.");
        /*wattroff(w, COLOR_PAIR(2));*/
        mvwhline(w, 8, 1, 0, 48);

        wattron(w, COLOR_PAIR(10));
        mvwprintw(w, 8, 19, " Game INFO ");
        wattroff(w, COLOR_PAIR(10));

        /*wattron(w, COLOR_PAIR(2));*/
        mvwprintw(w, 10, 2, "In this game, you can launch rocket to destory");
        mvwprintw(w, 11, 2, "saucers. The number of rocket left show on the");
        mvwprintw(w, 12, 2, "bottom right corner, and you can miss maxiume");
        mvwprintw(w, 13, 2, "saucers called 'life' which at bottom left.");
        /*wattroff(w, COLOR_PAIR(2));*/

        wattron(w, COLOR_PAIR(3));
        mvwprintw(w, 15,  9,  "+-------------------------------+");
        mvwprintw(w, 16 , 9,  "|  Press Any Key to Continue... |");
        mvwprintw(w, 17,  9,  "+-------------------------------+");
        wattroff(w, COLOR_PAIR(3));
        wrefresh(w);
        /*refresh();*/

        getch();
        /* clear the windows */
        wborder(w, ' ', ' ', ' ',' ',' ',' ',' ',' ');
        wbkgd(w, COLOR_PAIR(1));
        wclear(w);
        wrefresh(w);
        delwin(w);

}

/* to set up the color pair() */
void setup_color()
{
        start_color();
        /*use_default_colors();*/
        setup_black_pair();
        refresh();

        if(can_change_color()) {
                change_color();
        }


        if (COLORS >=16 ) {
                setup_color_pair();
        }


}

/* game over */
void show_gameover_windows()
{
        WINDOW *win;
        win = newwin(20, 47, (sys_info.max_y - 20)/2, (sys_info.max_x - 47)/2);
        box(win, 0, 0);
        draw_gameover(win);
        wattron(win, COLOR_PAIR(3));
        mvwprintw(win, 15,  5,  "+-----------+");
        mvwprintw(win, 16 , 5 , "| Retry (R) |");
        mvwprintw(win, 17,  5,  "+-----------+");
        wattroff(win, COLOR_PAIR(3));

        wattron(win, COLOR_PAIR(4));
        mvwprintw(win, 15,  29,  "+-----------+");
        mvwprintw(win, 16 , 29 , "| Quit  (Q) |");
        mvwprintw(win, 17,  29,  "+-----------+");
        wattroff(win, COLOR_PAIR(4));
        wrefresh(win);
        delwin(win);

}

/* level up */
void show_levelup()
{
        WINDOW *win;
        win = newwin(20, 58, (sys_info.max_y - 20)/2, (sys_info.max_x - 58)/2);
        box(win, 0, 0);
        draw_levelup(win);
        wattron(win, COLOR_PAIR(3));
        mvwprintw(win, 15,  11,  "+-------------------------------+");
        mvwprintw(win, 16 , 11,  "|  Press Any Key to Continue... |");
        mvwprintw(win, 17,  11,  "+-------------------------------+");
        wattroff(win, COLOR_PAIR(3));
        wrefresh(win);
        delwin(win);

}

/* control each level of the game */
void gameMain()
{

        int i;
        char ch;
        pthread_t SaucerManageThread, RocketManageThread, StatusManageThread;
        saucer_list Saucer[1];
        rocket_list Rocket[1];
        bzero(Saucer, sizeof(saucer_list));
        bzero(Rocket, sizeof(rocket_list));

        struct arg_list arg;
        arg.r = Rocket;
        arg.s = Saucer;

        /* create game sub thread */
        pthread_create(&SaucerManageThread, NULL, saucerManager, Saucer);
        pthread_create(&RocketManageThread, NULL, rocketManager, &arg);
        pthread_create(&StatusManageThread, NULL, statusManager, NULL);

        move(sys_info.max_y - 1, 0);
        attron(COLOR_PAIR(2));
        for (i = 0; i < sys_info.max_x; i++)
                addch(' ');
        attroff(COLOR_PAIR(2));

        //initiazil the player info
        player->x = sys_info.max_x / 2;
        player->y = sys_info.max_y - 2;

        move(player->y, player->x);
        addch('|');
        refresh();

        while(end_flag) {
                ch = getch();
                if (ch == 'Q' && exit_flag) {
                        exit_flag = 0;
                        end_flag = 0;
                        break;
                } else if(ch == ' ' && exit_flag) {
                    /* launch rocket */
                        fire_flag = 1;
                } else if(ch == ',' && exit_flag) {
                        if(player->x > 0) {
                                pthread_mutex_lock(&sx);
                                player->x--;
                                move(player->y, player->x);
                                addstr("| ");
                                refresh();
                                pthread_mutex_unlock(&sx);
                        }
                } else if(ch == '.' && exit_flag) {
                        if(player->x < sys_info.max_x) {
                                pthread_mutex_lock(&sx);
                                move(player->y, player->x);
                                addstr(" |");
                                refresh();
                                player->x++;
                                pthread_mutex_unlock(&sx);
                        }
                } else if(ch == 'H' && exit_flag) {
                        pthread_mutex_lock(&sx);
                        show_help();
                        pthread_mutex_unlock(&sx);
                } else if(ch == 'R' && game_flag == 0) {
                        //game over.
                        break;

                } else if(exit_flag == 0) {
                        /*fprintf(stderr, "exit_flag antjet\n");*/
                        break;
                }
        }


        /* clean the thread. */
        pthread_join(SaucerManageThread, NULL);
        pthread_join(RocketManageThread, NULL);
        pthread_join(StatusManageThread, NULL);
        /*pthread_exit(NULL);*/
        usleep(500000);

}

/* read the score from the file */
int read_score()
{

        FILE *fp;
        int s = 0;
        fp = fopen(sys_info.path, "r");
        if (fp != NULL) {
                fread(&s, 4, 1, fp);
                fclose(fp);
        }

        return s;

}

/* save the score from the file */
int write_score(int score)
{

        FILE *fp;
        int s = 0;
        fp = fopen(sys_info.path, "w");
        if(fp != NULL) {
                s = fwrite(&score, 4, 1, fp);
                fclose(fp);
        }
        return s;

}

int main(int argc, const char *argv[])
{


        srand(getpid());

        /*init term if xterm set xterm-256 */
        init_term();

        /*init ncurses*/
        initscr();
        crmode();
        noecho();
        clear();
        curs_set(0);
        /* setup the color system */
        setup_color();

        sys_info.max_x = COLS;
        sys_info.max_y = LINES;
        sys_info.level = 1;

        bzero(&sys_info.path, 100);
        struct passwd *pw = getpwuid(getuid());

        sprintf(sys_info.path, "%s/%s", pw->pw_dir, CONFIG_PATH);
        sys_info.score = read_score();

        while(end_flag) {
                /*clean the screen*/
                erase();
                refresh();
                exit_flag = 1;
                game_flag = 1;
                /*start the game*/
                set_level_info();
                gameMain();
                write_score(sys_info.score);

        }


        endwin();
        return 0;
}

