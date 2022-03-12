#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <windows.h>
#include <synchapi.h>
#include <time.h>

typedef void (*thread_func)(void* arg);

struct thread_pool_work 
{
    thread_func func;
    void* arg;          
    struct thread_pool_work* next;
};

struct thread_pool 
{
    thread_pool_work* work_first;
    thread_pool_work* work_last;
    CRITICAL_SECTION work_mutex;
    CONDITION_VARIABLE work_cond;
    CONDITION_VARIABLE working_cond;
    size_t working_counter;             
    size_t thread_counter;           
    bool stop;                  
};

static thread_pool_work* thread_pool_work_get(thread_pool* tp)
{
    thread_pool_work* work = tp->work_first;
    if (work == NULL)
        return NULL;

    tp->work_first = work->next;

    return work;
}

static DWORD WINAPI thread_pool_worker(LPVOID arg)
{
    thread_pool* tp = (thread_pool*)arg;
    thread_pool_work* work;

    while (1) 
    {
        EnterCriticalSection(&(tp->work_mutex));
        if (tp->stop)
            break;

        if (tp->work_first == NULL)
            SleepConditionVariableCS(&(tp->work_cond), &(tp->work_mutex), INFINITE);

        work = thread_pool_work_get(tp);
        tp->working_counter++;
        LeaveCriticalSection(&(tp->work_mutex));

        if (work != NULL) 
        {
            work->func(work->arg);
            deletemod(&work);
        }

        EnterCriticalSection(&(tp->work_mutex));
        tp->working_counter--;

        if (!tp->stop && tp->working_counter == 0 && tp->work_first == NULL)
            WakeConditionVariable(&(tp->working_cond));

        LeaveCriticalSection(&(tp->work_mutex));
    }

    tp->thread_counter--;
    if (tp->thread_counter == 0)
        WakeConditionVariable(&(tp->working_cond));

    LeaveCriticalSection(&(tp->work_mutex));

    return NULL;
}

thread_pool* thread_pool_create(size_t num)
{
    thread_pool* tp = new thread_pool{};
    tp->thread_counter = num;

    InitializeCriticalSection(&(tp->work_mutex));
    InitializeConditionVariable(&(tp->work_cond));
    InitializeConditionVariable(&(tp->working_cond));

    tp->work_first = NULL;
    tp->work_last = NULL;

    HANDLE thread;
    for (size_t i = 0; i < num; i++)
    {
        thread = CreateThread(NULL, 0, thread_pool_worker, tp, 0, NULL);
        CloseHandle(thread);
    }

    return tp;
}

void thread_pool_wait(thread_pool* tp)
{
    EnterCriticalSection(&(tp->work_mutex));
    while (1)
    {
        if ((!tp->stop && tp->working_counter != 0) || (tp->stop && tp->thread_counter != 0))
        {
            SleepConditionVariableCS(&(tp->working_cond), &(tp->work_mutex), INFINITE);
        }
        else {
            break;
        }
    }
    LeaveCriticalSection(&(tp->work_mutex));
}

void thread_pool_destroy(thread_pool* tp)
{
    EnterCriticalSection(&(tp->work_mutex));

    thread_pool_work* work = tp->work_first;
    thread_pool_work* work2;

    while (work != NULL)
    {
        work2 = work->next;
        deletemod(&work);
        work = work2;
    }
    tp->stop = true;

    WakeAllConditionVariable(&(tp->work_cond));
    LeaveCriticalSection(&(tp->work_mutex));

    thread_pool_wait(tp);

    DeleteCriticalSection(&(tp->work_mutex));

    deletemod(&tp);
}

bool thread_pool_add_work(thread_pool* tp, thread_func func, void* arg)
{
    thread_pool_work* work = new thread_pool_work{};
    work->func = func;
    work->arg = arg;
    work->next = NULL;

    EnterCriticalSection(&(tp->work_mutex));

    if (tp->work_first == NULL) 
    {
        tp->work_first = work;
        tp->work_last = tp->work_first;
    }
    else {
        tp->work_last->next = work;
        tp->work_last = work;
    }

    WakeAllConditionVariable(&(tp->work_cond));
    LeaveCriticalSection(&(tp->work_mutex));

    return true;
}

int get_cpu_threads()
{
    DWORD len = 0;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION info = NULL;

    GetLogicalProcessorInformation(info, &len);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return 1;

    info = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION) new uint8_t[len];
    if (!GetLogicalProcessorInformation(info, &len))
    {
        deletemod(&info);
        return 1;
    }

    int num = 0;
    ULONG_PTR n;
    len = len / sizeof(*info);
    for (DWORD i = 0; i < len; i++)
    {
        switch (info[i].Relationship)
        {
            case RelationProcessorCore:
            {
                n = info[i].ProcessorMask;
                while (n > 0)
                {
                    n &= n - 1;
                    num++;
                }
                break;
            }
            default:
                break;
        }
    }

    deletemod(&info);

    if (num == 0)
        num = 1;

    return num;
}