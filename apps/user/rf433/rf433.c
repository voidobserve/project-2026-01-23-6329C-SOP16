#include "system/includes.h"
#include "rf433.h"

#if TCFG_RF433_ENABLE

u8 rf_key_get(void);

struct key_driver_para rf_scan_para = {
    .scan_time 	  	  = 50,				//按键扫描频率, 单位: ms
    .last_key 		  = NO_KEY,  		//上一次get_value按键值, 初始化为NO_KEY;
    .filter_time  	  = 0,				//按键消抖延时;
    .long_time 		  = 100,  			//按键判定长按数量
    .hold_time 		  = (75 + 15),  	//按键判定HOLD数量
    .click_delay_time = 2,//20,			//按键被抬起后等待连击延时数量
    .key_type		  = KEY_DRIVER_TYPE_RF433,
    .get_value 		  = rf_key_get,
};


#define RF433_PIN	IO_PORTA_07
void rf433_gpio_init(void)
{	
    gpio_set_die(RF433_PIN, 1);
	gpio_set_direction(RF433_PIN, 1);
	gpio_set_pull_up(RF433_PIN,0);
	gpio_set_pull_down(RF433_PIN,0);
}




sys_cb_t sys_cb;


#define RF1527_PLAY_AND_MODE     0x02
#define RF1527_PREV     0x01
#define RF1527_NEXT     0x08
#define RF1527_LOCK     0x04
#define RF1527_MODE     0x06
#define RF1527_PLAY_POWER   0x09

#define RF2260_PLAY_POWER     0x3c
#define RF2260_PREV     0x03
#define RF2260_NEXT     0x0c
#define RF2260_LOCK     0x30
#define RF2260_MODE    0xc0

#define RF1527_ONLY         0
#define RG433_LONG_T		3000 //3秒

u8 rf_key_val = NO_KEY;


u16 rf_long_cnt;

u8 rf_key_get(void)
{
    u8 key_val = NO_KEY;
    key_val = rf_key_val;
	
	
	return key_val;
}

 u16 T1;      /* 定时10ms++ */
#define LONG_PRESS_T    200    /* 定时1秒返回long press */
u8 long_flag = 0;
u16 T0;  

/* 长按计数器 */
void rf433_long_timer(void)
{
   	
	
	//printf("press_cnt = %d", press_cnt);
}




void timer125us_hook(void)
{
	static bool Data_Flag,bData_0_1; //数据标志位，数据0,1状态位
	static bool bit_end,bit_suceess,bit_Syn;
	static u8 Pluse_H_cnt,Pluse_L_cnt;
	//static u8 press_cnt;
	static u16 key_up_time = 0;
	static u16 key_tmp;
	static u8 rf_bit_cnt;
	//static u16 key_cool_down;
#if (!RF1527_ONLY)
	u8 sKeyCode_tmp,move_bit;
	u32 code_mask;
#endif
	u32 rf_inside_code;
	u8 rf_key_data;
	static u16 last_key_tmp = 0;
	static u8 wrong_cnt = 0;
	static u32 last_code = 0;
	u32 TmpDet = 0;
	u8  TmpCnt = 0;
	static u8 TmpKey = 0;




			/*					开始
							_____
							| 			|
							| 			|
							| 			| 	结束 上一个码也检波成功
			_______|			 |_________

			*/
	// 每个码是由高开始，下一个高之前结束。
	if(gpio_read(RF433_PIN)!=0)
	{
			if(bit_end)
			{
					bit_end = 0;
					bit_suceess = 1;
			}
			else
			{
					Pluse_H_cnt++;
					Pluse_L_cnt =0;
			}
	}
	else
	{
			bit_end = 1;
			Pluse_L_cnt++;
			if(bit_Syn==0){
					Pluse_H_cnt = 0;
			}
	}


	////////////////有个新波形来
	//低电平后在8ms到 15ms		//76
	// if(bit_suceess&&(Pluse_L_cnt>48)&&(Pluse_L_cnt<120))
	if(bit_suceess&&(Pluse_L_cnt>80)&&(Pluse_L_cnt<164))
	{
		// printf("\n reset");

		//同步码就是开始是8ms到15ms的低电平，后来就是码值
		bit_suceess = 0;
		bit_Syn =1;
		Pluse_H_cnt = 0;
		Pluse_L_cnt = 0;
		rf_bit_cnt = 0;
		Data_Flag = 0;
		sys_cb.RFData = 0;
	}

	///////////////////////////////译码//////////////////////
	if(bit_Syn)//同步码解码成功后执行
	{
		if(bit_suceess)
		{
				bit_suceess = 0;
				/*	    (5~13 )*125us														1
						    _____________
						   | 			|
						   | 			|  (1~4)*125US
						   | 			| 						|
					_______|			|_______ |

				*/		 //编码为1，高电平在
				if((Pluse_H_cnt>=6)&&(Pluse_H_cnt<=16)&&(Pluse_L_cnt<=4)&&(Pluse_L_cnt>=2))
				{
						bData_0_1 = 1;
						Data_Flag =1;
						// printf("h\n");
				}

				/*	(1~4)*125US 																			0
						_____
						| 			|
						| 			| 																 |
						| 			| 	(5~13 )*125us 				 |
				_______|			 |____________________|

				*/
				//编码为0，
				else if((Pluse_L_cnt>=6)&&(Pluse_L_cnt<=13)&&(Pluse_H_cnt<=4)&&(Pluse_H_cnt>=1))
				{
						bData_0_1 = 0;
						Data_Flag =1;
						// printf("l\n");
				}
				else
				{	
					// printf("\n Pluse_H_cnt=%d",Pluse_H_cnt);
					// printf("\n Pluse_L_cnt=%d",Pluse_L_cnt);

					//两者都不是
					bit_suceess = 0;
					Pluse_H_cnt = 0;
					Pluse_L_cnt = 0;
					rf_bit_cnt = 0;
					Data_Flag = 0;
					sys_cb.RFData = 0;
	


				}

				Pluse_H_cnt = 1;	/*现在解码是解上一个码的，
									译码标志是以现在这个码第一个高电平计数。，
									在上面没有叠加高位时间，现在补偿H_pluse，所以从1开始*/
				Pluse_L_cnt = 0;
		}
	}

	

	if(Data_Flag) //数据标志位有效
	{
		Data_Flag =0;
			
		//先到的数据低位数组BYTE ，
		//先发地址，再发数据共24bit ；3个数组
		sys_cb.RFData >>= 1;
		rf_bit_cnt++;
		// printf("\n rf_bit_cnt=%d",rf_bit_cnt);

		if(bData_0_1)
		{
			sys_cb.RFData |= 0x800000;
		}
			

		if(rf_bit_cnt == 24) //接收完20位内码 + 4位数据
		{
			bit_Syn=0;
			rf_bit_cnt = 0;
			// RF_BuffNum = 0;
			bit_suceess = 0;
			bit_end = 0;
			Pluse_H_cnt = 0;
			Pluse_L_cnt = 0;

			T0 = 0;
			T1++;
	
			/* 提取键值 */
			// 头部0xFF
			// printf("\n sys_cb.RFData=%d",sys_cb.RFData);
			sys_cb.RFAddrCode1 = sys_cb.RFData & 0xFFFF;
			//if( sys_cb.RFAddrCode1 == 0xFFFF)
		//	{
				rf_key_val = sys_cb.RFData >> 16;
				printf("\n rf_key_val=%X",rf_key_val);
		//	}
			sys_cb.RFData = 0;

		}
	}
	


}

#endif

