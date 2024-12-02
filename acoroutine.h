#ifndef _ACOROUTINE_H_
#define _ACOROUTINE_H_

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#ifdef _WIN32
#include <Windows.h>
#else

#include <ucontext.h>
#include <stddef.h>
#include <stdint.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define _INT_STACK        (1024 * 1024)
#define _INT_COROUTINE    (32)

struct AcoScheduler_st;
typedef struct AcoScheduler_st AcoScheduler;

struct ACoroutine_st;
typedef struct ACoroutine_st ACoroutine;

typedef void (* acoroutine_func)(AcoScheduler *aschd, void * arg);

typedef enum AcoStatus_st {       
    ACO_Dead       = 0,       
    ACO_Ready      = 1,      
    ACO_Running    = 2,     
    ACO_Suspend    = 3,     
} AcoStatus;


extern AcoScheduler *aco_scheduler_start(void);
extern void aco_scheduler_close(AcoScheduler *aschd);
extern ACoroutine * acoroutine_create(AcoScheduler *aschd, const char *name, acoroutine_func func, void * arg);
extern void acoroutine_destroy(ACoroutine *co);
extern void aco_scheduler_resume(AcoScheduler *aschd, ACoroutine *co);
extern void aco_scheduler_yield(AcoScheduler *aschd);
extern AcoStatus acoroutine_status(ACoroutine *co);
extern const char * acoroutine_name(ACoroutine *co);
extern ACoroutine * aco_scheduler_running(AcoScheduler *manager);

#endif 