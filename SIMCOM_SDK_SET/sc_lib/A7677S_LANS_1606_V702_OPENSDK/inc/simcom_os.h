#ifndef _SIMCOM_OS_H
#define _SIMCOM_OS_H

//#include "simcom_common.h"

#ifndef UINT8
typedef unsigned char   UINT8;
#endif

#ifndef UINT16
typedef unsigned short  UINT16;
#endif

#ifndef UINT32
typedef unsigned long   UINT32;
#endif

#ifndef UINT64
typedef unsigned long long  UINT64;
#endif

#ifndef BOOL
typedef unsigned char   BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef INT8
typedef signed char     INT8;
#endif

#ifndef INT16
typedef signed short    INT16;
#endif

#ifndef INT32
typedef signed long     INT32;
#endif

#ifndef INT64
typedef long long  INT64;
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif


typedef void   *sTaskRef;
typedef void   *sSemaRef;
typedef void   *sMutexRef;
typedef void   *sMsgQRef;
typedef void   *sTimerRef;
typedef void   *sFlagRef;

typedef struct sim_msg_cell
{
    UINT32 msg_id;
    int arg1;
    int arg2;
    void *arg3;
} SIM_MSG_T;

typedef struct
{
    UINT32  status;         /* timer status SC_ENABLED, SC_DISABLED    */
    UINT32  expirations;    /* number of expirations for cyclic timers */
} sTimerStatus;

typedef struct
{
    int tv_sec;        /* seconds */
    int tv_usec;       /* and microseconds */
} sTimeval;

typedef enum
{
	SC_TASK_READY,
	SC_TASK_COMPLETED,
	SC_TASK_TERMINATED,
	SC_TASK_SUSPENDED,
	SC_TASK_SLEEP,
	SC_TASK_QUEUE_SUSP,
	SC_TASK_SEMAPHORE_SUSP,
	SC_TASK_EVENT_FLAG,
	SC_TASK_BLOCK_MEMORY,
	SC_TASK_MUTEX_SUSP,
	SC_TASK_STATE_UNKNOWN,
}SC_TASK_STATE;

typedef struct
{
    char                *task_name;                /* Pointer to thread's name     */
    unsigned int        task_priority;             /* Priority of thread (0-255)  */
    unsigned long        task_stack_def_val;             /* default vaule of  thread  */
    SC_TASK_STATE      task_state;                /* Thread's execution state     */
    unsigned long       task_stack_ptr;           /* Thread's stack pointer   */
    unsigned long       task_stack_start;         /* Stack starting address   */
    unsigned long       task_stack_end;           /* Stack ending address     */
    unsigned long       task_stack_size;           /* Stack size               */
    unsigned long       task_run_count;            /* Thread's run counter     */

} sTaskInfo;

#ifdef SIMCOM_ACC_TIMER_SUPPORT
typedef void(*SC_ACC_TIMER_CALLBACK)(UINT32);	/* Function pointer */

typedef struct 				/* ACC timer configuration */
{
    unsigned int flag;
    unsigned int period;
    SC_ACC_TIMER_CALLBACK timerCallbackFunc;
    unsigned int timerParams;
} sAccTimerConfig;

typedef enum    /* determine whether the timer is started or not*/
{
    SC_ACC_ACTIVE,                      /* the node has been added in the active_list and callback function has not been executed */
    SC_ACC_INACTIVE,                    /* the node is not in the active_list*/
    SC_ACC_TIMER_ID_NOT_EXIST           /* timer_id does not exist */
}SC_ACC_TIMER_STATUS;

typedef enum   /* The meaning of the API flag*/
{
    SC_ACC_TIMER_PERIODIC = 0x1,		/* periodic execution */
    SC_ACC_TIMER_AUTO_DELETE = 0x2	    /* one execution */
}SC_ACC_TIMER_FLAG;
#endif

#define SC_MIN_STACK_SIZE      256
#define SC_ENABLE_INTERRUPTS   1
#define SC_DISABLE_INTERRUPTS  2
#define SC_SUSPEND             0xFFFFFFFF
#define SC_NO_SUSPEND          0
#define SC_FLAG_AND            5
#define SC_FLAG_AND_CLEAR      6
#define SC_FLAG_OR             7
#define SC_FLAG_OR_CLEAR       8
#define SC_FIXED               9
#define SC_VARIABLE            10
#define SC_FIFO                11
#define SC_PRIORITY            12
#define SC_GLOBAL              13
#define SC_OS_INDEPENDENT      14
#define SC_ENABLED             2
#define SC_DISABLED            3

