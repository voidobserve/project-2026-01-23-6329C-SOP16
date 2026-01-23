#ifndef rf433_app_h
#define rf433_app_h

#define RFKEY_ON_OFF 0x80
#define RFKEY_SPEED_PLUS 0xC0
#define RFKEY_SPEED_SUB 0x20
#define RFKEY_MODE_ADD 0x40

#define RFKEY_MUSIC1 0xA8





//17键流星遥控 62FC

#define _17_ON_OFF         0x80  //开关
#define _17_METEOR_LEN_PUL 0xA0  //拖尾+
#define _17_SPEED_SUB      0xE0  //速度-
#define _17_RESET          0x10  //复位 流星7
#define _17_SPEED_PUL      0x90//速度+
#define _17_PERIOD_PUL     0x50  //时间间隔+
#define _17_METEOR_LEN_SUB 0xD0  //拖尾-
#define _17_BRIGHT_PUL     0x30  //亮度+
#define _17_PERIOD_SUB     0xB0  //时间间隔-
#define _17_METEOR_DIR     0x70  //正反流
#define _17_BRIGHT_SUB     0xF0  //亮度-
#define _17_MIN_PERIOD     0x08  //周期最短
#define _17_PERIOD_10S     0x88 //定周期 10s
#define _17_MAX_PERIOD     0x48 //周期最长
#define _17_METEOR_MUSIC1  0xC8 //律动1
#define _17_METEOR_MUSIC2  0x28  //律动2
#define _17_METEOR_MUSIC3  0xA8 //律动3


#endif

