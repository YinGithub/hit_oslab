#include "pthread.h"
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <stdio.h>
_syscall3(int, clone, long, fn, long, stack_addr, long, threadid)
_syscall0(int, gettid)
struct pthread* _head_thread;
struct pthread_attr _default_attr = {};
int allocate_stack( const pthread_attr_t * attr, struct pthread** pdp)
{
	void* addr;
	int size;
	if (attr->flags & ATTR_FLAG_SET_STACK)
	{
		/* allien with 4 bytes*/
		addr = attr->stackaddr;
		size = attr->stacksize;
		if (((long)(addr + size)) & 0x03)
			return -1;
		if (attr->stacksize < STACK_SIZE_MIN)
			return -1;
	}
	else
	{
		addr = malloc(STACK_SIZE_DEFAULT);
		size = STACK_SIZE_DEFAULT;
		if (((long)(addr + size)) & 0x03)
			return -1;
	}
	memset(addr, 0, sizeof(struct pthread));
	*(pdp) = (struct pthread *)addr;
	(*(pdp))->stack_addr = (void*)(addr);
	(*(pdp))->stack_size = size;
	/*pid same as thread id*/
	(*(pdp))->tid = (long)addr;
	return 0;
}
void _thread_exit(void)
{
	pthread_exit(NULL);
	return;
}
int _free_tcb_stack(struct pthread * tcb)
{
	struct pthread* head = _head_thread;
	/* find in tcb chain*/
	if (tcb == _head_thread)
	{
		_head_thread = _head_thread->next_thread;
	}
	else
	{
		while (head->next_thread && head->next_thread != tcb)
		{
			head = head->next_thread;
		}
		if (head->next_thread == NULL)
			return -1;
		head->next_thread = tcb->next_thread;
	}
	/*free malloc sapce */
	free(tcb->stack_addr);
	return 0;
}
int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine) (void*), void* arg)
{
	/*allocate a new stack */
	struct pthread * pd;
	void* stack_addr;
	if (attr == NULL)
		attr = &_default_attr;
	if (allocate_stack(attr,(struct pthread**)thread) != 0)
		return -1 ;
	/* fill thread struct*/
	pd = (struct pthread*)(*thread);
	pd->start_routine = start_routine;
	pd->arg = arg;
	if (attr->flags & ATTR_FLAG_DETACHED)
		pd->joined_thread_id = (int)pd;
	else
		pd->joined_thread_id = -1;
	/*insert in thread list*/
	if (_head_thread == NULL)
		_head_thread = pd;
	else
	{
		pd->next_thread = _head_thread;
		_head_thread = pd;
	}
	stack_addr = (void*)(pd->stack_size + pd->stack_addr);
	*(--((long*)stack_addr)) = (long)arg;
	*(--((long*)stack_addr)) = (long)_thread_exit;
	pd->pid = clone((long)(pd->start_routine), (long)stack_addr,(long)pd);
	if (pd->pid < 0)
	{
		/*free tcb , free stack*/
		_free_tcb_stack(pd);
	}
	return 0;
}

int pthread_attr_init(pthread_attr_t* attr)
{
	if (attr == NULL)
		return -1;
	memset((void*)attr,0, sizeof(pthread_attr_t));
	return 0;
}

int _find_thread(struct pthread* tcb)
{
	struct pthread* head = _head_thread;
	while (head)
	{
		if (head == tcb)
			break;
		head = head->next_thread;
	}
	if (head == NULL)
		return -1;
	else
		return 0;

}
int pthread_join(pthread_t thread, void** value_ptr)
{
	struct pthread* pd;
	pthread_t current_tid;
	if (_find_thread((struct pthread *)thread) < 0)
		return -1;
	pd = (struct pthread*)thread;
	current_tid = (pthread_t)gettid();
	/*caution!!!thread competion*/
	if ((pd->flags & PTHREAD_FLAG_JOINED) && pd->joined_thread_id != current_tid)
		return -2;
	pd->joined_thread_id = current_tid;
	pd->flags = pd->flags | PTHREAD_FLAG_JOINED;
	if (waitpid(pd->pid, (int*)value_ptr,0) != pd->pid)
		return -1;
	*value_ptr = (void *)(WEXITSTATUS(*((int*)value_ptr)));
	_free_tcb_stack((struct pthread *)thread);
	return 0;
}

void pthread_exit(void* value_ptr)
{
	struct pthread* head;
	void* state;
	/* in main thread*/
	if (gettid() == 0)
	{
		/*wait all child thread to be end */
		head = _head_thread;
		while (head != NULL)
		{
			pthread_join((pthread_t)head, &state);
			head = head->next_thread;
		}
	}
	exit((int)value_ptr);
}

int pthread_cancel(pthread_t thread)
{
	struct pthread* tcb = (struct pthread*)thread;
	return kill(tcb->pid, SIGKILL);
}