#define SC_TIMER_DEAD          0
#define SC_TIMER_CREATED       1
#define SC_TIMER_ACTIVE        2
#define SC_TIMER_INACTIVE      3


#define SC_DEFAULT_TASK_PRIORITY    150
#define SC_DEFAULT_THREAD_STACKSIZE 4096
/*========================================================================
 *  SC Return Error Codes
 *========================================================================*/

typedef enum
{
    SC_SUCCESS = 0,        /* 0x0 -no errors                                        */
    SC_FAIL,               /* 0x1 -operation failed code                            */
    SC_TIMEOUT,            /* 0x2 -Timed out waiting for a resource                 */
    SC_NO_RESOURCES,       /* 0x3 -Internal OS resources expired                    */
    SC_INVALID_POINTER,    /* 0x4 -0 or out of range pointer value                  */
    SC_INVALID_REF,        /* 0x5 -invalid reference                                */
    SC_INVALID_DELETE,     /* 0x6 -deleting an unterminated task                    */
    SC_INVALID_PTR,        /* 0x7 -invalid memory pointer                           */
    SC_INVALID_MEMORY,     /* 0x8 -invalid memory pointer                           */
    SC_INVALID_SIZE,       /* 0x9 -out of range size argument                       */
    SC_INVALID_MODE,       /* 0xA, 10 -invalid mode                                 */
    SC_INVALID_PRIORITY,   /* 0xB, 11 -out of range task priority                   */
    SC_UNAVAILABLE,        /* 0xC, 12 -Service requested was unavailable or in use  */
    SC_POOL_EMPTY,         /* 0xD, 13 -no resources in resource pool                */
    SC_QUEUE_FULL,         /* 0xE, 14 -attempt to send to full messaging queue      */
    SC_QUEUE_EMPTY,        /* 0xF, 15 -no messages on the queue                     */
    SC_NO_MEMORY,          /* 0x10, 16 -no memory left                              */
    SC_DELETED,            /* 0x11, 17 -service was deleted                         */
    SC_SEM_DELETED,        /* 0x12, 18 -semaphore was deleted                       */
    SC_MUTEX_DELETED,      /* 0x13, 19 -mutex was deleted                           */
    SC_MSGQ_DELETED,       /* 0x14, 20 -msg Q was deleted                           */
    SC_MBOX_DELETED,       /* 0x15, 21 -mailbox Q was deleted                       */
    SC_FLAG_DELETED,       /* 0x16, 22 -flag was deleted                            */
    SC_INVALID_VECTOR,     /* 0x17, 23 -interrupt vector is invalid                 */
    SC_NO_TASKS,           /* 0x18, 24 -exceeded max # of tasks in the system       */
    SC_NO_FLAGS,           /* 0x19, 25 -exceeded max # of flags in the system       */
    SC_NO_SEMAPHORES,      /* 0x1A, 26 -exceeded max # of semaphores in the system  */
    SC_NO_MUTEXES,         /* 0x1B, 27 -exceeded max # of mutexes in the system     */
    SC_NO_QUEUES,          /* 0x1C, 28 -exceeded max # of msg queues in the system  */
    SC_NO_MBOXES,          /* 0x1D, 29 -exceeded max # of mbox queues in the system */
    SC_NO_TIMERS,          /* 0x1E, 30 -exceeded max # of timers in the system      */
    SC_NO_MEM_POOLS,       /* 0x1F, 31 -exceeded max # of mem pools in the system   */
    SC_NO_INTERRUPTS,      /* 0x20, 32 -exceeded max # of isr's in the system       */
    SC_FLAG_NOT_PRESENT,   /* 0x21, 33 -requested flag combination not present      */
    SC_UNSUPPORTED,        /* 0x22, 34 -service is not supported by the OS          */
    SC_NO_MEM_CELLS,       /* 0x23, 35 -no global memory cells                      */
    SC_DUPLICATE_NAME,     /* 0x24, 36 -duplicate global memory cell name           */
    SC_INVALID_PARM        /* 0x25, 37 -invalid parameter                           */
} SC_STATUS;

