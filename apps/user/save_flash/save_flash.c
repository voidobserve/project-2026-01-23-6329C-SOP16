
#include "system/includes.h"
#include "syscfg_id.h"
#include "save_flash.h"
#include "rf433.h"
#define     CFG_USER_LED_LEDGTH_DATA    3

const u8 frist_mode[]={0x3D,0x00,0x00,0x0B,0x00,0x01,0x00,0x14,0x00,0x00,0x01,0x03,0xE8,0x03,0xE8}; //第一次上电默认模式



extern sys_cb_t sys_cb;
u8 ble_state ; //默认开启BLE模块
extern u32 RF433_CODE;
/*******************************************************************************************************
**函数名：上电读取FLASH里保存的指令数据
**输  出：
**输  入：读取CFG_USER_COMMAND_BUF_DATA里保存的最后一条接收到的指令，
**描  述：读取CFG_USER_LED_LEDGTH_DATA里保存的第一次上电标志，灯带长度，顺序是：：第1字节：第一次上电标志位，第2、3字节：灯带长度
**说  明：
**版  本：
**修改日期：
*******************************************************************************************************/
void read_flash_device_status_init(void)
{
    extern void set_custom_index(u8 m);
    extern void set_sensitive(u8 s);

    // printf("\n read_flash 1");
    #if 1
    u8 res;
    save_flash_t save_flash3;

    memset((u8*)&save_flash3,0,sizeof(save_flash_t));
    // printf("\n read_flash 2");

    res = syscfg_read(CFG_USER_LED_LEDGTH_DATA,(u8 *)(&save_flash3), sizeof(save_flash_t));
    // printf("\n read_flash 3");


    // printf("\n read_flash 4");

    if(save_flash3.header != 0x55)  //第一次上电
    {
        fc_data_init();
        ble_state = 1;//默认开启BLE模块
    }
    else
    {
        memcpy( (u8*)(&fc_effect), (u8*)(&save_flash3.fc_save) , sizeof(fc_effect_t));
        set_custom_index(fc_effect.custom_index);
        set_sensitive(fc_effect.music.s);
        ble_state = save_flash3.ble_state;
        RF433_CODE = save_flash3.rf433_code;
    }


    
    printf("\n read_flash 5");

#endif
}


// 把用户数据写到区域3
void save_user_data_area3(void)
{
    extern u8 get_sensitive(void);
    extern u8 get_custom_index(void);

    save_flash_t save_data;
    save_data.header = 0x55;
    // 不保存开关机状态，默认开机
    // fc_effect.on_off_flag = DEVICE_ON;
    fc_effect.sensitive     =   get_sensitive();
    fc_effect.custom_index  =   get_custom_index();
    save_data.ble_state = ble_state;
    save_data.rf433_code = RF433_CODE;
    memcpy((u8*)(&save_data.fc_save) , (u8*)(&fc_effect), sizeof(fc_effect_t));
    syscfg_write(CFG_USER_LED_LEDGTH_DATA, (u8 *)(&save_data), sizeof(save_flash_t));

}


void read_user_data_area3(void)
{

}
















