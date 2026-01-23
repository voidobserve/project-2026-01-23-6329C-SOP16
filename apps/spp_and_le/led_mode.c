/*********************************************************************************************
    *   Filename        : app_main.c

    *   Description     :

    *   Author          : Bingquan

    *   Email           : caibingquan@zh-jieli.com

    *   Last modifiled  : 2019-05-11 14:54

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
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
#include "local_music.h"


extern LED_STATE led_state;
extern u8 rgb_func_isr_en;
extern u8 ir_detect_isr_en;

/****************************************************************************************
**名称:单色渐变
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void colour_gradual_change(void)
{
//    u8 ledlight;
    u8 dark_or_bright=0;
    u8 R_flag,G_flag,B_flag,light;
    u32 redled_duty,blueled_duty,greenled_duty;

    light = led_state.ledlight;
//    led_state.speed = 1;
    while(led_state.dynamic_state_flag >= RED_GRADUAL_CHANGE && led_state.dynamic_state_flag <= WHITE_GRADUAL_CHANGE && led_state.running_task == DYNAMIC_TASK)
    {
  
        switch(led_state.dynamic_state_flag)
        {
            case RED_GRADUAL_CHANGE:    R_flag = 0xff, G_flag = 0;     B_flag = 0;       break;            //红
            case GREEN_GRADUAL_CHANGE:  R_flag = 0,    G_flag = 0xff;  B_flag = 0;       break;            //蓝
            case BLUE_GRADUAL_CHANGE:   R_flag = 0,    G_flag = 0;     B_flag = 0xff;    break;            //绿
            case YELLOW_GRADUAL_CHANGE: R_flag = 0xff, G_flag = 0xff;  B_flag = 0;       break;            //黄
            case CYAN_GRADUAL_CHANGE:   R_flag = 0,    G_flag = 0xff;  B_flag = 0xff;    break;            //青
            case PURPLE_GRADUAL_CHANGE: R_flag = 0xff, G_flag = 0;     B_flag = 0xff;    break;            //紫
            case WHITE_GRADUAL_CHANGE:  R_flag = 0xff, G_flag = 0xff;  B_flag = 0xff;    break;            //白
        }

        redled_duty = R_flag*light*100/255;
        blueled_duty = G_flag*light*100/255;
        greenled_duty = B_flag*light*100/255;
        Set_Duty(redled_duty,blueled_duty,greenled_duty);

        if(dark_or_bright == 0)
        {
            light++;
            if(light>led_state.ledlight)
                dark_or_bright = 1;
        }
        if(dark_or_bright == 1)
        {
            light--;
            if(light<5)
                dark_or_bright = 0;
        }
        os_time_dly(9-led_state.speed);
    }
//    led_state.ledlight = ledlight;
}
#define OFF_VAL  10
/****************************************************************************************
**名称:三色渐变
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void three_colour_gradual_change(void)
{
    u8 colour_step;
    u8 R_flag,G_flag,B_flag,light;
    u32 redled_duty,blueled_duty,greenled_duty;

    R_flag = OFF_VAL;
    G_flag = 0;
    B_flag = 0;

    colour_step = 1;
    light = led_state.ledlight;
    while(led_state.dynamic_state_flag == THREE_GRADUAL_CHANGE && led_state.running_task == DYNAMIC_TASK)
    {
        switch(colour_step)
        {
            case 1: R_flag +=3;
                    if(R_flag >= 250)
                        colour_step = 2;
                    break;
            case 2: R_flag -=3;
                    if(R_flag <= OFF_VAL)
                    {
                        R_flag = 0;
                        G_flag = OFF_VAL;
                        colour_step = 3;
                        light = led_state.ledlight;
                    }
                    break;
            case 3: G_flag +=3;
                    if(G_flag >= 250)
                        colour_step = 4;
                    break;
            case 4: G_flag -=3;
                    if(G_flag <= OFF_VAL)
                    {
                        colour_step = 5;
                        G_flag = 0;
                        B_flag = OFF_VAL;
                        light = led_state.ledlight;
                    }
                    break;
            case 5: B_flag +=3;
                    if(B_flag >= 250)
                        colour_step = 6;
                    break;
            case 6: B_flag -=3;
                    if(B_flag <= OFF_VAL)
                    {
                        colour_step = 1;
                        B_flag = 0;
                        R_flag = OFF_VAL;
                        light = led_state.ledlight;
                    }
                    break;
        }

        redled_duty = R_flag*light*100/255;
        blueled_duty = G_flag*light*100/255;
        greenled_duty = B_flag*light*100/255;
        Set_Duty(redled_duty,blueled_duty,greenled_duty);
        os_time_dly(9-led_state.speed);
    }
}

/****************************************************************************************
**名称:七色渐变
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void seven_colour_gradual_change(void)
{
    u8 colour_step;
    int R_flag,G_flag,B_flag;
    u32 redled_duty,blueled_duty,greenled_duty;
    u8 change_step;
    R_flag = 0xff;
    G_flag = 0;
    B_flag = 0;

    colour_step = 1;
    while(led_state.dynamic_state_flag == SEVEN_GRADUAL_CHANGE && led_state.running_task == DYNAMIC_TASK)
    {
        if(dream_mic.mute_state == MIC_TRIGGER)
        {
            change_step = 10;
        }
        else
        {
            change_step = 3;
        }
        switch(colour_step)
        {
            case 1: G_flag += change_step;
                    if(G_flag >= 0xfe)
                        colour_step = 2;
                    break;
            case 2: R_flag -= change_step;
                    if(R_flag <= 3)
                        colour_step = 3;
                    break;
            case 3: B_flag += change_step;
                    if(B_flag >= 0xfe)
                            colour_step = 4;
                    break;
            case 4: G_flag -= change_step;
                    if(G_flag <= 3)
                        colour_step = 5;
                    break;
            case 5: R_flag += change_step;
                    if(R_flag >= 0xfe)
                        colour_step = 6;
                    break;
            case 6: B_flag -= change_step;
                    if(B_flag <= 3)
                        colour_step = 1;
                    break;
        }
        redled_duty = R_flag*led_state.ledlight*100/255;
        blueled_duty = G_flag*led_state.ledlight*100/255;
        greenled_duty = B_flag*led_state.ledlight*100/255;
        Set_Duty(redled_duty,blueled_duty,greenled_duty);
        os_time_dly(9-led_state.speed);
    }
}
/****************************************************************************************
**名称:红绿渐变
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void red_green_gradual_change(void)
{
    u8 colour_step;
    u8 R_flag,G_flag,B_flag;
    u32 redled_duty,blueled_duty,greenled_duty;

    R_flag = 0xff;
    G_flag = 0;
    B_flag = 0;
    colour_step = 1;
    while(led_state.dynamic_state_flag == RED_GREEN_GRADUAL_CHANGE && led_state.running_task == DYNAMIC_TASK)
    {
        switch(colour_step)
        {
            case 1: G_flag += 3;
                    if(G_flag >= 0xfe)
                        colour_step = 2;
                    break;
            case 2: R_flag -= 3;
                    if(R_flag <= 3)
                        colour_step = 3;
                    break;
            case 3: R_flag += 3;
                    if(R_flag >= 0xfe)
                        colour_step = 4;
                    break;
            case 4: G_flag -= 3;
                    if(G_flag <= 3)
                        colour_step = 1;
                    break;
        }
        redled_duty = R_flag*led_state.ledlight*100/255;
        blueled_duty = G_flag*led_state.ledlight*100/255;
        greenled_duty = B_flag*led_state.ledlight*100/255;
        Set_Duty(redled_duty,blueled_duty,greenled_duty);
        os_time_dly(9-led_state.speed);
    }
}
/****************************************************************************************
**名称:红蓝渐变
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void red_blue_gradual_change(void)
{
    u8 colour_step;
    u8 R_flag,G_flag,B_flag;
    u32 redled_duty,blueled_duty,greenled_duty;

    R_flag = 0xff;
    G_flag = 0;
    B_flag = 0;
    colour_step = 1;
    while(led_state.dynamic_state_flag == RED_BLUE_GRADUAL_CHANGE && led_state.running_task == DYNAMIC_TASK)
    {
        switch(colour_step)
        {
            case 1: B_flag += 3;
                    if(B_flag >= 0xfe)
                        colour_step = 2;
                    break;
            case 2: R_flag -= 3;
                    if(R_flag <= 3)
                        colour_step = 3;
                    break;
            case 3: R_flag += 3;
                    if(R_flag >= 0xfe)
                        colour_step = 4;
                    break;
            case 4: B_flag -= 3;
                    if(B_flag <= 3)
                        colour_step = 1;
                    break;
        }
        redled_duty = R_flag*led_state.ledlight*100/255;
        blueled_duty = G_flag*led_state.ledlight*100/255;
        greenled_duty = B_flag*led_state.ledlight*100/255;
        Set_Duty(redled_duty,blueled_duty,greenled_duty);
        os_time_dly(9-led_state.speed);
    }
}
/****************************************************************************************
**名称:蓝绿渐变
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void blue_green_gradual_change(void)
{
    u8 colour_step;
    u8 R_flag,G_flag,B_flag;
    u32 redled_duty,blueled_duty,greenled_duty;

    R_flag = 0;
    G_flag = 0xff;
    B_flag = 0;

    colour_step = 1;
    while(led_state.dynamic_state_flag == GREEN_BLUE_GRADUAL_CHANGE && led_state.running_task == DYNAMIC_TASK)
    {
        switch(colour_step)
        {
            case 1: B_flag += 3;
                    if(B_flag >= 0xfe)
                        colour_step = 2;
                    break;
            case 2: G_flag -= 3;
                    if(G_flag <= 3)
                        colour_step = 3;
                    break;
            case 3: G_flag += 3;
                    if(G_flag >= 0xfe)
                        colour_step = 4;
                    break;
            case 4: B_flag -= 3;
                    if(B_flag <= 3)
                        colour_step = 1;
                    break;
        }
        redled_duty = R_flag*led_state.ledlight*100/255;
        blueled_duty = G_flag*led_state.ledlight*100/255;
        greenled_duty = B_flag*led_state.ledlight*100/255;
        Set_Duty(redled_duty,blueled_duty,greenled_duty);
        os_time_dly(9-led_state.speed);
    }
}
/****************************************************************************************
**名称:三色跳变，七色跳变
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void three_seven_colour_gradual_change(void)
{
    u8 colour_step;
    u8 R_flag,G_flag,B_flag;
    u32 redled_duty,blueled_duty,greenled_duty;

    colour_step = 1;
    while((led_state.dynamic_state_flag == SEVEN_JUMP_CHANGE || led_state.dynamic_state_flag == THREE_JUMP_CHANGE) && led_state.running_task == DYNAMIC_TASK)
    {
        switch(colour_step)
        {
            case 1:     R_flag = 0xff, G_flag = 0;     B_flag = 0;       break;            //红
            case 2:     R_flag = 0,    G_flag = 0xff;  B_flag = 0;       break;            //蓝
            case 3:     R_flag = 0,    G_flag = 0;     B_flag = 0xff;    break;            //绿
            case 4:     R_flag = 0xff, G_flag = 0xff;  B_flag = 0;       break;            //黄
            case 5:     R_flag = 0,    G_flag = 0xff;  B_flag = 0xff;    break;            //青
            case 6:     R_flag = 0xff, G_flag = 0;     B_flag = 0xff;    break;            //紫
            case 7:     R_flag = 0xff, G_flag = 0xff;  B_flag = 0xff;    break;            //白
        }
        redled_duty = R_flag*led_state.ledlight*100/255;
        blueled_duty = G_flag*led_state.ledlight*100/255;
        greenled_duty = B_flag*led_state.ledlight*100/255;
        Set_Duty(redled_duty,blueled_duty,greenled_duty);
        colour_step++;
        if((colour_step>7) && (led_state.dynamic_state_flag == SEVEN_JUMP_CHANGE))
            colour_step = 1;
        if((colour_step>3) && (led_state.dynamic_state_flag == THREE_JUMP_CHANGE))
            colour_step = 1;
        os_time_dly((9-led_state.speed)*15);
    }
}
/****************************************************************************************
**名称:单色频闪
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void colour_flash_twinkle(void)
{
    u8 light;
    u8 R_flag,G_flag,B_flag;
    u32 redled_duty,blueled_duty,greenled_duty;

    light = led_state.ledlight;

    while(led_state.dynamic_state_flag >= RED_FAST_FLASH && led_state.dynamic_state_flag <= WHITE_FAST_FLASH && led_state.running_task == DYNAMIC_TASK)
    {
        switch(led_state.dynamic_state_flag)
        {
            case RED_FAST_FLASH:        R_flag = 0xff, G_flag = 0;     B_flag = 0;       break;            //红
            case GREEN_FAST_FLASH:      R_flag = 0,    G_flag = 0xff;  B_flag = 0;       break;            //蓝
            case BLUE_FAST_FLASH:       R_flag = 0,    G_flag = 0;     B_flag = 0xff;    break;            //绿
            case YELLOW_FAST_FLASH:     R_flag = 0xff, G_flag = 0xff;  B_flag = 0;       break;            //黄
            case CYAN_FAST_FLASH:       R_flag = 0,    G_flag = 0xff;  B_flag = 0xff;    break;            //青
            case PURPLE_FAST_FLASH:     R_flag = 0xff, G_flag = 0;     B_flag = 0xff;    break;            //紫
            case WHITE_FAST_FLASH:      R_flag = 0xff, G_flag = 0xff;  B_flag = 0xff;    break;            //白
        }
        redled_duty = R_flag*light*100/255;
        blueled_duty = G_flag*light*100/255;
        greenled_duty = B_flag*light*100/255;
        Set_Duty(redled_duty,blueled_duty,greenled_duty);
        if(light)
            light = 0;
//            rgb_func_isr_en = 0;
        else
            light = led_state.ledlight;
//            rgb_func_isr_en = 1;
        os_time_dly((9-led_state.speed)*10); //太快的话会一直亮
    }
//    rgb_func_isr_en = 1;
}
/****************************************************************************************
**名称:七色频闪
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void seven_colour_flash_twinkle(void)
{
    u8 light,colour_step;
    u8 R_flag,G_flag,B_flag;
    u32 redled_duty,blueled_duty,greenled_duty;

    light = led_state.ledlight;

    colour_step = 1;
    while(led_state.dynamic_state_flag == SEVEN_FAST_FLASH && led_state.running_task == DYNAMIC_TASK)
    {
 
        switch(colour_step)
        {
            case 1:      R_flag = 0xff, G_flag = 0;     B_flag = 0;       break;            //红
            case 2:      R_flag = 0,    G_flag = 0xff;  B_flag = 0;       break;            //蓝
            case 3:      R_flag = 0,    G_flag = 0;     B_flag = 0xff;    break;            //绿
            case 4:      R_flag = 0xff, G_flag = 0xff;  B_flag = 0;       break;            //黄
            case 5:      R_flag = 0,    G_flag = 0xff;  B_flag = 0xff;    break;            //青
            case 6:      R_flag = 0xff, G_flag = 0;     B_flag = 0xff;    break;            //紫
            case 7:      R_flag = 0xff, G_flag = 0xff;  B_flag = 0xff;    break;            //白
        }
        redled_duty = R_flag*light*100/255;
        blueled_duty = G_flag*light*100/255;
        greenled_duty = B_flag*light*100/255;
        Set_Duty(redled_duty,blueled_duty,greenled_duty);
        if(light)
            light=0;
//            rgb_func_isr_en = 0;
        else
        {
            light = led_state.ledlight;
//            rgb_func_isr_en = 1;
            colour_step++;
            if(colour_step>7)
            {
                colour_step = 1;
            }
        }
        os_time_dly((9-led_state.speed)*10);
    }
//    rgb_func_isr_en = 1;
}
/****************************************************************************************
**名称:七色跳变
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
/*
void three_seven_colour_gradual_change(void)
{
    u8 colour_step;
    u8 R_flag,G_flag,B_flag;

    R_flag = led_state.R_flag;
    G_flag = led_state.G_flag;
    B_flag = led_state.B_flag;

    colour_step = 1;
    while((led_state.dynamic_state_flag == SEVEN_JUMP_CHANGE || led_state.dynamic_state_flag == THREE_JUMP_CHANGE) && led_state.running_task == DYNAMIC_TASK)
    {
        switch(colour_step)
        {
            case 1:     led_state.R_flag = 0xff, led_state.G_flag = 0;     led_state.B_flag = 0;       break;            //红
            case 2:     led_state.R_flag = 0,    led_state.G_flag = 0xff;  led_state.B_flag = 0;       break;            //蓝
            case 3:     led_state.R_flag = 0,    led_state.G_flag = 0;     led_state.B_flag = 0xff;    break;            //绿
            case 4:     led_state.R_flag = 0xff, led_state.G_flag = 0xff;  led_state.B_flag = 0;       break;            //黄
            case 5:     led_state.R_flag = 0,    led_state.G_flag = 0xff;  led_state.B_flag = 0xff;    break;            //青
            case 6:     led_state.R_flag = 0xff, led_state.G_flag = 0;     led_state.B_flag = 0xff;    break;            //紫
            case 7:     led_state.R_flag = 0xff, led_state.G_flag = 0xff;  led_state.B_flag = 0xff;    break;            //白
        }
        colour_step++;
        if((colour_step>7) && (led_state.dynamic_state_flag == SEVEN_JUMP_CHANGE))
            colour_step = 1;
        if((colour_step>3) && (led_state.dynamic_state_flag == THREE_JUMP_CHANGE))
            colour_step = 1;
        os_time_dly(led_state.speed*20);
    }
    led_state.R_flag = R_flag;
    led_state.G_flag = G_flag;
    led_state.B_flag = B_flag;
}
*/