/*memory opreation*/
/*****************************************************************************
 * FUNCTION
 *  sAPI_Malloc
 *
 * DESCRIPTION
 *  Allocate a block of Size bytes of memory from the default memory pool,
 *  returning a pointer to the beginning of the block.
 *
 * PARAMETERS
 *  size: Number of bytes to be allocated.
 *
 * RETURNS
 *  Points to the first address of an allocated memory space.
 *  If return is NULL, description failed to allocate memory
 *
 * NOTE
 *
 *****************************************************************************/
#define sAPI_Malloc(size) malloc(size)


/*****************************************************************************
 * FUNCTION
 *  sAPI_Free
 *
 * DESCRIPTION
 *  sAPI_Free() is used to free the memory to the defaultmemory pool
 *
 * PARAMETERS
 *  p: The first address of the memory that needs to be released
 *
 * RETURNS
 *  None.
 *
 * NOTE
 *
 *****************************************************************************/
#define sAPI_Free(pArg)  \
    do {  \
        if (pArg != NULL) {  \
            free(pArg);  \
            pArg = NULL;  \
        } \
    }while(0);


/*****************************************************************************
 * FUNCTION
 *  sAPI_TaskCreate
 *
 * DESCRIPTION
 *  sAPI_TaskCreate() is used to Create a task, this task will be started automatically.
 *
 * PARAMETERS
 *  taskRef: OS assigned reference to the task
 *  stackPtr: Pointer to the low address of the stack
 *  stackSize: Maximum size of the stack
 *  priority: initial priority of the task. Range 0��31 where 0 is the highest priority and 31 is the lowest priority.
 *  SC_NO_PRIORITY_CONVERSION is enabled, priority range is defaulted to the old 0 �� 255 range.
 *  taskName: Pointer to an 8 character name for the task. The name does not have to be null-terminated.
 *  taskStart: Entry function of the task
 *  argv: Argument to be passed into task entry function
 *
 * RETURNS
 *  @SC_STATUS
 *
 * NOTE
 *
 *****************************************************************************/
SC_STATUS sAPI_TaskCreate(sTaskRef *taskRef,
                          void *stackPtr,
                          UINT32 stackSize,
                          UINT8 priority,
                          char *taskName,
                          void (*taskStart)(void *),
                          void *argv);

/*****************************************************************************
* FUNCTION
*  sAPI_TaskDelete
*
* DESCRIPTION
*  sAPI_TaskDelete() is used to deleted the specified task.
*
* PARAMETERS
*  taskRef: Reference to the task.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_TaskDelete(sTaskRef taskRef);

/*****************************************************************************
* FUNCTION
*  sAPI_TaskSuspend
*
* DESCRIPTION
*  sAPI_TaskSuspend() is used to request that a specific task be suspended including the current task.
*
* PARAMETERS
*  taskRef: Reference to the task.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_TaskSuspend(sTaskRef taskRef);

/*****************************************************************************
* FUNCTION
*  sAPI_TaskResume
*
* DESCRIPTION
*  sAPI_TaskResume() is used to request that a specified task be resumed.
*
* PARAMETERS
*  taskRef: Reference to the task.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_TaskResume(sTaskRef taskRef);

/*****************************************************************************
* FUNCTION
*  sAPI_TaskSleep
*
* DESCRIPTION
*  Suspend the current executing task for specified time. 1s = 200 tick
*
* PARAMETERS
*  ticks: Number of OS clock ticks to sleep.
*
* RETURNS
*  None.
*
* NOTE
*
*****************************************************************************/
void sAPI_TaskSleep(UINT32 ticks);

/*****************************************************************************
* FUNCTION
*  sAPI_TaskGetCurrentRef
*
* DESCRIPTION
*  sAPI_TaskResume() is used to get the currently executing task.
*
* PARAMETERS
*  taskRef: OS assigned reference to the task.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_TaskGetCurrentRef(sTaskRef *taskRef);

/*****************************************************************************
* FUNCTION
*  sAPI_TaskTerminate
*
* DESCRIPTION
*  Terminate a task. Please use this API carefully. A task in a terminated state cannotexecute again.
*
* PARAMETERS
*  taskRef: Reference to the task.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_TaskTerminate(sTaskRef taskRef);

void sAPI_TaskYield(void);


SC_STATUS sAPI_TaskChangePriority(sTaskRef OsaRef, UINT8 newPriority, UINT8 *oldPriority);

SC_STATUS sAPI_TaskGetPriority(sTaskRef OsaRef,UINT8* pPriority );

SC_STATUS sAPI_GetTaskInfo(sTaskRef taskRef,sTaskInfo *task_info);

/*****************************************************************************
* FUNCTION
*  sAPI_GetTaskStackInfo
*
* DESCRIPTION
*  Calculates the stack consumed by the specified task.
*
* PARAMETERS
*  taskRef: Handle of the task.
*  pName: name of the task.
*  pStackSize: the total stack size of the task.
*  pStackInuse: the address of the current stack pointer relative to the bottom of the stack.
*  pStackPeak: the highest position used by the stack, generally speaking, only needs to pay attention to stackPeak.
*
* RETURNS
*  None.
*
* NOTE
*
*****************************************************************************/
void sAPI_GetTaskStackInfo(sTaskRef taskRef, char **pName, unsigned long *pStackSize, unsigned long *pStackInuse, unsigned long *pStackPeak);



