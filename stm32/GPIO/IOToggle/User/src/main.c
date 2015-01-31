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
/* Ϊ��ʹ�����д���㣬���Ƕ��弸������LED���صĺ� */
#define LED1_ON()	{GPIO_ResetBits(GPIOF, GPIO_Pin_6);}    /* PF6 = 0 ����LED1 */
#define LED1_OFF()	{GPIO_SetBits(GPIOF, GPIO_Pin_6);}		/* PF6 = 1 Ϩ��LED1 */
#define LED2_ON()	{GPIO_ResetBits(GPIOF, GPIO_Pin_7);}	/* PF7 = 0 ����LED2 */
#define LED2_OFF()	{GPIO_SetBits(GPIOF, GPIO_Pin_7);}		/* PF7 = 1 ����LED2 */
#define LED3_ON()	{GPIO_ResetBits(GPIOF, GPIO_Pin_8);}	/* PF8 = 0 ����LED3 */
#define LED3_OFF()	{GPIO_SetBits(GPIOF, GPIO_Pin_8);}		/* PF8 = 1 ����LED3 */
#define LED4_ON()	{GPIO_ResetBits(GPIOF, GPIO_Pin_9);}	/* PF9 = 0 ����LED4 */
#define LED4_OFF()	{GPIO_SetBits(GPIOF, GPIO_Pin_9);}		/* PF9 = 1 ����LED4 */

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

	/*  armfly : ���ڴ������ⲿSRAM����ʱ��GPIOD,E,F,G����IO����FSMC��
      ��˶���ЩIO�������ã�������ȡָ�쳣
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

	/* ��GPIOFʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);

	/* �ڽ�LED����Ϊ���ǰ���������1������һ��ʼ�͵���LED */
	GPIO_SetBits(GPIOF, GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);

	/* ���Զ�PF6 - PF9 һ���ʼ�� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7
								| GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* �������ģʽ */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure); /* ���ÿ⺯����ʼ��GPIO */

	/* �������whileѭ��ʵ�ּ򵥵�����ƹ��� */
	while (1)
	{
		LED1_ON();		/* ����LED1 */
		Delay(0xAFFFF); /* ������ʱ */

		LED2_ON();		/* ����LED2 */
		LED3_ON();		/* ����LED3 */
		LED1_OFF();		/* Ϩ��LED1 */
		Delay(0xAFFFF); /* ������ʱ */

		LED4_ON();		/* ����LED4 */
		LED2_OFF();		/* �ر�LED2 */
		LED3_OFF();		/* �ر�LED3 */
		Delay(0xAFFFF); /* ������ʱ */

		LED4_OFF();		/* �ر�LED4 */
		Delay(0xAFFFF); /* ������ʱ */
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
