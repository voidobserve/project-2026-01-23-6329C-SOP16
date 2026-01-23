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
#include "key_app.h"
#include "local_music.h"
#include "rf433.h"
#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
#include "jl_kws/jl_kws_api.h"
#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */

#define LOG_TAG_CONST APP
#define LOG_TAG "[APP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

OS_SEM LedStaticSem;
OS_SEM LedActionSem;
OS_SEM mic_led_Sem;
OS_SEM phaseSequenceSem;
OS_SEM phaseSequenceKeySem;

u8 Ble_Addr[6]; // BLE地址
APP_VAR app_var;
LED_STATE led_state;
LED_STATE save_led_state;
TIME_CLOCK time_clock;
ALARM_CLOCK alarm_clock[3];
u32 mic_val;
u8 rgb_func_isr_en = 1;
u8 ir_detect_isr_en = 1;
u8 Ir_Key_Val;

extern int app_send_user_data_check(u16 len);
extern int app_send_user_data_do(void *priv, u8 *data, u16 len);
extern int app_send_user_data(u16 handle, u8 *data, u16 len, u8 handle_type);

extern void mcpwm_test_init();
extern void timer_pwm_init(JL_TIMER_TypeDef *JL_TIMERx, u32 pwm_io, u32 fre, u32 duty);
extern void set_duty(JL_TIMER_TypeDef *JL_TIMERx, u32 duty);
extern void mcpwm_init(struct pwm_platform_data *arg);
extern void mcpwm_set_duty(pwm_ch_num_type pwm_ch, u16 duty);
extern uint8_t LedCommand[200];
extern uint16_t LedCommandLegth;
extern const u8 *bt_get_mac_addr();
extern void GCLight_ZeroDetect(void);

#define LED_OFF rgb_func_isr_en = 0 // 关闭可控硅触发功能
#define LED_ON rgb_func_isr_en = 1  // 开启可控硅触发功能

/*任务列表 */
const struct task_info task_info_table[] = {
    {"app_core", 1, 640, 128},
    {"sys_event", 7, 256, 0},
    {"btctrler", 4, 512, 256},
    {"btencry", 1, 512, 128},
    {"btstack", 3, 768, 256},
    {"systimer", 7, 128, 0},
    {"update", 1, 320, 0},
    {"dw_update", 2, 256, 128},
#if (RCSP_BTMATE_EN)
    {"rcsp_task", 2, 640, 128},
#endif
#if (USER_UART_UPDATE_ENABLE)
    {"uart_update", 1, 256, 128},
#endif
#if (XM_MMA_EN)
    {"xm_mma", 2, 640, 256},
#endif
    {"usb_msd", 1, 512, 128},
#if TCFG_AUDIO_ENABLE
    {"audio_dec", 3, 768, 128},
    {"audio_enc", 4, 512, 128},
#endif /*TCFG_AUDIO_ENABLE*/
#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
    {"kws", 2, 256, 64},
#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */
    // {"led_static",          2,      256,  64    },  //静态任务
    /*  {"led_action",          2,      256,  64    },  //动态任务
     {"ir_key",              3,      256,  64    },  //红外接收任务
     {"phase_sequence",      2,      256,  64    },  //遥控器调整相序任务

     {"android_led",         2,      256,  64    },  //音乐律动任务
     {"alarm_clock",         2,      256,  64    },  //闹钟任务
     {"local_music",         2,      256,    64  },  //本地音乐任务 */
    //    {"user_deal",           7,     512,   512   },//定义线程 tuya任务调度
    {0, 0},
};

typedef struct _HALF_WAVE_PERIOD
{
    u32 period_buff[10];
    u32 period_cnt;
    u32 period_average;
    u32 red_period;
    u32 green_period;
    u32 blue_period;
} HALF_WAVE_PERIOD;

HALF_WAVE_PERIOD half_wave_period;

//*****************************************************//
//*****************************************************//
static const u16 timer_div[] = {
    /*0000*/ 1,
    /*0001*/ 4,
    /*0010*/ 16,
    /*0011*/ 64,
    /*0100*/ 2,
    /*0101*/ 8,
    /*0110*/ 32,
    /*0111*/ 128,
    /*1000*/ 256,
    /*1001*/ 4 * 256,
    /*1010*/ 16 * 256,
    /*1011*/ 64 * 256,
    /*1100*/ 2 * 256,
    /*1101*/ 8 * 256,
    /*1110*/ 32 * 256,
    /*1111*/ 128 * 256,
};

#define APP_TIMER_CLK (CONFIG_BT_NORMAL_HZ / 2) // clk_get("timer")
#define MAX_TIME_CNT 0x7fff
#define MIN_TIME_CNT 0x100
#define TIMER_UNIT 1

#define TIMER_CON JL_TIMER2->CON
#define TIMER_CNT JL_TIMER2->CNT
#define TIMER_PRD JL_TIMER2->PRD
#define TIMER_VETOR IRQ_TIME2_IDX

