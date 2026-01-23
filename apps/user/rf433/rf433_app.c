
#include "event.h"
#include "rf433.h"
#include "key_driver.h"
#include "app_main.h"
#include "rf433_app.h"
#include "local_music.h"
#include "led_strip_sys.h"
// ad_key_handle


// 读取键值有效时间
u8 rf433_valid_t;

void rf433_valid_cnt(void)
{
    if(rf433_valid_t < 80)//80ms
    {
        rf433_valid_t += 10;
    }
}

u8 ble_state = 1; //默认开启BLE模块
extern void save_user_data_area3(void);
extern void set_meteor_p(u8 p);
extern void set_mereor_mode(u8 m);
extern void fc_meteor_p(void);
extern void fc_meteor_speed(void);
extern  u16 T0;
extern u16 T1;
extern u8 rf_key_val;
u8 have_long_press = 0;


u32 RF433_CODE = 0x62FC;  //默认的433遥控

extern sys_cb_t sys_cb;

extern void save_user_data_area3(void);



#define duanan_cnt 170
#define duanan_cnt1 500
#define changan_cnt 4
#define changan1_cnt 45

extern void run_white_tips(void);


u8 start_tisp = 1;

void rf433_handle(struct sys_event *event)
{
    u8 event_type = 0;
    u8 key_value = 0;
    T0++;

//短按
    if(T0 > duanan_cnt && T1 <= changan_cnt  )  // T0 > 80MS   T1 <= 50*100 =5000ms
    {
        

        key_value = rf_key_get();

        if(key_value == 255) return ;
        rf_key_val = NO_KEY;  //
        T0 = 0; 
        T1 = 0;
      
        printf("duan an \r\n");

        if(sys_cb.RFAddrCode1 == 0xFFFF && RF433_CODE == 0xFFFF)  //判断客户码
        {


            if(key_value == RFKEY_SPEED_PLUS )
            {
            
                extern void ls_speed_plus(void);
                    ls_speed_plus();
                    printf("RFKEY_SPEED_PLUS");
        
                
            }
            else if(key_value == RFKEY_SPEED_SUB )
            {
                extern void ls_speed_sub(void);
                    ls_speed_sub();
                        printf("RFKEY_SPEED_SUB");

            }
            else if(key_value == RFKEY_MODE_ADD )
            {
                change_meteor_mode();
                    printf("RFKEY_MODE_ADD");
            }
           
            save_user_data_area3();

            if(key_value == RFKEY_ON_OFF)
            {
                if(get_on_off_state() == DEVICE_ON)
                {
                    soft_rurn_off_lights();     //软关灯
                    OnOff_Synchro(led_state.OpenorCloseflag);   //同步关机
                }
                else
                {
                    soft_turn_on_the_light();   //软开灯
                    OnOff_Synchro(led_state.OpenorCloseflag);   //同步开机
                }
                save_user_data_area3();
            }
    
        }  //判断客户码
        else if(sys_cb.RFAddrCode1 == 0x62FC && RF433_CODE == 0x62FC)  //判断客户码
        {
            //开关机
            if(key_value == _17_ON_OFF)  
            {
                if(get_on_off_state() == DEVICE_ON)
                {
                    soft_rurn_off_lights();     //软关灯
                    OnOff_Synchro(led_state.OpenorCloseflag);   //同步关机
                }
                else
                {
                    soft_turn_on_the_light();   //软开灯
                    OnOff_Synchro(led_state.OpenorCloseflag);   //同步开机
                }
                save_user_data_area3();
            }
            if(get_on_off_state())
            {
                if(key_value == _17_METEOR_LEN_PUL)     //拖尾+
                {
                    extern void meteor_len_pul(void);
                    meteor_len_pul();
                }
                else if(key_value == _17_METEOR_LEN_SUB)  //拖尾-
                {
                   extern void meteor_len_sub(void);
                   meteor_len_sub();
                }
                else if(key_value == _17_SPEED_PUL) //速度+
                {
                    extern void ls_speed_plus(void);
                    ls_speed_plus();
                    fc_meteor_speed();
                    printf("RFKEY_SPEED_PLUS");
        
                }
                else if(key_value == _17_SPEED_SUB) //速度-
                {
                    extern void ls_speed_sub(void);
                    ls_speed_sub();
                    fc_meteor_speed();

                    printf("RFKEY_SPEED_SUB");
                }
                else if(key_value == _17_RESET) //复位 流星7
                {
                   extern void meteor_reset(void);
                   meteor_reset();
                }
                else if(key_value == _17_PERIOD_PUL) //时间间隔+
                {
                    void meteor_p_pul(void);
                    meteor_p_pul();
                    fc_meteor_p();
                }
                else if(key_value == _17_PERIOD_SUB)//时间间隔-
                {
                    void meteor_p_sub(void);
                    meteor_p_sub();
                    fc_meteor_p();
                }
                else if(key_value == _17_BRIGHT_PUL) //亮度+
                {
                  extern void bright_plus(void);
                    bright_plus();
                }
                else if(key_value == _17_BRIGHT_SUB) //亮度-
                {
                   extern void bright_sub(void);
                    bright_sub();
                }
                else if(key_value == _17_METEOR_DIR) //正反流
                {
                    u8 temp = get_custom_index();
                    if(temp == 1 || temp == 2 || temp == 3 || temp == 4 || temp == 9)
                    {
                        switch(temp)
                        {
                            case 1: temp = 5; break;
                            case 2: temp = 6; break;
                            case 3: temp = 7; break;
                            case 4: temp = 8; break;
                            case 9: temp = 10; break;

                        }
                    }
                    else if(temp == 5 || temp == 6 || temp == 7 || temp == 8 || temp == 10)
                    {
                        switch(temp)
                        {
                            case 5: temp = 1; break;
                            case 6: temp = 2; break;
                            case 7: temp = 3; break;
                            case 8: temp = 4; break;
                            case 10: temp = 9; break;

                        }
                    }
                    else
                    {
                        temp = 1;
                    }
                    printf("temp = %d", temp);
                    set_mereor_mode(temp);
                    set_fc_effect();
                }
                else if(key_value == _17_MIN_PERIOD) //周期最短
                {
                 
                    set_meteor_p(2);
                    fc_meteor_p();
                }
                else if(key_value == _17_PERIOD_10S) //定周期 10s
                {
                    set_meteor_p(10);
                    fc_meteor_p();
                }
                else if(key_value == _17_MAX_PERIOD) //周期最长
                {
                    set_meteor_p(20);
                    fc_meteor_p();
                }
                else if(key_value == _17_METEOR_MUSIC1) //律动1
                {
                    set_mereor_mode(11);
                }
                else if(key_value == _17_METEOR_MUSIC2) //律动2
                {
                    set_mereor_mode(12);
                }
                else if(key_value == _17_METEOR_MUSIC3) //律动3
                {

                    set_mereor_mode(13);
                }
            
                save_user_data_area3();
                
            }  // 是开机状态
        }
    
    
    }
    else  if(T0 > duanan_cnt1)  //松手
    {
        T1 = 0;
       start_tisp = 1;
          key_value = rf_key_get();
          rf_key_val = NO_KEY;  //
        printf("clead.......................");
    }
//长按
    if(T1 > changan1_cnt )  //3s左右   每次红外间隔是50ms
    {

         key_value = rf_key_get();
    

        if(key_value == 255) return ;
        rf_key_val = NO_KEY;
        T0 = 0; 

        if(key_value == RFKEY_ON_OFF && sys_cb.RFAddrCode1 == 0xFFFF) //黑色遥控
        {
            RF433_CODE = 0xFFFF;
            save_user_data_area3();
            if(start_tisp)
            {
                start_tisp = 0;
                run_white_tips();
            }
            
            
           
            printf("change rf433");
        }


        if(key_value == _17_RESET && sys_cb.RFAddrCode1 == 0x62FC)    //白色遥控
        {
            RF433_CODE = 0x62FC;
            save_user_data_area3();
            
            if(start_tisp)
            {
                start_tisp = 0;
                run_white_tips();
            }
            
           
            printf("change rf433 ");
        }

        if(sys_cb.RFAddrCode1 == 0xFFFF && RF433_CODE == 0xFFFF)  //判断客户码 原本遥控
        {

            if(key_value == RFKEY_SPEED_PLUS)
            {
                printf("open ble*************************");
                ble_state =1;
                bt_ble_init();
                save_user_data_area3();
            }
            else if(key_value == RFKEY_SPEED_SUB)
            {
                //printf(" LONG  RFKEY_SPEED_SUB");
                printf("close ble*************************");
                bt_ble_exit();
                ble_state = 0;
                save_user_data_area3();
            }

            if(key_value == RFKEY_MODE_ADD)
            {
                // set_meteor_p(8);
                // set_mereor_mode(1);
                // set_fc_effect();
                // printf("\n reset merteor**************************");
                //  save_user_data_area3();

                set_mereor_mode(7);
                
                save_user_data_area3();

            }
            if(key_value == RFKEY_MUSIC1)
            {
                set_mereor_mode(13);
                
                save_user_data_area3();
            }

           


        }


    }

  
 

    // rf433_valid_cnt();
    // key_value = rf_key_get();
    // if(key_value == 255) return;
    // printf("key_value = %d", key_value);

    // 时间无效，返回
    // if(rf433_valid_t < 80)
    // {
       
    //     rf433_valid_t = 0;
    //     return;
    // }
    // rf433_valid_t = 0;
    

    event_type = KEY_EVENT_CLICK;
    
 

  
      //单击
    // if(event_type == KEY_EVENT_CLICK && get_on_off_state() == DEVICE_ON &&  long_press_cnt < 4 && press_cnt > 60  )
    // {

    //     if(key_value == RFKEY_SPEED_PLUS )
    //     {
           
    //         extern void ls_speed_plus(void);
    //             // ls_speed_plus();
    //             printf("RFKEY_SPEED_PLUS");
    //         press_cnt = 0;
    //         long_press_cnt = 0;
            
    //     }
    //     else if(key_value == RFKEY_SPEED_SUB )
    //     {
    //         extern void ls_speed_sub(void);
    //             // ls_speed_sub();
    //                 printf("RFKEY_SPEED_SUB");

    //     }
    //     else if(key_value == RFKEY_MODE_ADD )
    //     {
    //          //change_meteor_mode();
    //             printf("RFKEY_MODE_ADD");
    //     }

        
        
      
    //     extern void save_user_data_area3(void);
    //     save_user_data_area3();
    // }



    //    //长按
    //    else  if(event_type == KEY_EVENT_CLICK && get_on_off_state() == DEVICE_ON && long_press_cnt > 5 )
    //     {
    //         long_press_cnt = 0;
    //         extern void set_meteor_p(u8 p);
    //         extern void set_mereor_mode(u8 m);
        
    //         if(key_value == RFKEY_MODE_ADD)
    //         {
    //             // set_meteor_p(8);
    //             // set_mereor_mode(1);
    //             // set_fc_effect();
    //             printf("\n reset merteor");
    //         }
            
    

    //         if(key_value == RFKEY_SPEED_PLUS )  //开启蓝牙
    //         {
    //             printf("open ble");
    //            // bt_ble_init();
              
    //         }

    //         if(key_value == RFKEY_SPEED_SUB )  //关闭蓝牙
    //         {
    //            printf("close ble");
    //              //   bt_ble_exit();

    //         }

    //         extern void save_user_data_area3(void);
    //         save_user_data_area3();
    //     }
       


                /* case RFKEY_OFF:  
                    soft_rurn_off_lights();     //软关灯
                    OnOff_Synchro(led_state.OpenorCloseflag);   //同步关机
                break; */
                #if 0
                case RFKEY_LIGHT_PLUS:  
                    led_state.ledlight += 25;
                    if(led_state.ledlight > Light_Max)
                    {
                        led_state.ledlight = Light_Max;
                    }
                    if(led_state.running_task == STATIC_TASK)
                    {
                        led_state.ledlight_temp = (led_state.ledlight-Light_Min)*100/(Light_Max-Light_Min);
                        switch_static_task(); //为什么要推送多次,为了重设亮度
                        os_time_dly(1);
                        Light_Synchro(led_state.ledlight_temp);      //同步亮度
                    }
                    else if(led_state.running_task == DYNAMIC_TASK)
                    {
                        switch_dynamic_task();
                        os_time_dly(1);
                    }
                    break;
                case RFKEY_LIGHT_SUB:  
                    if(led_state.ledlight > Light_Min)
                    {
                        led_state.ledlight -= Light_Min;
                    }
                    if(led_state.ledlight < Light_Min)
                    {
                        led_state.ledlight = Light_Min;
                    }
                    if(led_state.running_task == STATIC_TASK)
                    {
                        led_state.ledlight_temp = (led_state.ledlight-Light_Min)*100/(Light_Max-Light_Min);
                        switch_static_task();
                        Light_Synchro(led_state.ledlight_temp);      //同步亮度
                        #ifdef MY_DEBUG
                        printf("led_state.ledlight =%d",led_state.ledlight);
                        #endif
                    }
                    else if(led_state.running_task == DYNAMIC_TASK)
                    {
                        switch_dynamic_task();
                        os_time_dly(1);
                    }
                    break;

                case RFKEY_R:  
                    led_state.static_state_flag = RED;
                    switch_static_task();
                    RGB_Synchro(led_state.R_flag,led_state.G_flag,led_state.B_flag);
                    break;
                case RFKEY_G:  
                    led_state.static_state_flag = GREEN;
                    switch_static_task();
                    RGB_Synchro(led_state.R_flag,led_state.G_flag,led_state.B_flag);
                    break;
                case RFKEY_B:  
                    led_state.static_state_flag = BLUE;
                    switch_static_task();
                    RGB_Synchro(led_state.R_flag,led_state.G_flag,led_state.B_flag);
                    break;
                case RFKEY_YELLOW:  
                    led_state.static_state_flag = CUSTOMIZE;
                    led_state.R_flag = 0xff, led_state.G_flag = 255;  led_state.B_flag = 0;
                    switch_static_task();
                    RGB_Synchro(led_state.R_flag,led_state.G_flag,led_state.B_flag);
                    break;
                case RFKEY_CYAN:  
                    led_state.static_state_flag = CUSTOMIZE;
                    led_state.R_flag = 0,    led_state.G_flag = 0xff;  led_state.B_flag = 255;
                    switch_static_task();
                    RGB_Synchro(led_state.R_flag,led_state.G_flag,led_state.B_flag);
                    break;
                case RFKEY_PURPLE:  
                    led_state.static_state_flag = CUSTOMIZE;
                    led_state.R_flag = 255, led_state.G_flag = 0;     led_state.B_flag = 0xff;
                    switch_static_task();
                    RGB_Synchro(led_state.R_flag,led_state.G_flag,led_state.B_flag);
                    break;
                case RFKEY_ORANGE:  
                    led_state.static_state_flag = CUSTOMIZE;
                    led_state.R_flag = 0xff, led_state.G_flag = 0x3f;  led_state.B_flag = 0;
                    switch_static_task();
                    RGB_Synchro(led_state.R_flag,led_state.G_flag,led_state.B_flag);
                    break;
                case RFKEY_W: 
                    led_state.static_state_flag = WHITE;
                    switch_static_task();
                    break;
                case RFKEY_JUMP3:  
                    led_state.dynamic_state_flag = THREE_JUMP_CHANGE;     
                    Mode_Synchro(led_state.dynamic_state_flag);  
                    switch_dynamic_task();   
                    break;
                case RFKEY_JUMP7:  
                    led_state.dynamic_state_flag = SEVEN_JUMP_CHANGE;     
                    Mode_Synchro(led_state.dynamic_state_flag);  
                    switch_dynamic_task();
                    break;
                case RFKEY_FADE3:  
                    led_state.dynamic_state_flag = THREE_GRADUAL_CHANGE;  
                    Mode_Synchro(led_state.dynamic_state_flag);  
                    switch_dynamic_task();
                    break;
                case RFKEY_FADE7:  
                    led_state.dynamic_state_flag = SEVEN_GRADUAL_CHANGE;     
                    Mode_Synchro(led_state.dynamic_state_flag);  
                    switch_dynamic_task(); 
                    break;
                case RFKEY_MUSIC1:  
                    set_local_mic_mode(MIC_SPEED);
                    set_local_music_task();
                    break;
                case RFKEY_MUSIC2:  
                    set_local_mic_mode(MIC_BRIGHTNESS);
                    set_local_music_task();
                    break;
                case RFKEY_MUSIC3:  
                    set_local_mic_mode(MIC_CHANGE_MODE);
                    set_local_music_task();
                    break;
                case RFKEY_MUSIC4:  
                    set_local_mic_mode(MIC_CHANGE_CLR);
                    set_local_music_task();
                    break;
                #endif
          
      //  }//if(event_type == KEY_EVENT_CLICK)






}//end


