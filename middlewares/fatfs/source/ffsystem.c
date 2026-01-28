/*------------------------------------------------------------------------*/
/* FatFs - OS Dependent Functions for FreeRTOS                            */
/*------------------------------------------------------------------------*/

#include "ff.h"

/* * 引入 FreeRTOS 头文件
 * 确保 CMakeLists.txt 中已经链接了 freertos 库，否则找不到这些头文件
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*------------------------------------------------------------------------*/
/* 内存管理 (Memory Management)                                            */
/*------------------------------------------------------------------------*/
/* 只有当 ffconf.h 中 FF_USE_LFN == 3 时才需要编译这部分                  */

#if FF_USE_LFN == 3

/* 分配内存 */
void* ff_memalloc (UINT msize) {
    /* 使用 FreeRTOS 的安全内存分配，而不是标准 malloc */
    return pvPortMalloc(msize);
}

/* 释放内存 */
void ff_memfree (void* mblock) {
    /* 使用 FreeRTOS 的释放函数 */
    vPortFree(mblock);
}

#endif


/*------------------------------------------------------------------------*/
/* 重入保护/互斥锁 (Reentrancy / Mutex)                                    */
/*------------------------------------------------------------------------*/
/* 只有当 ffconf.h 中 FF_FS_REENTRANT == 1 时才需要编译这部分              */

#if FF_FS_REENTRANT

/* * 创建同步对象
 * vol: 卷号 (0..FF_VOLUMES-1)
 * sobj: 指向 FF_SYNC_t (通常是 void* 或 SemaphoreHandle_t) 的指针
 */
int ff_cre_syncobj (BYTE vol, FF_SYNC_t* sobj) {
    /* 创建 FreeRTOS 互斥量 */
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();

    if (mutex != NULL) {
        *sobj = (FF_SYNC_t)mutex; /* 将句柄存入 FatFs 提供的指针中 */
        return 1; /* 成功 */
    }
    return 0; /* 失败 */
}

/* 删除同步对象 */
int ff_del_syncobj (FF_SYNC_t sobj) {
    if (sobj) {
        vSemaphoreDelete((SemaphoreHandle_t)sobj);
    }
    return 1;
}

/* 获取锁 (Request Grant) */
int ff_req_grant (FF_SYNC_t sobj) {
    if (!sobj) return 0;

    /* 等待互斥量，超时时间由 FF_FS_TIMEOUT 定义 */
    if (xSemaphoreTake((SemaphoreHandle_t)sobj, FF_FS_TIMEOUT) == pdTRUE) {
        return 1; /* 获取成功 */
    }
    return 0; /* 获取失败/超时 */
}

/* 释放锁 (Release Grant) */
void ff_rel_grant (FF_SYNC_t sobj) {
    if (sobj) {
        xSemaphoreGive((SemaphoreHandle_t)sobj);
    }
}

#endif