/////下面函数调用的使用函数都必须放在ram
extern void ir_detect_isr(void);
/****************************************************************************************
**名称:过零检测中断服务
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
//___interrupt
AT_VOLATILE_RAM_CODE
void rgb_func_isr(void)
{
    static bool detect_level = 1;
    static u8 num = 0;

    if (gpio_read(IO_PORTB_06))
    {
        if (detect_level == 0)
        {
            detect_level = 1;
            half_wave_period.period_buff[num] = half_wave_period.period_cnt;
            num++;
            if (num >= 4)
            {
                num = 0;
                half_wave_period.period_average = (half_wave_period.period_buff[0] + half_wave_period.period_buff[1] + half_wave_period.period_buff[2] + half_wave_period.period_buff[3]) / 4;
                // printf("%d ",app_var.period_average);
            };
            JL_PORTB->OUT &= ~BIT(7);
            JL_PORTA->OUT &= ~BIT(0);
            JL_PORTA->OUT &= ~BIT(1);
        }
    }
    else
    {
        if (detect_level)
        {
            detect_level = 0;
            half_wave_period.period_cnt = 0;
        }
    }

    if (detect_level)
        return;

    if (half_wave_period.red_period < half_wave_period.period_cnt)
    {
        JL_PORTB->OUT ^= BIT(7);
    }

    if (half_wave_period.green_period < half_wave_period.period_cnt)
    {
        JL_PORTA->OUT ^= BIT(0);
    }

    if (half_wave_period.blue_period < half_wave_period.period_cnt)
    {
        JL_PORTA->OUT ^= BIT(1);
    }

    if (half_wave_period.period_cnt < 0xffff)
        half_wave_period.period_cnt++;
}
/****************************************************************************************
**名称:定时中断服务函数
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
#define USER_IR_ENABLE 0
___interrupt
    AT_VOLATILE_RAM_CODE void user_timer_isr(void) // 50us
{
    static u8 timer_cnt;
    TIMER_CON |= BIT(14);

    timer_cnt++;
#if USER_IR_ENABLE
    if (timer_cnt % 4 == 0)
    {
        if (ir_detect_isr_en)
            ir_detect_isr();
    }
#endif

#if TCFG_RF433_ENABLE
    extern void timer125us_hook(void);
    timer125us_hook();
#endif

#if 0
    if(rgb_func_isr_en)
        rgb_func_isr();
    else
    {
        JL_PORTB->OUT &= ~ BIT(7);
        JL_PORTA->OUT &= ~ BIT(0);
        JL_PORTA->OUT &= ~ BIT(1);
    }
#endif
}
/****************************************************************************************
**名称:定时器设置，定时50us中断
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void user_timer_init(void)
{
    u32 prd_cnt;
    u8 index;

    //	printf("********* user_timer_init **********\n");
    for (index = 0; index < (sizeof(timer_div) / sizeof(timer_div[0])); index++)
    {
        prd_cnt = TIMER_UNIT * (APP_TIMER_CLK / 8000) / timer_div[index];
        if (prd_cnt > MIN_TIME_CNT && prd_cnt < MAX_TIME_CNT)
        {
            break;
        }
    }

    TIMER_CNT = 0;
    TIMER_PRD = prd_cnt;
    request_irq(TIMER_VETOR, 0, user_timer_isr, 0);
    TIMER_CON = (index << 4) | BIT(0) | BIT(3);
}
__initcall(user_timer_init);
/****************************************************************************************
**名称:将LED状态保存到FLASH
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
#define FLASH_DAT_QTY 14
void LED_state_write_flash(void)
{
    u8 led_state_flag[FLASH_DAT_QTY];

    led_state_flag[0] = led_state.ledlight;
    led_state_flag[1] = led_state.ledlight_temp;
    led_state_flag[2] = led_state.running_task;
    led_state_flag[3] = led_state.dynamic_state_flag;
    led_state_flag[4] = led_state.static_state_flag;
    led_state_flag[5] = led_state.OpenorCloseflag;
    led_state_flag[6] = led_state.R_flag;
    led_state_flag[7] = led_state.G_flag;
    led_state_flag[8] = led_state.B_flag;
    led_state_flag[9] = led_state.interface_mode;
    led_state_flag[10] = led_state.speed;
    led_state_flag[11] = get_local_mic_sen();
    led_state_flag[12] = get_local_mic_mode();
    led_state_flag[13] = get_ir_auto();

    syscfg_write(CFG_USER_LED_STATE, (u8 *)(&led_state_flag), sizeof(led_state_flag));

    //   printf("write flash data = \n");
    //    printf_buf(led_state_flag,sizeof(led_state_flag));
}

/****************************************************************************************
**名称:   将LED状态从flash读出
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/

void flash_read_LED_state(void)
{
    u8 led_state_flag[FLASH_DAT_QTY];

    u8 len = syscfg_read(CFG_USER_LED_STATE, (u8 *)(&led_state_flag), sizeof(led_state_flag));
    if (len != FLASH_DAT_QTY) // 第一次上电
    {
        memset(led_state_flag, 0, sizeof(led_state_flag));

        led_state_flag[0] = Light_Max;
        led_state_flag[1] = Light_Max;
        led_state_flag[2] = STATIC_TASK;
        led_state_flag[3] = NO_LED;
        led_state_flag[4] = RED;
        led_state_flag[5] = OPEN_STATE;
        led_state_flag[6] = 0xff;
        led_state_flag[7] = 0;
        led_state_flag[8] = 0;
        led_state_flag[9] = LED_R_B_G;
        led_state_flag[10] = 5;
        led_state_flag[11] = 1;        /* 灵敏度 */
        led_state_flag[12] = 1;        /* 声控模式 */
        led_state_flag[13] = IR_PAUSE; /* 不开启自动模式 */

        syscfg_write(CFG_USER_LED_STATE, (u8 *)(&led_state_flag), sizeof(led_state_flag));
    }
    led_state.ledlight = led_state_flag[0];
    led_state.ledlight_temp = led_state_flag[1];
    led_state.running_task = led_state_flag[2];
    led_state.dynamic_state_flag = led_state_flag[3];
    led_state.static_state_flag = led_state_flag[4];
    led_state.OpenorCloseflag = led_state_flag[5];
    led_state.R_flag = led_state_flag[6];
    led_state.G_flag = led_state_flag[7];
    led_state.B_flag = led_state_flag[8];
    led_state.interface_mode = led_state_flag[9];
    led_state.speed = led_state_flag[10];
    set_local_mic_sen(led_state_flag[11]);
    set_local_mic_mode(led_state_flag[12]);
    set_ir_auto(led_state_flag[13]);
    //   printf("read flash data = \n");
    //    printf_buf(led_state_flag,sizeof(led_state_flag));
    //   printf("led_state.R_flag = %d,led_state.G_flag = %d,led_state.B_flag = %d\n",led_state.R_flag,led_state.G_flag,led_state.B_flag);
}
/****************************************************************************************
**名称:LED GPIO初始化
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void led_gpio_init(void)
{

    gpio_set_die(IO_PORTA_00, 1);
    gpio_direction_output(IO_PORTA_00, 0);

    gpio_set_die(IO_PORTA_01, 1);
    gpio_direction_output(IO_PORTA_01, 0);

    gpio_set_die(IO_PORTB_07, 1);
    gpio_direction_output(IO_PORTB_07, 0);

    gpio_set_die(IO_PORTA_02, 1);
    gpio_direction_output(IO_PORTA_02, 0);

    LED_ON; // 上电默认开灯
}

void led_pwm_init(void)
{
    // R
    struct pwm_platform_data pwm_p_data;
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned; // 边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch0;                // 通道号
    pwm_p_data.frequency = 1000;                    // 1KHz
    pwm_p_data.duty = 0;                            // 上电输出0%占空比
    pwm_p_data.h_pin = IO_PORTB_07;                 // 任意引脚
    pwm_p_data.l_pin = -1;                          // 任意引脚,不需要就填-1
    pwm_p_data.complementary_en = 0;                // 两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
    mcpwm_init(&pwm_p_data);
    // G
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned; // 边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch1;                // 通道号
    pwm_p_data.frequency = 1000;                    // 1KHz
    pwm_p_data.duty = 0;                            // 上电输出0%占空比
    pwm_p_data.h_pin = IO_PORTA_00;                 // 任意引脚
    pwm_p_data.l_pin = -1;                          // 任意引脚,不需要就填-1
    pwm_p_data.complementary_en = 0;                // 两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
    mcpwm_init(&pwm_p_data);
    // B
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned; // 边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch2;                // 通道号
    pwm_p_data.frequency = 1000;                    // 1KHz
    pwm_p_data.duty = 0;                            // 上电输出0%占空比
    pwm_p_data.h_pin = IO_PORTA_01;                 // 任意引脚
    pwm_p_data.l_pin = -1;                          // 任意引脚,不需要就填-1
    pwm_p_data.complementary_en = 0;                // 两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
    mcpwm_init(&pwm_p_data);
    // W
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned; // 边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch3;                // 通道号
    pwm_p_data.frequency = 1000;                    // 1KHz
    pwm_p_data.duty = 0;                            // 上电输出0%占空比
    pwm_p_data.h_pin = IO_PORTA_02;                 // 任意引脚
    pwm_p_data.l_pin = -1;                          // 任意引脚,不需要就填-1
    pwm_p_data.complementary_en = 0;                // 两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
    mcpwm_init(&pwm_p_data);
}

/****************************************************************************************
**名称:软关灯处理
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
// void soft_rurn_off_lights(void)
// {
//     u8 buf[9];

//     if(led_state.OpenorCloseflag == OPEN_STATE) //在开灯的状态下才能关灯
//     {
//         set_ir_timer(IR_TIMER_0);
//         save_led_state = led_state;   //保存当前状态

//         led_state.R_flag = 0;
//         led_state.G_flag = 0;
//         led_state.B_flag = 0;
//         led_state.W_flag = 0;
//         led_state.ledlight = 0;
//         led_state.running_task = NO_TASK;
//         led_state.static_state_flag = NO_LED;
//         led_state.dynamic_state_flag = NO_LED;
//         led_state.OpenorCloseflag = CLOSE_STATE;

//         memcpy(buf,Ble_Addr, 6);    //发送开关机状态，与APP同步
//         buf[6] = 0x01;
//         buf[7] = 0x01;
//         buf[8] = 0x00;
//         app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, buf,9, ATT_OP_AUTO_READ_CCC);

// //        display_led();
//         Set_Duty(0,0,0);

// //        rgb_func_isr_en = 0;    //关闭可控硅触发功能
//         LED_OFF;

//         printf("--------Close Light--------");
//     }
// }

/****************************************************************************************
**名称:软开灯处理
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
// void soft_turn_on_the_light(void)   //软开灯处理
// {
//     u8 buf[9];

//     if(led_state.OpenorCloseflag == CLOSE_STATE) //在关灯的状态下才能开灯
//     {

//         led_state = save_led_state;                  //读取关机前的状态
// //        led_state.ledlight = led_state.ledlight_temp;
//         led_state.OpenorCloseflag = OPEN_STATE;

//         LED_ON;     //打开灯光

//         if(led_state.running_task == DYNAMIC_TASK)
//         {
//             os_sem_post(&LedActionSem); //启动动态任务
//         }
//         else
//         {
//             led_state.running_task = STATIC_TASK;
//             os_sem_post(&LedStaticSem); //启动静态任务
//         }

// /*        if(led_state.running_task == DYNAMIC_TASK)
//         {
//             os_sem_post(&LedActionSem); //启动动态任务
//         }
//         else if(led_state.running_task == STATIC_TASK)
//         {
//             os_sem_post(&LedStaticSem); //启动静态任务
//         }
//         else if(led_state.running_task == PHASE_SEQUENCE)   //调节相序
//         {
//             os_sem_post(&LedStaticSem);
//         }
//         else if(led_state.running_task == MUSIC_LED)    //IOS音乐律动任务
//         {
//             os_sem_post(&LedStaticSem);
//         }
//         else if(led_state.running_task == MIC_LED)      //IOS麦克风任务
//         {
//             os_sem_post(&LedStaticSem); //启动静态任务
//         }

//         else if(led_state.running_task == ANDROID_LED)      //安卓音乐及麦克风任务
//         {
//             os_sem_post(&LedStaticSem); //启动静态任务
//         }
// */
//         memcpy(buf,Ble_Addr, 6);    //发送开关机状态，与APP同步
//         buf[6] = 0x01;
//         buf[7] = 0x01;
//         buf[8] = 0x01;
//         app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, buf,9, ATT_OP_AUTO_READ_CCC);

//         printf("--------Open Light--------");
//         printf("led_state.running_task %d\n",led_state.running_task);
//     }
// }

/*
void app_var_init(void)
{
    app_var.play_poweron_tone = 1;

    app_var.auto_off_time =  TCFG_AUTO_SHUT_DOWN_TIME;
    app_var.warning_tone_v = 340;
    app_var.poweroff_tone_v = 330;
}
*/
__attribute__((weak)) // 如果有同名的外部函数，则调用外部函数，没有就调用这个函数，声明下面的函数是弱函数
u8 get_charge_online_flag(void)
{
    return 0;
}

/****************************************************************************************
**名称:初始化及创建各种任务
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void app_main()
{
    struct intent it;

    if (!UPDATE_SUPPORT_DEV_IS_NULL())
    {
        int update = 0;
        update = update_result_deal();
    }

    if (get_charge_online_flag())
    {
#if (TCFG_SYS_LVD_EN == 1) // 电量检测使能,0
        vbat_check_init();
#endif
    }
    else
    {
        check_power_on_voltage();
    }

#if TCFG_AUDIO_ENABLE // 0
    extern int audio_dec_init();
    extern int audio_enc_init();
    audio_dec_init();
    audio_enc_init();
#endif /*TCFG_AUDIO_ENABLE*/