/*****************************************************************************
 * FUNCTION
 *  sAPI_MsgQCreate
 *
 * DESCRIPTION
 *  This function requests that a message queue be created. All memory used to store messages on the message queue is allocated by the operating system.
 *
 * PARAMETERS
 *  msgQRef: Pointer to location to hold message queue reference allocated by the operating system
 *  queueName: 8 character name of queue.The name does not have to be null-terminated.
 *  maxSize: Maximum size of a message on the queue. This is used for error checking by sAPI_MsgQSend().
 *  maxNumber: Maximum number of messages on the queue.
 *  waitingMode: Defines scheduling of waiting events: SC_FIFO, or SC_PRIORITY.
 *
 * RETURNS
 *  @SC_STATUS
 *
 * NOTE
 *
 *****************************************************************************/
SC_STATUS sAPI_MsgQCreate(sMsgQRef *msgQRef,
                          char *queueName,
                          UINT32 maxSize,
                          UINT32 maxNumber,
                          UINT8 waitingMode);

/*****************************************************************************
* FUNCTION
*  sAPI_MsgQDelete
*
* DESCRIPTION
*  This function requests that the specified message queue be deleted.
*
* PARAMETERS
*  msgQRef: Identifier that uniquely identifies the message queue
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_MsgQDelete(sMsgQRef msgQRef);
/*****************************************************************************
* FUNCTION
*  sAPI_MsgQFlush
*
* DESCRIPTION
*  Clear the number of messages in queue.
*
* PARAMETERS
*  msgQRef: Identifier that uniquely identifies the message queue
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*  If the transmitted msg contains parameters such as the requested memory pointer,
*  clearing the msg in the message queue using the MsgQFlush interface will cause
*  these memories to be unable to be released, causing a memory leak and panic.
*  If such a situation occurs,
*  please use the following example to clear the messages in the queue:
*
*  {
*       UINT32 msgcount = 0;
*       sAPI_MsgQPoll(tRespMsgQ, &msgcount);
*       while(msgcount>0)
*       {
*           sAPI_MsgQRecv(tRespMsgQ, (UINT8 *)&resp_msg, sizeof(resp_msg), 0);
*           if (resp_msg.MsgData)
*           {
*               free(resp_msg.MsgData);
*           }
*           msgcount--;
*       }
*  }
*
*****************************************************************************/
SC_STATUS sAPI_MsgQFlush(sMsgQRef msgQRef);
/*****************************************************************************
* FUNCTION
*  sAPI_MsgQSend
*
* DESCRIPTION
*  This function requests that a message be sent to the specified message queue.
*
* PARAMETERS
*  msgQRef: Identifier that uniquely identifies the message queue
*  msgPtr: Starting address of the data.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*  This function will not block.
*****************************************************************************/
SC_STATUS sAPI_MsgQSend(sMsgQRef msgQRef, SIM_MSG_T *msgPtr);
SC_STATUS sAPI_MsgQSendEx(sMsgQRef msgQRef, UINT32 size, UINT8 *msgPtr, UINT32 timeout );


