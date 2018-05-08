#include <msp430.h> 
#include "oled.h"
#include "bmp.h"

#define      ADS1118_CS         BIT7  //这是配置单片机的管脚，与ads1118上的名字对应
#define      ADS1118_CLK        BIT1
#define      ADS1118_OUT        BIT5
#define      ADS1118_IN         BIT3


#define      ADS1118_Port_OUT   P1OUT  //这个是单片机上管脚开头编号，即P1.0接CS,P1.1接CLK,P1.2接DIN,P1.3接DOUT
#define      ADS1118_Port_DIR   P1DIR
#define      ADS1118_Port_IN    P1IN

#define      ADS1118_CS_OUT     (ADS1118_Port_DIR|=ADS1118_CS)
#define      SET_ADS1118_CS     (ADS1118_Port_OUT|=ADS1118_CS)
#define      CLR_ADS1118_CS     (ADS1118_Port_OUT&=~ADS1118_CS)

#define      ADS1118_CLK_OUT    (ADS1118_Port_DIR|=ADS1118_CLK)
#define      SET_ADS1118_CLK    (ADS1118_Port_OUT|=ADS1118_CLK)
#define      CLR_ADS1118_CLK    (ADS1118_Port_OUT&=~ADS1118_CLK)

#define      ADS1118_OUT_IN     (ADS1118_Port_DIR&=~ADS1118_OUT)
#define      ADS1118_OUT_Val    (ADS1118_Port_IN&ADS1118_OUT)

#define      ADS1118_IN_OUT     (ADS1118_Port_DIR|=ADS1118_IN)
#define      SET_ADS1118_IN     (ADS1118_Port_OUT|=ADS1118_IN)
#define      CLR_ADS1118_IN     (ADS1118_Port_OUT&=~ADS1118_IN)


#define      SS          BITF      //    x    Unused in Continuous conversion mode(Always reads back as 0)
#define      MUX2        BITE      //    1
#define      MUX1        BITD      //    1
#define      MUX0        BITC      //    1    111 = AINP is AIN3 and AINN is GND
#define      PGA2        BITB      //    0
#define      PGA1        BITA      //    0
#define      PGA0        BIT9      //    1    001 = FS is ±4.096 V
#define      MODE        BIT8      //    0    0 = Continuous conversion mode

#define      FS          2.048

#define      DR2         BIT7      //    1
#define      DR1         BIT6      //    0
#define      DR0         BIT5      //    0    100 = 128 SPS (default)
#define      TS_MODE     BIT4      //    0    0 = ADC mode (default)        (1 = Temperature sensor mode)
#define      PULL_UP_EN  BIT3      //    1    1 = Pull-up resistor enabled on DOUT/DRDY pin (default)
#define      NOP1        BIT2      //    0
#define      NOP0        BIT1      //    1    01 = Valid data, update the Config register (default)
#define      NOT_USED    BIT0      //    x    Always reads '1'

#define      Control_Regist_MSB  (MUX2+MUX1+MUX0+PGA0)
#define      Control_Regist_LSB  (DR2+PULL_UP_EN+NOP0+NOT_USED)
/**
 * main.c
 */
float ADS1118_Voltage, ADS1118_Current;
unsigned char Config_M = 0x54, Config_L = 0x8B;
unsigned int Config_Result_M,Config_Result_L;
unsigned int mode = 0, cnt = 0;

void ADS1118_init(void)
{
	ADS1118_CS_OUT;
	ADS1118_CLK_OUT;
	ADS1118_IN_OUT;
	ADS1118_OUT_IN;
	CLR_ADS1118_CS;
	_NOP();
	CLR_ADS1118_CLK;
	_NOP();
	CLR_ADS1118_IN;
	_NOP();
}

unsigned char ADS1118_Read(unsigned char data)   //SPI为全双工通信方式
{
	unsigned char i,temp,Din;
	temp=data;
	for(i=0;i<8;i++)
	{
		Din = Din<<1;
		SET_ADS1118_CLK;
		__delay_cycles(200);
		if(ADS1118_OUT_Val)
			Din |= 0x01;
		if(0x80&temp)
			SET_ADS1118_IN;
		else
			CLR_ADS1118_IN;
		CLR_ADS1118_CLK;
		__delay_cycles(200);
		temp = (temp<<1);
	}
	return Din;
}