#if TCFG_KWS_VOICE_RECOGNITION_ENABLE // 0
    jl_kws_main_user_demo();
#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */

    init_intent(&it); // 清空任务it的数据,下面重新加载
#if CONFIG_APP_SPP_LE // 1
    it.name = "spp_le";
    it.action = ACTION_SPPLE_MAIN;
#endif

#if CONFIG_APP_AT_COM || CONFIG_APP_AT_CHAR_COM
    it.name = "at_com";
    it.action = ACTION_AT_COM;
#endif

#if CONFIG_APP_DONGLE
    it.name = "dongle";
    it.action = ACTION_DONGLE_MAIN;
#endif

#if CONFIG_APP_MULTI
    it.name = "multi_conn";
    it.action = ACTION_MULTI_MAIN;
#endif

#if CONFIG_APP_TUYA
    it.name = "tuya";
    it.action = ACTION_TUYA_MAIN;
#endif
    log_info("app_name:%s\n", it.name);
    /* it.name = "idle"; */
    /* it.action = ACTION_IDLE_MAIN; */

    start_app(&it); // 进入任务调度
#if TCFG_PC_ENABLE  // 0
    void usb_start();
    usb_start();
#endif
}

/****************************************************************************************
**名称:任务切换
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void app_switch(const char *name, int action)
{
    struct intent it;
    struct application *app;

    log_info("app_exit\n");

    init_intent(&it);        // 初始化任务列表
    app = get_current_app(); // 获取当前APP
    if (app)
    {
        /*
         * 退出当前app, 会执行state_machine()函数中APP_STA_STOP 和 APP_STA_DESTORY
         */
        it.name = app->name;
        it.action = ACTION_BACK;
        start_app(&it);
    }

    /*
     * 切换到app (name)并执行action分支
     */
    it.name = name;
    it.action = action;
    start_app(&it);
}

int eSystemConfirmStopStatus(void)
{
    /* 系统进入在未来时间里，无任务超时唤醒，可根据用户选择系统停止，或者系统定时唤醒(100ms) */
    // 1:Endless Sleep
    // 0:100 ms wakeup
    /* log_info("100ms wakeup"); */
    return 1;
}

__attribute__((used)) int *__errno()
{
    static int err;
    return &err;
}
/*
///自定义事件推送的线程

#define Q_USER_DEAL   0xAABBCC ///自定义队列类型
#define Q_USER_DATA_SIZE  10///理论Queue受任务声明struct task_info.qsize限制,但不宜过大,建议<=6

void user_deal_send_ver(void)
{
    //os_taskq_post("user_deal", 1,KEY_USER_DEAL_POST);
    os_taskq_post_msg("user_deal", 1, KEY_USER_DEAL_POST_MSG);
    //os_taskq_post_event("user_deal",1, KEY_USER_DEAL_POST_EVENT);
}

void user_deal_rand_set(u32 rand)
{
    os_taskq_post("user_deal", 2, KEY_USER_DEAL_POST_2, rand);
}

void user_deal_send_array(int *msg, int argc)
{
    if (argc > Q_USER_DATA_SIZE) {
        return;
    }
    os_taskq_post_type("user_deal", Q_USER_DEAL, argc, msg);
}
void user_deal_send_msg(void)
{
    os_taskq_post_event("user_deal", 1, KEY_USER_DEAL_POST_EVENT);
}

void user_deal_send_test(void)///模拟测试函数,可按键触发调用，自行看打印
{
    user_deal_send_ver();
    user_deal_rand_set(0x11223344);
    static u32 data[Q_USER_DATA_SIZE] = {0x11223344, 0x55667788, 0x11223344, 0x55667788, 0x11223344,
                                         0xff223344, 0x556677ee, 0x11223344, 0x556677dd, 0x112233ff,
                                        };
    user_deal_send_array(data, sizeof(data) / sizeof(int));
}

static void user_deal_task_handle(void *p)
{
    int msg[Q_USER_DATA_SIZE + 1] = {0, 0, 0, 0, 0, 0, 0, 0, 00, 0};
    int res = 0;
    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));  //阻塞方式等待消息队例信息
        if (res != OS_TASKQ) {
            continue;
        }
        r_printf("user_deal_task_handle:0x%x", msg[0]);
        put_buf(msg, (Q_USER_DATA_SIZE + 1) * 4);
        if (msg[0] == Q_MSG) {
            printf("use os_taskq_post_msg");
            switch (msg[1]) {
            case KEY_USER_DEAL_POST_MSG:
                printf("KEY_USER_DEAL_POST_MSG");
                break;
            default:
                break;
            }
        } else if (msg[0] == Q_EVENT) {
            printf("use os_taskq_post_event");
            switch (msg[1]) {
            case KEY_USER_DEAL_POST_EVENT:
                printf("KEY_USER_DEAL_POST_EVENT");
                break;
            default:
                break;
            }
        } else if (msg[0] == Q_CALLBACK) {
        } else if (msg[0] == Q_USER) {
            printf("use os_taskq_post");
            switch (msg[1]) {
            case KEY_USER_DEAL_POST:
                printf("KEY_USER_DEAL_POST");
                break;
            case KEY_USER_DEAL_POST_2:
                printf("KEY_USER_DEAL_POST_2:0x%x", msg[2]);
                break;
            default:
                break;
            }
        } else if (msg[0] == Q_USER_DEAL) {
            printf("use os_taskq_post_type");
            printf("0x%x 0x%x 0x%x 0x%x 0x%x", msg[1], msg[2], msg[3], msg[4], msg[5]);
            printf("0x%x 0x%x 0x%x 0x%x 0x%x", msg[6], msg[7], msg[8], msg[9], msg[10]);
        }
        puts("");
    }
}

void user_deal_init(void)
{
    task_create(user_deal_task_handle, NULL, "user_deal");
}

void user_deal_exit(void)
{
    task_kill("user_deal");
}
*/
/****************************************************************************************
**名称:设置LED的占空比
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void Set_Duty(u16 duty1, u16 duty2, u16 duty3)
{
    u8 interface_ch1, interface_ch2, interface_ch3;
    int tmp_r, tmp_g, tmp_b;
    // interface_ch1:物理接口的R
    // interface_ch2:物理接口的G
    // interface_ch3：物理接口的B
    if (duty1 == 0 && duty2 == 0 && duty3 == 0)
    {
        mcpwm_set_duty(pwm_ch0, 0); // PA7
        mcpwm_set_duty(pwm_ch1, 0); // PA8
        mcpwm_set_duty(pwm_ch2, 0); // PA2
    }
    else
    {
        LED_ON;

        switch (led_state.interface_mode)
        {
        case LED_R_G_B:
            interface_ch1 = RED_CH;
            interface_ch2 = GREEN_CH;
            interface_ch3 = BLUE_CH;
            break;
        case LED_R_B_G:
            interface_ch1 = RED_CH;
            interface_ch2 = BLUE_CH;
            interface_ch3 = GREEN_CH;
            break;
        case LED_G_R_B:
            interface_ch1 = GREEN_CH;
            interface_ch2 = RED_CH;
            interface_ch3 = BLUE_CH;
            break;
        case LED_G_B_R:
            interface_ch1 = GREEN_CH;
            interface_ch2 = BLUE_CH;
            interface_ch3 = RED_CH;
            break;
        case LED_B_R_G:
            interface_ch1 = BLUE_CH;
            interface_ch2 = RED_CH;
            interface_ch3 = GREEN_CH;
            break;
        case LED_B_G_R:
            interface_ch1 = BLUE_CH;
            interface_ch2 = GREEN_CH;
            interface_ch3 = RED_CH;
            break;
        }

        // tmp_r = duty1 * 5000 / 255;
        // tmp_g = duty2 * 5000 / 255;
        // tmp_b = duty3 * 5000 / 255;

        mcpwm_set_duty(interface_ch1, duty3); // PA7
        mcpwm_set_duty(interface_ch2, duty2); // PA8
        mcpwm_set_duty(interface_ch3, duty1); // PA2
    }
    mcpwm_set_duty(3, 0); // W通道清0
}
#define MAX_PWM 10000
#define RGB_CTL_USE_SCR 1
void sys_set_pwm_duty(u8 io_num, u16 duty)
{
    // printf("duty = %d\n",duty);
#if RGB_CTL_USE_SCR
    switch (io_num)
    {
    case 0:
        half_wave_period.red_period = half_wave_period.period_average - (duty * half_wave_period.period_average / MAX_PWM);
        // printf("red_period = %d\n",half_wave_period.red_period);
        break;

    case 1:
        half_wave_period.blue_period = half_wave_period.period_average - (duty * half_wave_period.period_average / MAX_PWM);
        // printf("blue_period = %d\n",app_var.blue_period);
        break;

    case 2:
        half_wave_period.green_period = half_wave_period.period_average - (duty * half_wave_period.period_average / MAX_PWM);
        // printf("green_period = %d\n",app_var.green_period);
        break;
    }
#else
    switch (io_num)
    {
    case 0:
        mcpwm_set_duty(pwm_ch0, duty);
        break;

    case 1:
        mcpwm_set_duty(pwm_ch1, duty);
        break;

    case 2:
        mcpwm_set_duty(pwm_ch2, duty);
        break;

    case 3:
        mcpwm_set_duty(pwm_ch3, duty);
        break;
    }
#endif
}
/****************************************************************************************
**名称:动态处理任务
**功能:如果LED的模式为跳变，浙变，就启动这个任务
**说明:
**备注:
**日期:
*****************************************************************************************/
static void led_action_task_handle(void *p)
{
    static u32 err, ledlight_count1;
    static u8 count = 1, count2 = 1, count3 = 1, flag;
    static u8 change_count1 = 5, change_count2 = 1, duty_add_or_lessen = 1, change_count = 1;
    static u32 duty_step;

    static u8 dark_or_bright = 0;
    static u8 colour_step;

    while (1)
    {
        err = os_sem_pend(&LedActionSem, 0);
#ifdef MY_DEBUG
        printf("----------------running led_action_task\n");
#endif
        while (led_state.running_task == DYNAMIC_TASK)
        {
            //            printf("led_state.R_flag = %d,led_state.G_flag = %d,led_state.B_flag = %d\n",led_state.R_flag,led_state.G_flag,led_state.B_flag);
            switch (led_state.dynamic_state_flag)
            {
            case RED_GRADUAL_CHANGE: // 红色-白色渐变
            case BLUE_GRADUAL_CHANGE:
            case GREEN_GRADUAL_CHANGE:
            case CYAN_GRADUAL_CHANGE:
            case YELLOW_GRADUAL_CHANGE:
            case PURPLE_GRADUAL_CHANGE:
            case WHITE_GRADUAL_CHANGE:
                colour_gradual_change();
                break;
            case RED_GREEN_GRADUAL_CHANGE:
                red_green_gradual_change();
                break; // 红绿渐变

            case RED_BLUE_GRADUAL_CHANGE:
                red_blue_gradual_change();
                break; // 红蓝渐变

            case GREEN_BLUE_GRADUAL_CHANGE:
                blue_green_gradual_change();
                break; // 绿蓝渐变

            case THREE_GRADUAL_CHANGE:
                three_colour_gradual_change();
                break; // 三色渐变

            case SEVEN_GRADUAL_CHANGE:
                seven_colour_gradual_change();
                break; // 七色渐变

            case THREE_JUMP_CHANGE: // 三色跳变
            case SEVEN_JUMP_CHANGE:
                three_seven_colour_gradual_change();
                break; // 七色跳变

            case RED_FAST_FLASH:    // 红色频闪
            case BLUE_FAST_FLASH:   // 蓝色频闪
            case GREEN_FAST_FLASH:  // 绿色频闪
            case CYAN_FAST_FLASH:   // 青色频闪
            case YELLOW_FAST_FLASH: // 黄色频闪
            case PURPLE_FAST_FLASH: // 紫色频闪
            case WHITE_FAST_FLASH:
                colour_flash_twinkle();
                break; // 白色频闪

            case SEVEN_FAST_FLASH:
                seven_colour_flash_twinkle();
                break; // 七色频闪
            case SOUND_CTRL_THREE_GRADUAL:
                sound_ctrl_3_clr_gradual();
                break; // 声控三色渐变
            }
            //            printf("----------------exit led_action_task\n");
        }
    }
}
/****************************************************************************************
**名称:静态处理任务
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
u8 static_mode_cnt;
u8 static_mode[14] =
    {
        RED,
        GREEN,
        BLUE,
        YELLOW,
        CYAN,
        PURPLE,
        WHITE,
        ORANGE,
        BlueViolet,
        SpringGreen1,
        SkyBlue,
        RoyalBlue,
        OrangeRed,
        YellowGreen,
};

static void led_static_task_handle(void *p)
{
    u32 err;

    while (1)
    {
        err = os_sem_pend(&LedStaticSem, 0);
        led_state.W_flag = 0; // ZD0157-220528-SM-RGBW-Z修改
#ifdef MY_DEBUG
        printf("----------------running led_static_task\n");
#endif

        switch (led_state.static_state_flag)
        {
        case RED:
            led_state.R_flag = 0xff, led_state.G_flag = 0;
            led_state.B_flag = 0;
            break; // 红
        case GREEN:
            led_state.R_flag = 0, led_state.G_flag = 0xff;
            led_state.B_flag = 0;
            break; // 蓝
        case BLUE:
            led_state.R_flag = 0, led_state.G_flag = 0;
            led_state.B_flag = 0xff;
            break; // 绿
        case YELLOW:
            led_state.R_flag = 0xff, led_state.G_flag = 0xff;
            led_state.B_flag = 0;
            break; // 黄
        case CYAN:
            led_state.R_flag = 0, led_state.G_flag = 0xff;
            led_state.B_flag = 0xff;
            break; // 青
        case PURPLE:
            led_state.R_flag = 0xff, led_state.G_flag = 0;
            led_state.B_flag = 0xff;
            break; // 紫
        // ZD0157-220528-SM-RGBW-Z修改WHITE
        case WHITE:
            led_state.R_flag = 0, led_state.G_flag = 0;
            led_state.B_flag = 0;
            led_state.W_flag = 0xff;
            break; // 白
        case ORANGE:
            led_state.R_flag = 0xff, led_state.G_flag = 0x3f;
            led_state.B_flag = 0;
            break;
        case BlueViolet:
            led_state.R_flag = 138, led_state.G_flag = 43;
            led_state.B_flag = 226;
            break;
        case SpringGreen1:
            led_state.R_flag = 0, led_state.G_flag = 255;
            led_state.B_flag = 127;
            break;
        case SkyBlue:
            led_state.R_flag = 135, led_state.G_flag = 206;
            led_state.B_flag = 235;
            break;
        case RoyalBlue:
            led_state.R_flag = 65, led_state.G_flag = 105;
            led_state.B_flag = 225;
            break;
        case OrangeRed:
            led_state.R_flag = 255, led_state.G_flag = 69;
            led_state.B_flag = 0;
            break;
        case YellowGreen:
            led_state.R_flag = 154, led_state.G_flag = 205;
            led_state.B_flag = 50;
            break;

        case NO_LED:
            led_state.R_flag = 0, led_state.G_flag = 0;
            led_state.B_flag = 0;
            break;
        case CUSTOMIZE:
            break; // 白
        }
        display_led();
    }
}

/****************************************************************************************
**名称:红外遥控器按键处理任务
**功能:
**说明:有键按下就启动，作出相应的处理
**备注:
**日期:
*****************************************************************************************/
extern void RGB_Synchro(u8 r_flag, u8 g_flag, u8 b_flag);
u8 ir_lock;

