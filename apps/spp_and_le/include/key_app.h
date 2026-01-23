#ifndef key_app_h
#define key_app_h
// @ key_app.h
#include "cpu.h"
// ----------------------------------------------硬件配置，根据PCB实际修改

// @ board_ac632n_demo.c
// 通过irkey_data配置引脚
// ---------------------------------------------typedef
typedef enum
{
    IR_TIMER_0,     //无定时
    IR_TIMER_1,     //定时1小时
    IR_TIMER_2,     //定时2小时
}ir_timer_e;


// ---------------------------------------------定义AD_KEY键值

// ---------------------------------------------定义IR_KEY键值
#define IRKEY_AUTO 0x2d
#define IRKEY_ON 0x26
#define IRKEY_OFF 0x25
#define IRKEY_LIGHT_PLUS 0xF
#define IRKEY_LIGHT_SUB 0x6E
#define IRKEY_SPEED_PLUS 0x5B
#define IRKEY_SPEED_SUB 0x5C
#define IRKEY_R 0x6
#define IRKEY_G 0x8
#define IRKEY_B 0x5A
#define IRKEY_YELLOW 0x9
#define IRKEY_CYAN 0x10
#define IRKEY_PURPLE 0x1C
#define IRKEY_ORANGE 0xC
#define IRKEY_B1 0x8A //浅蓝色30,130,255,
#define IRKEY_B2 0x52 //深蓝色72,61,139
#define IRKEY_G1 0x53 //浅绿色50,205,50
#define IRKEY_W 0x1A
#define IRKEY_MODE_ADD 0xB
#define IRKEY_MODE_DEC 0xA
#define IRKEY_MUSIC1 0x1
#define IRKEY_MUSIC2 0x2
#define IRKEY_COLOR 0x1f
#define IRKEY_LOCK 0x0


typedef enum
{
    IR_AUTO,
    IR_PAUSE,
} ir_auto_e;




void  ir_mode_sub(void);
void ir_auto_change_mode(void);
void ir_mode_plus(void);
void key_event_task_handle(void *p);
u8 get_ir_mode(void);
/* 自动 */
void set_ir_auto(ir_auto_e auto_f);
ir_auto_e get_ir_auto(void);
void set_ir_timer(ir_timer_e timer);
void ir_timer_handle(void);
ir_timer_e get_ir_timer(void);

#endif


