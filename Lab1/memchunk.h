#define MAX_MEM_ADDR     0xfffff000

//static sig_atomic_t segv_read_flag;
//static sig_atomic_t segv_write_flag;
static sig_atomic_t segv_flag;
static sigjmp_buf jmp_buff;

//static struct sigaction read_sa, read_oldsa;
//static struct sigaction write_sa, write_oldsa;
static struct sigaction sa, oldsa;

struct memchunk
{
	void *start;
	unsigned long length;
	int RW;
};


int get_mem_layout (struct memchunk *chunk_list, int size);
static void handler(int signum);
int mem_read_test(void *buf, unsigned long size);
int mem_write_test(void *buf, unsigned long size);

