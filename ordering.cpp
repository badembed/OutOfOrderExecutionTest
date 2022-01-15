#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

//-------------------------------------
//  Main program, as decribed in the post:
//  https://preshing.com/20120515/memory-reordering-caught-in-the-act/
//-------------------------------------
sem_t beginSema1;
sem_t beginSema2;
sem_t endSema;

volatile int X, Y;
volatile int r1, r2;

void *thread1Func(void *param)
{
    for (;;)
    {
        sem_wait(&beginSema1);  // Wait for signal

        // ----- THE TRANSACTION! -----
        X = 1;
#if USE_CPU_FENCE
        asm volatile("mfence" ::: "memory");  // Prevent CPU reordering
#endif
        r1 = Y;

        sem_post(&endSema);  // Notify transaction complete
    }
    return NULL;  // Never returns
};

void *thread2Func(void *param)
{
    for (;;)
    {
        sem_wait(&beginSema2);  // Wait for signal

        // ----- THE TRANSACTION! -----
        Y = 1;
#if USE_CPU_FENCE
        asm volatile("mfence" ::: "memory");  // Prevent CPU reordering
#endif
        r2 = X;

        sem_post(&endSema);  // Notify transaction complete
    }
    return NULL;  // Never returns
};

int main()
{
    // Initialize the semaphores
    sem_init(&beginSema1, 0, 0);
    sem_init(&beginSema2, 0, 0);
    sem_init(&endSema, 0, 0);

    // Spawn the threads
    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, thread1Func, NULL);
    pthread_create(&thread2, NULL, thread2Func, NULL);

    // Repeat the experiment ad infinitum
    int detected = 0;
    int iterations = 1;
    while (1)
    {
        // Reset X and Y
        X = 0;
        Y = 0;
        // Signal both threads
        sem_post(&beginSema1);
        sem_post(&beginSema2);
        // Wait for both threads
        sem_wait(&endSema);
        sem_wait(&endSema);
        // Check if there was a simultaneous reorder
        if (r1 == 0 && r2 == 0)
        {
            detected++;
            printf("%d reorders detected after %d iterations\n", detected, iterations);
        }
        iterations++;
    }
    return 0;  // Never returns
}
