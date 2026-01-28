#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* 包含 HC32 的头文件以获取 SystemCoreClock 定义 */
#include "hc32_ll.h"

/* --------------------------------------------------------------------
 * 基础配置
 * -------------------------------------------------------------------- */
#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      ( SystemCoreClock ) // 关键：使用系统变量
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES                    ( 16 )
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 128 )
#define configMAX_TASK_NAME_LEN                 ( 16 )
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               8
#define configENABLE_FPU                        1
#define configENABLE_MPU                        0


/* --------------------------------------------------------------------
 * 内存分配 (Heap_4.c)
 * -------------------------------------------------------------------- */
#define configSUPPORT_static_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 64 * 1024 ) ) // 堆栈大小，可能要变化

/* --------------------------------------------------------------------
 * Hook 函数
 * -------------------------------------------------------------------- */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCHECK_FOR_STACK_OVERFLOW          2 // 调试阶段建议开启
#define configUSE_MALLOC_FAILED_HOOK            0

/* --------------------------------------------------------------------
 * Cortex-M 中断优先级配置 (非常重要！)
 * -------------------------------------------------------------------- */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS       __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS       4     /* HC32F460 也是 4 位优先级 */
#endif

/* 这里的逻辑是：FreeRTOS 管理的中断优先级必须低于或等于 configMAX_SYSCALL_INTERRUPT_PRIORITY
   数值越大，优先级越低。5 表示优先级 5-15 的中断可以调用 FreeRTOS API。
   0-4 的高优先级中断不能调用 FreeRTOS API，保证实时性。 */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* --------------------------------------------------------------------
 * 中断处理函数映射
 * HC32 启动文件中的中断名 -> FreeRTOS 的处理函数
 * -------------------------------------------------------------------- */
#define vPortSVCHandler         SVC_Handler
#define xPortPendSVHandler      PendSV_Handler
#define xPortSysTickHandler     SysTick_Handler

/* --------------------------------------------------------------------
 * 软件计时器
 * -------------------------------------------------------------------- */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE * 2 )

#endif /* FREERTOS_CONFIG_H */