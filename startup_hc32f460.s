/**
 ******************************************************************************
 * @file  startup_hc32f460.S
 * @brief 修复版启动文件 - 解决 HardFloat UsageFault 及初始化顺序问题
 ******************************************************************************
 */

                .syntax     unified
                .arch       armv7e-m
                .cpu        cortex-m4
                /* 关键修复：匹配编译器的 Hard Float 设置 */
                .fpu        fpv4-sp-d16
                .thumb

/* 外部符号声明 */
                .extern     __StackTop
                .extern     _siram_code
                .extern     _sram_code
                .extern     _eram_code
                .extern     __etext
                .extern     __data_start__
                .extern     __data_end__
                .extern     __etext_ret_ram
                .extern     __data_start_ret_ram__
                .extern     __data_end_ret_ram__
                .extern     __bss_start__
                .extern     __bss_end__
                .extern     __bss_start_ret_ram__
                .extern     __bss_end_ret_ram__
                .extern     SystemInit
                .extern     __libc_init_array
                .extern     main

/* 中断向量表 */
                .section    .vectors, "a", %progbits
                .align      2
                .globl      __Vectors
__Vectors:
                .long       __StackTop                         /* 栈顶地址 */
                .long       Reset_Handler                      /* 复位句柄 */
                .long       NMI_Handler                        /* -14 NMI */
                .long       HardFault_Handler                  /* -13 Hard Fault */
                .long       MemManage_Handler                  /* -12 MPU Fault */
                .long       BusFault_Handler                   /* -11 Bus Fault */
                .long       UsageFault_Handler                 /* -10 Usage Fault */
                .long       0, 0, 0, 0                         /* 保留 */
                .long       SVC_Handler                        /* -5 SVCall */
                .long       DebugMon_Handler                   /* -4 Debug Monitor */
                .long       0                                  /* 保留 */
                .long       PendSV_Handler                     /* -2 PendSV */
                .long       SysTick_Handler                    /* -1 SysTick */

                /* 外部中断占位 (保持对齐) */
                .irp        irq_num, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20, \
                            21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40, \
                            41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60, \
                            61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80, \
                            81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100, \
                            101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116, \
                            117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132, \
                            133,134,135,136,137,138,139,140,141,142,143
                .long       IRQ\irq_num\()_Handler
                .endr

                .size       __Vectors, . - __Vectors

/* 复位处理程序 */
                .section    .text.Reset_Handler
                .align      2
                .weak       Reset_Handler
                .type       Reset_Handler, %function
                .globl      Reset_Handler
Reset_Handler:
                /* 0. [关键修复] 早期开启 FPU (CP10, CP11) */
                ldr     r0, =0xE000ED88     /* CPACR 寄存器地址 */
                ldr     r1, [r0]
                orr     r1, r1, #(0xF << 20) /* 设置 CP10, CP11 为 Full Access */
                str     r1, [r0]
                dsb                         /* 数据同步屏障 */
                isb                         /* 指令同步屏障 */

                /* 1. 配置 SRAMC (HC32F460 特有) */
                ldr         r0, =0x40050810
                ldr         r1, =0x1F
                str         r1, [r0]

                /* 2. 搬运段 (即使大小为0也安全) */
                ldr         r1, =_siram_code
                ldr         r2, =_sram_code
                ldr         r3, =_eram_code
                bl          Copy_Routine

                ldr         r1, =__etext
                ldr         r2, =__data_start__
                ldr         r3, =__data_end__
                bl          Copy_Routine

                ldr         r1, =__etext_ret_ram
                ldr         r2, =__data_start_ret_ram__
                ldr         r3, =__data_end_ret_ram__
                bl          Copy_Routine

                /* 3. 清零 BSS */
                ldr         r1, =__bss_start__
                ldr         r2, =__bss_end__
                bl          Zero_Routine

                ldr         r1, =__bss_start_ret_ram__
                ldr         r2, =__bss_end_ret_ram__
                bl          Zero_Routine

                /* 4. 配置 SRAM 等待周期 */
                ldr         r0, =0x40050804
                mov         r1, #0x77
                str         r1, [r0]
                ldr         r0, =0x40050800
                mov         r1, #0x1100
                str         r1, [r0]

                /* 5. [关键顺序调整] 先初始化系统 (时钟/外设)，再初始化 C 库 */
                bl          SystemInit
                bl          __libc_init_array

                /* 6. 进入主函数 */
                bl          main

                /* 死循环兜底 */
                b           .
                .size       Reset_Handler, . - Reset_Handler

/* 数据拷贝子程序 (优化版) */
Copy_Routine:
                cmp         r2, r3
                bge         Copy_End      /* 如果长度为0，直接跳过 */
Copy_Loop:
                ldr         r0, [r1], #4
                str         r0, [r2], #4
                cmp         r2, r3
                blt         Copy_Loop
Copy_End:
                bx          lr

/* 清零子程序 */
Zero_Routine:
                cmp         r1, r2
                bge         Zero_End
                movs        r0, 0
Zero_Loop:
                str         r0, [r1], #4
                cmp         r1, r2
                blt         Zero_Loop
Zero_End:
                bx          lr

/* 默认异常处理 */
                .section    .text.Default_Handler, "ax", %progbits
Default_Handler:
                b           .
                .size       Default_Handler, . - Default_Handler

                .macro      Set_Default_Handler  Handler_Name
                .weak       \Handler_Name
                .set        \Handler_Name, Default_Handler
                .endm

                Set_Default_Handler    NMI_Handler
                Set_Default_Handler    HardFault_Handler
                Set_Default_Handler    MemManage_Handler
                Set_Default_Handler    BusFault_Handler
                Set_Default_Handler    UsageFault_Handler
                Set_Default_Handler    SVC_Handler
                Set_Default_Handler    DebugMon_Handler
                Set_Default_Handler    PendSV_Handler
                Set_Default_Handler    SysTick_Handler

                .irp        irq_num, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20, \
                            21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40, \
                            41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60, \
                            61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80, \
                            81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100, \
                            101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116, \
                            117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132, \
                            133,134,135,136,137,138,139,140,141,142,143
                Set_Default_Handler    IRQ\irq_num\()_Handler
                .endr

                .end