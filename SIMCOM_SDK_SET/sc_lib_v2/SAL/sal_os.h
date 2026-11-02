#ifndef __SAL_OS_H__
#define __SAL_OS_H__

#include <stdbool.h>

#define SAL_32  (0x20)
#define SAL_64  (0x40)
#define SAL_128 (0x80)
#define SAL_256 (0x100)
#define SAL_512 (0x200)
#define SAL_1K  (0x400)
#define SAL_2K  (0x800)
#define SAL_3K  (0xc00)
#define SAL_4K  (0x1000)
#define SAL_5K  (0x1400)
#define SAL_6K  (0x1800)
#define SAL_7K  (0x1c00)
#define SAL_8K  (0x2000)
#define SAL_9K  (0x2400)
#define SAL_10K (0x2800)
#define SAL_11K (0x2c00)
#define SAL_12K (0x3000)
#define SAL_13K (0x3400)
#define SAL_14K (0x3800)
#define SAL_15K (0x3c00)
#define SAL_16K (0x4000)
#define SAL_17K (0x4400)
#define SAL_18K (0x4800)
#define SAL_19K (0x4c00)
#define SAL_20K (0x5000)
#define SAL_24K (0x6000)
#define SAL_28K (0x7000)
#define SAL_32K (0x8000)


typedef void *sal_task_ref;
typedef void *sal_msgq_ref;
typedef void *sal_timer_ref;
typedef void *sal_sema_ref;
typedef void *sal_mutex_ref;
typedef void *sal_flag_ref;

#define SAL_OS_SUCCESS            (0)
#define SAL_OS_FAIL               (1)
#define SAL_OS_TIMEOUT_SUSPEND    (0xFFFFFFFF)
#define SAL_OS_TIMEOUT_NO_SUSPEND (0)

enum sal_task_priority
{
    sal_task_priority_lowest_3,
    sal_task_priority_lowest_2,
    sal_task_priority_lowest_1,
    sal_task_priority_low_3,
    sal_task_priority_low_2,
    sal_task_priority_low_1,  // app default priority
    sal_task_priority_middle_3,
    sal_task_priority_middle_2,
    sal_task_priority_middle_1,
    /* unnecessary,don't use priority below */
    sal_task_priority_high_3,
    sal_task_priority_high_2,
    sal_task_priority_high_1,
    sal_task_priority_highest_3,
    sal_task_priority_highest_2,
    sal_task_priority_highest_1,
    sal_task_priority_system_3,
    sal_task_priority_system_2,
    sal_task_priority_system_1,
    sal_task_priority_max
};

/**
 * The function allocates memory and initializes it to zero.
 *
 * @param size The size parameter is specifies the number of bytes to allocate for the memory block.
 *
 * @return The function `sal_malloc` is returning a pointer to a block of memory allocated using.
 */
#ifdef SIMCOM_DEBUG
inline void *_sal_malloc(unsigned size, bool trace, const char *func, const int line);

inline void _sal_free(void *ptr, const char *func, const int line);


void _clear_memory_info(void *ptr);
#define sal_clear_memory_info(ptr) _clear_memory_info(ptr)

#define sal_malloc(size) _sal_malloc((size), true, __FUNCTION__, __LINE__)
#define sal_malloc_notrace(size) _sal_malloc((size), false, __FUNCTION__, __LINE__)

#define sal_free(ptr) do{ \
        if(ptr){_sal_free(ptr, __FUNCTION__, __LINE__);ptr=NULL;} \
    }while(0)

#else

#define sal_clear_memory_info(ptr)

inline void *sal_malloc(unsigned size);
inline void _sal_free(void *ptr);

#define sal_malloc_notrace(size) sal_malloc(size)

#define sal_free(ptr) do{ \
        if(ptr){_sal_free(ptr);ptr=NULL;} \
    }while(0)
#endif



/**
 * This function creates a new task .
 *
 * @param ref The pointer to save task reference of the created task.
 * @param stack A pointer to the memory block that will be used as the task's stack. Or NULL to malloc stack automatically.
 * @param stackSize The size of the stack allocated for the task. This determines how much memory the task can use for its local variables and function calls.
 * @param priority The priority of the task being created. It is an enum value of type "sal_task_priority".
 * @param name A string that represents the name of the task being created.
 * @param task A pointer to the function that will be executed by the task.
 * @param args args is a void pointer that can be used to pass any additional arguments to the task function. The task function can then cast this pointer to the appropriate type and use the arguments as needed.
 *
 * @return SAL_OS_SUCCESS when success.
 */
