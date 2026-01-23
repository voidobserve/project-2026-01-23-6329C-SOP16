/*********************************************************************************************
    *   Filename        : led_music.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  :

    *
*********************************************************************************************/
#include "system/includes.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "update.h"
#include "update_loader_download.h"
#include "app_charge.h"
#include "app_power_manage.h"
#include "system/event.h"
#include "asm/mcpwm.h"
#include "le_trans_data.h"
#include "btstack/bluetooth.h"
#include "led_music.h"


typedef struct _MUSIC_STATE{
    u8 music_pwm_level;
    u8 rgb_mode;
}MUSIC_STATE;

MUSIC_STATE music_state;
extern LED_STATE led_state;
/****************************************************************************************
**名称:
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void ble_user_06_proc(u8 *buf)
{
	if(buf[1]==0x01)
	{
		if(buf[2]<=30)
		{
			music_state.music_pwm_level = 0;
		}
		else
		{
			music_state.music_pwm_level = buf[2]-30;
		}
		music_state.rgb_mode = 31;
//		printf("buf = %d, per = %d\n",buf[2],music_state.music_pwm_percent);
		//music_led_func();
	}
/*	else if(buf[1]==0x02)
	{
		user_time.hour = buf[2];
		user_time.min= buf[3];
		user_time.sec = buf[4];
		user_time.wday = buf[5];

		log_info("######### time update ##########\n");
		app_led_on();
		user_time.rtc_is_update = 1;
	}
*/
	else if(buf[1]==0x03)
	{
		if(buf[2]<=30)
		{
			music_state.music_pwm_level = 0;
		}
		else
		{
			music_state.music_pwm_level = buf[2]-30;
		}
		music_state.rgb_mode = 32;
	}
/*	else if(buf[1]==0x04)
	{
		music_state.rgb_mode = 33;
		rgb_static_color_set(buf[2],buf[3],buf[4],buf[5]);
	}
*/
}

const u16 energy_qtz_tbl[51] =
{
	100,
	100,110,120,130,140,150,160,170,180,190,
	200,210,220,230,240,250,260,270,280,290,
	300,310,320,330,340,350,360,370,380,390,
	400,410,420,430,440,450,460,470,480,490,
	500,550,600,650,700,750,800,850,900,1000,
};

/****************************************************************************************
**名称:
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void music_led_func(void)
{
	static u8 last_level,disp_level,low_value,high_value,disp_pwm,last_pwm,rgb_mode = 0;
	static bool falling_flag,rising_flag,change_color_flag;

	if(last_level!=music_state.music_pwm_level)
	{
#if 1
		if(last_level>music_state.music_pwm_level)
		{
			if(rising_flag)
			{
				rising_flag = 0;
				high_value = last_level;
				change_color_flag = 1;
			}
			falling_flag = 1;
		}
		else if(last_level<music_state.music_pwm_level)
		{
			if(falling_flag)
			{
				falling_flag = 0;
				low_value = last_level;
			}
			rising_flag = 1;
		}

		if(change_color_flag)
		{
			change_color_flag = 0;
#if 0
			if(music_state.rgb_mode==32)
			{
				if(high_value>(low_value+10))
				{
					rgb_mode++;
					if(rgb_mode>=7)
					{
						rgb_mode = 0;
					}
				}
			}
			else
#endif
			{
				if(high_value>(low_value+5))
				{
					rgb_mode++;
					if(rgb_mode>=7)
					{
						rgb_mode = 0;
					}
				}
			}
			printf("rgb_mode = %d\n",rgb_mode);
		}
#endif
		last_level = music_state.music_pwm_level;
		last_pwm = energy_qtz_tbl[last_level];
	}

	if(disp_pwm==last_pwm)
		return;

	if(disp_pwm>last_pwm)
		disp_pwm-=5;
	else
		disp_pwm = last_pwm;

	rgb_mode_set(rgb_mode,disp_pwm*10);
}
void rgb_mode_set(u8 rgb_mode,u32 disp_pwm)
{
    printf("disp_pwm = %d\n",disp_pwm);

    switch(rgb_mode)
        {
            case 1:     led_state.R_flag = 0xff, led_state.G_flag = 0;     led_state.B_flag = 0;       break;            //红
            case 2:     led_state.R_flag = 0,    led_state.G_flag = 0xff;  led_state.B_flag = 0;       break;            //蓝
            case 3:     led_state.R_flag = 0,    led_state.G_flag = 0;     led_state.B_flag = 0xff;    break;            //绿
            case 4:     led_state.R_flag = 0xff, led_state.G_flag = 0xff;  led_state.B_flag = 0;       break;            //黄
            case 5:     led_state.R_flag = 0,    led_state.G_flag = 0xff;  led_state.B_flag = 0xff;    break;            //青
            case 6:     led_state.R_flag = 0xff, led_state.G_flag = 0;     led_state.B_flag = 0xff;    break;            //紫
            case 7:     led_state.R_flag = 0xff, led_state.G_flag = 0xff;  led_state.B_flag = 0xff;    break;            //白
        }
        led_state.ledlight = disp_pwm;
}