//void ADS1118_Get_Voltage(void)
//{
//	//CLR_ADS1118_CS;
//	unsigned int i=0;
//	unsigned char Data_REG_H,Data_REG_L;
//	unsigned int Data_REG;
//	while((ADS1118_OUT_Val)&&(i<1000)) i++;
//	Data_REG_H=ADS1118_Read(Config_M);
//	__delay_cycles(100);
//	Data_REG_L=ADS1118_Read(Config_L);
//	Data_REG=(Data_REG_H<<8)+Data_REG_L;
//	Config_Result_M = ADS1118_Read(Config_M);
//	__delay_cycles(100);
//	Config_Result_L = ADS1118_Read(Config_L);
//	CLR_ADS1118_IN;
//	_NOP();
//	if(Data_REG>=0x8000)
//	{
//		Data_REG=0xFFFF-Data_REG;//把0xFFFF改成0x10000
//		ADS1118_Voltage=(-1.0)*((Data_REG*FS/0x8000));
//	}
//	else
//		ADS1118_Voltage=(1.0)*((Data_REG*FS/32768));
//	cnt++;
//	mode = (cnt==10)?1:0;
//	cnt %= 10;
//}ADS1118_Current

void ADS1118_Get_UI(void)
{
	//CLR_ADS1118_CS;
	unsigned int i=0;
	unsigned char Data_REG_H,Data_REG_L;
	unsigned int Data_REG;
	while((ADS1118_OUT_Val)&&(i<1000)) i++;
	Data_REG_H=ADS1118_Read(Config_M);
	__delay_cycles(100);
	Data_REG_L=ADS1118_Read(Config_L);
	Data_REG=(Data_REG_H<<8)+Data_REG_L;
	Config_Result_M = ADS1118_Read(Config_M);
	__delay_cycles(100);
	Config_Result_L = ADS1118_Read(Config_L);
	CLR_ADS1118_IN;
	_NOP();
	if(Data_REG>=0x8000)
	{
		Data_REG=0xFFFF-Data_REG;//把0xFFFF改成0x10000
		if(!mode)
			ADS1118_Voltage=(-1.0)*((Data_REG*FS/0x8000));
		else
			ADS1118_Current=(-1.0)*((Data_REG*FS/0x8000));
	}
	else
	{
		if(!mode)
			ADS1118_Voltage=(1.0)*((Data_REG*FS/32768));
		else
			ADS1118_Current=(1.0)*((Data_REG*FS/32768));
	}
}

void Timer_Init()
{
	TA0CCTL0 = CCIE;                          // CCR0 interrupt enabled
	TA0CCR0 = 5000;
	TA0CTL = TASSEL_2 + MC_1 + TACLR;         // SMCLK, upmode, clear TAR
}

void GPIO_Init()
{
	P2DIR |= BIT7;
	P2OUT &=~ BIT7;
	P4REN |= BIT0 + BIT1 + BIT2 + BIT3;
	P4OUT |= BIT0 + BIT1 + BIT2 + BIT3;
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	Timer_Init();
	GPIO_Init();
	ADS1118_init();
	OLED_Init();
	OLED_Clear();
	ADS1118_Read(Config_M);
	ADS1118_Read(Config_L);
	__delay_cycles(100);
	SET_ADS1118_CS;
	__delay_cycles(10000);
	__bis_SR_register(GIE);
}
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
	int mini = 0;
	CLR_ADS1118_CS;

	cnt++;
	mode = (cnt==10)?1:0;
	cnt %= 10;
	if(mode)
		Config_M = 0x44;
	else
		Config_M = 0x54;

	__delay_cycles(6800);
	ADS1118_Get_UI();
	__delay_cycles(800);
	//SET_ADS1118_CS;
	__delay_cycles(6400);
 	mini = (int)(ADS1118_Voltage*1000)%1000;
	OLED_ShowString(29, 6, "Vo=");
	OLED_ShowNum(53, 6, ((int)(ADS1118_Voltage*1000))/1000, 1, 16);
	OLED_ShowChar(53+8, 6,'.');
	if(mini<10)
	{
		OLED_ShowNum(53+16, 6, 0, 1, 16);
		OLED_ShowNum(53+24, 6, 0, 1, 16);
		OLED_ShowNum(53+32, 6, mini, 1, 16);
	}
	else if(mini>9&&mini<100)
	{
		OLED_ShowNum(53+16, 6, 0, 1, 16);
		OLED_ShowNum(53+24, 6, mini, 2, 16);
	}
	else
		OLED_ShowNum(53+16, 6, mini, 3, 16);
	OLED_ShowChar(53+40, 6,'V');
}