/*****************************************************************************
* FUNCTION
*  sAPI_MsgQSendSuspend
*
* DESCRIPTION
*  This function requests that a message be sent to the specified message queue.
*
* PARAMETERS
*  msgQRef: Identifier that uniquely identifies the message queue
*  msgPtr: Starting address of the data.
*  timeout: Specified ticks. 1 ticks = 5ms.If set timeout = SC_SUSPEND,
*  the function will block until there is space available on the queue.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_MsgQSendSuspend(sMsgQRef msgQRef, SIM_MSG_T *msgPtr, UINT32 timeout);


/*****************************************************************************
* FUNCTION
*  sAPI_MsgQRecv
*
* DESCRIPTION
*  This function requests that a message be received from the specified message queue.
*  If the queue is empty, the blocking behavior of the call is determined by the value of the ��timeout�� argument.
*
* PARAMETERS
*  msgQRef: Identifier that uniquely identifies the message queue
*  recvMsg: Starting address of the data.
*  timeout: If timeout is set to SC_NO_SUSPEND, this call will not block.
*  If timeout is set to SC_SUSPEND, this call will block until a message is available on the queue.
*  If a timeout value between 1 and 4,294,967,293 is specified, the call will block until a message is available or until the timeout period,
*  in number of OS clock ticks, elapses.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_MsgQRecv(sMsgQRef msgQRef, SIM_MSG_T *recvMsg, UINT32 timeout);
SC_STATUS sAPI_MsgQRecvEx(sMsgQRef msgQRef,UINT8 *msgPtr, UINT32 size, UINT32 timeout );


/*****************************************************************************
* FUNCTION
*  sAPI_MsgQPoll
*
* DESCRIPTION
*  This function checks the number of messages on the message queue.
*
* PARAMETERS
*  msgQRef: Identifier that uniquely identifies the message queue.
*  msgCount: Return from this function, msgCount contains the number of messages on the queue.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_MsgQPoll(sMsgQRef msgQRef, UINT32 *msgCount);

/*****************************************************************************
* FUNCTION
*  sAPI_SemaphoreCreate
*
* DESCRIPTION
*  This function requests that a counting semaphore be created.
*
* PARAMETERS
*  semaRef: OS assigned reference to the semaphore.
*  initialCount: Initial count of the semaphore.
*  waitingMode: SC_FIFO or SC_PRIORITY. ��waitingMode�� specifies how tasks are added to a semaphore��s Wait queue.They may be added in first-in-first-out order (SC_FIFO)
*  or in priority order (SC_PRIORITY) with the highest priority waiting task at the front on the queue.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_SemaphoreCreate(sSemaRef *semaRef,
                               UINT32 initialCount,
                               UINT8 waitingMode);

/*****************************************************************************
* FUNCTION
*  sAPI_SemaphoreDelete
*
* DESCRIPTION
*  This function requests that a counting semaphore be deleted.
*
* PARAMETERS
*  semaRef: OS assigned reference to the semaphore
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_SemaphoreDelete(sSemaRef semaRef);

/*****************************************************************************
* FUNCTION
*  sAPI_SemaphoreAcquire
*
* DESCRIPTION
*  This function requests that the specified semaphore be decremented. If the semaphore count is zero before this call,
*  the service cannot be satisfied immediately. In that case, the blocking behavior is specified by the ��timeout�� parameter.
*
* PARAMETERS
*  semaRef: OS assigned reference to the semaphore
*  timeout: If timeout is set to SC_NO_SUSPEND, this call will not block. If timeout is set to SC_SUSPEND, this call will block until the semaphore count is greater than zero.
*  If a timeout value between 1 and 4,294,967,293 is specified, the call will block until the semaphore count is greater than zero or the timeout period, in number of OS clock ticks, elapses.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_SemaphoreAcquire(sSemaRef semaRef, UINT32 timeout);

/*****************************************************************************
* FUNCTION
*  sAPI_SemaphoreRelease
*
* DESCRIPTION
*  If there are any tasks waiting for the semaphore, the first waiting task is made ready to run.
*  If there are no tasks waiting, the value of the semaphore is incremented by one.
*
* PARAMETERS
*  semaRef: OS assigned reference to the semaphore
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_SemaphoreRelease(sSemaRef semaRef);

/*****************************************************************************
* FUNCTION
*  sAPI_SemaphorePoll
*
* DESCRIPTION
*  This function requests that a counting semaphore be created.
*
* PARAMETERS
*  semaRef: OS assigned reference to the semaphore
*  count: Current value of the semaphore.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_SemaphorePoll(sSemaRef semaRef, UINT32 *count);

/*****************************************************************************
* FUNCTION
*  sAPI_MutexCreate
*
* DESCRIPTION
*  This function requests that a mutex be created. Mutexes use the priority inheritance protocol to bound the time spent in priority inversions.
*
* PARAMETERS
*  mutexRef: OS assigned reference to the Mutex.
*  waitingMode: SC_FIFO or SC_PRIORITY.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_MutexCreate(sMutexRef *mutexRef, UINT8 waitingMode);

/*****************************************************************************
* FUNCTION
*  sAPI_MutexDelete
*
* DESCRIPTION
*  This function requests that the specified mutex be deleted.
*
* PARAMETERS
*  mutexRef: OS assigned reference to the Mutex.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_MutexDelete(sMutexRef mutexRef);

/*****************************************************************************
* FUNCTION
*  sAPI_MutexLock
*
* DESCRIPTION
*  This function requests that the specified mutex be locked. If the mutex is locked by another task before this call,
*  the service cannot be satisfied immediately. In this case, the blocking behavior is specified by the ��timeout�� parameter.
*
* PARAMETERS
*  mutexRef: OS assigned reference to the Mutex.
*  timeout: If timeout is set to SC_NO_SUSPEND, this call will not block. If timeout is set to SC_SUSPEND,
*  this call will block until the semaphore count is greater than zero. If a timeout value between 1 and 4,294,967,293 is specified,
*  the call will block until the mutex is unlocked or the timeout period, in number of OS clock ticks, elapses.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_MutexLock(sMutexRef mutexRef, UINT32 timeout);

/*****************************************************************************
* FUNCTION
*  sAPI_MutexUnLock
*
* DESCRIPTION
*  If there are any tasks waiting for the mutex, the task at the front of the Wait queue is made ready to run.
*  Only the mutex owner may unlock a mutex.
*
* PARAMETERS
*  mutexRef: OS assigned reference to the Mutex.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_MutexUnLock(sMutexRef mutexRef);

/*****************************************************************************
* FUNCTION
*  sAPI_FlagCreate
*
* DESCRIPTION
*  This function requests that a flag group be created.
*
* PARAMETERS
*  flagRef: OS assigned reference to the flag.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_FlagCreate(sFlagRef *flagRef);

/*****************************************************************************
* FUNCTION
*  sAPI_FlagDelete
*
* DESCRIPTION
*  This function requests that a flag group be deleted.
*
* PARAMETERS
*  flagRef: OS assigned reference to the flag.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_FlagDelete(sFlagRef flagRef);

/*****************************************************************************
* FUNCTION
*  sAPI_FlagSet
*
* DESCRIPTION
*  This function executes a logical OR or AND of the flag group with the input mask.
*
* PARAMETERS
*  flagRef: OS assigned reference to the flag.
*  mask: Mask specifying which bits need to be set. A 1 in a certain bit position will set the same bit position in the flag.
*  operation: Logical operation to be executed on the flag group. SC_AND executes a logical AND and SC_OR executes a logical OR.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_FlagSet(sFlagRef flagRef, UINT32 mask, UINT32 operation);

/*****************************************************************************
* FUNCTION
*  sAPI_FlagWait
*
* DESCRIPTION
*  This function waits for the specified operation on a flag group to complete. The operation is defined by the ��operation�� input parameter.
*  If the timeout input parameter is SC_NO_SUSPEND the operation completes immediately and the current value of the flags is returned in the output parameter ��flags��.
*  By specifying SC_NO_SUSPEND, an application can read the current value of the flags without blocking.
*
* PARAMETERS
*  flagRef: OS assigned reference to the flag.
*  mask: Mask of flags to wait for
*  operation: may be one of the following:
*  SC_FLAG_AND �C Wait for all of the bits in the input mask
*  SC_FLAG_AND_CLEAR �C Wait for all of the bits in the input mask to be set, clear all event flags on successful
*  SC_FLAG_OR �C Wait for any of the bits in the input mask to be set, don��t clear the event flags
*  SC_FLAG_OR_CLEAR �C Wait for any of the bits in the input mask to completion to be set, don��t clear the event flags
*  timeout: If timeout is set to SC_NO_SUSPEND, the operation completes immediately and the current value of the flags is returned in the output parameter ��flags��.
*  If timeout is set to SC_SUSPEND, this call will block until the condition specified by the ��mask�� and ��operation�� inputs is satisfied. If a timeout value between 1 and 4,294,967,293 is specified,
*  the call will block until the wait condition is satisfied or until the timeout period, in number of OS clock ticks, elapses.
*  flag: The current value of all flags
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_FlagWait(sFlagRef flagRef, UINT32 mask, UINT32 operation, UINT32 *flags, UINT32 timeout);

/*****************************************************************************
* FUNCTION
*  sAPI_TimerCreate
*
* DESCRIPTION
*  This function allocates a timer. The state of the allocated timer is inactive. sAPI_TimerStart() is used to activate the timer.
*
* PARAMETERS
*  timerRef: Address to store a reference to the timer allocated by the operating system.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_TimerCreate(sTimerRef *timerRef);

/*****************************************************************************
* FUNCTION
*  sAPI_TimerStart
*
* DESCRIPTION
*  This function requests that an inactive timer be started and the callback function executed at the expiration of the timer.
*
* PARAMETERS
*  timerRef: OS supplied timer reference.
*  initialTime: Initial expiration time in OS clock ticks
*  rescheduleTime: If 0,cyclic timing is disabled and the timer only expires once. If not zero, it indicates the period, in OS clock ticks, of a cyclic timer.
*  callBackRoutine: Specifies the application routine to execute each time the timer expires. The callback function must not invoke any ��blocking�� operating system calls.
*  timerArgc: Argument to be passed to callback routine on expiration.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_TimerStart(sTimerRef timerRef, UINT32 initialTime, UINT32 rescheduleTime,
                          void (*callBackRoutine)(UINT32), UINT32 timerArgc);

/*****************************************************************************
* FUNCTION
*  sAPI_TimerStop
*
* DESCRIPTION
*  This function requests that the state of an active timer be changed to inactive. Calling this function when the state of the timer is inactive has no effect.
*
* PARAMETERS
*  timerRef: OS assigned reference to the timer.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_TimerStop(sTimerRef timerRef);

/*****************************************************************************
* FUNCTION
*  sAPI_TimerDelete
*
* DESCRIPTION
*  This function requests that the specified timer be deleted. The timer must be inactive when this function is called.
*
* PARAMETERS
*  timerRef: OS assigned reference to the timer.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_TimerDelete(sTimerRef timerRef);

/*****************************************************************************
* FUNCTION
*  sAPI_TimerGetStatus
*
* DESCRIPTION
*  This function requests that the status of the specified timer be returned in the OSATimerStatus structure.
*
* PARAMETERS
*  timerRef: OS assigned reference to the timer.
*  timerStatus: Pointer to the structure that will be filled in.
*
* RETURNS
*  @SC_STATUS
*
* NOTE
*
*****************************************************************************/
SC_STATUS sAPI_TimerGetStatus(sTimerRef timerRef, sTimerStatus *timerStatus);
#ifdef SIMCOM_ACC_TIMER_SUPPORT
/*****************************************************************************
* FUNCTION
*  sAPI_AccTimerStart
*
* DESCRIPTION
*  Set ACC timer parameters and start the timer.
*
* PARAMETERS
*  flag: acc timer execution method: "Loop execution" or "Manual delete after execution".
*        Loop execution: 0x1      Manual delete after execution: 0x2
*  period: The length of timeout required, in units of us. The length of the timeout time is relative, that is, the length of the delay from the start time.
*  timerCallbackFunc: Function pointer, user's timer callback function.
*  callBackRoutine: Specifies the application routine to execute each time the timer expires. The callback function must not invoke any ��blocking�� operating system calls.
*  timerParams: The input parameters corresponding to the timer callback function.
*
* RETURNS
*  Successfully started, return the timer number acc_timer_id --- (a number between 1 and 32).
*  Insufficient memory space, returning 0.
*  Input parameter error, returning -1.
*
* NOTE
*   period needs to be>=150us. The period here is counted in units of us, and when converting the unit from us to tick,
*   the decimal point will be discarded, resulting in an error of 30 us.
*****************************************************************************/
int sAPI_AccTimerStart(SC_ACC_TIMER_FLAG flag,unsigned int period,SC_ACC_TIMER_CALLBACK timerCallbackFunc,unsigned int timerParams);

