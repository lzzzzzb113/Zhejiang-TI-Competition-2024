/* DriverLib Includes */
#include <driverlib.h>

//引脚配置：
//sbit LCD_RS=P2^6;
//sbit LCD_RW=P2^5;
//sbit LCD_EN=P2^7;
//#define LCD_DataPort P0

void czk_GPIOPinWrite(uint32_t ui32Port,uint8_t ui8Pins,uint8_t ui8Val)
{
ASSERT(_GPIOBaseValid(ui32Port));
//这一步就是帮你与好了对应的掩码。
if(ui8Val>0){
ui8Val= 0xff & ui8Pins;
}
GPIOPinWrite(ui32Port,ui8Pins,ui8Val);
}
void LCD_WriteCommand();
void LCD_Init(void)
{
	// Enable the GPIO port that is used for the LED.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)) ;
	
	// Enable the GPIO pin for the LED .  Set the direction as output, and
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_2);
	
	// Enable the GPIO port that is used for the LCD.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG); //PG
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG)) ;
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK); //PK
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)) ;
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM); //PM
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)) ;
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH); //PH
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOH)) ;
	
	// Enable the GPIO pin for the LDC .  Set the direction as output, and
	// enable the GPIO pin for digital function.
	GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_5);
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOOutput(GPIO_PORTH_BASE, GPIO_PIN_0);
	GPIOPinTypeGPIOOutput(GPIO_PORTH_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_6);
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_7);
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_6);
	
	LCD_WriteCommand(0x38);//八位数据接口，两行显示，5*7点阵
	LCD_WriteCommand(0x0c);//显示开，光标关，闪烁关
	LCD_WriteCommand(0x06);//数据读写操作后，光标自动加一，画面不动
	LCD_WriteCommand(0x01);//光标复位，清屏
}

//函数定义：
/**
  * @brief  LCD1602延时函数，12MHz调用可延时1ms
  * @param  无
  * @retval 无
  */
void LCD_Delay()
{
	unsigned char i, j;

	i = 2;
	j = 239;
	do
	{
		while (--j);
	} while (--i);
}

/**
  * @brief  LCD1602写命令
  * @param  Command 要写入的命令
  * @retval 无
  */

void LCD_WriteCommand(uint8_t Command)
{
	czk_GPIOPinWrite(GPIO_PORTG_BASE,GPIO_PIN_1,1);
	czk_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_4,0);					//	LCD_RW=0;
	czk_GPIOPinWrite(GPIO_PORTM_BASE,GPIO_PIN_6,Command&0x80);	//	LCD_DataPort=Command;
	czk_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_7,Command&0x40);
	czk_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_6,Command&0x20);	//	LCD_EN=1;
	czk_GPIOPinWrite(GPIO_PORTH_BASE,GPIO_PIN_1,Command&0x10);
	czk_GPIOPinWrite(GPIO_PORTH_BASE,GPIO_PIN_0,Command&0x08);
	czk_GPIOPinWrite(GPIO_PORTM_BASE,GPIO_PIN_2,Command&0x04);
	czk_GPIOPinWrite(GPIO_PORTM_BASE,GPIO_PIN_1,Command&0x02);
	czk_GPIOPinWrite(GPIO_PORTM_BASE,GPIO_PIN_0,Command&0x01);
	czk_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,1);
	LCD_Delay();
	czk_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
	LCD_Delay();
													
}
	//	LCD_RW=0;
//	LCD_DataPort=Command;/**
//	LCD_EN=1;  * @brief  LCD1602写数据
	//	LCD_Delay();  * @param  Data 要写入的数据
	//	LCD_EN=0;  * @retval 无
	//	LCD_Delay();  */
void LCD_WriteData(uint8_t  Data)
{
	czk_GPIOPinWrite(GPIO_PORTG_BASE,GPIO_PIN_1,1);
	czk_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_4,0);
	
	czk_GPIOPinWrite(GPIO_PORTM_BASE,GPIO_PIN_6,Data&0x80);
	czk_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_7,Data&0x40);
	czk_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_6,Data&0x20);
	czk_GPIOPinWrite(GPIO_PORTH_BASE,GPIO_PIN_1,Data&0x10);
	czk_GPIOPinWrite(GPIO_PORTH_BASE,GPIO_PIN_0,Data&0x08);
	czk_GPIOPinWrite(GPIO_PORTM_BASE,GPIO_PIN_2,Data&0x04);
	czk_GPIOPinWrite(GPIO_PORTM_BASE,GPIO_PIN_1,Data&0x02);
	czk_GPIOPinWrite(GPIO_PORTM_BASE,GPIO_PIN_0,Data&0x01);
	
	czk_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,1);
	LCD_Delay();
	czk_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
	LCD_Delay();
	
	
	
	
	
}
//	LCD_RS=1;
//	LCD_RW=0;
//	LCD_DataPort=Data;
//	LCD_EN=1;
//	LCD_Delay();
//	LCD_EN=0;
//	LCD_Delay();

