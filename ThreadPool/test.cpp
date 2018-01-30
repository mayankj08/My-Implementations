#include "ThreadPool.h"
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <cassert>


void *letsWait(void *)
{
    for(int i=0;i<1000;i++){
        ;
    }
    return NULL;
}


void conventionalThreadCreation(){

    std::clock_t begin = clock();
    int rc;
    for(int i=0;i<30;i++){
        pthread_t  th1, th2,th3,th4,th5,th6,th7,th8,th9,th10;
        if(i%10 == 0)
            std::cerr << "Completed " << i << " iterations\n";
        rc = pthread_create(&th1, NULL, letsWait, (void *)0); assert(rc == 0);
        rc = pthread_create(&th2, NULL, letsWait, (void *)0); assert(rc == 0);
        rc = pthread_create(&th3, NULL, letsWait, (void *)0); assert(rc == 0);
        rc = pthread_create(&th4, NULL, letsWait, (void *)0); assert(rc == 0);
        rc = pthread_create(&th5, NULL, letsWait, (void *)0); assert(rc == 0);
        rc = pthread_create(&th6, NULL, letsWait, (void *)0); assert(rc == 0);
        rc = pthread_create(&th7, NULL, letsWait, (void *)0); assert(rc == 0);
        rc = pthread_create(&th8, NULL, letsWait, (void *)0); assert(rc == 0);
        rc = pthread_create(&th9, NULL, letsWait, (void *)0); assert(rc == 0);
        rc = pthread_create(&th10, NULL, letsWait, (void *)0); assert(rc == 0);
        
        sleep(1);

        rc = pthread_join(th1,NULL);assert(rc == 0);
        rc = pthread_join(th2,NULL);assert(rc == 0);
        rc = pthread_join(th3,NULL);assert(rc == 0);
        rc = pthread_join(th4,NULL);assert(rc == 0);
        rc = pthread_join(th5,NULL);assert(rc == 0);
        rc = pthread_join(th6,NULL);assert(rc == 0);
        rc = pthread_join(th7,NULL);assert(rc == 0);
        rc = pthread_join(th8,NULL);assert(rc == 0);
        rc = pthread_join(th9,NULL);assert(rc == 0);
        rc = pthread_join(th10,NULL);assert(rc == 0);
    }

    std::clock_t end = clock();
    std::cout << "Time elapsed using pthread lib:" << end-begin << " clocks"<<"\n\n";

}

void threadPoolMethod(){

    std::clock_t begin = clock();
    
    struct ThreadPool tp;
    struct ThreadID thd1,thd2,thd3,thd4,thd5,thd6,thd7,thd8,thd9,thd10;
    int rc;


    for(int i=0;i<30;i++){
        if(i%10 == 0)
            std::cerr << "Completed " << i << " iterations\n";

        rc = ThreadPool_run(&tp,&thd1,letsWait, (void *)NULL); assert(rc == 0);
        rc = ThreadPool_run(&tp,&thd2,letsWait, (void *)NULL); assert(rc == 0);
        rc = ThreadPool_run(&tp,&thd3,letsWait, (void *)NULL); assert(rc == 0);
        rc = ThreadPool_run(&tp,&thd4,letsWait, (void *)NULL); assert(rc == 0);
        rc = ThreadPool_run(&tp,&thd5,letsWait, (void *)NULL); assert(rc == 0);
        rc = ThreadPool_run(&tp,&thd6,letsWait, (void *)NULL); assert(rc == 0);
        rc = ThreadPool_run(&tp,&thd7,letsWait, (void *)NULL); assert(rc == 0);
        rc = ThreadPool_run(&tp,&thd8,letsWait, (void *)NULL); assert(rc == 0);
        rc = ThreadPool_run(&tp,&thd9,letsWait, (void *)NULL); assert(rc == 0);
        rc = ThreadPool_run(&tp,&thd10,letsWait, (void *)NULL); assert(rc == 0);

        sleep(1);

        rc = ThreadPool_join(thd1, NULL); assert(rc == 0);
        rc = ThreadPool_join(thd2, NULL); assert(rc == 0);
        rc = ThreadPool_join(thd3, NULL); assert(rc == 0);
        rc = ThreadPool_join(thd4, NULL); assert(rc == 0);
        rc = ThreadPool_join(thd5, NULL); assert(rc == 0);
        rc = ThreadPool_join(thd6, NULL); assert(rc == 0);
        rc = ThreadPool_join(thd7, NULL); assert(rc == 0);
        rc = ThreadPool_join(thd8, NULL); assert(rc == 0);
        rc = ThreadPool_join(thd9, NULL); assert(rc == 0);
        rc = ThreadPool_join(thd10, NULL); assert(rc == 0);
    }
    std::clock_t end = clock();

    std::cout << "Time elapsed using thread pool:" << end-begin <<" clocks"<<'\n';
}

int main(){
    
    conventionalThreadCreation();
    threadPoolMethod();
    return 0;
}
