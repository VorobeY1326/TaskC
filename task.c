#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/time.h>
   
int alarmed = 0;
int finishedCnt = 0;

void onalarm(nsig)
{
	alarmed = 1;
}

void print14(
	ucontext_t *loop_context,
	ucontext_t *other_context,
	int *finished)
{
	int i;
	
	for (i=0; i < 10; i++)
	{
		printf("High Priority Thread: %d\n", 14);
		usleep(10000);
		swapcontext(loop_context, other_context);
	}
	*finished = 1;
}

void print26(
	ucontext_t *loop_context,
	ucontext_t *other_context,
	int *finished)
{
	int i;
	
	for (i=0; i < 10; i++)
	{
		printf("Low Priority Thread: %d\n", 26);
		usleep(10000);
		swapcontext(loop_context, other_context);
	}
	*finished = 1;
}

void sumNumbers(
	ucontext_t *loop_context,
	ucontext_t *other_context,
	int *finished)
{
	int i;
	int sum = 0;
	
	for (i=0; i < 10; i++)
	{
		sum += i;
		printf("Background Counting Summary: %d\n", sum);
		usleep(10000);
		swapcontext(loop_context, other_context);
	}
	*finished = 1;
}

int threadCount = 0;
ucontext_t threads[10];
int threadFinished[10];
int priorities[10];
int stage[10];
char stacks[10][1000];

int currentThread = 0;

void addThread(void (*func)(ucontext_t*,ucontext_t*,int*), ucontext_t *back, ucontext_t *finish, int priority)
{
	threads[threadCount].uc_link = finish;
	threads[threadCount].uc_stack.ss_sp = stacks[threadCount];
	threads[threadCount].uc_stack.ss_size = sizeof(stacks[threadCount]);
	getcontext(&threads[threadCount]);
	makecontext(&threads[threadCount], (void (*)(void)) func, 3, &threads[threadsCount], back, &threadFinished[threadCount]);
	priorities[threadCount] = priority;
	threadCount++;
}

void selectCurrentThread()
{
	while (1)
	{
		currentThread++;
		if (currentThread >= threadCount)
			currentThread = 0;
		if (threadFinished[currentThread])
			continue;
		stage[currentThread]++;
		if (stage[currentThread] >= priorities[currentThread])
			stage[currentThread] = 0;
		if (stage[currentThread] == 0)
			break;
	}
}

ucontext_t main_context1, main_context2;

int main(void)
{
	struct sigaction sa;
	struct itimerval timer;
	
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &onalarm;
	
	sigaction(SIGALRM, &sa, NULL);
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 25000;
	setitimer(ITIMER_REAL, &timer, NULL);
	
	addThread(print14, &main_context2, &main_context1, 1);
	addThread(print26, &main_context2, &main_context1, 2);
	addThread(sumNumbers, &main_context2, &main_context1, 5);
	
	getcontext(&main_context1);
	finishedCnt++;
	
	if (finishedCnt <= threadCount)
	{
		while (1)
		{
			if (alarmed)
			{
				alarmed = 0;
				selectCurrentThread();
			}
			
			if (threadFinished[currentThread])
				selectCurrentThread();
			
			swapcontext(&main_context2, &threads[currentThread]);
		}
	}
	return 0;
}