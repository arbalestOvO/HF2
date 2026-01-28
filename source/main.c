/**
 *******************************************************************************
 * @file  main.c
 * @brief Main program.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2026-01-28       CDT             First version
 @endverbatim
 *******************************************************************************
 * Copyright (C) 2022-2025, Xiaohua Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by XHSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "main.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
/* INT_SRC_USART1_TI Callback. */
static void INT_SRC_USART1_TI_IrqCallback(void);
/* INT_SRC_USART1_TCI Callback. */
static void INT_SRC_USART1_TCI_IrqCallback(void);
/* INT_SRC_USART1_RI Callback. */
static void INT_SRC_USART1_RI_IrqCallback(void);
/* INT_SRC_USART1_EI Callback. */
static void INT_SRC_USART1_EI_IrqCallback(void);
/* INT_SRC_USART1_WUPI Callback. */
static void INT_SRC_USART1_WUPI_IrqCallback(void);
/* INT_SRC_USART2_TI Callback. */
static void INT_SRC_USART2_TI_IrqCallback(void);
/* INT_SRC_USART2_TCI Callback. */
static void INT_SRC_USART2_TCI_IrqCallback(void);
/* Configures Timer0. */
static void App_Timer0Cfg(void);
/* Configures USARTx. */
static void App_USARTxCfg(void);
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
//Clock Config
static void App_ClkCfg(void)
{
    /* Set bus clock div. */
    CLK_SetClockDiv(CLK_BUS_CLK_ALL, (CLK_HCLK_DIV1 | CLK_EXCLK_DIV2 | CLK_PCLK0_DIV1 | CLK_PCLK1_DIV2 | \
                                   CLK_PCLK2_DIV4 | CLK_PCLK3_DIV4 | CLK_PCLK4_DIV2));
    /* sram init include read/write wait cycle setting */
    SRAM_SetWaitCycle(SRAM_SRAM_ALL, SRAM_WAIT_CYCLE1, SRAM_WAIT_CYCLE1);
    SRAM_SetWaitCycle(SRAM_SRAMH, SRAM_WAIT_CYCLE0, SRAM_WAIT_CYCLE0);
    /* flash read wait cycle setting */
    EFM_SetWaitCycle(EFM_WAIT_CYCLE5);
    /* XTAL config */
    stc_clock_xtal_init_t stcXtalInit;
    (void)CLK_XtalStructInit(&stcXtalInit);
    stcXtalInit.u8State = CLK_XTAL_ON;
    stcXtalInit.u8Drv = CLK_XTAL_DRV_HIGH;
    stcXtalInit.u8Mode = CLK_XTAL_MD_OSC;
    stcXtalInit.u8StableTime = CLK_XTAL_STB_2MS;
    (void)CLK_XtalInit(&stcXtalInit);
    /* MPLL config */
    stc_clock_pll_init_t stcMPLLInit;
    (void)CLK_PLLStructInit(&stcMPLLInit);
    stcMPLLInit.PLLCFGR = 0UL;
    stcMPLLInit.PLLCFGR_f.PLLM = (1UL - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLN = (50UL - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLP = (2UL - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLQ = (2UL - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLR = (2UL - 1UL);
    stcMPLLInit.u8PLLState = CLK_PLL_ON;
    stcMPLLInit.PLLCFGR_f.PLLSRC = CLK_PLL_SRC_XTAL;
    (void)CLK_PLLInit(&stcMPLLInit);
    /* 3 cycles for 126MHz ~ 200MHz */
    GPIO_SetReadWaitCycle(GPIO_RD_WAIT3);
    /* Switch driver ability */
    PWC_HighSpeedToHighPerformance();
    /* Set the system clock source */
    CLK_SetSysClockSrc(CLK_SYSCLK_SRC_PLL);
}

//Port Config
static void App_PortCfg(void)
{
    /* GPIO initialize */
    stc_gpio_init_t stcGpioInit;
    /* PH2 set to GPIO-Input */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PullUp = PIN_PU_ON;
    stcGpioInit.u16ExtInt = PIN_EXTINT_ON;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_H, GPIO_PIN_02, &stcGpioInit);

    /* PA3 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_A, GPIO_PIN_03, &stcGpioInit);

    /* PA4 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_A, GPIO_PIN_04, &stcGpioInit);

    /* PA5 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_A, GPIO_PIN_05, &stcGpioInit);

    /* PA7 set to GPIO-Input */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PullUp = PIN_PU_ON;
    stcGpioInit.u16ExtInt = PIN_EXTINT_ON;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_A, GPIO_PIN_07, &stcGpioInit);

    /* PB15 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_B, GPIO_PIN_15, &stcGpioInit);

    /* PA8 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_A, GPIO_PIN_08, &stcGpioInit);

    /* PA9 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_A, GPIO_PIN_09, &stcGpioInit);

    /* PA10 set to GPIO-Input */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PullUp = PIN_PU_ON;
    stcGpioInit.u16ExtInt = PIN_EXTINT_ON;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_A, GPIO_PIN_10, &stcGpioInit);

    /* PB4 set to GPIO-Output */
    GPIO_SetDebugPort(GPIO_PIN_TRST, DISABLE);
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_B, GPIO_PIN_04, &stcGpioInit);

    /* PB5 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_B, GPIO_PIN_05, &stcGpioInit);

    /* PB8 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_B, GPIO_PIN_08, &stcGpioInit);

    /* PB9 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_B, GPIO_PIN_09, &stcGpioInit);

    GPIO_SetFunc(GPIO_PORT_A,GPIO_PIN_00,GPIO_FUNC_32);//USART1-TX
    
    GPIO_SetFunc(GPIO_PORT_A,GPIO_PIN_01,GPIO_FUNC_33);//USART1-RX
    
    GPIO_SetFunc(GPIO_PORT_B,GPIO_PIN_00,GPIO_FUNC_9);//SDIO2-CMD
    
    GPIO_SetFunc(GPIO_PORT_B,GPIO_PIN_01,GPIO_FUNC_9);//SDIO2-D3
    
    GPIO_SetFunc(GPIO_PORT_B,GPIO_PIN_02,GPIO_FUNC_9);//SDIO2-D2
    
    GPIO_SetFunc(GPIO_PORT_B,GPIO_PIN_10,GPIO_FUNC_45);//SPI4-MISO
    
    GPIO_SetFunc(GPIO_PORT_B,GPIO_PIN_13,GPIO_FUNC_47);//SPI4-SCK
    
    GPIO_SetFunc(GPIO_PORT_B,GPIO_PIN_14,GPIO_FUNC_44);//SPI4-MOSI
    
    GPIO_SetFunc(GPIO_PORT_A,GPIO_PIN_11,GPIO_FUNC_36);//USART2-TX
    
    GPIO_SetFunc(GPIO_PORT_A,GPIO_PIN_12,GPIO_FUNC_37);//USART2-RX
    
    GPIO_SetDebugPort(GPIO_PIN_TDI, DISABLE);
    GPIO_SetFunc(GPIO_PORT_A,GPIO_PIN_15,GPIO_FUNC_9);//SDIO2-D1
    
    GPIO_SetDebugPort(GPIO_PIN_SWO, DISABLE);
    GPIO_SetFunc(GPIO_PORT_B,GPIO_PIN_03,GPIO_FUNC_9);//SDIO2-D0
    
    GPIO_SetFunc(GPIO_PORT_B,GPIO_PIN_06,GPIO_FUNC_9);//SDIO2-CK
    
}

//Int Config
static void App_IntCfg(void)
{
    stc_irq_signin_config_t stcIrq;

    /* IRQ sign-in */
    stcIrq.enIntSrc = INT_SRC_USART1_TI;
    stcIrq.enIRQn = INT080_IRQn;
    stcIrq.pfnCallback = &INT_SRC_USART1_TI_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    /* NVIC config */
    NVIC_ClearPendingIRQ(INT080_IRQn);
    NVIC_SetPriority(INT080_IRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(INT080_IRQn);

    /* IRQ sign-in */
    stcIrq.enIntSrc = INT_SRC_USART1_TCI;
    stcIrq.enIRQn = INT081_IRQn;
    stcIrq.pfnCallback = &INT_SRC_USART1_TCI_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    /* NVIC config */
    NVIC_ClearPendingIRQ(INT081_IRQn);
    NVIC_SetPriority(INT081_IRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(INT081_IRQn);

    /* IRQ sign-in */
    stcIrq.enIntSrc = INT_SRC_USART1_RI;
    stcIrq.enIRQn = INT082_IRQn;
    stcIrq.pfnCallback = &INT_SRC_USART1_RI_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    /* NVIC config */
    NVIC_ClearPendingIRQ(INT082_IRQn);
    NVIC_SetPriority(INT082_IRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(INT082_IRQn);

    /* IRQ sign-in */
    stcIrq.enIntSrc = INT_SRC_USART1_EI;
    stcIrq.enIRQn = INT083_IRQn;
    stcIrq.pfnCallback = &INT_SRC_USART1_EI_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    /* NVIC config */
    NVIC_ClearPendingIRQ(INT083_IRQn);
    NVIC_SetPriority(INT083_IRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(INT083_IRQn);

    /* IRQ sign-in */
    stcIrq.enIntSrc = INT_SRC_USART1_WUPI;
    stcIrq.enIRQn = INT110_IRQn;
    stcIrq.pfnCallback = &INT_SRC_USART1_WUPI_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    /* NVIC config */
    NVIC_ClearPendingIRQ(INT110_IRQn);
    NVIC_SetPriority(INT110_IRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(INT110_IRQn);

    /* IRQ sign-in */
    stcIrq.enIntSrc = INT_SRC_USART2_TI;
    stcIrq.enIRQn = INT084_IRQn;
    stcIrq.pfnCallback = &INT_SRC_USART2_TI_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    /* NVIC config */
    NVIC_ClearPendingIRQ(INT084_IRQn);
    NVIC_SetPriority(INT084_IRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(INT084_IRQn);

    /* IRQ sign-in */
    stcIrq.enIntSrc = INT_SRC_USART2_TCI;
    stcIrq.enIRQn = INT085_IRQn;
    stcIrq.pfnCallback = &INT_SRC_USART2_TCI_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    /* NVIC config */
    NVIC_ClearPendingIRQ(INT085_IRQn);
    NVIC_SetPriority(INT085_IRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(INT085_IRQn);

    /* IRQ sign-in */
    stcIrq.enIntSrc = INT_SRC_USART2_RI;
    stcIrq.enIRQn = INT000_IRQn;
    stcIrq.pfnCallback = &USART2_RxFull_IrqHandler;
    (void)INTC_IrqSignIn(&stcIrq);
    /* NVIC config */
    NVIC_ClearPendingIRQ(INT000_IRQn);
    NVIC_SetPriority(INT000_IRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(INT000_IRQn);

    /* IRQ sign-in */
    stcIrq.enIntSrc = INT_SRC_USART2_EI;
    stcIrq.enIRQn = INT001_IRQn;
    stcIrq.pfnCallback = &USART2_RxError_IrqHandler;
    (void)INTC_IrqSignIn(&stcIrq);
    /* NVIC config */
    NVIC_ClearPendingIRQ(INT001_IRQn);
    NVIC_SetPriority(INT001_IRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(INT001_IRQn);

}

/**
 * @brief  Main function of the project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Register write unprotected for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_ALL);
    //Clock Config
    App_ClkCfg();
    //Port Config
    App_PortCfg();
    //Int Config
    App_IntCfg();
    //Timer0 Config
    App_Timer0Cfg();
    //USARTx Config
    App_USARTxCfg();
    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_ALL);
    for (;;) {

    }
}

/* INT_SRC_USART1_TI Callback. */
static void INT_SRC_USART1_TI_IrqCallback(void)
{
    //add your codes here
}
/* INT_SRC_USART1_TCI Callback. */
static void INT_SRC_USART1_TCI_IrqCallback(void)
{
    //add your codes here
}
/* INT_SRC_USART1_RI Callback. */
static void INT_SRC_USART1_RI_IrqCallback(void)
{
    //add your codes here
}
/* INT_SRC_USART1_EI Callback. */
static void INT_SRC_USART1_EI_IrqCallback(void)
{
    //add your codes here
}
/* INT_SRC_USART1_WUPI Callback. */
static void INT_SRC_USART1_WUPI_IrqCallback(void)
{
    //add your codes here
}
/* INT_SRC_USART2_TI Callback. */
static void INT_SRC_USART2_TI_IrqCallback(void)
{
    //add your codes here
}
/* INT_SRC_USART2_TCI Callback. */
static void INT_SRC_USART2_TCI_IrqCallback(void)
{
    //add your codes here
}
/* INT_SRC_USART2_RI Callback. */
void USART2_RxFull_IrqHandler(void)
{
    //add your codes here
}
/* INT_SRC_USART2_EI Callback. */
void USART2_RxError_IrqHandler(void)
{
    //add your codes here
}

//Timer0 Config
static void App_Timer0Cfg(void)
{
    stc_tmr0_init_t stcTmr0Init;

    /* Enable AOS clock */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);
    /* Timer0 trigger event set */
    AOS_SetTriggerEventSrc(AOS_TMR0, EVT_SRC_PORT_EIRQ0);

    /* Enable timer0_1 clock */
    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR0_1, ENABLE);

    /************************* Configure TMR0_1_A***************************/
    (void)TMR0_StructInit(&stcTmr0Init);
    stcTmr0Init.u32ClockSrc = TMR0_CLK_SRC_XTAL32;
    stcTmr0Init.u32ClockDiv = TMR0_CLK_DIV1;
    stcTmr0Init.u32Func = TMR0_FUNC_CMP;
    stcTmr0Init.u16CompareValue = 0xFFFFU;
    (void)TMR0_Init(CM_TMR0_1, TMR0_CH_A, &stcTmr0Init);
    DDL_DelayMS(1U);
    TMR0_HWStartCondCmd(CM_TMR0_1, TMR0_CH_A, ENABLE);
    DDL_DelayMS(1U);
    TMR0_HWClearCondCmd(CM_TMR0_1, TMR0_CH_A, ENABLE);
    DDL_DelayMS(1U);
}

//USARTx Config
static void App_USARTxCfg(void)
{
    stc_usart_uart_init_t stcUartInit;

    /* Enable USART1 clock */
    FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART1, ENABLE);
    /************************* Configure USART1***************************/
    USART_DeInit(CM_USART1);
    (void)USART_UART_StructInit(&stcUartInit);
    stcUartInit.u32ClockSrc = USART_CLK_SRC_INTERNCLK;
    stcUartInit.u32ClockDiv = USART_CLK_DIV1;
    stcUartInit.u32CKOutput = USART_CK_OUTPUT_DISABLE;
    stcUartInit.u32Baudrate = 115200UL;
    stcUartInit.u32DataWidth = USART_DATA_WIDTH_8BIT;
    stcUartInit.u32StopBit = USART_STOPBIT_1BIT;
    stcUartInit.u32Parity = USART_PARITY_NONE;
    stcUartInit.u32OverSampleBit = USART_OVER_SAMPLE_16BIT;
    stcUartInit.u32FirstBit = USART_FIRST_BIT_LSB;
    stcUartInit.u32StartBitPolarity = USART_START_BIT_FALLING;
    stcUartInit.u32HWFlowControl = USART_HW_FLOWCTRL_RTS;
    USART_UART_Init(CM_USART1, &stcUartInit, NULL);
    /* Enable USART_TX | USART_RX | USART_INT_TX_EMPTY | USART_INT_TX_CPLT | USART_INT_RX function */
    USART_FuncCmd(CM_USART1, (USART_TX | USART_RX | USART_INT_TX_EMPTY | USART_INT_TX_CPLT | USART_INT_RX), ENABLE);

    /* Enable USART2 clock */
    FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART2, ENABLE);
    /************************* Configure USART2***************************/
    USART_DeInit(CM_USART2);
    (void)USART_UART_StructInit(&stcUartInit);
    stcUartInit.u32ClockSrc = USART_CLK_SRC_INTERNCLK;
    stcUartInit.u32ClockDiv = USART_CLK_DIV1;
    stcUartInit.u32CKOutput = USART_CK_OUTPUT_DISABLE;
    stcUartInit.u32Baudrate = 115200UL;
    stcUartInit.u32DataWidth = USART_DATA_WIDTH_8BIT;
    stcUartInit.u32StopBit = USART_STOPBIT_1BIT;
    stcUartInit.u32Parity = USART_PARITY_NONE;
    stcUartInit.u32OverSampleBit = USART_OVER_SAMPLE_16BIT;
    stcUartInit.u32FirstBit = USART_FIRST_BIT_LSB;
    stcUartInit.u32StartBitPolarity = USART_START_BIT_FALLING;
    stcUartInit.u32HWFlowControl = USART_HW_FLOWCTRL_RTS;
    USART_UART_Init(CM_USART2, &stcUartInit, NULL);
    /* Enable USART_TX | USART_RX | USART_INT_TX_EMPTY | USART_INT_TX_CPLT | USART_INT_RX function */
    USART_FuncCmd(CM_USART2, (USART_TX | USART_RX | USART_INT_TX_EMPTY | USART_INT_TX_CPLT | USART_INT_RX), ENABLE);
}

/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