/*****************************************************************************
* FUNCTION
*  sAPI_AccTimerStop
*
* DESCRIPTION
*  Timer node is disconnected from the active_list, node information is preserved, and space is not released.
*
* PARAMETERS
*  acc_timer_id: timer number to be removed (a number between 1 and 32).
*
* RETURNS
*  Stop successful, return the timer number acc_timer_id (a number between 1 and 32) for successful removal;
*  Stop failed, return to -1.
* NOTE
*
*****************************************************************************/
int sAPI_AccTimerStop(int acc_timer_id);

/*****************************************************************************
* FUNCTION
*  sAPI_AccTimerDelete
*
* DESCRIPTION
*  Break the timer node from the active_list, clear the node information, and free up space.
*
* PARAMETERS
*  acc_timer_id: timer number to be deleted (a number between 1 and 32).
*
* RETURNS
*  delete successful, return timer number timer_ ID (a number between 1 and 32);
*  delete failed,return -1, the parameter passed is incorrect.
*
* NOTE
*
*****************************************************************************/
int sAPI_AccTimerDelete(int acc_timer_id);

/*****************************************************************************
* FUNCTION
*  sAPI_AccTimerGetStatus
*
* DESCRIPTION
*  Return timer node status: active or inactive.
*
* PARAMETERS
*  acc_timer_id: Timer number to be viewed (a number between 1 and 32).
*
* RETURNS
*  2: The timer number to be viewed does not exist (the input parameter is not a number between 1 and 32).
*  1: The timer to be viewed is in the active_list, but the timeout has not expired and the callback function has not been executed.
*  0: The timer number acc_timer_id to be viewed is not in the active_list.
* NOTE
*
*****************************************************************************/
SC_ACC_TIMER_STATUS sAPI_AccTimerGetStatus(int acc_timer_id);
#endif
/*****************************************************************************
* FUNCTION
*  sAPI_GetTicks
*
* DESCRIPTION
*  This function requests the elapsed time, in OS clock tick, since the last system start-up.
*
* PARAMETERS
*  None
*
* RETURNS
*  Time elapsed in OS clock ticks
*
* NOTE
*
*****************************************************************************/
UINT32 sAPI_GetTicks(void);

