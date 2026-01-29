//
// Created by Administrator on 2026/1/29.
//

#ifndef _ULOG_H_
#define _ULOG_H_

#include "hc32_ll_dma.h"
#include "hc32_ll_usart.h"
#include <stdio.h>

/* ============================================================================== */
/* 用户配置区域                                                                    */
/* ============================================================================== */

/* 缓冲区大小，必须是 2 的幂次方 (如 512, 1024, 2048, 4096) */
#define ULOG_BUF_SIZE           (2048U)
/* 日志最大长度，超过会被截断 */
#define ULOG_PRINTF_BUF_SIZE    (256U)

/* 硬件定义配置 */
/* 注意：这里需要确保 CM_DMA1 和 DMA_CH0 与您的实际硬件连接一致 */
#ifndef CM_DMA1
#define ULOG_DMA_UNIT           (CM_DMA1)
#else
#define ULOG_DMA_UNIT           CM_DMA1
#endif

#define ULOG_DMA_CH             (DMA_CH0)
#define ULOG_DMA_TC_FLAG        (DMA_FLAG_TC_CH0)
#define ULOG_DMA_BTC_FLAG       (DMA_FLAG_BTC_CH0)


/* ============================================================================== */
/* 函数声明                                                                       */
/* ============================================================================== */

/**
 * @brief  初始化 ULog 模块 (DMA + RingBuffer)
 * @note   在使用 Printf 之前必须调用。需自行确保 GPIO 和 USART 时钟/引脚已初始化。
 */
void ULog_Init(void);

/**
 * @brief  格式化打印 (类似 printf)
 * @param  TAG    日志标签 (会以 [TAG] 形式显示在最前面), 可传 NULL
 * @param  format 格式字符串
 * @param  ...    参数列表
 */
void ULog_Printf(const char *TAG, const char *format, ...);

/**
 * @brief  DMA 传输完成中断回调
 * @note   请放在 INT_SRC_DMAx_TCx_IrqCallback 中调用
 */
void ULog_DMA_TC_IrqHandler(void);

/**
 * @brief  DMA 块传输完成中断回调
 * @note   通常高性能模式主要依赖 TC 中断
 */
void ULog_DMA_BTC_IrqHandler(void);

#endif // _ULOG_H_