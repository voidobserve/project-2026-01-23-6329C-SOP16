/*
- led_strip_drive.c
- 幻彩灯带驱动
- 麦克风驱动
 */
#include "led_strip_sys.h"
#include "led_strip_drive.h"
#include "asm/adc_api.h"

MIC_OFFON MIC_ENABLE;           //0-关闭麦克风，1-开启麦克风

const struct ledc_platform_data ledc_data =
{
    .index = 0,         //控制器号
    .port = LEDC_PIN,//输出引脚
    .idle_level = 1,    //当前帧的空闲电平，0：低电平， 1：高电平
    .out_inv = 1,       //起始电平，0：高电平开始， 1：低电平开始
    .bit_inv = 1,       //取数据时高低位镜像，0：不镜像，1：8位镜像，2:16位镜像，3:32位镜像
    .t_unit = t_42ns,   //时间单位
    .t1h_cnt = 21,      //1码的高电平时间 = t1h_cnt * t_unit;21*42=882
    .t1l_cnt = 9,       //1码的低电平时间 = t1l_cnt * t_unit;7*42=294
    .t0h_cnt = 9,       //0码的高电平时间 = t0h_cnt * t_unit;8*42=336
    .t0l_cnt = 20,      //0码的低电平时间 = t0l_cnt * t_unit;*/30*42=1260

    .t_rest_cnt = 20000,//复位信号时间 = t_rest_cnt * t_unit;20000*42=840000
    .cbfun = NULL,      //中断回调函数
};


void led_state_init(void)
{
    #ifdef MY_DEBUG
    printf("-----------------led_state_init--------------------");
    #endif
    ledc_init(&ledc_data);

}


void led_pwr_on(void)
{
    gpio_disable_fun_output_port(IO_PORTA_02);
    gpio_set_die(IO_PORTA_02, 1);
	gpio_set_direction(IO_PORTA_02, 0);
	gpio_set_pull_up(IO_PORTA_02,1);
    gpio_set_pull_down(IO_PORTA_02, 0);   //1，下拉；0，不下拉

    gpio_direction_output(IO_PORTA_02,1);
}
/*********************************mic脚IO口初始化***************************************************************/

void mic_gpio_init()
{
    // #define AD_CH_PA1    (0x0)
    // #define AD_CH_PA3    (0x1)
    // #define AD_CH_PA5    (0x2)
    // #define AD_CH_PA7    (0x3)
    // #define AD_CH_PA8    (0x4)
    // #define AD_CH_DP1    (0x5)
    // #define AD_CH_DM1    (0x6)
    // #define AD_CH_PB1    (0x7)
    // #define AD_CH_PA9    (0x8)
    // #define AD_CH_PB4    (0x9)
    // #define AD_CH_DP0    (0xA)
    // #define AD_CH_DM0    (0xB)
    // #define AD_CH_DB6    (0xC)
    // #define AD_CH_PMU    (0xD)
    // #define AD_CH_OSC32K (0xE)
    // #define AD_CH_BT     (0xF)

    adc_add_sample_ch(AD_CH_PA8);          //注意：初始化AD_KEY之前，先初始化ADC
    gpio_set_die(IO_PORTA_08, 0);
    gpio_set_direction(IO_PORTA_08, 1);
    gpio_set_pull_down(IO_PORTA_08, 0);
}

u16 check_mic_adc(void)
{
    return adc_get_value(AD_CH_PA8);
}

#define MAX_SOUND   10

u32 sound_v;    //平均值
u8 sound_cnt = 0;

typedef struct
{
    int buf[MAX_SOUND];
    int v;          //平均值
    int c_v;        //当前值
    u8  valid;      //数据有效
    u8  sensitive;  //灵敏度 0~100
}sound_t;

sound_t sound =
{
    .c_v=0,
    .v = 0,
    .valid = 0,
    .sensitive = 10,
};

// void set_sensitive(u8 s)
// {
//     sound.sensitive = s;
//     // fc_effect.music.s = s;
//     printf("\n sound.sensitive=%d",sound.sensitive);
// }

u8 get_sensitive(void)
{
    return sound.sensitive;
}

void check_mic_sound(void)
{
    u8 i;
    sound.buf[sound_cnt] = check_mic_adc();
    sound.c_v = sound.buf[sound_cnt];   //记录当前值
    sound_cnt++;
    if(sound_cnt > (MAX_SOUND-1) ||  sound.valid==1)
    {
        sound_cnt = 0;
        sound.valid = 1;
        sound.v = 0;
        for(i=0; i < MAX_SOUND; i++)
        {
            sound.v += sound.buf[i];
        }
        sound.v = sound.v / MAX_SOUND;    //计算平均值
    }
}

// 获取声控结果
// 触发条件：（（当前声音大小 - 平均值）* 100 ）/ 平均值 > 灵敏度（0~100）
// 0:没触发
// 1:触发
u8 get_sound_result(void)
{
    if(sound.valid==1)
    {
        if(sound.v > sound.c_v)
        {
            if( (sound.v - sound.c_v) * 100 / sound.v > sound.sensitive)
            {
                printf("\n sound.v =%d",sound.v);
                printf("\n sound.c_v =%d",sound.c_v);
                extern void WS2812FX_trigger();
                WS2812FX_trigger();
                return 1;
            }
        }
    }

    return 0;

}