/******************************************************************************
 * FUNCTION
 *  gettimeofday(struct timeval *tv, void* dummy)
 * INPUT
 *      time_p == pointer to an empty utlAbsoluteTime_T structure
 * OUTPUT
 *      none
 * RETURNS
 *      utlSUCCESS for success, utlFAILED for failure
 * DESCRIPTION
 *  Fetches the current system time since the system epoch.
*****************************************************************************/

INT32 sAPI_Gettimeofday(sTimeval *tv, void *dummy);


/******************************************************************************
 * FUNCTION
 *  sAPI_GettimeofdaySyncRtc(struct timeval *tv, void* dummy)
 * INPUT
 *      time_p == pointer to an empty utlAbsoluteTime_T structure
 * OUTPUT
 *      none
 * RETURNS
 *      utlSUCCESS for success, utlFAILED for failure
 * DESCRIPTION
 *  Fetches the current system time since the system epoch sync RTC
*****************************************************************************/
INT32 sAPI_GettimeofdaySyncRtc(sTimeval *tv, void *dummy);


/******************************************************************************
* FUNCTION
*   sAPI_DelayUs(unsigned long *t)
* INPUT
*   If t is non-NULL, the return value is also stored in the memory pointed to by t.
* RETURNS
*   returns the time as the number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
*****************************************************************************/

