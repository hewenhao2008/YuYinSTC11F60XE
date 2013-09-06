#include "STC11F60XE.h"
#include "intrins.h"
#include "ds18b20.h"
#include <string.h>

sbit JS = P2 ^ 4;//������ն˿�!
sbit Y  = P2 ^ 5; //���ⷢ��˿� 	 
sbit WF = P0 ^ 4; //���߷���˿� 

sbit WIFI_LED = P3 ^ 6; //wifi��λLEDָʾ��
sbit WAKEUP_LED = P0 ^ 3; //����״ָ̬ʾ��
sbit RST = P1 ^ 4; //wifi��λRST 

#define LED_ON 1
#define LED_OFF !(LED_ON)

bit F = 0;	  //�Ƿ��38KH��������
bit Wifi_Command_Mode = 0; //=1 wifi����������ģʽ =0 ���������ݴ���ģʽ
bit Check_wifi = 1;
bit Wifi_AP_OPEN_MODE = 0;
unsigned int RST_count1 = 0; //����
unsigned int RST_count2 = 0;
unsigned char Temperature = 0; //�¶�
unsigned int T = 0;	//����


unsigned int i = 0;//������ 
unsigned int j = 0;//������
unsigned int c = 0;//������

unsigned int ui = 0;//���ڽ������ݳ���!
xdata unsigned char US[800];//xdata unsigned char US[256]; //���崮�ڽ������ݱ���!

/*
void Delay10us()		//@22.1184MHz
{
	unsigned char i;

	_nop_();
	i = 52;
	while (--i);
}
*/

void U1_in()//����1��������
{
	j = 0; //��ʱ�˳�!
	ui = 0;
	while(j < 40000)//��ʱ�˳�(��Լ1ms)!��Ҫ���Դ�ֵ�Ƿ���ȷ! 5000
	{
		if(RI == 1)
		{
			US[ui] = SBUF;
			if(US[ui] == '<' && US[ui - 1] == '<')
				break;
			RI = 0;
			ui++;
			j = 0;			
		}
		else
			j++;
		//Delay10us();//��ʱʱ����Ҫ���Դ�ֵ�Ƿ���ȷ!(�˴�Ҫ����ʱ,Ҫ�����ݽ��ղ���ȷ!)
	}	
	RI = 0;	
}

void U1_send(unsigned char i)//����1���͵��ֽ�����
{
	TI = 0;			//������жϱ�־λΪ0��������㣩
	SBUF = i;	//�������� SBUF Ϊ��Ƭ���Ľ��շ��ͻ���Ĵ���
	while(TI==0);
	TI = 0;			//������жϱ�־λΪ0��������㣩
}

void U1_sendS(unsigned char s[], unsigned int m)//����1�����ַ�������,U1_sendS���������"<<"������־!
{
	unsigned int n = 0;
	for(n = 0;n < m;n++)
		U1_send(s[n]);
}


void T0Init(void)		//13΢��@22.1184MHz
{
	AUXR &= 0x7F;		//��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TMOD |= 0x02;		//���ö�ʱ��ģʽ
	TL0 = 0xE8;		//���ö�ʱ��ֵ
	TH0 = 0xE8;		//���ö�ʱ����ֵ
	TF0 = 0;		//���TF0��־
	TR0 = 0;		//��ʱ��0��ʼ��ʱ
	
	ET0 = 1;
	EA = 1;
}

void T0_C1 (void) interrupt 1  using 2 //��Ƭ�����жϺ�1��Ӧ���ж�:��ʱ���ж�0
{		 
	T++;
	if(F == 1)
	    Y = ~Y;
}

typedef union //char������תint�������� 
{  
	unsigned short int ue; 
	unsigned char 	 u[2]; 
}U16U8;
U16U8 idata M;//����8λת16λ

void U1Init(void)		//115200bps@22.1184MHz
{
	PCON |= 0x80;		//ʹ�ܲ����ʱ���λSMOD
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x04;		//���������ʷ�����ʱ��ΪFosc,��1T
	BRT = 0xF4;		//�趨���������ʷ�������װֵ
	AUXR |= 0x01;		//����1ѡ����������ʷ�����Ϊ�����ʷ�����
	AUXR |= 0x10;		//�������������ʷ�����
}

void Rstinit()
{
	//����Ϊ������
	P1M1 |= (1<<4);
	P1M0 &= ~(1<<4);
}

