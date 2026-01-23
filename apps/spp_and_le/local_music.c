// 通过采样本地MIC，通过音量控制灯光效果
// @local_music.c

#include "local_music.h"
#include "system/includes.h"
#include "cpu.h"
#include "app_main.h"
/**************************************************移植修改**************************************************/
#define MIC_PIN     IO_PORTB_06

/**************************************************移植修改 - END********************************************/




#define SENSITIVE_MAX 1 //最灵敏
#define SENSITIVE_MIN 9 //不灵敏
DREAM_LIGHT_MIC dream_mic =
{
    .sensitive = 5,
};
OS_SEM local_music_task_sem;        //音乐律动-本地模式任务信号量

u32 Sound_val=0;                    //麦克风读取到的音量值，分10级，0最低音量，10最高音量


static void mic_gpio_init();
static void mic_braeth_mode(DREAM_LIGHT_MIC *dream_mic);
void read_sound_task_handle(void);

/**************************************************API**************************************************/
/* ---------------------------------灵敏度---------------- */
// 增加灵敏度,dream_mic->sensitive值越低越灵敏
void locla_mic_sen_plus(void)
{
    if(dream_mic.sensitive > SENSITIVE_MAX)
        dream_mic.sensitive--;
    #ifdef MY_DEBUG
    printf("dream_mic.sensitive = %d",dream_mic.sensitive);
    #endif  
}

// 减少灵敏度
void locla_mic_sen_sub(void)
{
    if(dream_mic.sensitive < SENSITIVE_MIN)
        dream_mic.sensitive++;
    #ifdef MY_DEBUG
    printf("dream_mic.sensitive = %d",dream_mic.sensitive);
    #endif  
}
// 取值范围：SENSITIVE_MAX - SENSITIVE_MIN
void set_local_mic_sen(u8 sensitive)
{
    if( SENSITIVE_MAX <= sensitive && sensitive <=SENSITIVE_MIN)
    {
        dream_mic.sensitive = sensitive;
    }
    else
    {
        dream_mic.sensitive = 1;
    }
}

// 返回：dream_mic.sensitive
u8 get_local_mic_sen(void)
{
    return dream_mic.sensitive;
}
// -------------------------------------------------音频模式
//设置本地音频模式
void set_local_mic_mode(local_music_mode_e mode)
{
    dream_mic.music_number = mode;
}

local_music_mode_e get_local_mic_mode(void)
{
    return dream_mic.music_number;
}


// -------------------------------------------------音频开关

//开启本地音乐任务
void set_local_music_task(void)
{
    /* 让任务退出单循环，重新初始化。 在local_music_task_handle开启 */
    dream_mic.mic_switch = OFF;
    os_sem_post(&local_music_task_sem);
    
}




// 关闭本地音乐任务
void reset_local_music_task(void)
{
    dream_mic.mic_switch = OFF;
    dream_mic.music_number = MIC_NULL;
}