#ifdef SIMCOM_DEBUG
int _sal_task_create(sal_task_ref *ref,
                 void *stack,
                 unsigned stackSize,
                 enum sal_task_priority priority,
                 char *name,
                 void (*task)(void *),
                 void *args,
                 const char *func,
                 const int line);

inline int _sal_task_delete(sal_task_ref *ref,
                        const char *func,
                        const int line);

#define sal_task_create(refp, stack, stackSize, priority, name, task, args) \
    _sal_task_create(refp, stack, stackSize, priority, name, task, args, __FUNCTION__, __LINE__)

#define sal_task_delete(refp) \
    _sal_task_delete(refp, __FUNCTION__, __LINE__)

#else

int sal_task_create(sal_task_ref *ref,
                    void *stack,
                    unsigned stackSize,
                    enum sal_task_priority priority,
                    char *name,
                    void (*task)(void *),
                    void *args);

inline int sal_task_delete(sal_task_ref *ref);
#endif


inline int sal_task_detach(sal_task_ref *ref);

inline void sal_task_sleep(int ms);

/**
 * This function creates a message queue with a specified maximum size and number of messages
 *
 * @param ref A pointer will hold the reference to the created message queue.
 * @param name The name of the message queue.
 * @param maxSize The maximum size of each message that can be stored in the message queue.
 * @param maxNumber The maximum number of messages that can be stored in the message queue.
 *
 * @return SAL_OS_SUCCESS on sucess.
 */
#ifdef SIMCOM_DEBUG
inline int _sal_msgq_create(sal_msgq_ref *ref, char *name,
                        unsigned maxSize, unsigned maxNumber,
                        const char *func, const int line);

inline int _sal_msgq_delete(sal_msgq_ref *ref,
                        const char *func,
                        const int line);

#define sal_msgq_create(ref, name, maxSize, maxNumber) \
    _sal_msgq_create(ref, name, maxSize, maxNumber, __FUNCTION__, __LINE__)

#define sal_msgq_delete(ref) \
    _sal_msgq_delete(ref, __FUNCTION__, __LINE__)

#else

inline int sal_msgq_create(sal_msgq_ref *ref, char *name,
                           unsigned maxSize, unsigned maxNumber);

inline int sal_msgq_delete(sal_msgq_ref *ref);
#endif

/**
 * This function sends a message to a message queue with a specified timeout.
 *
 * @param ref A reference to the message queue to which the message will be sent.
 * @param msg A pointer to the message to be sent through the message queue.
 * @param size The size of the message being sent in bytes.
 * @param timeout_ms The timeout in milliseconds for sending a message to the message queue.
 * If the value is SAL_OS_TIMEOUT_SUSPEND, the task will be suspended until the message can be sent.
 *  If the value is SAL_OS_TIMEOUT_NO_SUSPEND, the function will return immediately if the message cannot be sent.
 * Otherwise, the function will return after timeout_ms if the message cannot be send.
 *
 * @return SAL_OS_SUCCESS on sucess.
 */
inline int sal_msgq_send(sal_msgq_ref ref, void *msg, unsigned size, unsigned timeout_ms);

/**
 * This function receives a message from a message queue with a specified timeout.
 *
 * @param ref A reference to the message queue to receive the message from.
 * @param msg A pointer to the buffer where the received message will be stored.
 * @param size The size parameter specifies the maximum size of the message that can be received from the message queue.
 * @param timeout_ms The maximum time in milliseconds that the function will wait for a message to be received from the message queue. If no message is received within this time, the function will return with an error code.
 * If the value of timeout_ms is SAL_OS_TIMEOUT_SUSPEND, the function will
 * wait indefinitely until a message received from the message queue.
 *
 * @return SAL_OS_SUCCESS on sucess.
 */
inline int sal_msgq_recv(sal_msgq_ref ref, void *msg, unsigned size, unsigned timeout_ms);