/*--------------
---wifi mode----
--------------*/
void Delay10ms()		//@22.1184MHz
{
	unsigned char i, j, k;

	i = 1;
	j = 216;
	k = 35;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

void Delay100ms()		//@22.1184MHz
{
	unsigned char i, j, k;

	i = 9;
	j = 104;
	k = 139;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

void wifi_ap_open_led_blink()
{
	//����˸����
	WIFI_LED = !WIFI_LED;
	Delay100ms();
	Delay100ms();
	WIFI_LED = !WIFI_LED;	
	Delay100ms();
	Delay100ms();	
}

int start_wifi_command()
{
	U1_sendS("+++",3);
	memset(US,0x00,sizeof(US));	
	U1_in();
	if(US[0] == 'a')
	{	
		memset(US,0x00,sizeof(US));
		//Delay50ms();
		U1_send('a');
		U1_in();			
		if(strstr(US,"+ok") != NULL)
		{
			Wifi_Command_Mode = 1;
			memset(US,0x00,sizeof(US));
			return 0; //�л��ɹ�
		}	
	}
	memset(US,0x00,sizeof(US));
	return 1;
}

int start_wifi_data()
{
	U1_sendS("AT+ENTM\r\n",9);
	U1_in();
	if(strstr(US,"+ok") != NULL)
	{		
		Wifi_Command_Mode = 0;
		memset(US,0x00,sizeof(US));
		return 0; //�л��ɹ�
	}
	memset(US,0x00,sizeof(US));
	return 1;	
}

void main (void)
{
	WF = 0;
	WIFI_LED =LED_ON;// LED_ON;
	WAKEUP_LED = LED_OFF;
	U1Init();
	T0Init();	
	Rstinit();
	Init_DS18B20();	
	//CH:<< 			����ɼ�����		//CH:����+����<<	//�ɼ��󷵻ص�����
	//FH:����+����<<	���ⷢ������
	//FW:����+����<<  	���߷�������
	//FS:<<				����
	//���紫������byte��ʽ������
	//while(1);
	while(1)
	{
		if(Check_wifi)
		{
			if(!Wifi_Command_Mode)
			{
				start_wifi_command();
			}
			if(Wifi_Command_Mode)
			{
				//Delay50ms();
				Delay10ms();
				U1_sendS("AT+WMODE\r\n",10);
				Check_wifi = 0;	
			}
		}	
		WIFI_LED = RST;	
		if(RST == 0)
		{
			while(RST == 0)
			{
				RST_count1++;
				if(RST_count1 == 65535)
				{
					RST_count1 = 0;
					RST_count2++;
				}
			}
			if(RST_count2 >= 5)
			{
				Wifi_Command_Mode = 0;
				Check_wifi = 1;
				RST_count1 = 0;
				RST_count2 = 0;
			}	
		}	
		if(Wifi_AP_OPEN_MODE == 1)
		{
			wifi_ap_open_led_blink();
		}
		if(RI==1)
		{
			U1_in();//��ȡ���ڷ��͵�SJ����!

			if(US[2] == ':')//���յ���ȷ�Ŀ�������!
			{
				switch(US[0])
				{
					case 'F'://���⡢�������ݷ���!
						WIFI_LED = LED_OFF;
						if(US[1]=='H')//����
						{							
							i = 4;//��3��4λ�����ݳ���,�ӵ�4λ�Ǻ��⡢���߿�������
							M.u[0] = US[3];
							M.u[1] = US[4];
							j = M.ue;						
							TR0 = 1;		//������ʱ��0
							while(i < j)//j�����ݳ���-1!
							{
								T = 0;
								F = 1;
								i++;
								if(US[i] == 0)//&&US[i+1]==0)
								{
									i += 2;
									M.u[0] = US[i];
									i++;	
									M.u[1] = US[i];
								}
								else
								{
									M.u[0] = 0;	
									M.u[1] = US[i];
								}
								while(T < M.ue);

								T = 0;
								F = 0;
								Y = 1;
								i++;
								if(US[i] == 0)//&&uip_appdata[i+1]==0)
								{
									i += 2;
									M.u[0] = US[i];
									i++;	
									M.u[1] = US[i];
								}
								else
								{
									M.u[0] = 0;	
									M.u[1] = US[i];
								}
								while(T < M.ue);								
							}
							TR0 = 0;		//�رն�ʱ��0
							U1_sendS("FH<<", 4); 
							WIFI_LED = LED_ON;
						}
						else if(US[1]=='W')
						{						
							c = 0;
							TR0 = 1;		//������ʱ��0
							while(c < 6)//�ظ�����!
							{
								T = 0;
								WF = 1;
								i = 4;//��3��4λ�����ݳ���,�ӵ�5λ�Ǻ��⡢���߿�������
								while(T < 28);//(13 * 808 = 10504ͬ������!									
								T = 0;
								WF = 0;
								M.u[0] = US[3];
								M.u[1] = US[4];
								j = M.ue;//�������ɵĳ���Ҫ��1
								while(T < 808);//(13 * 808 = 10504ͬ������!

								while(i < j)
								{
									T = 0;
									WF = 1;
									i++;
									while(T < US[i]);

									T = 0;
									WF = 0;
									i++;//i�ڴ�,��׼һЩ
									while(T < US[i]);
								}
								c++;
							}
							TR0 = 0;
							WF = 0;		//�رն�ʱ��0
							U1_sendS("FW<<", 4);
						}
						else if(US[1]=='S')
						{
							U1_sendS("FS<<", 4);
						}

						break;

					case 'C'://����ɼ�!
				   	U1_sendS("CA<<", 4);//���ص������밴ң����("<<"��U1_sendS�����)
						i = 5;//��3��4λ�����ݳ���,�ӵ�4λ�Ǻ��⡢���߿�������
						j = 0;
						TR0 = 1;		//������ʱ��0
						while(i < 756) //���ȸ�����й�-->>US[2] = i;//����λ�����ݳ���
						{ 
							T = 1;   //Ӧ�������׼ȷ��
     	  			while(JS == 0);
   	  				if(T > 5)
							{
								M.ue = T;
	              T = 1;
								if(M.u[0] > 0)
								{
									US[i] = 0;	//�����յ����ݷ��ͻ�ȥ��ɾ��//����Ч��
									i++;
									US[i] = 0;	//�����յ����ݷ��ͻ�ȥ��ɾ��//����Ч��
									i++;
									US[i] = M.u[0];	//�����յ����ݷ��ͻ�ȥ��ɾ��//����Ч��
									i++;
								}
								US[i] = M.u[1];
								i++;	
								while(JS == 1)
								{								
									if(T > 6000)//�������˳�								
									{
										US[i] = 0;
										i++;

										M.ue = i;
										US[3] = M.u[0];//��3��4λ�����ݳ���(��������ͷ,��������β!)
										US[4] = M.u[1];//��3��4λ�����ݳ���(��������ͷ,��������β!)
									 		   
										US[i] = '<';
										i++;
										US[i] = '<';
										i++;

										US[0] = 'C';
										US[1] = 'H';
										US[2] = ':';																	
										U1_sendS(US, i);//����ɼ��ɹ�

										i = 756;
										break;
									}
								}
								if(i < 756)
								{				
									M.ue = T;
				
									if(M.u[0] > 0)
									{
										US[i] = 0;
										i++;
										US[i] = 0;
										i++;
										US[i] = M.u[0];
										i++;
									}
									US[i] = M.u[1];
									i++;
									j = 0;
								}
							}
							else
							{
								while(JS == 1)
								{
									if(T > 50000)
									{
										T = 0;
										j++;
										if(j > 30)
										{
											i = 756;
											U1_sendS("CC<<", 4);//��ʱ�˳�!��Լ20���޲����˳�!
											break;
										}
									}
								}
							}
						}
						TR0 = 0;		//�رն�ʱ��0
						break;
					case 'D':		//�¶�
							if(US[1] == 'T')
							{
								memset(US,0x00,sizeof(US));
								US[0] = 'D';
								US[1] = 'T';
								while((US[2] = GetTemperature()) == 0x55);
								US[3] = '<';
								US[4] = '<';
								U1_sendS(US, 5);
							}
							break;
					case 'L': //����״ָ̬ʾ��
							if(US[1] == 'B')
							{
								WAKEUP_LED = LED_ON;
								U1_sendS("LB<<",4);
							}	
							else if(US[1] == 'D')
							{
								WAKEUP_LED = LED_OFF;
								U1_sendS("LD<<",4);
							}
							break;
					case 'S': //wifi��λ
							if(US[1] == 'D')
							{
								Check_wifi = 1;
								Wifi_Command_Mode = 0;
								U1_sendS("SD<<",4);
							}
							break;
					default:break;	
				}
			}
			else if(strstr(US,"+o") != NULL) //�յ�wifiģ�鷵�ص����� +ok
			{
				if(strstr(US,"AP") != NULL) 	//wifi������APģʽ
				{
					Delay10ms();
					U1_sendS("AT+WAKEY\r\n",10);
				}
				else if(strstr(US,"OPEN") != NULL) //APģʽ�µ�open����  
				{
					Check_wifi = 0;
					Wifi_AP_OPEN_MODE = 1;
					if(start_wifi_data())
					{
						Check_wifi = 0;
						Wifi_Command_Mode = 0;
					}
					//wifi_ap_open_led_blink();
				}
				else
				{
					if(start_wifi_data())
					{
						Check_wifi = 0;
						Wifi_Command_Mode = 0;
						Wifi_AP_OPEN_MODE = 0;
					}
				}
			}
		}
		US[2] = 0x00;//һ����������ִ�����, ���
	}
}