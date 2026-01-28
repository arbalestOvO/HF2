/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include <string.h>

#include "mmc.h"
#include "hc32_ll_sdioc.h"

/* 定义使用的 SDIOC 实例，你使用的是 SDIO2 */
#define MMC_SDIOC_INSTANCE      (CM_SDIOC2)

/* 定义超时时间 (ms) */
#define MMC_TIMEOUT             (10000U)

/* 定义物理驱动器号 */
#define EMMC_DRV_NO             0

/* 扇区大小 */
#define EMMC_BLOCK_SIZE         512

/* * 全局 MMC 句柄
 * 注意：必须是全局变量，不能是局部变量，否则栈内存释放后句柄失效
 */
static stc_mmc_handle_t hMMC;

/* * 用于解决地址不对齐问题的临时缓冲区
 * HC32 的 DMA 或 FIFO 操作通常要求 4 字节对齐
 */
static uint8_t scratch_buffer[EMMC_BLOCK_SIZE] __attribute__((aligned(4)));

/* 状态标记 */
static volatile DSTATUS Stat = STA_NOINIT;

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    int32_t i32Ret;

    if (pdrv != EMMC_DRV_NO) return STA_NOINIT;

    /* 如果已经初始化过，直接返回成功 */
    if ((Stat & STA_NOINIT) == 0) {
        return Stat;
    }

    /* 1. 配置句柄基本参数 */
    /* 清零结构体 */
    memset(&hMMC, 0, sizeof(stc_mmc_handle_t));

    /* 关联硬件寄存器 */
    hMMC.SDIOCx = MMC_SDIOC_INSTANCE;

    /* 2. 配置 SDIOC 初始化参数 */
    /* 官方库通常要求先 SDIOC_StructInit，再根据需求修改 */
    (void)SDIOC_StructInit(&hMMC.stcSdiocInit);

    /* 针对 eMMC 的关键配置 */
    hMMC.stcSdiocInit.u32Mode = SDIOC_MD_MMC;           // 必须是 MMC 模式
    hMMC.stcSdiocInit.u8BusWidth = SDIOC_BUS_WIDTH_4BIT; // 4-bit 模式
    hMMC.stcSdiocInit.u8SpeedMode = SDIOC_SPEED_MD_HIGH; // eMMC 通常支持高速
    hMMC.stcSdiocInit.u16ClockDiv = SDIOC_CLK_DIV4;    // 设置时钟分频，根据PCLK1实际频率调整
                                                          // 初始枚举时库内部会自动降频，这里设的是传输频率

    /* * 注意：MMC_Init 内部会自动完成以下流程：
     * CMD0 -> CMD1 (OpCond) -> CMD2 (CID) -> CMD3 (RCA) -> CSD解析 -> 选卡 -> 切4bit
     */
    i32Ret = MMC_Init(&hMMC);

    if (i32Ret == LL_OK) {
        Stat &= ~STA_NOINIT; /* 清除未初始化标志 */
    } else {
        Stat = STA_NOINIT;   /* 初始化失败 */
    }

    return Stat;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    if (pdrv != EMMC_DRV_NO) return STA_NOINIT;
    return Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
    int32_t i32Ret;

    if (pdrv != EMMC_DRV_NO || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    /* 处理地址不对齐的情况 (HC32 常见陷阱) */
    if (((uint32_t)buff & 0x03) != 0) {
        /* 如果地址未 4 字节对齐，必须逐个扇区读到内部 Buffer 再拷贝 */
        for (UINT i = 0; i < count; i++) {
            /* 1. 读到对齐的 scratch_buffer */
            i32Ret = MMC_ReadBlocks(&hMMC, (uint32_t)(sector + i), 1, scratch_buffer, MMC_TIMEOUT);

            if (i32Ret != LL_OK) return RES_ERROR;

            /* 2. 拷贝到用户 buffer */
            memcpy((void*)&buff[i * EMMC_BLOCK_SIZE], scratch_buffer, EMMC_BLOCK_SIZE);
        }
        return RES_OK;
    }

    /* 地址对齐，直接读取 (Blocking 模式) */
    /* 注意：HC32 库的 ReadBlocks 第2个参数是 Block Address (LBA)，不需要乘以 512 */
    i32Ret = MMC_ReadBlocks(&hMMC, (uint32_t)sector, (uint16_t)count, buff, MMC_TIMEOUT);

    return (i32Ret == LL_OK) ? RES_OK : RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data buffer to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
    int32_t i32Ret;

    if (pdrv != EMMC_DRV_NO || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    /* 处理地址不对齐的情况 */
    if (((uint32_t)buff & 0x03) != 0) {
        for (UINT i = 0; i < count; i++) {
            /* 1. 拷贝用户数据到对齐 Buffer */
            memcpy(scratch_buffer, (const void*)&buff[i * EMMC_BLOCK_SIZE], EMMC_BLOCK_SIZE);

            /* 2. 写入 */
            i32Ret = MMC_WriteBlocks(&hMMC, (uint32_t)(sector + i), 1, scratch_buffer, MMC_TIMEOUT);

            if (i32Ret != LL_OK) return RES_ERROR;
        }
        return RES_OK;
    }

    /* 地址对齐，直接写入 */
    /* 需要去掉 const 限定符，因为库函数原型通常是 uint8_t* */
    i32Ret = MMC_WriteBlocks(&hMMC, (uint32_t)sector, (uint16_t)count, (uint8_t *)buff, MMC_TIMEOUT);

    return (i32Ret == LL_OK) ? RES_OK : RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
    DRESULT res = RES_ERROR;

    if (pdrv != EMMC_DRV_NO) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    switch (cmd) {
        /* 确保写缓存写入物理介质 */
        case CTRL_SYNC :
            /* * MMC_WriteBlocks 内部通常已经等待了 Transfer Complete，
             * 但为了保险，可以检查一下卡状态是否为 TRAN (Transfer State)
             */
            res = RES_OK;
            break;

        /* 获取扇区数量 (f_mkfs 和 f_getfree 需要) */
        case GET_SECTOR_COUNT :
            /* 从 MMC_Init 解析的 CSD 信息中获取逻辑块数量 */
            *(LBA_t*)buff = hMMC.stcMmcCardInfo.u32LogBlockNum;
            res = RES_OK;
            break;

        /* 获取扇区大小 */
        case GET_SECTOR_SIZE :
            *(WORD*)buff = EMMC_BLOCK_SIZE;
            res = RES_OK;
            break;

        /* 获取擦除块大小 (扇区为单位) */
        case GET_BLOCK_SIZE :
            *(DWORD*)buff = 1; /* 默认设为 1，或者根据 eMMC 规格书设置 */
            res = RES_OK;
            break;

        default:
            res = RES_PARERR;
    }

    return res;
}

/* * 获得当前时间 (FatFs 用于文件时间戳)
 * 如果你有 RTC，请在此填充实际时间
 * 返回格式: bit31:25=Year(from 1980), 24:21=Month, 20:16=Day, 15:11=Hour, 10:5=Min, 4:0=Sec/2
 */
DWORD get_fattime (void)
{
    return 0;
}