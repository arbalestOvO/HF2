//
// Created by Administrator on 2026/1/28.
//

#include "ring_buffer.h"

#include <string.h>

#include "hc32_ll.h" // 包含 HC32 库以使用 __DMB()，或者使用 cmsis_gcc.h

#define IS_POWER_OF_2(x) ((x) != 0 && (((x) & ((x) - 1)) == 0))

void rb_init(ring_buffer_t *rb, uint8_t *buffer, uint32_t size) {
    // 强制要求大小为 2 的幂，否则位运算会出错
    if (!IS_POWER_OF_2(size)) {
        // 错误处理：在这里可以触发断言或死循环
        while(1);
    }

    rb->p_buffer = buffer;
    rb->size_mask = size - 1;
    rb->head = 0;
    rb->tail = 0;
}

uint32_t rb_used_count(const ring_buffer_t *rb) {
    return (rb->head - rb->tail); // 无符号溢出特性使得这个计算永远正确
}

uint32_t rb_free_count(const ring_buffer_t *rb) {
    // 总容量 - 已用容量
    return (rb->size_mask + 1) - rb_used_count(rb);
}

bool rb_put(ring_buffer_t *rb, uint8_t data) {
    if (rb_free_count(rb) == 0) {
        return false;
    }

    // 使用 mask 进行快速取模: index = head & mask
    rb->p_buffer[rb->head & rb->size_mask] = data;

    __DMB(); // 数据存储屏障，确保数据写入后再更新指针
    rb->head++;
    return true;
}

bool rb_get(ring_buffer_t *rb, uint8_t *data) {
    if (rb_used_count(rb) == 0) {
        return false;
    }

    *data = rb->p_buffer[rb->tail & rb->size_mask];

    __DMB();
    rb->tail++;
    return true;
}

// 批量写入
uint32_t rb_write(ring_buffer_t *rb, const uint8_t *data, uint32_t len) {
    uint32_t free = rb_free_count(rb);
    if (len > free) len = free;

    if (len == 0) return 0;

    uint32_t write_idx = rb->head & rb->size_mask;
    uint32_t size = rb->size_mask + 1;
    uint32_t first_chunk = size - write_idx;

    if (len <= first_chunk) {
        // 不需要回绕
        memcpy(&rb->p_buffer[write_idx], data, len);
    } else {
        // 需要回绕
        memcpy(&rb->p_buffer[write_idx], data, first_chunk);
        memcpy(&rb->p_buffer[0], &data[first_chunk], len - first_chunk);
    }

    __DMB();
    rb->head += len;
    return len;
}

// --- DMA 优化核心功能 ---

// 获取线性可读区域 (Zero-Copy 关键)
// 返回指针可以直接给 DMA Source Address
void* rb_get_read_ptr(ring_buffer_t *rb, uint32_t *cnt) {
    uint32_t used = rb_used_count(rb);
    if (used == 0) {
        *cnt = 0;
        return NULL;
    }

    uint32_t read_idx = rb->tail & rb->size_mask;
    uint32_t size = rb->size_mask + 1;
    uint32_t continuous_len = size - read_idx;

    // 如果数据回绕了，DMA 只能先传第一段
    // 剩下的数据需要第二次调用此函数获取
    if (used < continuous_len) {
        *cnt = used;
    } else {
        *cnt = continuous_len;
    }

    return (void*)&rb->p_buffer[read_idx];
}

// DMA 传输完成后调用此函数更新 Tail
void rb_advance_tail(ring_buffer_t *rb, uint32_t len) {
    __DMB();
    rb->tail += len;
}