/****************************************************************************************
**名称:声控三色渐变
**功能:声控时，亮度突变，然后亮度慢慢回落
**说明:
@local_music_task_handle()
音量大时，led_state.speed和dream_mic.change_speed(全局变量，渐变步进值)变大

*****************************************************************************************/

void sound_ctrl_3_clr_gradual(void)
{
    u8 colour_step,light;
    u16 R_flag,G_flag,B_flag;
    u32 redled_duty,blueled_duty,greenled_duty;

    R_flag = OFF_VAL;
    G_flag = 0;
    B_flag = 0;

    colour_step = 1;
    light = led_state.ledlight;
    #ifdef MY_DEBUG
    printf("\SOUND_CTRL_THREE_GRADUAL--------------->");
    #endif   
    while(led_state.dynamic_state_flag == SOUND_CTRL_THREE_GRADUAL && led_state.running_task == DYNAMIC_TASK)
    {
        if(dream_mic.mute_state == MIC_NO_TRIGGER)
        {
            switch(colour_step)
            {
                case 1: R_flag +=3;
                        if(R_flag >= 250)
                            colour_step = 2;
                        break;
                case 2: R_flag -=3;
                        if(R_flag <= OFF_VAL)
                        {
                            R_flag = 0;
                            G_flag = OFF_VAL;
                            colour_step = 3;
                            light = led_state.ledlight;
                        }
                        break;
                case 3: G_flag +=3;
                        if(G_flag >= 250)
                            colour_step = 4;
                        break;
                case 4: G_flag -=3;
                        if(G_flag <= OFF_VAL)
                        {
                            colour_step = 5;
                            G_flag = 0;
                            B_flag = OFF_VAL;
                            light = led_state.ledlight;
                        }
                        break;
                case 5: B_flag +=3;
                        if(B_flag >= 250)
                            colour_step = 6;
                        break;
                case 6: B_flag -=3;
                        if(B_flag <= OFF_VAL)
                        {
                            colour_step = 1;
                            B_flag = 0;
                            R_flag = OFF_VAL;
                            light = led_state.ledlight;
                        }
                        break;
            }
        }
        else
        {
            R_flag += 20;
            G_flag += 20;  
            B_flag += 20;
            if(R_flag > 255) R_flag = 255;
            if(G_flag > 255) G_flag = 255;
            if(B_flag > 255) B_flag = 255;
        }
       

        redled_duty = R_flag*light*100/255;
        blueled_duty = G_flag*light*100/255;
        greenled_duty = B_flag*light*100/255;
        Set_Duty(redled_duty,blueled_duty,greenled_duty);
        os_time_dly(9-led_state.speed);
    }
}

