#include <pthread.h>
#include <sys/types.h>
#include <vector>
#include <cassert>
#include <iostream>
#include <errno.h>
#include <string.h>
#include<unistd.h>

#define INITIAL_THREAD_COUNT 0
#define INCREMENT_THREAD_COUNT 5

struct ThreadID{
    pthread_t id;
    pthread_mutex_t* mtx;
    pthread_cond_t* cv; 
    bool* runFunc;
    void* (*run)(void *); 
    void* args;
    void** ret;
};

class ThreadPool{
    public:

        int totalThreadCount;
        std::vector<ThreadID*> threads;
        pthread_mutex_t globalMtx;
        ThreadPool();
        ~ThreadPool();
};

void* ThreadPool_helper(void* th){
    struct ThreadID* obj = (struct ThreadID*) th;

    // Initialize mutex and conditional variable
    int rv;

    while(1){
        rv = pthread_mutex_lock(obj->mtx); assert(rv == 0);
        while(*(obj->runFunc) == false){
            rv = pthread_cond_wait(obj->cv, obj->mtx); assert(rv == 0);
        }
        
        *(obj->ret) = (*(obj->run))(obj->args);
        *(obj->runFunc) = false;
        rv = pthread_mutex_unlock(obj->mtx); assert(rv == 0);
    }

    return 0;
  
}

ThreadPool::ThreadPool(){
    int rv;
    pthread_mutexattr_t attr;
    rv = pthread_mutexattr_init(&attr); assert(rv == 0);
    rv = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK); assert(rv == 0);    
    rv = pthread_mutex_init(&(globalMtx),&attr); assert(rv == 0);
    rv = pthread_mutexattr_destroy(&attr); assert(rv == 0);
    totalThreadCount = 0;

}

ThreadPool::~ThreadPool(){
    for(int i=0;i<totalThreadCount;i++){
        delete threads[i]->runFunc;
        delete threads[i]->ret;
        delete threads[i]->mtx;
        delete threads[i]->cv;
        delete threads[i];
    }

}

int createExtraThreads(struct ThreadPool *tp){
    int oldThreadcount = tp->totalThreadCount;
    //tp->threads.reserve(oldThreadcount + INCREMENT_THREAD_COUNT);

    for(int i=0;i<INCREMENT_THREAD_COUNT;i++){

        struct ThreadID* temp = new ThreadID();  
    

        pthread_t t1;

        temp->runFunc = new bool();
        temp->ret = new void*();
        temp->mtx = new pthread_mutex_t();
        temp->cv = new pthread_cond_t();
        *(temp->runFunc) = false;
        int rv;

        pthread_mutexattr_t attr;
        rv = pthread_mutexattr_init(&attr); assert(rv == 0);
        rv = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK); assert(rv == 0);    
        rv = pthread_mutex_init(temp->mtx,&attr); assert(rv == 0);
        rv = pthread_mutexattr_destroy(&attr); assert(rv == 0);

        rv = pthread_cond_init(temp->cv, nullptr); assert(rv == 0);

        //tp->threads[oldThreadcount+i].id = t1;
        tp->threads.push_back(temp);
        rv = pthread_create(&t1, NULL, &ThreadPool_helper, temp); assert(rv == 0);
        

    }

    // update total thread counts
    tp->totalThreadCount = oldThreadcount + INCREMENT_THREAD_COUNT; 
    return oldThreadcount;
}

int ThreadPool_run(struct ThreadPool *tp, struct ThreadID* threadId, void* (*run)(void *), void * args){
    // First find if we have any free thread available or not
    int rv;
    rv = pthread_mutex_lock(&(tp->globalMtx)); assert(rv == 0);
    
    // pick first free thread
    bool isThreadFree = false;
    for(int i=0;i<tp->totalThreadCount;i++){
        if(*(tp->threads[i]->runFunc) == false){
            rv = pthread_mutex_lock(tp->threads[i]->mtx); assert(rv == 0);
            tp->threads[i]->run = run;
            tp->threads[i]->args = args;
            rv = pthread_mutex_unlock(tp->threads[i]->mtx); assert(rv == 0);

            *(tp->threads[i]->runFunc) = true;
            rv = pthread_cond_signal(tp->threads[i]->cv); assert(rv == 0);

            *threadId = *(tp->threads[i]);
            isThreadFree = true;
            break;
        }        
    }

    // This means we need to create more threads
    if(isThreadFree == false){
        std::cerr << "Creating few more threads" << '\n';
        int i = createExtraThreads(tp); 
        int rv;        

        rv = pthread_mutex_lock(tp->threads[i]->mtx); assert(rv == 0);
        tp->threads[i]->run = run;
        tp->threads[i]->args = args;
        rv = pthread_mutex_unlock(tp->threads[i]->mtx); assert(rv == 0);
        *(tp->threads[i]->runFunc) = true;
        rv = pthread_cond_signal(tp->threads[i]->cv); assert(rv == 0);
        *threadId = *(tp->threads[i]);
    }
    
    rv = pthread_mutex_unlock(&(tp->globalMtx)); assert(rv == 0);

    return 0;
}

int ThreadPool_join(struct ThreadID thdid, void **ret){
    while(*(thdid.runFunc) == true);
    if(ret != NULL){
        *ret = *(thdid.ret);
    }
    return 0;
}
