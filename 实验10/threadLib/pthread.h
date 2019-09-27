#ifndef _PTHREAD_H_
#define _PTHREAD_H_
#define __LIBRARY__
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#define ATTR_FLAG_SET_STACK 0x01
#define ATTR_FLAG_DETACHED 0x02
#define PTHREAD_FLAG_JOINED 0x01
#define STACK_SIZE_MIN 2048
#define STACK_SIZE_MAX 4096
#define STACK_SIZE_DEFAULT 4096

#define MAX_THREAD  10 // including main thread


typedef unsigned int pthread_t;
struct pthread
{
	int flags;
	void* stack_addr;
	long stack_size;
	long tid;
	long pid;
	void* (*start_routine) (void*);
	void* arg;
	pthread_t joined_thread_id;
	struct pthread* next_thread;
};
typedef struct pthread_attr
{
	/* Scheduler parameters and priority.  */
	int flags;
	/* Stack handling.  */
	void* stackaddr;
	size_t stacksize;
}pthread_attr_t;


int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine) (void*), void* arg);
int pthread_attr_init(pthread_attr_t* attr);
int pthread_join(pthread_t thread, void** value_ptr);
void pthread_exit(void* value_ptr);
int pthread_cancel(pthread_t thread);
#endif