/*******************************************************************************************************
**函数名：音乐律动-本地模式任务
**输  出：打开MIC，读取音量，根据音量大小调节LED

MIC_SPEED模式：
    音量大时，led_state.speed和dream_mic.change_speed(全局变量，渐变步进值)变大
    然后慢慢回落
    调用这个函数实现：sound_ctrl_3_clr_gradual()
    
*******************************************************************************************************/
void local_music_task_handle(void*p)
{
    u32 err;
    /* MIC_CHANGE_CLR模式，切换mode计数器 */
    static u8 change_mode_cnt = 0;
    /* 声控触发效果的持续时间 */
    static u8 targe_time;
    /* MIC_CHANGE_MODE模式，声控换效果的列表
        输入：change_mode_cnt
     */
    const u8 mode_map[14] = {   
    THREE_JUMP_CHANGE,
    SEVEN_JUMP_CHANGE,
    THREE_GRADUAL_CHANGE,
    SEVEN_GRADUAL_CHANGE,
    RED_GRADUAL_CHANGE,
    BLUE_GRADUAL_CHANGE,
    GREEN_GRADUAL_CHANGE,
    CYAN_GRADUAL_CHANGE,
    YELLOW_GRADUAL_CHANGE,
    PURPLE_GRADUAL_CHANGE,
    WHITE_GRADUAL_CHANGE,
    RED_GREEN_GRADUAL_CHANGE,
    RED_BLUE_GRADUAL_CHANGE,
    GREEN_BLUE_GRADUAL_CHANGE,
                                
    };
    mic_gpio_init();    //麦克风管脚初始化
    dream_mic.music_number = MIC_NULL;
    while(1)
    {
        err = os_sem_pend(&local_music_task_sem, 0);
        printf("running dream_light_mic_task>>>>>>\n");
        /* 每次入声控模式清状态 */
        change_mode_cnt = 0;
        Sound_val = 0;
        switch(dream_mic.music_number)
        {
            // 静态模式，根据声音变更颜色
            case MIC_CHANGE_CLR:
                dream_mic.sensitive = 2;
                switch_static_task();
                dream_mic.mic_switch = ON;
            break;
            case MIC_SPEED:
                dream_mic.sensitive = 2;
                led_state.dynamic_state_flag = SEVEN_GRADUAL_CHANGE;
                dream_mic.mic_switch = ON;
                /* 最慢 */
                led_state.speed = 0;
                switch_dynamic_task();
            break;
            case MIC_BRIGHTNESS:
                dream_mic.sensitive = 2;
                led_state.dynamic_state_flag = SOUND_CTRL_THREE_GRADUAL;
                dream_mic.mic_switch = ON;
                /* 最慢 */
                led_state.speed = 0;
                /* 亮度默认50% */
                led_state.ledlight = 50;
                dream_mic.mute_state = MIC_TRIGGER;
                switch_dynamic_task();
            break;
            case MIC_CHANGE_MODE:
                dream_mic.sensitive = 2;
                switch_dynamic_task();
                dream_mic.mic_switch = ON;
            break;
        }


        printf("dream_mic.change_mode = %d,dream_mic.music_number = %d\n",dream_mic.change_mode,dream_mic.music_number);
        while(dream_mic.mic_switch==ON)
        {
            
            /* 每10ms采集音量 */
            read_sound_task_handle();
            switch(dream_mic.music_number)
            {
                // 静态模式，根据声音变更颜色
                case MIC_CHANGE_CLR:
                    
                    if(Sound_val >= dream_mic.sensitive)
                    {
                        change_mode_cnt++;
                        // 最多7种模式，依据：led_static_task_handle()
                        change_mode_cnt %= 7;  
                        led_state.static_state_flag = change_mode_cnt;
                        switch_static_task();
                        #ifdef MY_DEBUG
                        printf("\nSound_val = %d",Sound_val);
                        #endif  
                    }
                    

                break;
                // 渐变模式，改变跳变速度：SEVEN_GRADUAL_CHANGE
                
                case MIC_SPEED:
                    
                    if(Sound_val >= dream_mic.sensitive)
                    {
                        /* 高速模式 */
                        led_state.speed = 8;
                        /* 持续200ms */
                        targe_time = 20;
                        dream_mic.mute_state = MIC_TRIGGER;
                        Mode_Synchro(led_state.dynamic_state_flag); 
                        #ifdef MY_DEBUG
                        printf("\nMIC_SPEED");
                        printf("\nSound_val = %d",Sound_val);
                        #endif   
                    }
                    else
                    {
                        if(targe_time > 0)
                        {
                            targe_time--;
                        }
                        else
                        {
                            dream_mic.mute_state = MIC_NO_TRIGGER;
                            led_state.speed = 0;
                        }  
                    }
                break;
                case MIC_BRIGHTNESS:
                    /* 亮度突变，再慢慢回落 */
                    if(Sound_val >= dream_mic.sensitive && dream_mic.mute_state != MIC_TRIGGER)
                    {
                        /* 高速模式 */
                        led_state.speed = 8;
                        /* 持续200ms */
                        targe_time = 40;  
                        led_state.ledlight = 100;
                        dream_mic.mute_state = MIC_TRIGGER;
                        #ifdef MY_DEBUG
                        printf("\MIC_BRIGHTNESS");
                        printf("\nSound_val = %d",Sound_val);
                        #endif   
                    }
                    else
                    {
                        if(targe_time > 0)
                        {
                            targe_time--;
                        }
                        else
                        {
                            dream_mic.mute_state = MIC_NO_TRIGGER;
                        }  
                        if(targe_time < 20) //预留200ms把亮度降下来
                        {
                            led_state.ledlight = 50;
                            led_state.speed = 8;
                        }
                    }

                break;

                case MIC_CHANGE_MODE:
                   
                    if(Sound_val >= dream_mic.sensitive)
                    {
                        dream_mic.mute_state = MIC_TRIGGER;
                        targe_time = 2;  
                        
                    // }
                    // else
                    // {
                    //     if(targe_time > 0)
                    //     {
                    //         targe_time--;
                    //     }
                    //     else
                        // {
                            if(dream_mic.mute_state == MIC_TRIGGER)
                            {
                                dream_mic.mute_state = MIC_NO_TRIGGER;
                                led_state.dynamic_state_flag = mode_map[change_mode_cnt];
                                switch_dynamic_task();
                                change_mode_cnt++;
                                change_mode_cnt%=14;
                                #ifdef MY_DEBUG
                                printf("\MIC_CHANGE_MODE");
                                printf("\nSound_val = %d",Sound_val);
                                #endif   
                            }
                            led_state.ledlight = 50;
                            led_state.speed = 0;
                        // }  
                    }
                    
                break;
            }
            os_time_dly(1);
        }
    }
}

