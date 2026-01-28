/* middlewares/fatfs_port/ffconf.h */
#ifndef FFCONF_DEF
#define FFCONF_DEF 86604	/* Revision ID */

/* =============================================================
 * 功能裁剪 (Function Configurations)
 * ============================================================= */
#define FF_FS_READONLY   0      /* 0: Read/Write, 1: Read Only */
#define FF_FS_MINIMIZE   0      /* 0: Full function */
#define FF_USE_STRFUNC   1      /* 1: Enable string functions */
#define FF_USE_FIND      0      /* 1: Enable f_find*() */
#define FF_USE_MKFS      1      /* 1: Enable f_mkfs() (格式化功能) */
#define FF_USE_FASTSEEK  1      /* 1: Enable fast seek */
#define FF_USE_EXPAND    0      /* 1: Enable f_expand() */
#define FF_USE_CHMOD     0      /* 1: Enable f_chmod/f_utime */
#define FF_USE_LABEL     0      /* 1: Enable volume label */
#define FF_USE_FORWARD   0      /* 1: Enable f_forward() */

/* =============================================================
 * 本地化与长文件名 (Locale & LFN)
 * ============================================================= */
#define FF_CODE_PAGE     437    /* 437: U.S., 936: GBK (需要字库支持) */
#define FF_USE_LFN       1      /* 1: Enable LFN with static working buffer on BSS */
#define FF_MAX_LFN       255    /* Max LFN length */
#define FF_LFN_UNICODE   0      /* 0: ANSI/OEM, 1: Unicode, 2: UTF-8 */
#define FF_LFN_BUF       255
#define FF_SFN_BUF       12

/* =============================================================
 * 物理驱动配置 (Physical Drive)
 * ============================================================= */
#define FF_VOLUMES       1
#define FF_STR_VOLUME_ID 0
#define FF_MULTI_PARTITION 0
#define FF_MIN_SS        512
#define FF_MAX_SS        512    /* SD卡通常固定为512 */
#define FF_USE_TRIM      0

/* =============================================================
 * 系统与 RTOS 配置 (System & FreeRTOS)
 * ============================================================= */
#define FF_FS_TINY       0      /* 0: Normal buffer */
#define FF_FS_EXFAT      0      /* 0: Disable exFAT */
#define FF_FS_NORTC      0      /* 0: Use RTC */
#define FF_NORTC_MON     1
#define FF_NORTC_MDAY    1
#define FF_NORTC_YEAR    2022

/* --- FreeRTOS 配置 --- */
#define FF_FS_LOCK       10     /* 文件锁数量 */
#define FF_FS_REENTRANT  1      /* 1: Enable Reentrancy (Thread safe) */
#define FF_FS_TIMEOUT    1000   /* Timeout ticks (ms) */
#define FF_SYNC_t        void* /* FreeRTOS SemaphoreHandle_t is void* */

#endif /* FFCONF_DEF */