void ir_key_tips(void);
static void ir_key_task_handle(void *p)
{
    int msg[4] = {0, 0, 0, 0};
    int res = 0;
    u8 key_val = 0xff;
    u8 count = 0;
    u8 tmp;
    while (1)
    {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg)); // 阻塞方式等待消息队例信息
        if (res != OS_TASKQ)
        {
            continue;
        }
        if (msg[0] == Q_USER)
            key_val = msg[1];

        printf("----------------running ir_key_task\n");
        if (key_val == IRKEY_LOCK)
        {
            ir_key_tips();
            ir_lock = !ir_lock;
        }
        if (ir_lock)
            continue;

        if (led_state.OpenorCloseflag == CLOSE_STATE && key_val == IRKEY_ON) // 关灯状态下接收到开灯命令
        {
            soft_turn_on_the_light(); // 软开灯

            OnOff_Synchro(led_state.OpenorCloseflag); // 同步开机
        }
        if (led_state.OpenorCloseflag == OPEN_STATE && key_val == IRKEY_OFF) // 开灯状态下接收到关灯命令
        {
            soft_rurn_off_lights();                   // 软关灯
            OnOff_Synchro(led_state.OpenorCloseflag); // 同步关机
        }

        /* 关闭本地声控功能。以下情况不关闭 */
        if (key_val != IRKEY_MUSIC1 &&
            // key_val != IRKEY_MUSIC3 &&
            key_val != IRKEY_MUSIC2 &&
            // key_val != IRKEY_MUSIC4 &&
            key_val != IRKEY_LIGHT_PLUS &&
            key_val != IRKEY_LIGHT_SUB &&
            key_val != IRKEY_SPEED_PLUS &&
            key_val != IRKEY_SPEED_SUB &&
            key_val != IRKEY_SPEED_PLUS &&
            // key_val != IRKEY_SEN_PLUS &&
            // key_val != IRKEY_SEN_SUB &&
            // key_val != IRKEY_1H &&
            // key_val != IRKEY_2H &&
            key_val != IRKEY_ON &&
            key_val != IRKEY_OFF)
        {
#ifdef MY_DEBUG
            printf("reset_local_music_task");

#endif
            reset_local_music_task();
        }
        /* 公共功能不关闭自动 */
        if (
            key_val != IRKEY_LIGHT_PLUS &&
            key_val != IRKEY_LIGHT_SUB &&
            key_val != IRKEY_SPEED_PLUS &&
            key_val != IRKEY_SPEED_SUB &&
            key_val != IRKEY_SPEED_PLUS &&
            // key_val != IRKEY_SEN_PLUS &&
            // key_val != IRKEY_SEN_SUB &&
            // key_val != IRKEY_1H &&
            // key_val != IRKEY_2H &&
            key_val != IRKEY_ON &&
            key_val != IRKEY_OFF)
        {
            set_ir_auto(IR_PAUSE);
        }

        printf("\n key_val=%d", key_val);

        if (led_state.OpenorCloseflag == OPEN_STATE)
        {
            switch (key_val)
            {
            // ****************************************遥控器第1行
            case IRKEY_LIGHT_PLUS:
                led_state.ledlight += 10;
                if (led_state.ledlight > Light_Max)
                {
                    led_state.ledlight = Light_Max;
                }
                if (led_state.running_task == STATIC_TASK)
                {
                    led_state.ledlight_temp = (led_state.ledlight - Light_Min) * 100 / (Light_Max - Light_Min);
                    switch_static_task(); // 为什么要推送多次,为了重设亮度
                    os_time_dly(1);
                    Light_Synchro(led_state.ledlight_temp); // 同步亮度
#ifdef MY_DEBUG
                    printf("led_state.ledlight =%d", led_state.ledlight);
#endif
                }
                else if (led_state.running_task == DYNAMIC_TASK)
                {
                    switch_dynamic_task();
                    os_time_dly(1);
                }

                break;
            case IRKEY_LIGHT_SUB:
                if (led_state.ledlight > 10)
                {
                    led_state.ledlight -= 10;
                }
                if (led_state.ledlight < Light_Min)
                {
                    led_state.ledlight = Light_Min;
                }
                if (led_state.running_task == STATIC_TASK)
                {
                    led_state.ledlight_temp = (led_state.ledlight - Light_Min) * 100 / (Light_Max - Light_Min);
                    switch_static_task();
                    Light_Synchro(led_state.ledlight_temp); // 同步亮度
#ifdef MY_DEBUG
                    printf("led_state.ledlight =%d", led_state.ledlight);
#endif
                }
                else if (led_state.running_task == DYNAMIC_TASK)
                {
                    switch_dynamic_task();
                    os_time_dly(1);
                }

                break;
                /*
case IRKEY_ON: //在Switch外层处理

    break;
case IRKEY_OFF://在Switch外层处理

    break;
    */
                // ****************************************遥控器第2行

            case IRKEY_R:
                led_state.static_state_flag = RED;
                switch_static_task();
                RGB_Synchro(led_state.R_flag, led_state.G_flag, led_state.B_flag);
                break; // 红

            case IRKEY_G:
                led_state.static_state_flag = GREEN;
                switch_static_task();
                RGB_Synchro(led_state.R_flag, led_state.G_flag, led_state.B_flag);
                break; // 绿
            case IRKEY_B:
                led_state.static_state_flag = BLUE;
                switch_static_task();
                RGB_Synchro(led_state.R_flag, led_state.G_flag, led_state.B_flag);
                break; // 蓝

            case IRKEY_W:
                led_state.static_state_flag = WHITE;
                switch_static_task();
                RGB_Synchro(led_state.R_flag, led_state.G_flag, led_state.B_flag);
                break; // 白
                // ****************************************遥控器第3行

            case IRKEY_ORANGE:
                led_state.static_state_flag = CUSTOMIZE;
                led_state.R_flag = 0xff, led_state.G_flag = 0x3f;
                led_state.B_flag = 0;
                switch_static_task();
                RGB_Synchro(led_state.R_flag, led_state.G_flag, led_state.B_flag);
                break;

            case IRKEY_B1: // 浅蓝色30,130,255
                led_state.static_state_flag = CUSTOMIZE;
                led_state.R_flag = 30, led_state.G_flag = 130;
                led_state.B_flag = 255;
                switch_static_task();
                RGB_Synchro(led_state.R_flag, led_state.G_flag, led_state.B_flag);
                break;

            case IRKEY_B2: // 深蓝色72,61,139
                led_state.static_state_flag = CUSTOMIZE;
                led_state.R_flag = 72, led_state.G_flag = 61;
                led_state.B_flag = 139;
                switch_static_task();
                RGB_Synchro(led_state.R_flag, led_state.G_flag, led_state.B_flag);
                break;

            case IRKEY_G1: // 浅绿色50,205,50
                led_state.static_state_flag = CUSTOMIZE;
                led_state.R_flag = 50, led_state.G_flag = 205;
                led_state.B_flag = 50;
                switch_static_task();
                RGB_Synchro(led_state.R_flag, led_state.G_flag, led_state.B_flag);
                break;
                // ****************************************遥控器第6行

            case IRKEY_YELLOW:
                led_state.static_state_flag = CUSTOMIZE;
                led_state.R_flag = 0xff, led_state.G_flag = 255;
                led_state.B_flag = 0;
                switch_static_task();
                RGB_Synchro(led_state.R_flag, led_state.G_flag, led_state.B_flag);
                break;

            case IRKEY_CYAN:
                led_state.static_state_flag = CUSTOMIZE;
                led_state.R_flag = 0, led_state.G_flag = 0xff;
                led_state.B_flag = 255;
                switch_static_task();
                RGB_Synchro(led_state.R_flag, led_state.G_flag, led_state.B_flag);
                break;

            case IRKEY_PURPLE:
                led_state.static_state_flag = CUSTOMIZE;
                led_state.R_flag = 255, led_state.G_flag = 0;
                led_state.B_flag = 0xff;
                switch_static_task();
                RGB_Synchro(led_state.R_flag, led_state.G_flag, led_state.B_flag);
                break;

            // ****************************************遥控器第7行

            // case IRKEY_JUMP3:   led_state.dynamic_state_flag = THREE_JUMP_CHANGE;     Mode_Synchro(led_state.dynamic_state_flag);  switch_dynamic_task();   break;
            // case IRKEY_FADE3:   led_state.dynamic_state_flag = THREE_GRADUAL_CHANGE;  Mode_Synchro(led_state.dynamic_state_flag);  switch_dynamic_task();   break;
            // case IRKEY_FLASH:   led_state.dynamic_state_flag = SEVEN_FAST_FLASH;  Mode_Synchro(led_state.dynamic_state_flag);  switch_dynamic_task();   break;
            case IRKEY_SPEED_PLUS:
                if (led_state.running_task == DYNAMIC_TASK)
                {
                    led_state.speed += 1;
                    if (led_state.speed > 8)
                    {
                        led_state.speed = 8;
                    }

                    Speed_Synchro(led_state.speed); // 同步速度
                }
                break;
            // ****************************************遥控器第8行
            // case IRKEY_JUMP7:  led_state.dynamic_state_flag = SEVEN_JUMP_CHANGE;     Mode_Synchro(led_state.dynamic_state_flag);  switch_dynamic_task();   break;
            case IRKEY_AUTO:
                set_ir_auto(IR_AUTO);
                break;
                // case IRKEY_FADE7:
                //     led_state.dynamic_state_flag = SEVEN_GRADUAL_CHANGE;
                //     Mode_Synchro(led_state.dynamic_state_flag);
                //     switch_dynamic_task();
                //     break;

            case IRKEY_SPEED_SUB:
                if (led_state.running_task == DYNAMIC_TASK)
                {
                    led_state.speed -= 1;
                    if (led_state.speed < 1)
                    {
                        led_state.speed = 1;
                    }

                    Speed_Synchro(led_state.speed); // 同步速度
                }
                break;
            // ****************************************遥控器第9行
            case IRKEY_MUSIC1:
                set_local_mic_mode(MIC_SPEED);
                set_local_music_task();
                break;
            // case IRKEY_MUSIC3:
            //     set_local_mic_mode(MIC_CHANGE_MODE);
            //     set_local_music_task();

            // break;
            // case IRKEY_SEN_PLUS:
            //     locla_mic_sen_plus();
            // break;
            // case IRKEY_1H:
            //     if(get_ir_timer() != IR_TIMER_1)
            //     {
            //         set_ir_timer(IR_TIMER_1);
            //         ir_key_tips();
            //     }

            // break;

            // ****************************************遥控器第10行
            case IRKEY_MUSIC2:
                set_local_mic_mode(MIC_CHANGE_MODE);
                set_local_music_task();

                break;

            // case IRKEY_MUSIC4:
            //     set_local_mic_mode(MIC_CHANGE_CLR);
            //     set_local_music_task();

            // break;
            // case IRKEY_SEN_SUB:
            // locla_mic_sen_sub();
            // break;
            // case IRKEY_2H:
            //     if(get_ir_timer() != IR_TIMER_2)
            //     {
            //         set_ir_timer(IR_TIMER_2);
            //         ir_key_tips();
            //     }
            // break;
            case IRKEY_MODE_ADD:
                extern void mode_updata(void);
                ir_mode_plus();
                mode_updata();
                break;
                break;

            case IRKEY_MODE_DEC:
                extern void mode_updata(void);
                ir_mode_sub();
                mode_updata();
                break;

            case IRKEY_COLOR:
                extern u8 static_mode[14];
                extern u8 static_mode_cnt;
                led_state.static_state_flag = static_mode[static_mode_cnt];
                static_mode_cnt++;
                static_mode_cnt %= 14;
                switch_static_task();
                RGB_Synchro(led_state.R_flag, led_state.G_flag, led_state.B_flag);
                break;
            }
            LED_state_write_flash();
        }
        else if (led_state.OpenorCloseflag == CLOSE_STATE)
        {
            if (key_val == IRKEY_W) // 按3下白色键后进入相序调整模式
            {
                count++;
                printf("count = %d\n", count);
                if (count >= 3)
                {
                    count = 0;
                    led_state.running_task = PHASE_SEQUENCE;
                    os_sem_post(&phaseSequenceSem);
                }
            }
            else
                count = 0;
        }
        //       printf("----------------exit ir_key_task\n");
    }
}

