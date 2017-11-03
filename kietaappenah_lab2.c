#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#define NUM_THREADS 5
#define CALCS 1000000

typedef struct _thread_data
{
	int threadnum;
	double *arrayElement;
}thread_data;

void *calculate(void *threadarg)
{
	int threadnum;
	thread_data *my_data;
	
	my_data = threadarg;
	threadnum = my_data->threadnum;

	double count = (threadnum*(CALCS/NUM_THREADS))+2;
	double result = 0;
	double fraction;
	int j;

	for(j = 0; j < (CALCS/(NUM_THREADS*2)); j++)
	{
		fraction = 4/(count*(count+1)*(count+2));
		if(j % 2 == 0)
		{
			result += fraction;
		}
		else
		{
			result -= fraction;
		}
		count += 2;
	}
	
	*my_data->arrayElement = result;	
}

int main()
{
	double piArray[NUM_THREADS];
	pthread_t threads[NUM_THREADS];
	int rc, t;
	thread_data tdata[NUM_THREADS];
	
	clock_t start, end;
	start = clock();

	double elapsedTime;

	for(t = 0; t<NUM_THREADS; t++)
	{
		tdata[t].threadnum = t;
		tdata[t].arrayElement = &piArray[t];

		rc = pthread_create(&threads[t], NULL, calculate, (void *) &tdata[t]);
		if(rc)
		{
			printf("ERROR return code from pthread_create(): %d\n", rc);
			exit(-1);
		}
	}

	
	for(t = 0; t<NUM_THREADS;t++)
	{
		pthread_join(threads[t], NULL);
	}


	long double pi = 3;

	for(t = 0; t < NUM_THREADS; t++)
	{
		pi += piArray[t];
	}

	end = clock();
	elapsedTime = (end - start);

	printf("Time = %f\n", elapsedTime);
	printf("Pi = %.17Lf\n", pi);

	return 0;
}