/*******************************************************************************************************
**函数名：读mic管脚高低电平任务
**输  出：
**输  入：mic管脚PA1
**描  述：每隔10ms读一次MIC管脚的电平，记录总次数和低电平次数，总次数10次一个循环
**说  明：低电平次数/10*100=音量，音量分10级，把音量值放在Sound_val这个全局变量里
**版  本：
**修改日期：
*******************************************************************************************************/
void read_sound_task_handle(void)
{
    static u8 sound=0,count=0;


    // report_mic_state(1); //向APP上报麦克风的状态,用来让APP关闭麦克风面板
    if(dream_mic.mic_switch == ON)
    {
        if(gpio_read(MIC_PIN) == 0)
            sound++;
        count++;
        if(count>10)
        {
            count = 0;
            Sound_val = sound;
            sound = 0;

        }

    }
    // report_mic_state(0); //向APP上报麦克风的状态,用来让APP关闭麦克风面板

}


/*******************************************************************************************************
**函数名：mic脚IO口初始化
**输  出：
**输  入：
**描  述：
**说  明：
**版  本：
**修改日期：
*******************************************************************************************************/
static void mic_gpio_init()
{
    // gpio_set_die(MIC_PIN, 1);
	// gpio_set_direction(MIC_PIN, 1);
	// gpio_set_pull_up(MIC_PIN,1);
}


/*******************************************************************************************************
**函数名：本地音乐-呼吸模式
**输  出：跟据音量大小进行呼吸，声音大呼吸频率就大，转变就快，声音小频率就大，转变慢
**输  入：呼吸模式有三种：无声时灭，无声时维持亮度，无声时渐变
**描  述：
**说  明：
**版  本：
**修改日期：
*******************************************************************************************************/
#if 0
static void mic_braeth_mode(DREAM_LIGHT_MIC *dream_mic)
{
    u32 next_scene,add,braeth_frequency,sound_dat;
    u32 i,v_val,v_val_temp,v_val_top,step,switch_time;
    u32 r_val,g_val,b_val;
    HSV_COLOUR_DAT led_val[8];

    v_val = Command_buf_data[13];                //明度
    for(i=0;i<dream_mic->data_len;i++)
    {
        if(dream_mic->data_len>8)   //超出数据长度会崩溃
            break;
        led_val[i].h_val = (unsigned int)(Command_buf_data[14+i*3] << 8) | Command_buf_data[15+i*3];
        led_val[i].s_val = (unsigned int)Command_buf_data[16+i*3]*10;
//        printf("led_val[%d].h_val = %d,led_val[%d].s_val = %d\n",i,led_val[i].h_val,i,led_val[i].s_val);
    }

    memset(Disp_buf,0,Disp_buf_Len); //全灭

    add = 1;
    next_scene=0;
    v_val_temp=0;
    switch_time=0;

    while(Running_mode==MIC_BRAETH)
    {
        if(Sound_val>(101-dream_mic->sensitive)*6/100)    //灵敏度50%：Sound_val>3，灵敏度100%：Sound_val>6
        {
            sound_dat = Sound_val;      //步进值=音量值，音量值越大，步进值越大
            v_val_top = v_val*Sound_val/10; //最高亮度值=音量值/10，音量越大，最高亮度值越高
        }
        else
        {
            if(dream_mic->mute_state == 0)   //无声时灯灭
            {
                sound_dat = 1;         //如果无声，步进值维持1
                v_val_top = 0;        //最高亮度值=0
            }
            else if(dream_mic->mute_state == 1)   //无声时维持亮度
            {
                sound_dat = 0;         //如果无声，步进值维持0
                v_val_top = 20;        //最高亮度值=20%
            }
            else                             //无声时渐变
            {
                sound_dat = 1;         //如果无声，步进值维持0
                v_val_top = 20;        //最高亮度值=20%
            }
        }

        if(add)
            v_val_temp += sound_dat;    //亮度递增
        else
        {
            if(v_val_temp>=sound_dat)
                v_val_temp -= sound_dat;    //亮度递减
            else
                v_val_temp = 0;         //亮度少于步进值，直接=0
        }

        if(v_val_temp > v_val_top)  //当前亮度值>最高亮度值
            add=0;
        if(v_val_temp == 0)         //当前亮度值=0，表示一个呼吸周期的结束
            add=1;

        //    printf("Sound_val = %d,sound_dat = %d,v_val_temp = %d,v_val = %d,v_val_top = %d,sensitive = %d\n",Sound_val,sound_dat,v_val_temp,v_val,v_val_top,dream_mic->sensitive);

        hsv_to_rgb(&r_val,&g_val,&b_val,led_val[next_scene].h_val,led_val[next_scene].s_val,v_val_temp*10); //计算每段的RGB值

        for(i=0;i<LED_LEDGTH;i++)
        {
            Disp_buf[i*3]   = r_val;
            Disp_buf[i*3+1] = g_val;
            Disp_buf[i*3+2] = b_val;
        }

        if(switch_time<dream_mic->change_speed*10)
            switch_time++;
        else
        {
            if(v_val_temp==0)
            {
                switch_time = 0;
                next_scene++;
                if(next_scene>=dream_mic->data_len)
                    next_scene = 0;
                add = 1;
            }
        }
        os_time_dly(1);
    }
}
#endif
