/**
  ******************************************************************************
  * @file    GPIO/IOToggle/User/inc/main.c
  * @author  MCD Application Team
  * @version V3.4.0
  * @date    10/15/2010
  * @brief   Main program body.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup GPIO_IOToggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* 为了使编程书写方便，我们定义几个控制LED开关的宏 */
#define LED1_ON()	{GPIO_ResetBits(GPIOF, GPIO_Pin_6);}    /* PF6 = 0 点亮LED1 */
#define LED1_OFF()	{GPIO_SetBits(GPIOF, GPIO_Pin_6);}		/* PF6 = 1 熄灭LED1 */
#define LED2_ON()	{GPIO_ResetBits(GPIOF, GPIO_Pin_7);}	/* PF7 = 0 点亮LED2 */
#define LED2_OFF()	{GPIO_SetBits(GPIOF, GPIO_Pin_7);}		/* PF7 = 1 点亮LED2 */
#define LED3_ON()	{GPIO_ResetBits(GPIOF, GPIO_Pin_8);}	/* PF8 = 0 点亮LED3 */
#define LED3_OFF()	{GPIO_SetBits(GPIOF, GPIO_Pin_8);}		/* PF8 = 1 点亮LED3 */
#define LED4_ON()	{GPIO_ResetBits(GPIOF, GPIO_Pin_9);}	/* PF9 = 0 点亮LED4 */
#define LED4_OFF()	{GPIO_SetBits(GPIOF, GPIO_Pin_9);}		/* PF9 = 1 点亮LED4 */

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void Delay(uint32_t nCount);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Inserts a delay time.
  * @param  nCount: specifies the delay time length.
  * @retval None
  */
static void Delay(uint32_t nCount)
{
	for(; nCount != 0; nCount--);
}

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*!< At this stage the microcontroller clock setting is already configured,
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */

	/* Configure all unused GPIO port pins in Analog Input mode (floating input
     trigger OFF), this will reduce the power consumption and increase the device
     immunity against EMI/EMC *************************************************/

	/*  armfly : 由于代码在外部SRAM运行时，GPIOD,E,F,G部分IO用于FSMC，
      因此对这些IO不能重置，否则导致取指异常
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                         RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
                         RCC_APB2Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	*/

	/* 打开GPIOF时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);

	/* 在将LED设置为输出前先设置输出1，避免一开始就点亮LED */
	GPIO_SetBits(GPIOF, GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);

	/* 可以对PF6 - PF9 一起初始化 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7
								| GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* 推挽输出模式 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure); /* 调用库函数初始化GPIO */

	/* 下面这个while循环实现简单的跑马灯功能 */
	while (1)
	{
		LED1_ON();		/* 点亮LED1 */
		Delay(0xAFFFF); /* 插入延时 */

		LED2_ON();		/* 点亮LED2 */
		LED3_ON();		/* 点亮LED3 */
		LED1_OFF();		/* 熄灭LED1 */
		Delay(0xAFFFF); /* 插入延时 */

		LED4_ON();		/* 点亮LED4 */
		LED2_OFF();		/* 关闭LED2 */
		LED3_OFF();		/* 关闭LED3 */
		Delay(0xAFFFF); /* 插入延时 */

		LED4_OFF();		/* 关闭LED4 */
		Delay(0xAFFFF); /* 插入延时 */
	}
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
