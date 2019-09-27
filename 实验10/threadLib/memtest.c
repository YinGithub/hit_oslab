#include "pthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define MAX_THREADS_NUM 20
#define WHOLE_MEM_SIZE 0x100000

#define THREAD_STATUS_INITIALED 0x00
#define THREAD_STATUS_RUNNING 0x01
#define THREAD_STATUS_COMPLETED 0x02
#define THREAD_STATUS_ABORTED 0x03

int times_num;
int thread_num;
struct thread_info_s
{
	unsigned char* start_addr;
	int size;
	/*test result*/
	int correct_counter;
	int status;
};
void* memtest(void* arg_p)
{
	int i, t, j;
	struct thread_info_s* pt;
	unsigned long addr;
	pt = (struct thread_info_s*)arg_p;
	pt->status = THREAD_STATUS_RUNNING;
	srand((unsigned int)time(0));
	for (i = 0; i < pt->size; i++)
	{
		for (t = 0; t < times_num; t++)
		{
			unsigned char _set_value[5] = { 0x00,0xFF,0x55,0xAA };
			_set_value[4] = (unsigned char)rand();
			addr = (unsigned long)(pt->start_addr) + i;
			for (j = 0; j < 5; j++)
			{
				memset((void*)addr, _set_value[j], 1);
				if (*((unsigned char*)addr) != _set_value[j])
					break;
			}
			if (j < 5)
				break;
		}
		if (t == times_num)
			pt->correct_counter = pt->correct_counter + 1;
	}
	pt->status = THREAD_STATUS_COMPLETED;
	pthread_exit(NULL);
}

void main()
{
	char cmd[50];
	long i;
	int thread_state = 0;
	unsigned long t;
	int tmp_num;
	unsigned char* mem_addr;
	pthread_t threads[MAX_THREADS_NUM];
	struct thread_info_s thread_info[MAX_THREADS_NUM];
	mem_addr = malloc(WHOLE_MEM_SIZE);
	while (1)
	{
		printf(">>>");
		fflush(stdout);
		scanf("%s", cmd);
		if (!strcmp(cmd, "thread"))
		{
			scanf("%d", &tmp_num);
			if (tmp_num <1 || tmp_num > MAX_THREADS_NUM)
				printf("Wrong thread number!\n");
			else
			{
				thread_num = tmp_num;
				printf("Will generate %d threads\n", thread_num);
			}
		}
		else if (!strcmp(cmd, "times"))
		{
			scanf("%d", &tmp_num);
			if (tmp_num < 1)
				printf("Wrong times number!\n");
			else
			{
				times_num = tmp_num;
				printf("mem will test for %d times\n", times_num);
			}
		}
		else if (!strcmp(cmd, "go"))
		{
			for (t = 0; t < thread_num; t++)
			{
				memset((void*)(thread_info + t), 0x00, sizeof(struct thread_info_s));
				thread_info[t].start_addr = mem_addr + t * (WHOLE_MEM_SIZE / thread_num);
				if (t == (thread_num - 1))
					thread_info[t].size = WHOLE_MEM_SIZE - (thread_num - 1) * (WHOLE_MEM_SIZE / thread_num);
				else
					thread_info[t].size = WHOLE_MEM_SIZE / thread_num;
				pthread_create(&threads[t], NULL, memtest, (void*)(thread_info + t));
			}
			printf("started.\n");
		}
		else if (!strcmp(cmd, "abort"))
		{
			for (t = 0; t < thread_num; t++)
			{
				if (thread_info[t].status == THREAD_STATUS_RUNNING)
				{
					pthread_cancel(threads[t]);
					thread_info[t].status = THREAD_STATUS_ABORTED;
				}

			}
		}
		else if (!strcmp(cmd, "exit"))
		{
			printf("exited\n");
			free(mem_addr);
			pthread_exit(NULL);
		}
		else if (!strcmp(cmd, "status"))
		{
			for (t = 0; t < thread_num; t++)
			{
				if (thread_info[t].status == THREAD_STATUS_RUNNING)
					printf("Thread %d is runing,Tested mem:%08X - %08X,Result(%d/%d)\n", t, (unsigned int)thread_info[t].start_addr, \
					(unsigned int)thread_info[t].start_addr + thread_info[t].size, \
						thread_info[t].correct_counter, \
						thread_info[t].size);
				else if (thread_info[t].status == THREAD_STATUS_COMPLETED)
				{
					printf("Thread %d is completed,Tested mem:%08X - %08X,Result(%d/%d)\n", t, (unsigned int)thread_info[t].start_addr, \
						(unsigned int)thread_info[t].start_addr + thread_info[t].size, \
						thread_info[t].correct_counter, \
						thread_info[t].size);
				}
				else if (thread_info[t].status == THREAD_STATUS_ABORTED)
				{
					printf("Thread %d is aborted,Tested mem:%08X - %08X,Result(%d/%d)\n", t, (unsigned int)thread_info[t].start_addr, \
						(unsigned int)thread_info[t].start_addr + thread_info[t].size, \
						thread_info[t].correct_counter, \
						thread_info[t].size);
				}

			}

		}
		else
		{
			printf("wrong conmmand!\n");
		}
		fflush(stdout);

	}
}