// 设置定时，灵敏度，灯具效果没有反应，这个函数实现提示作用
// 如果是静态效果跳到红光200ms，亮度10%
// 如果是动态效果跳到红光400ms，亮度10%

void ir_key_tips(void)
{
    u8 effect_tmp;
    u8 brightness;
    LED_STATE tmp_state;
    if (led_state.running_task == DYNAMIC_TASK)
    {
        /* 备份效果 */
        effect_tmp = led_state.dynamic_state_flag;
        brightness = led_state.ledlight;
        /* 提示效果 */
        led_state.static_state_flag = RED;
        led_state.ledlight = 25;
        switch_static_task();
        os_time_dly(20);
        /* 恢复效果 */
        led_state.ledlight = brightness;
        switch_dynamic_task();
    }
    else if (led_state.running_task == STATIC_TASK)
    {
        /* 备份变量 */
        memcpy((u8 *)&tmp_state, (u8 *)&led_state, sizeof(LED_STATE));
        /* 提示效果 */
        led_state.static_state_flag = RED;
        led_state.ledlight = 25;
        switch_static_task();
        os_time_dly(20);
        memcpy((u8 *)&led_state, (u8 *)&tmp_state, sizeof(LED_STATE));
        switch_static_task();
    }
}
/****************************************************************************************
**名称:灯条刷新任务(弃用)
**功能:
**说明:通过颜色+亮度计算出RGB的占空比，并刷新，刷新频率是50Hz
**备注:
**日期:
*****************************************************************************************/
/*
void led_refresh_task_handle(void*p)
{
    u32 err;
    static u32 redled_duty,blueled_duty,greenled_duty;

    timer_pwm_init(JL_TIMER0, IO_PORTA_02, 100, 2000);  //用timer0做pwm

    user_timer_init();      //定时器2设置
    led_state_init();       //初始化LED状态参数
    led_gpio_init();        //RGB控制脚初始化

//    capture_cycle_time_init();    //用定时器3作捕获

    os_sem_create(&LedStaticSem,0);     //创建静态信号量
    os_sem_create(&LedActionSem,0);     //创建动态信号量

    task_create(led_action_task_handle, NULL, "led_action");            //创建LED动态操作任务
    task_create(led_static_task_handle, NULL, "led_static");            //创建LED静态操作任务
    task_create(ir_key_task_handle, NULL, "ir_key");                    //创建红外遥控处理任务
    task_create(phase_sequence_task_handle, NULL, "phase_sequence");    //创建遥控器调整相序任务
    task_create(music_led_task_handle, NULL, "music_led");              //创建IOS音乐律动任务
    task_create(mic_led_task_handle, NULL, "mic_led");                  //创建IOS麦克风任务
    task_create(android_led_task_handle, NULL, "android_led");          //创建安卓版音乐与麦克风任务
    task_create(alarm_clock_task_handle, NULL, "alarm_clock");          //创建闹钟任务


    if(led_state.running_task == DYNAMIC_TASK)
        os_sem_post(&LedActionSem);
    else if(led_state.running_task == STATIC_TASK)
        os_sem_post(&LedStaticSem);

//    printf("----------------running led_refresh_task\n");
    while(1)
    {
        redled_duty = led_state.R_flag*led_state.ledlight*100/255;
        blueled_duty = led_state.G_flag*led_state.ledlight*100/255;
        greenled_duty = led_state.B_flag*led_state.ledlight*100/255;

        Set_Duty(redled_duty,blueled_duty,greenled_duty);

        os_time_dly(1);
    }
}
*/
/****************************************************************************************
**名称:遥控器调整相序任务
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
/*
static void phase_sequence_task_handle(void*p)
{
    u32 err;
    u8 count,temp;

    os_sem_create(&phaseSequenceSem,0);         //创建调整相序信号量

    while(1)
    {
        err = os_sem_pend(&phaseSequenceSem, 0);
 //       printf("----------------running phase_sequence_task\n");
        LED_ON;
        count = 1;
        temp = 0;
        led_state.ledlight = 100;
        led_state.interface_mode = LED_R_G_B;
        while(led_state.running_task == PHASE_SEQUENCE)
        {
            switch(count)
            {
                case 1:  temp = 1;   led_state.R_flag = 0xff, led_state.G_flag = 0;     led_state.B_flag = 0;           break;
                case 2:  temp = 2;   led_state.R_flag = 0,    led_state.G_flag = 0xff;  led_state.B_flag = 0;           break;
                case 3:  temp = 3;   led_state.R_flag = 0,    led_state.G_flag = 0;     led_state.B_flag = 0xff;        break;
                case 4:  temp = 4;   led_state.R_flag = 0,    led_state.G_flag = 0;     led_state.B_flag = 0;           break;
                case 5:  temp = 5;   led_state.R_flag = 0xff, led_state.G_flag = 0;     led_state.B_flag = 0;           break;
                case 6:  temp = 6;   led_state.R_flag = 0,    led_state.G_flag = 0xff;  led_state.B_flag = 0;           break;
                case 7:  temp = 7;   led_state.R_flag = 0,    led_state.G_flag = 0;     led_state.B_flag = 0xff;        break;
                case 8:  temp = 8;   led_state.R_flag = 0,    led_state.G_flag = 0;     led_state.B_flag = 0;           break;
            }
            display_led();
            count++;
            if(count>8)
                count = 1;
            os_time_dly(150);
        }
        switch(temp)
        {
            case 1:     save_led_state.interface_mode = LED_R_G_B;   break;
            case 2:     save_led_state.interface_mode = LED_G_R_B;   break;
            case 3:     save_led_state.interface_mode = LED_B_R_G;   break;
            case 5:     save_led_state.interface_mode = LED_R_B_G;   break;
            case 6:     save_led_state.interface_mode = LED_G_B_R;   break;
            case 7:     save_led_state.interface_mode = LED_B_G_R;   break;
            default:    break;
        }
        save_led_state.running_task  = STATIC_TASK;
        save_led_state.static_state_flag = RED;
        soft_turn_on_the_light();   //软开灯
//        printf("----------------exit phase_sequence_task\n");
    }-
}*/
/****************************************************************************************
**名称:红，蓝，绿，灭，颜色跳变
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
void colour_jump_change(void)
{
    u8 count;

    ir_detect_isr_en = 0; // 停止按键检测

    for (count = 0; count < 4; count++)
    {
        switch (count)
        {
        case 0:
            led_state.R_flag = 0xff, led_state.G_flag = 0;
            led_state.B_flag = 0;
            break;
        case 1:
            led_state.R_flag = 0, led_state.G_flag = 0xff;
            led_state.B_flag = 0;
            break;
        case 2:
            led_state.R_flag = 0, led_state.G_flag = 0;
            led_state.B_flag = 0xff;
            break;
        case 3:
            led_state.R_flag = 0, led_state.G_flag = 0;
            led_state.B_flag = 0;
            break;
        }
        display_led();
        os_time_dly(80);
    }
    ir_detect_isr_en = 1; // 打开按键检测
}
void led_white_twinkle(u8 count)
{
    u8 i;

    ir_detect_isr_en = 0; // 停止按键检测

    for (i = 0; i < count; i++) // 进入后先闪烁3次
    {
        led_state.R_flag = 0xff;
        led_state.G_flag = 0xff;
        led_state.B_flag = 0xff;
        led_state.ledlight = 100;
        display_led();
        os_time_dly(50);
        led_state.R_flag = 0;
        led_state.G_flag = 0;
        led_state.B_flag = 0;
        led_state.ledlight = 100;
        display_led();
        os_time_dly(50);
    }
    ir_detect_isr_en = 1; // 打开按键检测
}
/****************************************************************************************
**名称:遥控器调整相序任务
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
static void phase_sequence_task_handle(void *p)
{
    //    int msg[8] = {0,0,0,0};
    //    int res = 0;
    u32 err;
    u8 i, interface_mode;

    os_sem_create(&phaseSequenceSem, 0);    // 创建调整相序信号量
    os_sem_create(&phaseSequenceKeySem, 0); // 创建调整相序按键信号量

    while (1)
    {
        err = os_sem_pend(&phaseSequenceSem, 0);
        //        printf("----------------running phase_sequence_task\n");
        LED_ON;

        led_white_twinkle(3);

        led_state.ledlight = 100;
        interface_mode = led_state.interface_mode;
        led_state.interface_mode = 5;
        while (led_state.running_task == PHASE_SEQUENCE)
        {
            err = os_sem_pend(&phaseSequenceKeySem, 0);

            switch (Ir_Key_Val)
            {
            case IRKEY_W:
                //    printf("IRKEY_WHITE\n");
                led_state.interface_mode++;
                if (led_state.interface_mode >= 6)
                    led_state.interface_mode = 0;
                //    printf("interface_mode = %d\n",led_state.interface_mode);
                colour_jump_change();
                break;
            case IRKEY_ON:
                //    printf("IRKEY_ON\n");
                led_white_twinkle(2);
                led_state.running_task = STATIC_TASK;
                led_state.static_state_flag = RED;
                save_led_state.interface_mode = led_state.interface_mode;
                soft_turn_on_the_light(); // 软开灯
                break;
            case IRKEY_OFF:
                //    printf("IRKEY_OFF\n");
                led_state.running_task = NO_TASK;
                led_state.interface_mode = interface_mode; // 退出，恢复原来的相序
                LED_OFF;
                break;
            }
        }
    }
}
/****************************************************************************************
**名称:IOS音乐律动任务
**功能:阻塞等待APP发能量值过来，再根据能量值做操作
**说明:
**备注:
**日期:
*****************************************************************************************/
/*
static void music_led_task_handle(void*p)
{
    u32 err;
    int msg[9] = {0,0,0,0,0,0,0,0,0};
    int res = 0;
    u8 R_flag,G_flag,B_flag,light,colour_step;
    u32 redled_duty,blueled_duty,greenled_duty;
    u8 time_count=1,last_music_val,val;
    u32 music_val;

    colour_step = 1;
    light = 100;
 //   printf("----------------running music_led_task\n");
    while(1)
    {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));  //阻塞方式等待消息队例信息
        if (res != OS_TASKQ) {
            continue;
        }
        if(msg[0] == Q_USER)
            music_val = msg[1];

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

        redled_duty = R_flag*light*100/255;
        blueled_duty = G_flag*light*100/255;
        greenled_duty = B_flag*light*100/255;
        Set_Duty(redled_duty,blueled_duty,greenled_duty);

        if(music_val > 30)
        {
            if(music_val > last_music_val)
            {
                val = music_val - last_music_val;
                if(val > 5)
                {
                    colour_step++;
                    if(colour_step>7)
                        colour_step = 1;
                }
            }
        }
        last_music_val = music_val;

        time_count++;
        if(time_count > 2)
        {
            time_count = 1;

            if(music_val >= 20 && music_val < 30)
                music_val = (music_val-20)*5;
            else if(music_val >= 30 && music_val < 40)
                music_val = 5+(music_val-30)*5;
            else if(music_val >= 40 && music_val < 50)
                music_val = 10+(music_val-40)*6;
            else if(music_val >= 50 && music_val < 60)
                music_val = 15+(music_val-50)*6;
            else if(music_val >= 60 && music_val < 70)
                music_val = 20+(music_val-60)*7;
            else if(music_val >= 70 && music_val < 80)
                music_val = 30+(music_val-70)*7;
            else
                music_val = 0;
            light = music_val;
        }
        else
            if(light > 20)
                light -= 10;
    }
}
*/
/****************************************************************************************
**名称:IOS麦克风任务
**功能:阻塞等待APP发能量值过来，再根据能量值做操作
**说明:
**备注:
**日期:
*****************************************************************************************/
/*
static void mic_led_task_handle(void*p)
{
    u32 err;
    u8 R_flag,G_flag,B_flag,light,colour_step;
    u32 redled_duty,blueled_duty,greenled_duty;
    u8 time_count=1,last_mic_val,val;

    colour_step = 1;
    light = 100;
 //   printf("----------------running mic_led_task\n");
    while(1)
    {
//        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));  //阻塞方式等待消息队例信息
//        if (res != OS_TASKQ) {
//            continue;
//        }
//       if(msg[0] == Q_USER)
//            music_val = msg[1];

        err = os_sem_pend(&mic_led_Sem, 0);

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
        redled_duty = R_flag*light*100/255;
        blueled_duty = G_flag*light*100/255;
        greenled_duty = B_flag*light*100/255;
        Set_Duty(redled_duty,blueled_duty,greenled_duty);

        if(mic_val >= 40)
        {
            if(mic_val > last_mic_val)
            {
                val = mic_val - last_mic_val;
                if(val > 30)
                {
                    colour_step++;
                    if(colour_step>7)
                        colour_step = 1;
                }
            }
            last_mic_val = mic_val;
        }
        time_count++;
        if(time_count > 2)
        {
            time_count = 1;

            if(mic_val >= 40 && mic_val <= 80)
            {
                if(mic_val > last_mic_val)
                {
                    val = mic_val - last_mic_val;
                    if(val > 30)
                    {
                        colour_step++;
                        if(colour_step>7)
                            colour_step = 1;
                    }
                }
                last_mic_val = mic_val;
                mic_val = 10+(mic_val-40)*90/40;

            }
            else
                mic_val = 0;
            light = mic_val;
        }
    }
}
*/
/****************************************************************************************
**名称:IOS麦克风任务
**功能:阻塞等待APP发能量值过来，再根据能量值做操作
**说明:
**备注:
**日期:
*****************************************************************************************/
/*
void mic_led_task_handle(void*p)
{
    int res = 0;
    int msg[9] = {0,0,0,0,0,0,0,0,0};
    u8 tim=5,op_time,count=0,Sound_val;
    u32 h_val,s_val,v_val,r_val,g_val,b_val,r_step,g_step,b_step,sond_last;
    u8 R_flag,G_flag,B_flag,colour_step;
    u32 redled_duty,blueled_duty,greenled_duty;

    colour_step = 1;
    printf("----------------running mic_led_task\n");
    while(1)
    {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));  //阻塞方式等待消息队例信息
        if (res != OS_TASKQ) {
            continue;
        }
        if(msg[0] == Q_USER)
        {
            Sound_val = msg[1];
            led_state.running_task = MIC_LED;
        }
        else
            continue;

        r_val=0;
        g_val=0;
        b_val=0;
        r_step=0;
        g_step=0;
        b_step=0;
        count=0;

        if(Sound_val>50)
            {
                if(Sound_val >= sond_last+20)
                {
                    colour_step++;
                    if(colour_step>6)
                        colour_step=0;
                }
                sond_last = Sound_val;
                switch(colour_step)
                {
                    case 0:     R_flag = 0xff, G_flag = 0;     B_flag = 0;       break;            //红
                    case 1:     R_flag = 0,    G_flag = 0xff;  B_flag = 0;       break;            //蓝
                    case 2:     R_flag = 0,    G_flag = 0;     B_flag = 0xff;    break;            //绿
                    case 3:     R_flag = 0xff, G_flag = 0xff;  B_flag = 0;       break;            //黄
                    case 4:     R_flag = 0,    G_flag = 0xff;  B_flag = 0xff;    break;            //青
                    case 5:     R_flag = 0xff, G_flag = 0;     B_flag = 0xff;    break;            //紫
                    case 6:     R_flag = 0xff, G_flag = 0xff;  B_flag = 0xff;    break;            //白
                }

                r_val = R_flag*100*Sound_val/100;
                g_val = G_flag*100*Sound_val/100;
                b_val = B_flag*100*Sound_val/100;

                r_step = r_val/tim;
                g_step = g_val/tim;
                b_step = b_val/tim;
            }

        while(led_state.running_task == MIC_LED)
        {
            if(Sound_val>50)
            {
                if(r_val >= r_step)
                    r_val -= r_step;

                if(g_val >= g_step)
                    g_val -= g_step;

                if(b_val >= b_step)
                    b_val -= b_step;

                redled_duty = r_val*led_state.ledlight/255;
                blueled_duty = g_val*led_state.ledlight/255;
                greenled_duty = b_val*led_state.ledlight/255;
                Set_Duty(redled_duty,blueled_duty,greenled_duty);
            }
            else
            {
                count++;
                if(count>10)
                {
                    count = 0;
                    redled_duty = rand() % 100;
                    blueled_duty = rand() % 100;
                    greenled_duty = rand() % 100;
                    Set_Duty(redled_duty,blueled_duty,greenled_duty);
                }

            }
            os_time_dly(1);
        }
    }
}
*/
/****************************************************************************************
**名称:安卓版的音乐律动和麦克风任务（IOS和安卓统一用这个）
**功能:APP直接发RGB数值过来，不需要处理，直接在灯条上显示即可
**说明:
**备注:
**日期:
*****************************************************************************************/
static void android_led_task_handle(void *p)
{
    int msg[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    int res = 0;
    u32 redled_duty, blueled_duty, greenled_duty, light_dat;
    u8 time_count = 1, last_music_val, val;
    u32 music_val;

    //   printf("----------------running android_led_task\n");
    while (1)
    {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg)); // 阻塞colour_gradual_change信息
        if (res != OS_TASKQ)
        {
            continue;
        }
        if (msg[0] == Q_USER)
        {
            redled_duty = msg[1] * ((Light_Max - Light_Min) * msg[4] / 100 + Light_Min) * 100 / 255;
            blueled_duty = msg[2] * ((Light_Max - Light_Min) * msg[4] / 100 + Light_Min) * 100 / 255;
            greenled_duty = msg[3] * ((Light_Max - Light_Min) * msg[4] / 100 + Light_Min) * 100 / 255;
            Set_Duty(redled_duty, blueled_duty, greenled_duty);
        }

        //        printf("R_flag = %d,G_flag = %d,B_flag = %d,ledlight = %d",led_state.R_flag,led_state.G_flag,led_state.B_flag,led_state.ledlight);
    }
}
/****************************************************************************************
**名称:闹钟任务
**功能:上电后创建任务，创建任务后先发指令获取当前时间，然后计时
**说明:
**备注:
**日期:
*****************************************************************************************/
static void alarm_clock_task_handle(void *p)
{

    /*--------设置初始时间----------*/
    time_clock.hour = 12;
    time_clock.minute = 0;
    time_clock.second = 0;
    time_clock.week = 1;

    alarm_clock[0].on_off = 0;
    alarm_clock[0].hour = 0;
    alarm_clock[0].minute = 0;
    alarm_clock[0].mode = 0;

    alarm_clock[1].on_off = 0;
    alarm_clock[1].hour = 0;
    alarm_clock[1].minute = 0;
    alarm_clock[1].mode = 0;

    alarm_clock[2].on_off = 0;
    alarm_clock[2].hour = 0;
    alarm_clock[2].minute = 0;
    alarm_clock[2].mode = 0;

    while (1)
    {
        // printf("running alarm_clock_task>>>>>\n");

        os_time_dly(100); // 延时一秒
        ir_auto_change_mode();
        ir_timer_handle();
        /*------计时-------*/
        time_clock.second++;
        if (time_clock.second >= 60)
        {
            time_clock.second = 0;
            time_clock.minute++;
            if (time_clock.minute >= 60)
            {
                time_clock.minute = 0;
                time_clock.hour++;
                if (time_clock.hour >= 24)
                {
                    time_clock.hour = 0;
                    time_clock.week++;
                    if (time_clock.week >= 8)
                        time_clock.week = 1;
                }
            }
        }
        if (alarm_clock[0].on_off == 0x80) // 第一个闹钟开启
        {
            if (alarm_clock[0].hour == time_clock.hour && alarm_clock[0].minute == time_clock.minute && time_clock.second == 0 && ((alarm_clock[0].mode >> (time_clock.week - 1)) & 0x01)) // 定时时间到
            {
                if (alarm_clock[0].mode & 0x80)
                    soft_turn_on_the_light();
                else
                    soft_rurn_off_lights();
            }
        }
        if (alarm_clock[1].on_off == 0x80) // 第二个闹钟开启
        {
            if (alarm_clock[1].hour == time_clock.hour && alarm_clock[1].minute == time_clock.minute && time_clock.second == 0 && ((alarm_clock[1].mode >> (time_clock.week - 1)) & 0x01)) // 定时时间到
            {
                if (alarm_clock[1].mode & 0x80)
                    soft_turn_on_the_light();
                else
                    soft_rurn_off_lights();
            }
        }
        if (alarm_clock[2].on_off == 0x80) // 第二个闹钟开启
        {
            if (alarm_clock[2].hour == time_clock.hour && alarm_clock[2].minute == time_clock.minute && time_clock.second == 0 && ((alarm_clock[2].mode >> (time_clock.week - 1)) & 0x01)) // 定时时间到
            {
                if (alarm_clock[2].mode & 0x80)
                    soft_turn_on_the_light();
                else
                    soft_rurn_off_lights();
            }
        }
        //        printf("hour = %d,minute = %d,second = %d,week = %d\n",time_clock.hour,time_clock.minute,time_clock.second,time_clock.week);
        //        printf("alarm_clock[0].on_off = %d,alarm_clock[0].hour = %d,alarm_clock[0].minute = %d,alarm_clock[0].mode = %d\n",alarm_clock[0].on_off,alarm_clock[0].hour,alarm_clock[0].minute,alarm_clock[0].mode);
        //        printf("alarm_clock[1].on_off = %d,alarm_clock[1].hour = %d,alarm_clock[1].minute = %d,alarm_clock[1].mode = %d\n",alarm_clock[1].on_off,alarm_clock[1].hour,alarm_clock[1].minute,alarm_clock[1].mode);
    }
}
/****************************************************************************************
**名称:应用主程序
**功能:用于应用初始化及创建各个任务
**说明:
**备注:
**日期:
*****************************************************************************************/
extern OS_SEM local_music_task_sem;
extern void check_mic_sound(void);

