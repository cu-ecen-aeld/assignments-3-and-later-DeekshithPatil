#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

#define MILLI_TO_MICRO    (1000)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    thread_data* data = (thread_data *) thread_param;

    //sleep for the specified time before acquiring the mutex
    int ret = usleep(data->wait_to_obtain_ms * MILLI_TO_MICRO);
    if(ret !=0)
    {
        //usleep has failed
        ERROR_LOG("usleep() has failed!\n");
        data->thread_complete_success = false;
        return thread_param;
    }

    //obtain the mutex
    ret = pthread_mutex_lock(data->mutex);
    if(ret!=0)
    {
        //unable to lock mutex
        ERROR_LOG("pthread_mutex_lock() has failed!\n");
        data->thread_complete_success = false;
        return thread_param;
    }

    //sleep for the specified time before releasing the mutex
    ret = usleep(data->wait_to_release_ms * MILLI_TO_MICRO);
    if(ret!=0)
    {
        //usleep has failed
        ERROR_LOG("usleep() has failed!\n");
        data->thread_complete_success = false;
        return thread_param;
    }

    //release the mutex
    ret = pthread_mutex_unlock(data->mutex);
    if(ret!=0)
    {
        //unable to unlock mutex
        ERROR_LOG("pthread_mutex_unlock() has failed!\n");
        data->thread_complete_success = false;
        return thread_param;
    }

    //set the thread_complete_success variable
    data->thread_complete_success = true;

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    thread_data *data = (thread_data *) malloc(sizeof(thread_data));

    if(data == NULL)
    {
        //Malloc was unable to allocate space for the required structure
        ERROR_LOG("Malloc failed!\n");
        return false;
    }

    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms;
    data->mutex = mutex;

    int ret = pthread_create(thread,NULL,threadfunc,(void *)data);

    if(ret!=0)
    {
        //Pthread_create() was unable to create the thread. 
        ERROR_LOG("pthread_create() failed!\n");
        return false;
    }
    return true;
}
