#ifndef local_music_h
#define local_music_h
#include "cpu.h"
typedef enum
{
    MIC_NULL,               //NULL
    MIC_SPEED,              //音量改变速度
    MIC_BRIGHTNESS,         //音量改变亮度
    MIC_CHANGE_MODE,        //音量改变模式
    MIC_CHANGE_CLR,         //音量改变颜色

}local_music_mode_e;

typedef enum
{
    OFF,    //mic关闭
    ON,     //mic打开
}MIC_OFFON;


typedef struct _LED_COLOUR_DAT
{
    u32 r_val;
    u32 g_val;
    u32 b_val;
}LED_COLOUR_DAT;

typedef enum
{
    MIC_NO_TRIGGER,
    MIC_TRIGGER,
}MIC_MUTE_STAT;

typedef struct _DREAM_LIGHT_MIC_MUSIC
{
    u8 data_len;                    //数据长度
    MIC_OFFON mic_switch;           //本地麦克风开关，0-关/ 1-开
    local_music_mode_e music_number;//音乐模式编号
    MIC_MUTE_STAT mute_state;       //变换后的状态,0000-无声时灯灭,0001-无声时灯维持静态亮度,0010-无声时灯光缓慢渐变
    u8 change_mode;                 //变换方式，跳变，渐变，呼吸，闪烁
    u8 change_speed;                //变化速度1-100
    u8 sensitive;                   //灵敏度1-100
    bool colour;                    //0-颜色手动；1-颜色自动
    LED_COLOUR_DAT colour_led[8];
}DREAM_LIGHT_MIC;


extern DREAM_LIGHT_MIC dream_mic;

void local_music_task_handle(void*p);
local_music_mode_e get_local_mic_mode(void);
void set_local_mic_mode(local_music_mode_e mode);
//开启本地音乐任务
void set_local_music_task(void);
// 关闭本地音乐任务
void reset_local_music_task(void);
void locla_mic_sen_plus(void);
void locla_mic_sen_sub(void);
void set_local_mic_sen(u8 sensitive);
u8 get_local_mic_sen(void);


#endif

