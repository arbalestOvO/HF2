/*
 * HF2.ld - Fixed Version
 * Fixes:
 * 1. Added .ret_ram_bss section (fixes undefined reference error)
 * 2. Fixed .data LMA syntax (fixes memory region warning)
 */

ENTRY(Reset_Handler)

MEMORY
{
  FLASH     (rx)  : ORIGIN = 0x00000000, LENGTH = 512K
  OTP_DATA  (rx)  : ORIGIN = 0x03000C00, LENGTH = 960
  OTP_LOCK  (rx)  : ORIGIN = 0x03000FC0, LENGTH = 60

  /* 自定义内存布局 */
  SRAMH     (rwx) : ORIGIN = 0x1FFF0000, LENGTH = 32K
  SRAM1     (rwx) : ORIGIN = 0x1FFF8000, LENGTH = 64K
  SRAM2     (rwx) : ORIGIN = 0x20000000, LENGTH = 64K

  RET_RAM   (rwx) : ORIGIN = 0x200F0000, LENGTH = 4K
}

SECTIONS
{
  .vectors :
  {
    . = ALIGN(4);
    KEEP(*(.vectors))
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } > FLASH

  .icg_sec 0x00000400 :
  {
    KEEP(*(.icg_sec))
  } > FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    . = ALIGN(4);
  } > FLASH

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } > FLASH

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > FLASH

  __exidx_start = .;
  .ARM.exidx :
  {
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } > FLASH
  __exidx_end = .;

  .preinit_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP(*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(4);
  } > FLASH

  .init_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(4);
  } > FLASH

  .fini_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } > FLASH

  /* 标记 Flash 中代码段结束位置，用于计算数据段搬运源地址 */
  __etext = ALIGN(4);

  /* ========================================================== */
  /* SRAMH: 极速代码区 (RamFunc) */
  /* ========================================================== */
  _siram_code = LOADADDR(.ram_code);

  .ram_code :
  {
    . = ALIGN(4);
    _sram_code = .;    /* VMA: RAM地址 */

    *(SECTION_RAM_CODE)
    *(.ram_code)
    *(.ramfunc)
    *motor_ctrl*.o(.text*)
    *hall_sensor*.o(.text*)

    . = ALIGN(4);
    _eram_code = .;
  } > SRAMH AT > FLASH

  /* ========================================================== */
  /* SRAM1: 主数据区 */
  /* ========================================================== */

  /* 修复: 使用 AT(__etext) 而不是 AT > __etext */
  .data : AT (__etext)
  {
    . = ALIGN(4);
    __data_start__ = .;
    _sdata = .;

    *(vtable)
    *(.data)
    *(.data*)

    . = ALIGN(4);
    __data_end__ = .;
    _edata = .;
  } > SRAM1

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    __bss_start__ = _sbss;

    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } > SRAM1

  /* ========================================================== */
  /* SRAM2: DMA 缓冲 (NOLOAD) */
  /* ========================================================== */
  .dma_buffer (NOLOAD) :
  {
    . = ALIGN(4);
    *(SECTION_DMA_BUF)
    *(.dma_buffer)
    . = ALIGN(4);
  } > SRAM2

  /* ========================================================== */
  /* 备份域 RAM (启动文件 ClearBss1 需要此段) */
  /* ========================================================== */
  __etext_ret_ram = __etext + SIZEOF(.data);

  .ret_ram_data : AT (__etext_ret_ram)
  {
    . = ALIGN(4);
    __data_start_ret_ram__ = .;
    *(.ret_ram_data)
    *(.ret_ram_data*)
    . = ALIGN(4);
    __data_end_ret_ram__ = .;
  } > RET_RAM

  /* [新增] 修复 undefined reference to __bss_start_ret_ram__ */
  .ret_ram_bss :
  {
    . = ALIGN(4);
    __bss_start_ret_ram__ = .;
    *(.ret_ram_bss)
    *(.ret_ram_bss*)
    . = ALIGN(4);
    __bss_end_ret_ram__ = .;
  } > RET_RAM

  .otp_data :
  {
    . = ALIGN(4);
    KEEP(*(.otp_b0_data*))
    KEEP(*(.otp_b1_data*))
  } > OTP_DATA

  .heap_stack :
  {
    . = ALIGN(8);
    PROVIDE(end = .);
    PROVIDE(_end = .);
    *(.heap*)
    . = ALIGN(8);
    __HeapLimit = .;
    __StackLimit = .;
    *(.stack*)
    . = ALIGN(8);
    __StackTop = .;
  } > SRAM1

  PROVIDE(_stack = __StackTop);
  __RamEnd = ORIGIN(SRAM1) + LENGTH(SRAM1);
  ASSERT(__StackTop <= __RamEnd, "region SRAM1 overflowed with stack")

  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}