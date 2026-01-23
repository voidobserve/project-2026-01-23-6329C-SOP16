/*
适用用2.4G遥控
基于中道版本2.4G遥控
1、app_config.h,把宏CONFIG_BT_GATT_CLIENT_NUM设置1
2、@bt_ble_init() 加入multi_client_init()
3、@le_gatt_client.c
   __resolve_adv_report（）
   HCI_EIR_DATATYPE_MORE_16BIT_SERVICE_UUIDS 加入键值处理函数
4、在key_driver.c 注册rf24g_scan_para
5、board_ac632n_demo_cfg.h 使能TCFG_RF24GKEY_ENABLE
6、@app_tuya.c tuya_key_event_handler()加入上层应用的键值处理函数
7、底层无法判断长按，需要靠上层应用实现
 */

#include "system/includes.h"

#include "task.h"
#include "event.h"
#include "rf24g.h"
#include "led_strip_sys.h"
#include "board_ac632n_demo_cfg.h"
#include "led_strand_effect.h"
#include "rf24g_app.h"

// #if TCFG_RF24GKEY_ENABLE
#if 1
extern rf24g_ins_t rf24g_ins;
#define PAIR_MAX 2

#pragma pack(1)
typedef struct
{
    u8 pair[3];
    u8 flag; // 0:表示该数组没使用，0xAA：表示改数组已配对使用
} rf24g_pair_t;

#pragma pack()

rf24g_pair_t rf24g_pair[PAIR_MAX]; // 需要写flash
/***********************************************************移植须修改****************************************************************/
extern void save_user_data_area3(void);
#define PAIR_TIME_OUT 5 * 1000 // 3秒
static u16 pair_tc = 0;

// 配对计时，10ms计数一次
void rf24g_pair_tc(void)
{
    if (pair_tc <= PAIR_TIME_OUT)
    {
        pair_tc += 10;
    }
}

/***********************************************************移植须修改 END****************************************************************/

/***********************************************************API*******************************************************************/
#define CFG_USER_PAIR_DATA 4 // 保存2.4G遥控客户码
void save_rf24g_pair_data(void)
{
    syscfg_write(CFG_USER_PAIR_DATA, (u8 *)(&rf24g_pair[0]), sizeof(rf24g_pair_t));
}

void read_rf24g_pair_data(void)
{
    syscfg_read(CFG_USER_PAIR_DATA, (u8 *)(&rf24g_pair[0]), sizeof(rf24g_pair_t));
    printf_buf((u8 *)(&rf24g_pair[0]), sizeof(rf24g_pair_t));
}

//-------------------------------------------------效果

// -----------------------------------------------声控

// -----------------------------------------------灵敏度

/***********************************************************APP*******************************************************************/

// pair_handle是长按执行，长按时会被执行多次
// 所以执行一次后，要把pair_tc = PAIR_TIME_OUT，避免误触发2次
extern void ls_lenght_add(u8 l);
extern void ls_lenght_sub(u8 l);

static void pair_handle(void)
{
    extern void save_rf24g_pair_data(void);
    u8 op = 0; // 1:配对，2：解码
    u8 i;

    // 开机3秒内
    if (pair_tc < PAIR_TIME_OUT)
    {
        // printf("\n pair_tc=%d",pair_tc);
        pair_tc = PAIR_TIME_OUT; // 避免误触发2次
        memcpy((u8 *)(&rf24g_pair[0].pair), (u8 *)(&rf24g_ins.pair), 3);
        rf24g_pair[0].flag = 0xaa;
        save_rf24g_pair_data();
        // printf("\n pair");
        // printf_buf(&rf24g_pair[0].pair, 3);
        extern void fc_24g_pair_effect(void); // 配对效果
        fc_24g_pair_effect();
        // 查找表是否存在
    }
}

extern u8 ble_state;

u8 off_long_cnt = 0;
extern u8 Ble_Addr[6];
extern u8 auto_set_led_num_f;
u8 pexil_switch = 1;

u8 yaokong_or_banzai = 0;
uint16_t need_to_set_num;
extern void set_ls_lenght(u16 l);

#define _3V_12jian_head1 0x34
#define _3V_12jian_head2 0x12

#define _12V_18jian_head1 0x01
#define _12V_18jian_head2 0x23

extern u16 rf24_T0;
extern u16 rf24_T1;
extern u8 last_key_v;
u8 key_value = 0;

extern u8 stepmotpor_speed_cnt;
u8 long_press_f = 0;
u8 long_press_cnt = 0;
void rf24_key_handle()
{

    if (rf24_T0 < 0xFFFF)
        rf24_T0++;
    // T0 10ms+=
    //     printf("T0 = %d", T0);
    if (rf24_T0 > 150 && rf24_T1 < 10)
    {
        key_value = last_key_v;
        if (key_value == NO_KEY)
            return;
        last_key_v = NO_KEY;
        rf24_T0 = 0;
        rf24_T1 = 0;

        /*CODE*/

        if (get_on_off_state() == DEVICE_ON)
        {

            if (key_value == RF24_K03)
            {

                extern void ls_speed_plus(void);
                ls_speed_plus();
                printf("RF24_K03");
            }
            else if (key_value == RF24_K04)
            {
                extern void ls_speed_sub(void);
                ls_speed_sub();
                printf("RF24_K04");
            }
            else if (key_value == RF24_K02)
            {
                change_meteor_mode();
                printf("RF24_K02");
            }
            extern void save_user_data_area3(void);
            save_user_data_area3();
        }

        if (key_value == RF24_K01)
        {
            if (get_on_off_state() == DEVICE_ON)
            {
                soft_turn_on_the_light(); // 软开灯
                // OnOff_Synchro(led_state.OpenorCloseflag);   //同步关机
            }
            else
            {
                soft_turn_on_the_light(); // 软开灯
                // OnOff_Synchro(led_state.OpenorCloseflag);   //同步开机
            }
            save_user_data_area3();
        }
    }
    else if (rf24_T0 > 150)
    {
        rf24_T1 = 0;
        long_press_f = 0;
        long_press_cnt = 0;
        printf("cled");
    }

    if (get_rf24g_long_state() == KEY_EVENT_LONG && rf24_T1 > 30) // 长按 rf24_T1 100ms+1
    {
        long_press_f = 1;
        key_value = last_key_v;
        last_key_v = NO_KEY;

        // if(key_value == RFKEY_SPEED_PLUS)
        // {
        //     printf("open ble*************************");
        //     ble_state =1;
        //     bt_ble_init();
        //     save_user_data_area3();
        // }
        // else if(key_value == RFKEY_SPEED_SUB)
        // {
        //     //printf(" LONG  RFKEY_SPEED_SUB");
        //     printf("close ble*************************");
        //     bt_ble_exit();
        //     ble_state = 0;
        //     save_user_data_area3();
        // }

        if (key_value == RF24_K02)
        {
            // set_meteor_p(8);
            // set_mereor_mode(1);
            // set_fc_effect();
            // printf("\n reset merteor**************************");
            //  save_user_data_area3();

            set_mereor_mode(7);

            save_user_data_area3();
        }
        if (key_value == RF24_K21)
        {
            set_mereor_mode(13);

            save_user_data_area3();
        }
    }
}

void long_press_handle(void)
{

    extern u8 single_flow_flag;
    if ((get_on_off_state() == DEVICE_ON) && long_press_f)
    {
        long_press_cnt++;
    }
}

#endif
