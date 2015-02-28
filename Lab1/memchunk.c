#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memchunk.h"

static void
handler(int signum)
{
        segv_flag = 0;
        siglongjmp(jmp_buff, 1);

}

int
get_mem_layout(struct memchunk *chunk_list, int size)
{

        unsigned long local_size = 0;
        int     last_RW = -1;
        int     local_RW = -1;
        void   *local_ptr = 0;
        int     flag = 1;

        /* check size */
        if (!size)
                return 0;

        /* get page size */
        unsigned long page_size = (unsigned long)getpagesize();

        void   *mem_ptr = 0;
        int     count = 0;

        /* printf("ADDR: %p\n", mem_ptr); */

        /* loop */
        do {

                /* printf("ADDR: %p, Page_size: %d \n", mem_ptr, count); */
                /* not readable must not writeable */
                if (mem_read_test(mem_ptr, page_size)) {

                        /* test writeable */
                        if (mem_write_test(mem_ptr, page_size)) {
                                last_RW = 1;

                        } else {
                                last_RW = 0;
                        }

                } else {
                        last_RW = -1;
                }


                if (!flag) {
                        /* same RW */
                        if (last_RW == local_RW) {

                                local_size += page_size;

                        } else {
                                /* different RW status */

                                /* check block size overflow */
                                if (count < size) {
                                        chunk_list[count].start = local_ptr;
                                        chunk_list[count].RW = local_RW;
                                        chunk_list[count].length = local_size;
                                }
                                /* reset flag */
                                flag = 1;
                                count++;
                        }
                }
                /* write flag, if this is the new start of block */
                if (flag && last_RW != -1) {

                        flag = 0;
                        local_ptr = mem_ptr;
                        local_RW = last_RW;
                        local_size = page_size;
                }
                mem_ptr += page_size;
                /* end loop */
        /* 
         *  In 32bits system, the result of void pointer do 0xfffff000 + 4096
         *  is 0x00000000. So we just need to set end of loop is 
         *  void pointer = 0.
         *  In fact the total memory address mod page size must be zero, so
         *  set the end of loop is 0 is corrent.
         */
        } while (mem_ptr != 0);

        return count;

}

int
mem_read_test(void *buf, unsigned long size)
{

        char   *p = (char *)buf;
        volatile char dummy;

        /* check for size */
        if (!size)
                return 0;

        /* storing the signal env and trapping SIGSEGV */
        sa.sa_handler = handler;
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);

        sigaction(SIGSEGV, &sa, NULL);
        /* set flag to 1 default. */
        segv_flag = 1;

        if (sigsetjmp(jmp_buff, 1) == 0) {

                dummy = p[0];
                dummy = p[size - 1];
        }
        sigaction(SIGSEGV, &oldsa, NULL);
        return segv_flag;
}

int
mem_write_test(void *buf, unsigned long size)
{

        char   *p = (char *)buf;

        /* check for size */
        if (!size)
                return 0;

        /* storing the signal env and trapping SIGSEGV */
        sa.sa_handler = handler;
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);

        sigaction(SIGSEGV, &sa, NULL);

        /* set flag to 1 default. */
        segv_flag = 1;

        if (sigsetjmp(jmp_buff, 1) == 0) {
                /* test for writeable, and do not change the data. */
                p[0] = p[0];
                p[size - 1] = p[size - 1];
        }
        sigaction(SIGSEGV, &oldsa, NULL);

        return segv_flag;
}

int
main()
{
        int     size = 2000;
        struct memchunk chunk_list[size];

        int     pid = (int)getpid();
        int     i = 0;
        int     c;

        /* print system memory maps first. */
        FILE   *fp = fopen("/proc/self/maps", "r");
        char    line[100];
        while (fgets(line, 90, fp))
                printf("%s", line);
        fclose(fp);

        c = get_mem_layout(chunk_list, size);

        printf("Total %d block found!\n", c);
        printf("The flags = 0 is R, the 1 is RW\n");
        c = (size > c) ? c : size;
        for (i = 0; i < c; i++) {
                printf("address %p, end_address %p, flags %d\n",
                       chunk_list[i].start, 
                       chunk_list[i].start + chunk_list[i].length,
                       chunk_list[i].RW);
        }

        return 0;

}


/**
 *
 *  When Call the function get_mem_layout it return follow memory layout.
 *  
 * The flags = 0 is R, the 1 is RW
 * address 0x8048000, end_address 0x804a000, flags 0
 * address 0x804a000, end_address 0x804b000, flags 1
 * address 0x8892000, end_address 0x88b3000, flags 1
 * address 0xf759d000, end_address 0xf759e000, flags 1
 * address 0xf759e000, end_address 0xf7744000, flags 0
 * address 0xf7744000, end_address 0xf7748000, flags 1
 * address 0xf7778000, end_address 0xf7779000, flags 1
 * address 0xf777a000, end_address 0xf777c000, flags 1
 * address 0xf777c000, end_address 0xf779e000, flags 0
 * address 0xf779e000, end_address 0xf779f000, flags 1
 * address 0xff80e000, end_address 0xff830000, flags 1
 *
 * 
 *  The first block which start at 0x8048000 is the text segment, the default
 *  Linux program start address at 0x8048000. So this address cannot be editing,
 *  the Flag is read only.
 *  After text segment, is the data segment, which is start at 0x804a000, and 
 *  flag is RW.
 *  The third block which start at 0x8892000 should be heap, the heap usually 
 *  located at low memory address.
 *  Finally, the last block which start at 0xff80e000 should be stack, because
 *  the stack is the highest address in the memory.
 *
 *
 *  Reference:
 *  Understanding Memory: www.ualberta.ca/CNS/RESEARCH/LinuxClusters/mem.html
 */

 



