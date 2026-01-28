//
// Created by Administrator on 2026/1/28.
//

#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>


/**
 * @file    ring_buffer.h
 * @brief   Ring Buffer 实现 (SPSC无锁, DMA零拷贝)
 * * =================================================================================
 * 使用指南 (USAGE)
 * =================================================================================
 * * 【前提条件】：
 * 1. 缓冲区大小 (Size) 必须是 2 的幂 (2, 4, ... 1024, 2048, 4096...)。
 * 2. 针对 HC32F460，建议将 buffer 定义在 SRAM2 (0x20000000) 以利用 DMA 独立总线。
 * * ---------------------------------------------------------------------------------
 * 用例 1: 基础定义与初始化 (针对 Scatter File: SECTION_DMA_BUF)
 * ---------------------------------------------------------------------------------
 * * // 在 main.c 或 driver.c 中
 * * // 1. 定义 Buffer (强制对齐并放入 SRAM2)
 * __attribute__((section("SECTION_DMA_BUF"), aligned(4)))
 * uint8_t u8TxRawBuf[1024]; // 大小必须是 2^n
 * * // 2. 定义控制句柄 (放在 SRAM1 或 SRAMH 均可)
 * ring_buffer_t tTxRB;
 * * // 3. 初始化
 * void App_Init(void) {
 * rb_init(&tTxRB, u8TxRawBuf, 1024);
 * }
 * * ---------------------------------------------------------------------------------
 * 用例 2: 普通模式读写 (中断/主循环)
 * ---------------------------------------------------------------------------------
 * * // 生产者 (如: 传感器数据采集)
 * void Sensor_Update(uint8_t val) {
 * if (!rb_put(&tTxRB, val)) {
 * // 缓冲区满处理
 * }
 * }
 * * // 消费者 (如: 主循环处理)
 * void Main_Loop(void) {
 * uint8_t data;
 * if (rb_get(&tTxRB, &data)) {
 * Process(data);
 * }
 * }
 * * ---------------------------------------------------------------------------------
 * 用例 3: 极致性能 DMA 发送 (Zero-Copy 模式)
 * ---------------------------------------------------------------------------------
 * 此模式直接获取 RingBuffer 内部指针给 DMA，避免数据拷贝。
 * * void Try_Start_DMA_Tx(void) {
 * // 检查 DMA 是否忙 ...
 * * uint32_t len = 0;
 * // 获取连续的线性内存块 (处理回绕逻辑)
 * void* p_src = rb_get_read_ptr(&tTxRB, &len);
 * * if (len > 0) {
 * // 配置 DMA 源地址为 p_src，长度为 len
 * DMA_SetSrcAddress(DMA_UNIT, DMA_CH, (uint32_t)p_src);
 * DMA_SetTransferCount(DMA_UNIT, DMA_CH, len);
 * * // 保存当前发送长度供中断使用
 * u32CurrentTxLen = len;
 * DMA_Enable(DMA_UNIT, DMA_CH);
 * }
 * }
 * * // DMA 传输完成中断 (TC IRQ)
 * void DMA_TC_IrqHandler(void) {
 * DMA_ClearFlag(...);
 * * // 关键：DMA 发送完才真正释放 RingBuffer 空间
 * rb_advance_tail(&tTxRB, u32CurrentTxLen);
 * * // 检查是否还有剩余数据 (处理 RingBuffer 回绕后的第二段数据)
 * Try_Start_DMA_Tx();
 * }
 * * =================================================================================
 */


typedef struct {
    uint8_t  *p_buffer;   // 指向实际的数据存储区 (位于 SRAM2)
    uint32_t  size_mask;  // 掩码，值为 (size - 1)，size 必须是 2 的幂
    volatile uint32_t head; // 写入索引 (生产者)
    volatile uint32_t tail; // 读取索引 (消费者)
} ring_buffer_t;

/**
 * @brief 初始化 Ring Buffer
 * @param rb 句柄
 * @param buffer 实际内存数组指针
 * @param size 缓冲区大小 (必须是 2 的幂，例如 512, 1024, 4096)
 */
void rb_init(ring_buffer_t *rb, uint8_t *buffer, uint32_t size);

// 写入数据 (单字节)
bool rb_put(ring_buffer_t *rb, uint8_t data);

// 读取数据 (单字节)
bool rb_get(ring_buffer_t *rb, uint8_t *data);

// 批量写入
uint32_t rb_write(ring_buffer_t *rb, const uint8_t *data, uint32_t len);

// 批量读取
uint32_t rb_read(ring_buffer_t *rb, uint8_t *data, uint32_t len);

// 获取已用空间大小
uint32_t rb_used_count(const ring_buffer_t *rb);

// 获取剩余空间大小
uint32_t rb_free_count(const ring_buffer_t *rb);

// 【DMA 专用】获取当前可读取的线性连续内存地址和长度
// 用于 DMA 发送，避免数据回绕时需要分两次发送
void* rb_get_read_ptr(ring_buffer_t *rb, uint32_t *cnt);

// 【DMA 专用】DMA 传输完成后手动更新 Tail
void rb_advance_tail(ring_buffer_t *rb, uint32_t len);

#endif