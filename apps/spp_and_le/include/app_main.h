#ifndef APP_MAIN_H
#define APP_MAIN_H
#include "os/os_type.h"
#include "local_music.h"
typedef struct _APP_VAR {
    s8 music_volume;
    s8 call_volume;
    s8 wtone_volume;
    u8 opid_play_vol_sync;
    u8 aec_dac_gain;
    u8 aec_mic_gain;
    u8 rf_power;
    u8 goto_poweroff_flag;
    u8 goto_poweroff_cnt;
    u8 play_poweron_tone;
    u8 remote_dev_company;
    u8 siri_stu;
    int auto_stop_page_scan_timer;     //用于1拖2时，有一台连接上后，超过三分钟自动关闭Page Scan
    volatile int auto_shut_down_timer;
    volatile int wait_exit_timer;
    u16 auto_off_time;
    u16 warning_tone_v;
    u16 poweroff_tone_v;
    u32 start_time;
    s8  usb_mic_gain;
} APP_VAR;

enum {
    KEY_USER_DEAL_POST = 0,
    KEY_USER_DEAL_POST_MSG,
    KEY_USER_DEAL_POST_EVENT,
    KEY_USER_DEAL_POST_2,
};

 typedef struct _LED_STATE {
    u32 ledlight;                   //灯条的亮度，范围由Light_Max，Light_Min决定
    u32 ledlight_temp;              //亮度的百分比，取值范围0-100
    u8 running_task;                //设备处在动态模式还是静态模式
    u8 dynamic_state_flag;          //如果是动态,设备处在动态的哪种模式
    u8 static_state_flag;           //如果是静态,设备处在静态的哪种模式
    u8 OpenorCloseflag;             //亮灯还是灭灯标志,默认关灯
    u8 R_flag;                      //红色，取值范围0~0xff
    u8 G_flag;                      //绿色，取值范围0~0xff
    u8 B_flag;                      //蓝色，取值范围0~0xff
    u8 W_flag;
    u8 interface_mode;              //灯条RBG接口顺序模式
    u8 speed;                       //动态时用于调节速度
}LED_STATE;

enum {
    LED_R_G_B = 0,
    LED_R_B_G,
    LED_G_R_B,
    LED_G_B_R,
    LED_B_R_G,
    LED_B_G_R,
}interface;

enum {
    RED_CH = 0,
    GREEN_CH,
    BLUE_CH,
}R_G_B;

typedef struct _TIME_CLOCK{
    u8 hour;    //小时
    u8 minute;  //分钟
    u8 second;  //秒钟
    u8 week;    //星期几
}TIME_CLOCK;    //计时

typedef struct _ALARM_CLOCK{
    u8 on_off;    //闹钟开关，0-开启闹钟，1-关闭闹钟
    u8 hour;      //小时
    u8 minute;    //分钟
    u8 mode;      //闹钟模式，0-关灯，1-开灯
    u8 monday;    //星期一
    u8 tuesday;   //星期二
    u8 wednesday; //星期三
    u8 thursday;  //星期四
    u8 friday;    //星期五
    u8 saturday;  //星期六
    u8 sunday;    //星期日
}ALARM_CLOCK;

extern APP_VAR app_var;


#define Light_Max 100    //最高调节亮度
#define Light_Min 25    //最低调节亮度

#define Speed_Max 1     //最大速度
#define Speed_Min 8     //最慢速度

#define NO_TASK         0   //没有运行任务
#define STATIC_TASK     1   //静态任务
#define DYNAMIC_TASK    2   //动态任务
#define PHASE_SEQUENCE  3   //调节相序任务
#define MUSIC_LED       4   //IOS音乐律动任务
#define MIC_LED         5   //IOS麦克风任务
#define ANDROID_LED     6   //安卓音乐及麦克风任务

#define OPEN_STATE      1   //亮灯标志
#define CLOSE_STATE     0   //灭灯标志

#define RED             0       //红色
#define GREEN           1       //绿色
#define BLUE            2       //蓝色
#define YELLOW          3       //黄色
#define CYAN            4       //青色
#define PURPLE          5       //紫色
#define WHITE           6       //白色

#define THREE_JUMP_CHANGE    7  //三色跳变
#define SEVEN_JUMP_CHANGE    8  //七色跳变

#define THREE_GRADUAL_CHANGE    9   //三色渐变
#define SEVEN_GRADUAL_CHANGE    10  //七色渐变

#define RED_GRADUAL_CHANGE      11  //红色渐变
#define BLUE_GRADUAL_CHANGE     12  //蓝色渐变
#define GREEN_GRADUAL_CHANGE    13  //绿色渐变
#define CYAN_GRADUAL_CHANGE     14  //青色渐变
#define YELLOW_GRADUAL_CHANGE   15  //黄色渐变
#define PURPLE_GRADUAL_CHANGE   16  //紫色渐变
#define WHITE_GRADUAL_CHANGE    17  //白色渐变

#define RED_GREEN_GRADUAL_CHANGE    18  //红绿渐变
#define RED_BLUE_GRADUAL_CHANGE     19  //红蓝渐变
#define GREEN_BLUE_GRADUAL_CHANGE   20  //绿蓝渐变

#define SEVEN_FAST_FLASH    21      //七色频闪
#define RED_FAST_FLASH      22      //红色频闪
#define BLUE_FAST_FLASH     23      //蓝色频闪
#define GREEN_FAST_FLASH    24      //绿色频闪
#define CYAN_FAST_FLASH     25      //青色频闪
#define YELLOW_FAST_FLASH   26      //黄色频闪
#define PURPLE_FAST_FLASH   27      //紫色频闪
#define WHITE_FAST_FLASH    28      //白色频闪

#define SOUND_CTRL_THREE_GRADUAL 101      //自定义:声控三色渐变
#define CUSTOMIZE               30          //自定义颜色
#define NO_LED                  50          //全灭


// 自定义其他色彩
#define ORANGE          31  //255,3f,0
#define BlueViolet      32  //138,43,226
#define SpringGreen1    33  //0,255,127
#define SkyBlue         34  //135,206,235
#define RoyalBlue       35  //65,105,225
#define OrangeRed       36  //255,69,0
#define YellowGreen     37  //154,205,50



extern OS_SEM LedStaticSem;
extern OS_SEM LedActionSem;
extern LED_STATE led_state;
extern DREAM_LIGHT_MIC dream_mic;

#define  switch_dynamic_task()      led_state.running_task = DYNAMIC_TASK;  os_sem_post(&LedActionSem); 
#define  switch_static_task()       led_state.running_task = STATIC_TASK;   os_sem_post(&LedStaticSem); 


void flash_read_LED_state(void);
void LED_state_write_flash(void);
void mic_led_task_handle(void*p);

static void led_static_task_handle(void*p);
static void led_action_task_handle(void*p);
static void ir_key_task_handle(void*p);
void led_refresh_task_handle(void*p);
static void phase_sequence_task_handle(void*p);
static void music_led_task_handle(void*p);
static void android_led_task_handle(void*p);
static void alarm_clock_task_handle(void*p);

void Set_Duty(u16 duty1,u16 duty2,u16 duty3);
void sys_set_pwm_duty(u8 io_num,u16 duty);
void user_timer_init(void);
static void ir_key_task_handle(void*p);
void user_deal_init(void);
void display_led(void);
void led_main(void);
void colour_jump_change(void);


#endif
