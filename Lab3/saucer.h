#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <pwd.h>
//#include <sys/type.h>
#include "game_control.h"

#define MAXSAUCER   20
#define MAXROCKET   20
#define MAXMISS     5
#define MAXROW      5
#define MAXCOL      80
#define S_SPEED     200000
#define R_SPEED     50000
#define TUNIT       25000
#define SCHAR       "<--->"
#define CONFIG_PATH   ".twu5_saucer"

struct system_info {

        int max_x;
        int max_y;
        int saucer_miss;
        int life;
        int saucer_destory;
        int rocket_left;
        int rocket_max;
        int color;
        int level;
        int level_up_hit;
        int saucer_speed;
        int saucer_num;
        int saucer_limit;
        int pass;
        int score;
        char path[100];
};

struct player {

        int x;
        int y;
};

typedef struct player player_info;

struct rocket {
        int id;
        int x;
        int y;
        int live;
        int speed;
        pthread_t thd;

        struct rocket *prev;
        struct rocket *next;
};

struct saucer {
        int id;
        int x;
        int y;
        int speed;
        int live;
        pthread_t thd;

        struct saucer *prev;
        struct saucer *next;
};

typedef struct rocket rocket_node;
typedef struct saucer saucer_node;

struct rocket_list {
        rocket_node *head;
        rocket_node *tail;

        int          size;
};

struct saucer_list {
        saucer_node *head;
        saucer_node *tail;

        int         size;
};

typedef struct saucer_list saucer_list;
typedef struct rocket_list rocket_list;


pthread_mutex_t rx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sysx = PTHREAD_MUTEX_INITIALIZER;
struct arg_list {
        rocket_list *r;
        saucer_list *s;
};



saucer_node *build_saucer();
rocket_node *build_rocket();
void add_saucer_node(saucer_list *l, saucer_node *n);
void remove_saucer_node(saucer_list *l, saucer_node *n);
void *saucerManager(void * list);
void remove_rocket_node(rocket_list *l, rocket_node *n);
void * rocketManager(void * list);
void *rocketThread(void * rocket);
void *saucerThread(void * saucer);
void draw_saucer(int x, int y);
void add_rocket_node(rocket_list *l, rocket_node *n);
void *statusManager();
void show_gameover_windows();
void change_color();
void show_help();
void setup_black_pair();
void setup_color_pair();
void show_levelup();
void set_level_info();
void ongame();
int read_score();
int write_score( int score);