#include "led_strand_effect.h"
extern u16 check_mic_adc(void);
u8 music_trigger = 0;
// u32 adc,adc_av;
#define SAMPLE_N 20
u8 adc_v_n, adc_avrg_n, adc_total_n;
u32 adc_sum = 0, adc_sum_n = 0;
extern uint8_t met_trg;
extern uint8_t trg_en;
extern void set_music_oc_trg(u8 p);
u8 i, j;
u32 adc, adc_av, adc_all;
u16 adc_v[SAMPLE_N]; // 记录20个ADC值
u32 adc_avrg[10];    // 记录5个平均值
u32 adc_total[15];   // __attribute__((aligned(4)));

// 声控
void sound_handle(void)
{
    extern u32 adc_get_value(u32 ch);
    extern void WS2812FX_trigger();
    u16 adc;
    u8 i, trg, trg_v;
    u32 adc_all, adc_ttl;

    extern u32 adc_sample(u32 ch);
    // 记录adc值
    adc = adc_get_value(AD_CH_PA8);

    // adc = adc_sample(AD_CH_PA8);
    // printf("adc = %d", adc);

    if (adc < 1000) // 当ADC值大于1000，说明硬件电路有问题
    {

        if (adc_sum_n < 2000)
        {
            adc_sum_n++;
        }
        if (adc_sum_n == 2000)
        {
            if (adc / (adc_sum / adc_sum_n) > 3)
                return; // adc突变，大于平均值的3倍，丢弃改值
            adc_sum = adc_sum - adc_sum / adc_sum_n;
        }
        adc_sum += adc;

        adc_v_n %= SAMPLE_N;
        adc_v[adc_v_n] = adc;
        adc_v_n++;
        adc_all = 0;
        for (i = 0; i < SAMPLE_N; i++)
        {
            adc_all += adc_v[i];
        }

        adc_avrg_n %= 10;
        adc_avrg[adc_avrg_n] = adc_all / SAMPLE_N;
        adc_avrg_n++;
        // printf("%d,",adc_all / SAMPLE_N);
        adc_ttl = 0;
        for (i = 0; i < 10; i++)
        {
            adc_ttl += adc_avrg[i];
        }
        memmove((u8 *)adc_total, (u8 *)adc_total + 4, 14 * 4);
        adc_total[14] = adc_ttl / 10; // 总数平均值

        // 查找峰值
        trg = 0;

        {
            if (adc_sum_n != 0)
            {
                extern void set_mss(uint16_t s);
                set_mss(adc + (adc)*fc_effect.music.s / 100);
                if (adc * fc_effect.music.s / 100 > adc_sum / adc_sum_n)
                {
                    // printf("\n adc=%d",adc);
                    // printf("\n adc_sum/adc_sum_n=%d",adc_sum/adc_sum_n);
                    music_trigger = 1;
                }
            }
        }
    }
}

