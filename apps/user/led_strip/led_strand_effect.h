#ifndef led_strand_effect_h
#define led_strand_effect_h
#include "cpu.h"
#include "led_strip_sys.h"
#include "WS2812FX.H"
#define MAX_SMEAR_LED_NUM 48  //最多48个灯/48段
//当前模式枚举
typedef enum
{
  ACT_TY_PAIR,          //配对效果
  ACT_CUSTOM,           //自定义效果
  IS_STATIC,            //静态模式
  IS_light_music = 27,    //音乐律动
  IS_light_scene = 56,   //炫彩情景
  IS_smear_adjust = 59  //涂抹功能


} Now_state_e;

//涂抹工具
typedef enum
{
  IS_drum = 1,  //油桶
  IS_pen = 2,   //画笔
  IS_eraser = 3 //橡皮檫
} smear_tool_e;

//方向
typedef enum
{
  IS_forward = 0,
  IS_back = 16
} direction_e;

//变化方式
typedef enum
{
  IS_SCENE_RAINBOW = 2,           //彩虹
  IS_SCENE_JUMP_CHANGE = 10,      //跳变模式
  IS_SCENE_BRAETH = 11,           //呼吸模式
  IS_SCENE_TWIHKLE = 12,          //闪烁模式
  IS_SCENE_FLOW_WATER = 13,       //流水模式
  IS_SCENE_CHAS_LIGHT = 14,       //追光模式
  IS_SCENE_COLORFUL = 15,         //炫彩模式
  IS_SCENE_GRADUAL_CHANGE = 16,   //渐变模式
} change_type_e;

#pragma pack (1)
/*----------------------------涂抹功能结构体----------------------------------*/
typedef struct
{
  smear_tool_e smear_tool;
  color_t rgb[MAX_SMEAR_LED_NUM];
} smear_adjust_t;

/*----------------------------静态模式结构体----------------------------------*/
  

/*----------------------------幻彩情景结构体----------------------------------*/
typedef struct
{
  color_t rgb[MAX_NUM_COLORS];
  unsigned short speed;       //由档位决定
  unsigned char c_n;          //颜色数量
  change_type_e change_type;  //变化类型
  direction_e direction;      
} dream_scene_t;

/*----------------------------倒计时结构体----------------------------------*/
typedef struct
{
  unsigned char set_on_off;
  unsigned long time;
} countdown_t;


typedef struct
{
  unsigned char m;  //模式
  unsigned char s;  //灵敏度
  
}music_t;
/*----------------------------幻彩灯串效果大结构体----------------------------------*/
typedef struct
{
  unsigned char on_off_flag;  //开关状态
  unsigned char led_num;//灯点数
  color_t rgb;          //静态模式颜色
  unsigned char b;      //亮度
  unsigned short speed;       //8档
  unsigned char meteor_period;  //周期值，单位秒
  unsigned char mode_cycle;     //1:模式完成一个循环。0：正在跑，和meteor_period搭配用
  u16 period_cnt;               //ms,运行时的计数器
  Now_state_e Now_state;//当前运行模式
  smear_adjust_t smear_adjust;//涂抹功能
  dream_scene_t dream_scene;//幻彩情景
  countdown_t countdown;//倒计时
  
  u8 custom_index;  //实际用，仅用于掉电保存
  u8 sensitive;     //实际用，仅用于掉电保存
  
  music_t music;                //音乐
} fc_effect_t;

#pragma pack ()

extern fc_effect_t fc_effect;

void effect_smear_adjust_updata(smear_tool_e tool, hsv_t *colour,unsigned short *led_place);

void set_fc_effect(void);

void full_color_init(void);
// void soft_rurn_off_lights(void); //软关灯处理
// void soft_turn_on_the_light(void) ;  //软开灯处理
void ls_set_speed(uint8_t s);


#endif
