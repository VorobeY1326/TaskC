#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/time.h>
 
int alarmed = 0;

void onalarm(nsig)
{
	alarmed = 1;
}

void print1(
	ucontext_t *loop_context,
	ucontext_t *other_context)
{
	while (1)
	{
		printf("1\r\n");
		swapcontext(loop_context, other_context);
	}
}

void print2(
	ucontext_t *loop_context,
	ucontext_t *other_context)
{
	while (1)
	{
		printf("2\r\n");
		swapcontext(loop_context, other_context);
	}
}

int threadCount = 0;
ucontext_t threads[10];
char stacks[10][18000];

void addThread(void (*func)(ucontext_t*,ucontext_t*), ucontext_t *back, ucontext_t *finish)
{
	threads[threadCount].uc_link = finish;
	threads[threadCount].uc_stack.ss_sp = stacks[threadCount];
	threads[threadCount].uc_stack.ss_size = sizeof(stacks[threadCount]);
	getcontext(&threads[threadCount]);
	makecontext(&threads[threadCount], (void (*)(void)) func, 2, &threads[threadCount], back);
	threadCount++;
}

ucontext_t main_context1, main_context2;

int main(void)
{
	struct sigaction sa;
	struct itimerval timer;
	
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &onalarm;
	
	sigaction(SIGALRM, &sa, NULL);
	timer.it_value.tv_sec = 1;
	timer.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);
	
	addThread(print1, &main_context2, &main_context1);
	addThread(print2, &main_context2, &main_context1);
	
	getcontext(&main_context1);
	
	int turn = 0;
	
	while (1)
	{
		if (alarmed)
		{
			alarmed = 0;
			swapcontext(&main_context2, &threads[0]);
			if (turn & 1)
				swapcontext(&main_context2, &threads[1]);
			turn ^= 1;
		}
	}
	return 0;
}