extern int ct_uart_init_a(u32 baud);
extern void test_uart_a(void);
// 10ms调用一次
void main_while(viod)
{
    u8 i;
    extern void run_tick_per_10ms(void);
    extern void WS2812FX_service();
    extern u8 get_sound_result(void);
    extern void meteor_period_sub(void);
    extern void rf24_key_handle();
    // rf433_handle(&i);
    // while(1)
    {
        run_tick_per_10ms();
        sound_handle();
        // rf433_long_timer();
        rf24_key_handle();
        rf24g_long_timer();

        // test_uart_a();
        uart_key_handle();
        // check_mic_sound();
        WS2812FX_service();
        meteor_period_sub();
        // os_time_dly(1);
    }
}

void led_main(void)
{
    // os_sem_create(&LedStaticSem,0);     //创建静态信号量

    // task_create(main_while, NULL, "led_static");            //创建LED静态操作任务

    // led_gpio_init();        //RGB控制脚初始化
    // led_pwm_init();
    extern void fc_data_init(void);
    extern void full_color_init(void);
    extern void led_state_init(void);
    extern void led_pwr_on(void);

    user_timer_init(); // 定时器2设置
    // fc_data_init();      //第一次上电初始化，放读flash
    extern void read_flash_device_status_init(void);
    read_flash_device_status_init();
    led_state_init(); // 初始化LED接口
    full_color_init();
    mic_gpio_init();
    led_pwr_on();
    // #if TCFG_RF433_ENABLE
    // extern void rf433_gpio_init(void);
    // rf433_gpio_init();
    // #endif

    ct_uart_init_a(9600);
    // os_sem_post(LedStaticSem);
    // 1ms调用
    sys_timer_add(NULL, main_while, 1);
}
/****************************************************************************************
**名称:输出显示
**功能:把led_state.R_flag，led_state.G_flag，led_state.B_flag，led_state.ledlight转成占空比输出
**说明:
**备注:
**日期:
*****************************************************************************************/
void display_led(void)
{
    u32 redled_duty, blueled_duty, greenled_duty;
    if (led_state.W_flag == 0)
    {
        redled_duty = led_state.R_flag * led_state.ledlight * 100 / 255;
        blueled_duty = led_state.G_flag * led_state.ledlight * 100 / 255;
        greenled_duty = led_state.B_flag * led_state.ledlight * 100 / 255;
        Set_Duty(redled_duty, blueled_duty, greenled_duty);
    }
    else
    {
        Set_Duty(0, 0, 0);
        mcpwm_set_duty(3, led_state.W_flag * led_state.ledlight * 100 / 255);
    }

#ifdef MY_DEBUG
    printf("\n redled_duty=%d,blueled_duty=%d,greenled_duty=%d", redled_duty, blueled_duty, greenled_duty);
#endif
    //    GCLight_PWM_SetDuty(redled_duty,blueled_duty,greenled_duty);
}
