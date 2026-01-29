/* HF2.ld - 完整修复版 */
ENTRY(Reset_Handler)

MEMORY
{
  FLASH     (rx)  : ORIGIN = 0x00000000, LENGTH = 512K
  OTP_DATA  (rx)  : ORIGIN = 0x03000C00, LENGTH = 960
  OTP_LOCK  (rx)  : ORIGIN = 0x03000FC0, LENGTH = 60
  SRAMH     (rwx) : ORIGIN = 0x1FFF8000, LENGTH = 32K
  SRAM1     (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
  SRAM2     (rwx) : ORIGIN = 0x20010000, LENGTH = 64K
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

  . = ALIGN(4);

  .tm_clone_table :
  {
    PROVIDE_HIDDEN (__tm_clone_table_start = .);
    KEEP(*(.tm_clone_table))
    PROVIDE_HIDDEN (__tm_clone_table_end = .);
  } > FLASH

  . = ALIGN(4);

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } > FLASH

  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
  .ARM.exidx :
  {
    __exidx_start = .;
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    __exidx_end = .;
  } > FLASH

  .tm_clone_table :
  {
    __tm_clone_table_start = .;
    KEEP(*(.tm_clone_table))
    __tm_clone_table_end = .;
  } > FLASH

  .eh_frame :
  {
  __eh_frame_start = .;
  KEEP(*(.eh_frame*))
  __eh_frame_end = .;
  } > FLASH

  .init_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);
  } > FLASH

  .fini_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } > FLASH

  /* 记录代码段结束位置，作为 Flash 加载区的起始点 */
  . = ALIGN(4);
  __flash_load_start = .;

  /* 1. RamFunc 段 (LMA 线性排列) */
  .ram_code : AT (__flash_load_start)
  {
    . = ALIGN(4);
    _sram_code = .;
    *(.ram_code*)
    *(.ramfunc*)
    *motor_ctrl*.o(.text*)
    *hall_sensor*.o(.text*)
    . = ALIGN(4);
    _eram_code = .;
  } > SRAMH
  _siram_code = LOADADDR(.ram_code);

  /* 2. Data 段 (LMA 紧随其后) */
  .data : AT (LOADADDR(.ram_code) + SIZEOF(.ram_code))
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
  __etext = LOADADDR(.data);

  .bss :
  {
    . = ALIGN(4);
    __bss_start__ = .;
    _sbss = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
    _ebss = .;
  } > SRAM1

  /* 3. 备份 RAM 数据 (LMA 继续顺延) */
  .ret_ram_data : AT (LOADADDR(.data) + SIZEOF(.data))
  {
    . = ALIGN(4);
    __data_start_ret_ram__ = .;
    *(.ret_ram_data*)
    . = ALIGN(4);
    __data_end_ret_ram__ = .;
  } > RET_RAM
  __etext_ret_ram = LOADADDR(.ret_ram_data);

  .ret_ram_bss :
  {
    . = ALIGN(4);
    __bss_start_ret_ram__ = .;
    *(.ret_ram_bss*)
    . = ALIGN(4);
    __bss_end_ret_ram__ = .;
  } > RET_RAM

  /* 4. DMA 与 堆栈 */
  .dma_buffer (NOLOAD) :
  {
    . = ALIGN(4);
    *(SECTION_DMA_BUF)
    *(.dma_buffer)
  } > SRAM2

  .heap_stack :
  {
    . = ALIGN(8);
    __HeapBase = .;
    . += 0x2000;
    __HeapLimit = .;
    . = ALIGN(8);
    __StackLimit = .;
    . += 0x2000;
    __StackTop = .;
  } > SRAM1

  PROVIDE(_stack = __StackTop);

  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}