#include "acoroutine.h"

struct ACoroutine_st;

struct AcoScheduler_st {
    char stack[_INT_STACK];
#ifdef _WIN32
    LPVOID main;             
#else
    ucontext_t main;   
#endif         
    int running;         
    int nco;        
    int cap;                    
    struct ACoroutine_st ** co; 
};

struct ACoroutine_st {
#ifdef _WIN32
    LPVOID ctx;  
#else        
    char * stack;
    ucontext_t ctx;   
    ptrdiff_t cap;
    ptrdiff_t size;    
#endif
    const char *name;
    acoroutine_func func;         
    void * ud;              
    AcoStatus status;           
    AcoScheduler * manager;    
    int index;
};


#ifndef _WIN32
static inline void _aco_delete(struct ACoroutine_st * co) {
    free(co->stack);
    co->status = ACO_Dead;
}
#endif




inline AcoScheduler * aco_scheduler_start(void) {
    AcoScheduler * manager = malloc(sizeof(AcoScheduler));
    if(NULL == manager) {
        return NULL;
    }
    manager->nco = 0;
    manager->running = -1;
    manager->co = calloc(manager->cap = _INT_COROUTINE, sizeof(struct ACoroutine_st *));
    if(NULL == manager->co) {
        free(manager);
        return NULL;
    }
#ifdef _WIN32
    manager->main = ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH);
#endif
    return manager;
}

void aco_scheduler_close(AcoScheduler *manager) {
    int i;
    for (i = 0; i < manager->cap; ++i) {
        struct ACoroutine_st * co = manager->co[i];
        if (co) {
#ifdef _WIN32
            co->status = ACO_Dead;
#else
            _aco_delete(co);
#endif
            manager->co[i] = NULL;
        }
    }
    free(manager->co);
    manager->co = NULL;
    free(manager);
#ifdef _WIN32
    ConvertFiberToThread();
#endif
}

static inline struct ACoroutine_st * _aco_new(AcoScheduler *manager, const char *name, acoroutine_func func, void * ud) {
    if(!(manager && func)) {
        return NULL;
    }
    struct ACoroutine_st * co = malloc(sizeof(struct ACoroutine_st));
    if(!co) {
        return NULL;
    }
    co->name = name;
    co->func = func;
    co->ud = ud;
    co->manager = manager;
    co->status = ACO_Ready;
#ifndef _WIN32
    co->cap = 0;
    co->size = 0;
    co->stack = NULL;
#endif
    return co;
}

ACoroutine * acoroutine_create(AcoScheduler *manager, const char *name, acoroutine_func func, void * ud) {
    struct ACoroutine_st * co = _aco_new(manager, name, func, ud);
    if (manager->nco < manager->cap) {
        int i;
        for (i = 0; i < manager->cap; ++i) {
            int id = (i + manager->nco) % manager->cap;
            if (NULL == manager->co[id]) {
                manager->co[id] = co;
                co->index = id;
                ++manager->nco;
                return co;
            }
        }
        free(co);
        return NULL;
    }
    manager->co = realloc(manager->co, sizeof(struct ACoroutine_st *) * manager->cap * 2);
    if(NULL == manager->co) {
        free(co);
        return NULL;
    }
    memset(manager->co + manager->cap, 0, sizeof(struct ACoroutine_st *) * manager->cap);
    manager->cap <<= 1;
    manager->co[manager->nco] = co;
    co->index = manager->nco;
    manager->nco++;
    return co;
}

inline void acoroutine_destroy(ACoroutine *co) {
#ifdef _WIN32
    DeleteFiber(co->ctx);
    co->ctx = NULL;
#endif
    free(co);
}

#ifdef _WIN32
static inline VOID WINAPI _aco_main(LPVOID ptr) {
    AcoScheduler * manager = ptr;
#else
static inline void _aco_main(uint32_t low32, uint32_t hig32) {
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hig32 << 32);
    AcoScheduler * manager = (AcoScheduler *)ptr;
#endif
    int id = manager->running;
    struct ACoroutine_st * co = manager->co[id];
    co->func(manager, co->ud);
#ifdef _WIN32
    co->status = ACO_Dead;
#else
    _aco_delete(co);
#endif
    manager->co[id] = NULL;
    --manager->nco;
    manager->running = -1;
#ifdef _WIN32
    SwitchToFiber(manager->main);
#endif
}

void aco_scheduler_resume(AcoScheduler *manager, ACoroutine *co) {
    if(NULL == co || co->status == ACO_Dead)
        return;
    switch(co->status) {
    case ACO_Dead:
        break;
    case ACO_Ready:
        manager->running = co->index;
        co->status = ACO_Running;
#ifdef _WIN32
        co->ctx = CreateFiberEx(_INT_STACK, 0, FIBER_FLAG_FLOAT_SWITCH, _aco_main, manager);
        manager->main = GetCurrentFiber();
        SwitchToFiber(co->ctx);
#else
        getcontext(&co->ctx);
        co->ctx.uc_stack.ss_sp = manager->stack;
        co->ctx.uc_stack.ss_size = _INT_STACK;
        co->ctx.uc_link = &manager->main;
        uintptr_t ptr = (uintptr_t)manager;
        makecontext(&co->ctx, (void (*)())_aco_main, 2, (uint32_t)ptr, (uint32_t)(ptr >> 32));
        swapcontext(&manager->main, &co->ctx);
#endif
        break;
    case ACO_Running:
        break;
    case ACO_Suspend:
        manager->running = co->index;
        co->status = ACO_Running;
#ifdef _WIN32
        manager->main = GetCurrentFiber();
        SwitchToFiber(co->ctx);
#else
        memcpy(manager->stack + _INT_STACK - co->size, co->stack, co->size);
        swapcontext(&manager->main, &co->ctx);
#endif
        break;
    }
}

#ifndef _WIN32
static void _aco_save_stack(struct ACoroutine_st * co, char * top) {
    char dummy = 0;
    if(top - &dummy > _INT_STACK) {
        printf("coroutine stack overflow\n");
        abort();
    }
    if(co->cap < top - &dummy) {
        free(co->stack);
        co->cap = top - &dummy;
        co->stack = malloc(co->cap);
    }
    co->size = top - &dummy;
    memcpy(co->stack, &dummy, co->size);
}
#endif

inline void aco_scheduler_yield(AcoScheduler *manager) {
    struct ACoroutine_st * co;
    int id = manager->running;
    if(id < 0) {
        return ;
    }
    co = manager->co[id];
#ifdef _WIN32
    co->status = ACO_Suspend;
    manager->running = -1;
    co->ctx = GetCurrentFiber();
    SwitchToFiber(manager->main);
#else
    if((char *)&co <= manager->stack) {
        printf("coroutine stack overflow\n");
        abort();
    }
    _aco_save_stack(co, manager->stack + _INT_STACK);
    co->status = ACO_Suspend;
    manager->running = -1;
    swapcontext(&co->ctx, &manager->main);
#endif
}

inline const char * acoroutine_name(ACoroutine *co) {
    return co->name;
}

inline AcoStatus acoroutine_status(ACoroutine *co) {
    return co ? co->status : ACO_Dead;
}

inline ACoroutine * aco_scheduler_running(AcoScheduler *manager) {
    int id = manager->running;
    if(id < 0) {
        return NULL;
    }
    return manager->co[id];
}