/**
  * @brief  LCD1602设置光标位置
  * @param  Line 行位置，范围：1~2
  * @param  Column 列位置，范围：1~16
  * @retval 无
  */
void LCD_SetCursor(uint8_t Line,uint8_t Column)
{
	if(Line==1)
	{
		LCD_WriteCommand(0x80|(Column-1));
	}
	else if(Line==2)
	{
		LCD_WriteCommand(0x80|(Column-1+0x40));
	}
}

/**
  * @brief  LCD1602初始化函数
  * @param  无
  * @retval 无
  */
//void LCD_Init()
//{
//	LCD_WriteCommand(0x38);//八位数据接口，两行显示，5*7点阵
//	LCD_WriteCommand(0x0c);//显示开，光标关，闪烁关
//	LCD_WriteCommand(0x06);//数据读写操作后，光标自动加一，画面不动
//	LCD_WriteCommand(0x01);//光标复位，清屏
//}

/**
  * @brief  在LCD1602指定位置上显示一个字符
  * @param  Line 行位置，范围：1~2
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的字符
  * @retval 无
  */
void LCD_ShowChar(uint8_t  Line,uint8_t  Column,int8_t  Char)
{
	LCD_SetCursor(Line,Column);
	LCD_WriteData(Char);
}

/**
  * @brief  在LCD1602指定位置开始显示所给字符串
  * @param  Line 起始行位置，范围：1~2
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串
  * @retval 无
  */
void LCD_ShowString(uint8_t  Line,uint8_t  Column,int8_t  *String)
{
	unsigned char i;
	LCD_SetCursor(Line,Column);
	for(i=0;String[i]!='\0';i++)
	{
		LCD_WriteData(String[i]);
	}
}

/**
  * @brief  返回值=X的Y次方
  */
int LCD_Pow(int16_t  X,int16_t  Y)
{
	unsigned char i;
	int Result=1;
	for(i=0;i<Y;i++)
	{
		Result*=X;
	}
	return Result;
}

/**
  * @brief  在LCD1602指定位置开始显示所给数字
  * @param  Line 起始行位置，范围：1~2
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~65535
  * @param  Length 要显示数字的长度，范围：1~5
  * @retval 无
  */
void LCD_ShowNum(uint8_t  Line,uint8_t Column,uint8_t  Number,uint8_t  Length)
{
	unsigned char i;
	LCD_SetCursor(Line,Column);
	for(i=Length;i>0;i--)
	{
		LCD_WriteData(Number/LCD_Pow(10,i-1)%10+'0');
	}
}

/**
  * @brief  在LCD1602指定位置开始以有符号十进制显示所给数字
  * @param  Line 起始行位置，范围：1~2
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：-32768~32767
  * @param  Length 要显示数字的长度，范围：1~5
  * @retval 无
  */
void LCD_ShowSignedNum(uint8_t  Line,uint8_t  Column,int16_t Number,uint8_t  Length)
{
	unsigned char i;
	unsigned int Number1;
	LCD_SetCursor(Line,Column);
	if(Number>=0)
	{
		LCD_WriteData('+');
		Number1=Number;
	}
	else
	{
		LCD_WriteData('-');
		Number1=-Number;
	}
	for(i=Length;i>0;i--)
	{
		LCD_WriteData(Number1/LCD_Pow(10,i-1)%10+'0');
	}
}

/**
  * @brief  在LCD1602指定位置开始以十六进制显示所给数字
  * @param  Line 起始行位置，范围：1~2
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~0xFFFF
  * @param  Length 要显示数字的长度，范围：1~4
  * @retval 无
  */
void LCD_ShowHexNum(uint16_t  Line,uint8_t  Column,uint8_t  Number,uint8_t  Length)
{
	unsigned char i,SingleNumber;
	LCD_SetCursor(Line,Column);
	for(i=Length;i>0;i--)
	{
		SingleNumber=Number/LCD_Pow(16,i-1)%16;
		if(SingleNumber<10)
		{
			LCD_WriteData(SingleNumber+'0');
		}
		else
		{
			LCD_WriteData(SingleNumber-10+'A');
		}
	}
}


/**
  * @brief  在LCD1602指定位置开始以二进制显示所给数字
  * @param  Line 起始行位置，范围：1~2
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
  * @param  Length 要显示数字的长度，范围：1~16
  * @retval 无
  */
void LCD_ShowBinNum(uint8_t Line,uint8_t Column,uint16_t Number,uint8_t Length)
{
	unsigned char i;
	LCD_SetCursor(Line,Column);
	for(i=Length;i>0;i--)
	{
		LCD_WriteData(Number/LCD_Pow(2,i-1)%2+'0');
	}
}
