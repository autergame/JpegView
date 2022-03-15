//author https://github.com/autergame
#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <synchapi.h>

#define thread_pool_func(func_name, arg_var) \
void CALLBACK func_name(PTP_CALLBACK_INSTANCE Instance, PVOID arg_var, PTP_WORK Work)

typedef struct thread_pool_
{
    TP_CALLBACK_ENVIRON CallBackEnviron;
    PTP_CLEANUP_GROUP cleanupgroup;
    PTP_POOL pool;
} thread_pool;

thread_pool* thread_pool_create(int cpu_threads)
{
    thread_pool* tp = (thread_pool*)calloc(1, sizeof(thread_pool));

    InitializeThreadpoolEnvironment(&tp->CallBackEnviron);

    tp->pool = CreateThreadpool(NULL);

    SetThreadpoolThreadMinimum(tp->pool, cpu_threads);
    SetThreadpoolThreadMaximum(tp->pool, cpu_threads);

    tp->cleanupgroup = CreateThreadpoolCleanupGroup();

    SetThreadpoolCallbackPool(&tp->CallBackEnviron, tp->pool);
    SetThreadpoolCallbackCleanupGroup(&tp->CallBackEnviron, tp->cleanupgroup, NULL);

    return tp;
}

void thread_pool_add_work(thread_pool* tp, PTP_WORK_CALLBACK func, void* arg)
{
    PTP_WORK work = CreateThreadpoolWork(func, arg, &tp->CallBackEnviron);
    SubmitThreadpoolWork(work);
}

void thread_pool_destroy(thread_pool* tp)
{
    CloseThreadpoolCleanupGroupMembers(tp->cleanupgroup, FALSE, NULL);
    CloseThreadpoolCleanupGroup(tp->cleanupgroup);

    DestroyThreadpoolEnvironment(&tp->CallBackEnviron);

    CloseThreadpool(tp->pool);

    free(tp);
}

int get_cpu_threads()
{
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    return sys_info.dwNumberOfProcessors;
}