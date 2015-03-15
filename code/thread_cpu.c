#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>

#define NUM 10000000
void *randfun(void *arg)
{
    unsigned int i;

    clock_t start, end;

    start = clock();

    for(i=0; i<NUM; i++)
        ;

    end = clock();

    printf("cpu random: %fs\n", (double)(end - start)/CLOCKS_PER_SEC);
    return;
}

void *fun(void *arg)
{
    int process_id = (int)arg;

    cpu_set_t mask;
    cpu_set_t get;

    CPU_ZERO(&mask);
    CPU_SET(process_id, &mask);

    pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);

    CPU_ZERO(&get);

    int ac_id = pthread_getaffinity_np(pthread_self(), sizeof(get), &get);
    if(ac_id < 0)
    {
        printf("set thread affinity failed\n");
        return NULL;
    }
    unsigned int i;

    clock_t start, end;

    start = clock();

    for(i=0; i<NUM; i++)
        ;

    end = clock();

    printf("cpu affinity: %fs\n", (double)(end - start)/CLOCKS_PER_SEC);
    return;
}

void cal()
{
    unsigned int i;

    clock_t start, end;

    start = clock();

    for(i=0; i<NUM; i++)
        ;

    end = clock();

    printf("time: %fs\n", (double)(end - start)/CLOCKS_PER_SEC);
    return;

}

int main()
{
    //cal();
    //return 0;

    int i;
    int num = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t *tidset = (pthread_t*)malloc(sizeof(pthread_t)*num);

    for(i=0; i<num; i++){
        pthread_create(tidset+i, NULL, (void*)fun, (void*)i);
    }

    for(i=0; i<num; i++){
        pthread_join(tidset[i], NULL);
    }

    for(i=0; i<num; i++){
        pthread_create(tidset+i, NULL, (void*)randfun, NULL);
    }

    for(i=0; i<num; i++){
        pthread_join(tidset[i], NULL);
    }

    free(tidset);
    return 0;
}
