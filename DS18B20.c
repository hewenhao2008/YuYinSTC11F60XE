#include "STC11F60XE.h"
#include "intrins.h"
#include "ds18b20.h"
sbit DQ = P0 ^ 2;//DS18B20数据引脚
extern void U1_send(unsigned char i);
void Delayus(int num)		//@22.1184MHz
{
	num*=4;
	while(num--);
}

void Delay15us()		//@22.1184MHz
{
	unsigned char i;

	i = 80;
	while (--i);
}

void Delay45us()		//@22.1184MHz
{
	unsigned char i, j;

	_nop_();
	i = 1;
	j = 244;
	do
	{
		while (--j);
	} while (--i);
}

void Init_DS18B20()
{
	DQ = 1;
	Delayus(10);
	DQ = 0;
	Delayus(510);
	DQ = 1;
	Delayus(30);
	if(DQ)
	{
		Delayus(30);
	}
	Delayus(510);
	DQ = 1;	
}

unsigned char ReadOneChar()
{
	unsigned char i,dat=0;
	for(i = 0;i < 8;i++)
	{
		dat >>= 1;	
		DQ = 1;
		_nop_();
		_nop_();		
		DQ = 0;
		_nop_();
		_nop_();
		_nop_();	
		DQ = 1;
		_nop_();
		_nop_();
		_nop_();		
		_nop_();
		_nop_();		
		if(DQ)
			dat |= 0x80;
		Delayus(35);
	}
	DQ = 1;
	Delayus(35);
	return dat;
}

void WriteOneChar(unsigned char dat)
{
	unsigned char i;
	for(i = 0;i < 8;i++)
	{
		DQ = 0;
		Delay15us();
		DQ = dat&0x01;
		dat >>= 1;
		Delay45us();
		DQ = 1;
	}	
	DQ = 1;
	Delayus(65);
}


int GetTemperature(void)
{
	unsigned char a=0;
	unsigned char b=0;
	int Temperature;
	int tmp = 0;

	Init_DS18B20();
	WriteOneChar(0xCC); // 跳过读序号列号的操作
	WriteOneChar(0x44); // 启动温度转换
	Delayus(510);
	Init_DS18B20();
	WriteOneChar(0xCC); //跳过读序号列号的操作 
	WriteOneChar(0xBE); //读取温度寄存器等（共可读9个寄存器） 前两个就是温度
	a=ReadOneChar();
	b=ReadOneChar();
	
	//U1_send(b);
	//U1_send(a);
	//U1_send(' ');
		
	tmp = b;
	tmp<<=8;
	tmp |= a;	//组合温度
	
	Temperature = (tmp&(~0xF800))*0.0625;	
	return (tmp&0xF800)?-Temperature:Temperature;
}


