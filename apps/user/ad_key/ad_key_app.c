#include "board_ac632n_demo_cfg.h"
#include "event.h"
#include "key_driver.h"
#include "app_main.h"
// ---------------------------------------------定义AD_KEY键值
// 要和board_ac632n_demo.c的定义：adkey_platform_data adkey_data[] 匹配
typedef enum
{

    AD_KEY_PWR = 0 ,             //灯光开关，仅支持长按
    AD_KEY_MODE = 2,
    AD_KEY_STATIC = 1
}ad_key_value_e;



u8 run_mode;    //0:静态模式；1：动态
void ad_key_handle(struct sys_event *event)
{
    u8 event_type = 0;
    u8 key_value = 0;
    static u16 timeout = 0;
    // 关机计时
    static u16 off_cnt = 0;
    event_type = event->u.key.event;
    key_value = event->u.key.value;


    if(event->u.key.type == KEY_DRIVER_TYPE_AD)
    {
        printf("\n key_value=%d",key_value);
        if(event_type == KEY_EVENT_HOLD && key_value == AD_KEY_PWR) //长按5秒，开/关机
        {
            off_cnt+=300;
            if(off_cnt > 3000)
            {
                off_cnt = 0;
                if(led_state.OpenorCloseflag == OPEN_STATE)
                {
                    soft_rurn_off_lights();
                }
                else
                {
                    soft_turn_on_the_light();
                }
            }
        }

        if(event_type == KEY_EVENT_CLICK && key_value == AD_KEY_PWR)
        {
            
            if(led_state.OpenorCloseflag == OPEN_STATE)
            {
                // 静态模式调节亮度
                if(run_mode ==0)
                {
                    led_state.ledlight += 25;
                    if(led_state.ledlight > Light_Max)
                    {
                        led_state.ledlight = 25;
                    }
                

                    if(led_state.running_task == STATIC_TASK)
                    {
                        led_state.ledlight_temp = (led_state.ledlight-Light_Min)*100/(Light_Max-Light_Min);
                        switch_static_task(); //为什么要推送多次,为了重设亮度
                        os_time_dly(1);
                        Light_Synchro(led_state.ledlight_temp);      //同步亮度
                        #ifdef MY_DEBUG
                        printf("led_state.ledlight =%d",led_state.ledlight);
                        #endif
                    }

                }
                else if(run_mode == 1)  //动态模式调速度
                {
                    led_state.ledlight = 100;
                    led_state.speed += 1;
                    led_state.speed %= 9;
                    Speed_Synchro(led_state.speed); //同步速度

                }
                
            }
        }


        if(event_type == KEY_EVENT_CLICK && led_state.OpenorCloseflag == OPEN_STATE)
        {
            switch(key_value)
            {

                case AD_KEY_STATIC:
                    run_mode = 0;
                    extern u8 static_mode[14];
                    extern u8 static_mode_cnt;
                    led_state.static_state_flag = static_mode[static_mode_cnt];
                    static_mode_cnt++;
                    static_mode_cnt%=14;
                    switch_static_task();
                    RGB_Synchro(led_state.R_flag,led_state.G_flag,led_state.B_flag);
                break;


                case AD_KEY_MODE:
                    run_mode = 1;
                    led_state.ledlight = 100;
                    extern void mode_updata(void);
                    ir_mode_sub();
                    mode_updata();
                break;

            }//switch
        }//if(event_type == KEY_EVENT_CLICK)



    }//if(event->u.key.type == KEY_DRIVER_TYPE_AD)

}//end
