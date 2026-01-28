#include "app_feedback.h"
#include "le_multi_trans.h"

// 向app反馈流星灯开关机的状态
void app_feedback_meteor_power_status(u8 status)
{
    u8 buff[3];
    buff[0] = 0x2F;
    buff[1] = 0x02;
    buff[2] = status;
    zd_fb_2_app(buff, 3);
}

// 向app反馈流星灯的速度
void app_feedback_meteor_speed(u8 speed)
{
    u8 buff[3];
    buff[0] = 0x2F;
    buff[1] = 0x01;
    buff[2] = speed;
    zd_fb_2_app(buff, 3);
}

// 向app反馈流星灯的时间间隔
void app_feedback_meteor_interval(u8 interval)
{
    u8 buff[3];
    buff[0] = 0x2F;
    buff[1] = 0x03;
    buff[2] = interval;
    zd_fb_2_app(buff, 3);
}

// 向app反馈流星灯的灵敏度
void app_feedback_meteor_sensitivity(u8 sensitivity)
{
    u8 buff[3];
    buff[0] = 0x2F;
    buff[1] = 0x05;
    buff[2] = sensitivity;
    zd_fb_2_app(buff, 3);
}

