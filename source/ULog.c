#include "ULog.h"
#include "ring_buffer.h"
#include <stdarg.h>
#include <string.h>

#define ULOG_USART          ((CM_USART_TypeDef *) CM_USART2_BASE)

#ifndef CM_DMA_TypeDef
    #define CM_DMA_TypeDef      CM_DMA_TypeDef
#endif



/* ============================================================================== */
/* 私有变量定义                                                                    */
/* ============================================================================== */

static uint8_t u8ULogRawBuf[ULOG_BUF_SIZE] __attribute__((aligned(4)));

/* 环形缓冲区控制句柄 */
static ring_buffer_t tLogRB;

/* 记录当前 DMA 正在发送的数据长度 */
static volatile uint32_t s_u32CurrentDmaTxLen = 0;

/* DMA 发送忙标志 */
static volatile bool s_bDmaBusy = false;


/* ============================================================================== */
/* 内部函数声明                                                                    */
/* ============================================================================== */
static void ULog_TryStartDmaTx(void);


/* ============================================================================== */
/* 函数实现                                                                       */
/* ============================================================================== */

/**
 * @brief 初始化 ULog
 */
void ULog_Init(void) {
    /* 1. 初始化 Ring Buffer */
    rb_init(&tLogRB, u8ULogRawBuf, ULOG_BUF_SIZE);
    
    s_u32CurrentDmaTxLen = 0;
    s_bDmaBusy = false;
}

/**
 * @brief 尝试启动 DMA 传输
 * @note  核心逻辑：查看 RingBuffer 头部是否有连续数据，如果有且 DMA 空闲，则启动。
 */
static void ULog_TryStartDmaTx(void) {
    uint32_t u32LinearLen = 0;
    void* pSrc = NULL;

    if (s_bDmaBusy) {
        return;
    }

    pSrc = rb_get_read_ptr(&tLogRB, &u32LinearLen);

    if (u32LinearLen > 0) {
        s_bDmaBusy = true;
        s_u32CurrentDmaTxLen = u32LinearLen;

        DMA_ChCmd(ULOG_DMA_UNIT, ULOG_DMA_CH, DISABLE);
        
        DMA_SetSrcAddr(ULOG_DMA_UNIT, ULOG_DMA_CH, (uint32_t)pSrc);
        DMA_SetTransCount(ULOG_DMA_UNIT, ULOG_DMA_CH, u32LinearLen);
        
        DMA_ClearTransCompleteStatus(ULOG_DMA_UNIT, ULOG_DMA_TC_FLAG);
        DMA_ClearErrStatus(ULOG_DMA_UNIT, DMA_FLAG_ERR_MASK);

        DMA_ChCmd(ULOG_DMA_UNIT, ULOG_DMA_CH, ENABLE);
    }
}

/**
 * @brief ULog 打印函数
 */
void ULog_Printf(const char *TAG, const char *format, ...) {
    va_list args;
    char tx_temp_buf[ULOG_PRINTF_BUF_SIZE];
    int len = 0;
    int ret = 0;

    if (TAG != NULL) {
        ret = snprintf(tx_temp_buf, sizeof(tx_temp_buf), "[%s] ", TAG);

        if (ret > 0) {
            if ((uint32_t)ret >= sizeof(tx_temp_buf)) {
                len = sizeof(tx_temp_buf) - 1;
            } else {
                len = ret;
            }
        }
    }

    if (len < sizeof(tx_temp_buf) - 1) {
        va_start(args, format);
        int available = sizeof(tx_temp_buf) - len;

        ret = vsnprintf(tx_temp_buf + len, available, format, args);
        va_end(args);

        if (ret > 0) {
            if (ret >= available) {
                len = sizeof(tx_temp_buf) - 1;
            } else {
                len += ret;
            }
        }
    }

    if (len > 0) {
        rb_write(&tLogRB, (uint8_t *)tx_temp_buf, (uint32_t)len);
        /* * 如果 DMA 空闲，立即启动；
         * 如果 DMA 忙，数据留在 Buffer 里，由中断接力发送。
         */
        ULog_TryStartDmaTx();
    }
}

/* ============================================================================== */
/* 中断回调实现                                                                    */
/* ============================================================================== */

/* INT_SRC_DMA1_BTC0 Callback. (Block Transfer Complete) */
/* * 某些配置下 BTC 和 TC 可能会同时触发，或者根据配置只触发一个。
 * 这里的逻辑主要依赖 TC。
 */
void INT_SRC_DMA1_BTC0_IrqCallback(void) {
    if (SET == DMA_GetTransCompleteStatus(ULOG_DMA_UNIT, ULOG_DMA_BTC_FLAG)) {
        DMA_ClearTransCompleteStatus(ULOG_DMA_UNIT, ULOG_DMA_BTC_FLAG);
    }
    ULog_DMA_TC_IrqHandler();
}

/* INT_SRC_DMA1_TC0 Callback. (Transfer Complete) */
void INT_SRC_DMA1_TC0_IrqCallback(void) {
    if (SET == DMA_GetTransCompleteStatus(ULOG_DMA_UNIT, ULOG_DMA_TC_FLAG)) {
        DMA_ClearTransCompleteStatus(ULOG_DMA_UNIT, ULOG_DMA_TC_FLAG);

        /* * 此时 DMA 已经把 s_u32CurrentDmaTxLen 长度的数据搬运到 USART TDR 了。
         * 我们通知 RingBuffer 移动 Tail 指针，腾出空间。
         */
        if (s_bDmaBusy) {
            rb_advance_tail(&tLogRB, s_u32CurrentDmaTxLen);
            s_u32CurrentDmaTxLen = 0;
            s_bDmaBusy = false;
        }

        /* 3. 检查是否还有剩余数据需要发送 */
        /* * 场景：RingBuffer 数据发生了回绕 (Wrap Around)。
         * 第一次 DMA 发送了 Buffer 尾部的数据。
         * 中断回来后，Buffer 头部可能还有数据，或者刚才 CPU 又写入了新数据。
         * TryStartDmaTx 会再次获取指针并启动下一次传输。
         */
        ULog_TryStartDmaTx();
    }
}

/* 为了兼容头文件声明的函数名重定向 */
void ULog_DMA_TC_IrqHandler(void) {
}

void ULog_DMA_BTC_IrqHandler(void) {
}