/**
 * The function creates a timer.
 *
 * @param ref A pointer that will be set to the newly created timer reference.
 * @param cb cb is a function pointer that points to the callback function that will be executed when the timer expires.
 * The callback function takes a void pointer as an argument, which can be used to pass any additional data or context to the callback function.
 * @param args args is a void pointer that represents any additional arguments that need to be passed to the callback function when the timer expires. It can be used to pass any data or context that the callback function needs to perform its task.
 *
 * @return SAL_OS_SUCCESS on sucess.
 */
#ifdef SIMCOM_DEBUG
inline int _sal_timer_create(sal_timer_ref *ref,
                         void (*cb)(void *args),
                         void *args,
                         const char *func,
                         const int line);

inline int _sal_timer_delete(sal_timer_ref *ref,
                         const char *func,
                         const int line);

#define sal_timer_create(ref, cb, args) \
    _sal_timer_create(ref, cb, args, __FUNCTION__, __LINE__)

#define sal_timer_delete(ref) \
    _sal_timer_delete(ref, __FUNCTION__, __LINE__)

#else

inline int sal_timer_create(sal_timer_ref *ref, void (*cb)(void *args), void *args);

inline int sal_timer_delete(sal_timer_ref *ref);
#endif

/**
 * This function starts a timer with a specified time interval and callback function.
 *
 * @param ref A reference to a timer object.
 * @param time_ms The duration of the timer in milliseconds.
 * @param repeat A boolean value indicating whether the timer should repeat after it expires or not.
 * If set to true, the timer will restart and repeat the same callback function after the specified time interval.
 * If set to false, the timer will only execute the callback function once and then stop.
 *
 * @return SAL_OS_SUCCESS on sucess.
 */
inline int sal_timer_start(sal_timer_ref ref, unsigned int time_ms, bool repeat);

inline int sal_timer_stop(sal_timer_ref ref);


#ifdef SIMCOM_DEBUG
inline int _sal_semaphore_create(sal_sema_ref *ref,
                             unsigned initalCount,
                             const char *func,
                             const int line);

inline int _sal_semaphore_delete(sal_sema_ref *ref,
                             const char *func,
                             const int line);

#define sal_semaphore_create(ref, initalCount) \
    _sal_semaphore_create(ref, initalCount, __FUNCTION__, __LINE__)

#define sal_semaphore_delete(ref) \
    _sal_semaphore_delete(ref, __FUNCTION__, __LINE__)

#else

inline int sal_semaphore_create(sal_sema_ref *ref, unsigned initalCount);

inline int sal_semaphore_delete(sal_sema_ref *ref);
#endif

inline int sal_semaphore_acquire(sal_sema_ref ref, unsigned timeout_ms);

inline int sal_semaphore_release(sal_sema_ref ref);


#ifdef SIMCOM_DEBUG
inline int _sal_mutex_create(sal_mutex_ref *ref,
                         const char *func,
                         const int line);

inline int _sal_mutex_delete(sal_mutex_ref *ref,
                         const char *func,
                         const int line);

#define sal_mutex_create(ref) \
    _sal_mutex_create(ref, __FUNCTION__, __LINE__)

#define sal_mutex_delete(ref) \
    _sal_mutex_delete(ref, __FUNCTION__, __LINE__)

#else

inline int sal_mutex_create(sal_mutex_ref *ref);

inline int sal_mutex_delete(sal_mutex_ref *ref);
#endif

inline int sal_mutex_lock(sal_mutex_ref ref, unsigned timeout_ms);

inline int sal_mutex_unlock(sal_mutex_ref ref);


#ifdef SIMCOM_DEBUG
inline int _sal_flag_create(sal_flag_ref *ref,
                        const char *func,
                        const int line);

inline int _sal_flag_delete(sal_flag_ref *ref,
                        const char *func,
                        const int line);

#define sal_flag_create(ref) \
    _sal_flag_create(ref, __FUNCTION__, __LINE__)

#define sal_flag_delete(ref) \
    _sal_flag_delete(ref, __FUNCTION__, __LINE__)

#else

inline int sal_flag_create(sal_flag_ref *ref);

inline int sal_flag_delete(sal_flag_ref *ref);
#endif

inline int sal_flag_set(sal_flag_ref ref, unsigned mask);

inline int sal_flag_wait(sal_flag_ref ref, unsigned mask, unsigned *flags, unsigned timeout_ms);

inline int sal_flag_clean(sal_flag_ref ref, unsigned mask);

#endif /* __SAL_OSAPI_H__ */
