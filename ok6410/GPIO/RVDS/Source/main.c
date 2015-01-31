/**************************************************************************************
* 
*	Project Name : OK6410 GPIO
*
*	Copyright 2015 by opentechshare.
*	All rights reserved.
*
**************************************************************************************/

#define rGPMCON (*(volatile unsigned *)(0x7F008820))
#define rGPMDAT (*(volatile unsigned *)(0x7F008824))
#define rGPMPUD (*(volatile unsigned *)(0x7F008828))

static void Delay(unsigned int time)
{
    unsigned int i = 0;
	unsigned int j = 0;

    for (i = 0; i < 2000000; i++)
	{
    	for (j=0; j < time; j++)
		{
			/* NOP */
		}
	}
}

static void GPIO_Init(void)
{
	rGPMCON = 0x1111;
    rGPMPUD = 0x00;
    rGPMDAT = 0X0F;
}

static void Led_Test(void)
{
	unsigned int i = 0;

	while (1)
	{
		for (i = 0; i < 4; i++)
		{
			rGPMDAT = ~(1 << i);
			Delay(10);
		}

	}
}

void Main(void)
{
	GPIO_Init();
	Led_Test();
}