void sAPI_DelayUs(unsigned int us);

/******************************************************************************
* FUNCTION
*   sAPI_Time(unsigned long t)
* INPUT
*   If t is non-NULL, the return value is also stored in the memory pointed to by t.
* RETURNS
*   NO.
*****************************************************************************/

unsigned long sAPI_Time(unsigned long *t);


/*****************************************************************************
* FUNCTION
*  sAPI_ContextLock
*
* DESCRIPTION
*  Lock context - No interrupts and no preemptions.
*
* PARAMETERS
*  None
*
* RETURNS
*  @SC_STATUS
*****************************************************************************/

SC_STATUS sAPI_ContextLock(void);

/*****************************************************************************
* FUNCTION
*  sAPI_ContextUnlock
*
* DESCRIPTION
*  Restore the context.
*
* PARAMETERS
*  None
*
* RETURNS
*  @SC_STATUS
*****************************************************************************/
SC_STATUS sAPI_ContextUnlock(void);

void sAPI_CacheCleanMemory(void *pMem, unsigned int size);

/*****************************************************************************
* FUNCTION
*  sAPI_GetDefautlPoolMaxFreeBufSize
*
* DESCRIPTION
*  Gets the maximum block size that can be allocated in the current memory.
*
* PARAMETERS
*  None
*
* RETURNS
*  @UINT32: The maximum size of blocks that can be allocated in the current memory.
*****************************************************************************/
UINT32 sAPI_GetDefautlPoolMaxFreeBufSize(void);
#endif
