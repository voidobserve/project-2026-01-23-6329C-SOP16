// #include "led_strip_sys.h"
#include "key_app.h"
#include "task.h"
#include "app_synchro.h"
#include "app_main.h"
// @key_app.c
// 实现以下功能APP
// 物理按键
// AD按键
// 红外遥控

/***********************************************************移植修改*******************************************************************/
/*
    *修改ir_auto_change_mode()
    -修改切换模式函数
*/
// 遥控一共5自动mode
#define IR_MODE_QTY 14

// extern OS_SEM ir_task_sem;                     //遥控任务信号量

// 输入ir_scan_mode，用于遥控自动模式，模式+，模式-
const u8 ir_auto_mode[IR_MODE_QTY] = 
{
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

/***********************************************************IR_KEY*******************************************************************/

//  计数器，遥控自动模式，模式+,-计数
u8 ir_mode_cnt;
ir_auto_e ir_auto_f = IR_PAUSE; 
// 自动模式下，改变模式计数器
u16 ir_auto_change_tcnt = 0;
/* 定时器状态 */
ir_timer_e ir_timer_state ;
/* 定时计数器,累减,单位S */
u16 ir_time_cnt;

/***********************************************************API*******************************************************************/

// -----------------------遥控开启定时功能
// 通过遥控设置，怎么关闭？
void set_ir_timer(ir_timer_e timer)
{
    if(timer <= IR_TIMER_2)
    {
        ir_timer_state = timer;
        if(ir_timer_state == IR_TIMER_0)
        {
            ir_time_cnt = 0;
        }
        else if(ir_timer_state == IR_TIMER_1)
        {
            ir_time_cnt = 60*60;
            // ir_time_cnt = 5;

        }
        else if(ir_timer_state == IR_TIMER_2)
        {
            ir_time_cnt = 2*60*60;
            // ir_time_cnt = 10;

        }
    }
}

ir_timer_e get_ir_timer(void)
{
    return ir_timer_state;
}

// ----------------------------------------自动
void set_ir_auto(ir_auto_e auto_f)
{
    if(auto_f <= IR_PAUSE)
    {
        ir_auto_f = auto_f;
        #ifdef MY_DEBUG
        printf("\r set_ir_auto OK");
        #endif
    }
    else
    {
        ir_auto_f = IR_PAUSE;
    }

}

ir_auto_e get_ir_auto(void)
{
    return ir_auto_f;
}

// 返回遥控器操作的模式
u8 get_ir_mode(void)
{
    return ir_auto_mode[ir_mode_cnt];
}
// 每次调用，循环递增模式，到IR_MODE_QTY从0开始
void ir_mode_plus(void)
{
    ir_mode_cnt++;
    ir_mode_cnt %= IR_MODE_QTY;
}

// 每次调用，循环递减模式，到0从IR_MODE_QTY开始
void  ir_mode_sub(void)
{
    if(ir_mode_cnt>0)
    {
        ir_mode_cnt--;
    }
    else
    {
        ir_mode_cnt = IR_MODE_QTY-1;
    }
}

// ----------------------------------------模式
void mode_updata(void)
{
    led_state.dynamic_state_flag = ir_auto_mode[ ir_mode_cnt ];     
    Mode_Synchro(led_state.dynamic_state_flag);  
    led_state.running_task = NO_TASK;
    os_time_dly(10);
    switch_dynamic_task();   
}

/***********************************************************APP*******************************************************************/
// 由alarm_clock_task_handle调用，频度1秒一次
void ir_timer_handle(void)
{
    if(ir_timer_state == IR_TIMER_1 || ir_timer_state == IR_TIMER_2)
    {
        if(ir_time_cnt > 0)
        {
            ir_time_cnt--;
            #ifdef MY_DEBUG
            printf("\r ir_time_cnt--");
            #endif
        }
        else
        {
            if(led_state.OpenorCloseflag == OPEN_STATE)
            {
                set_ir_timer(IR_TIMER_0);
                soft_rurn_off_lights();     //软关灯
                OnOff_Synchro(led_state.OpenorCloseflag);   //同步关机
                #ifdef MY_DEBUG
                printf("\r timer ok");
                #endif
            }
        }
    }
}


extern void adkey_mode_change(u8 light_mode);
#define IR_CHANGE_MODE_T    (10)     //单位1s，用于自动模式，10秒换一个模式

// 在alarm_clock_task_handle调用
extern LED_STATE led_state;
void ir_auto_change_mode(void)
{
    // u32 err;
    // 循环模式
    if(ir_auto_f == IR_AUTO)
    {
        // err = os_sem_pend(&ir_task_sem, IR_CHANGE_MODE_T);      //5秒换一次效果
        if(ir_auto_change_tcnt!=0)
            ir_auto_change_tcnt--;
        if(ir_auto_change_tcnt == 0)
        {
            #ifdef MY_DEBUG
            printf("\r ir_auto_change_mode");
            #endif
            ir_auto_change_tcnt = IR_CHANGE_MODE_T;
            ir_mode_plus();
            led_state.dynamic_state_flag = ir_auto_mode[ir_mode_cnt];     
            Mode_Synchro(led_state.dynamic_state_flag);  
            switch_dynamic_task();  
        }
    }
    else
    {
        // 单播模式
        // err = os_sem_pend(&ir_task_sem, 0);  
        ir_auto_change_tcnt = 0; //开启自动模式后马上输出
    }
}

/***********************************************************AD_KEY*******************************************************************/



/***********************************************************key_event_task_handle*******************************************************************/

void key_event_task_handle(void *p)
{
    u32 err;
    while (1)
    {
        #ifdef EFFECT_DEBUG
        printf("--------------------key_event_task_handle------------------");
        #endif           
        // ir_auto_change_mode();

    }
}