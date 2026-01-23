/*********************************************************************************************
    *   Filename        : le_server_module.c

    *   Description     :

    *   Author          :

    *   Email           : zh-jieli.com

    *   Last modifiled  : 2017-01-17 11:14

    *   Copyright:(c)JIELI  2011-2016  @ , All Rights Reserved.
*********************************************************************************************/

// *****************************************************************************
/* EXAMPLE_START(le_counter): LE Peripheral外设 - Heartbeat重要特征 Counter over GATT
 *
 * @text All newer operating systems provide GATT Client functionality.
 * The LE Counter examples demonstrates演示 how to specify具体 a minimal极小 GATT Database数据库
 * with a custom习性 GATT Service and a custom Characteristic特点 that sends periodic周期
 * notifications通知.
 */
// *****************************************************************************
#include "system/app_core.h"
#include "system/includes.h"

#include "app_config.h"
#include "app_action.h"

#include "btstack/btstack_task.h"
#include "btstack/bluetooth.h"
#include "user_cfg.h"
#include "vm.h"
#include "btcontroller_modules.h"
#include "bt_common.h"
#include "3th_profile_api.h"

#include "le_trans_data.h"
#include "le_common.h"

#include "rcsp_bluetooth.h"
#include "JL_rcsp_api.h"
#include "custom_cfg.h"
#include "asm/mcpwm.h"
#include "app_main.h"
#include "local_music.h"
#include "key_app.h"

#if (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_TRANS_DATA)   //1

//TRANS ANCS
#define TRANS_ANCS_EN  			  	 0
#if TRANS_ANCS_EN
#include "btstack/btstack_event.h"
#endif

#define TEST_SEND_DATA_RATE          0  //测试上行发送数据
#define TEST_SEND_HANDLE_VAL         ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE    //0x0008
/* #define TEST_SEND_HANDLE_VAL         ATT_CHARACTERISTIC_ae05_01_VALUE_HANDLE */
#define EXT_ADV_MODE_EN              0

#define TEST_AUDIO_DATA_UPLOAD       0 //测试文件上传


#if LE_DEBUG_PRINT_EN
extern void printf_buf(u8 *buf, u32 len);
/* #define log_info          printf */
#define log_info(x, ...)  printf("[LE_TRANS]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  printf_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

extern u8 Ble_Addr[];           //BLE地址
uint8_t LedCommand[200];        //接收缓存
uint8_t Send_buffer[50];        //发送缓存
uint16_t LedCommandLegth;       //接收到的数据的长度

extern LED_STATE led_state;
extern LED_STATE save_led_state;
extern OS_SEM LedStaticSem;     //静态信号量
extern OS_SEM LedActionSem;     //动态信号量
extern OS_SEM mic_led_Sem;
//extern u8 ledlight_temp;
extern TIME_CLOCK time_clock;
extern ALARM_CLOCK alarm_clock[];
extern u32 mic_val;


/* #define LOG_TAG_CONST       BT_BLE */
/* #define LOG_TAG             "[LE_S_DEMO]" */
/* #define LOG_ERROR_ENABLE */
/* #define LOG_DEBUG_ENABLE */
/* #define LOG_INFO_ENABLE */
/* #define LOG_DUMP_ENABLE */
/* #define LOG_CLI_ENABLE */
/* #include "debug.h" */

//------MTU：最大传输单元
//ATT发送的包长,    note: 20 <=need >= MTU
#define ATT_LOCAL_MTU_SIZE    (200)                   //
//ATT缓存的buffer大小,  note: need >= 20,可修改
#define ATT_SEND_CBUF_SIZE        (512)                   //

//共配置的RAM                            (188)
#define ATT_RAM_BUFSIZE           (ATT_CTRL_BLOCK_SIZE + ATT_LOCAL_MTU_SIZE + ATT_SEND_CBUF_SIZE)                   //note:
static u8 att_ram_buffer[ATT_RAM_BUFSIZE] __attribute__((aligned(4)));
//---------------

#if TEST_SEND_DATA_RATE    //0
static u32 test_data_count;
static u32 server_timer_handle = 0;
static u8 test_data_start;
#endif

/*
 打开流控使能后,确定使能接口 att_server_flow_enable 被调用
 然后使用过程 通过接口 att_server_flow_hold 来控制流控开关
 注意:流控只能控制对方使用带响应READ/WRITE等命令方式
 例如:ATT_WRITE_REQUEST = 0x12
 */
#define ATT_DATA_RECIEVT_FLOW           0//流控功能使能

//---------------
// 最小广播间隔 (unit:0.625ms)
#define ADV_INTERVAL_MIN          (160*5)

#define HOLD_LATENCY_CNT_MIN  (3)  //(0~0xffff)
#define HOLD_LATENCY_CNT_MAX  (15) //(0~0xffff)
#define HOLD_LATENCY_CNT_ALL  (0xffff)

static volatile hci_con_handle_t con_handle;    //uint16_t con_handle

//加密设置
/* static const uint8_t sm_min_key_size = 7; */

//连接参数更新请求设置
//是否使能参数请求更新,0--disable, 1--enable
static const uint8_t connection_update_enable = 1; ///0--disable, 1--enable
//当前请求的参数表index
static uint8_t connection_update_cnt = 0; //

//参数表，最小间隔，最大间隔，延迟，超时时间
static const struct conn_update_param_t connection_param_table[] = {
    {16, 24, 10, 600},//11
    {12, 28, 10, 600},//3.7
    {8,  20, 10, 600},
    /* {12, 28, 4, 600},//3.7 */
    /* {12, 24, 30, 600},//3.05 */
};

//共可用的参数组数
#define CONN_PARAM_TABLE_CNT      (sizeof(connection_param_table)/sizeof(struct conn_update_param_t))

#if (ATT_RAM_BUFSIZE < 64)  //内存过小
#error "adv_data & rsp_data buffer error!!!!!!!!!!!!"
#endif

//用户可配对的，这是样机跟客户开发的app配对的秘钥
/* const u8 link_key_data[16] = {0x06, 0x77, 0x5f, 0x87, 0x91, 0x8d, 0xd4, 0x23, 0x00, 0x5d, 0xf1, 0xd8, 0xcf, 0x0c, 0x14, 0x2b}; */
#define EIR_TAG_STRING   0xd6, 0x05, 0x08, 0x00, 'J', 'L', 'A', 'I', 'S', 'D','K'
static const char user_tag_string[] = {EIR_TAG_STRING};

static u8 adv_data_len;
static u8 adv_data[ADV_RSP_PACKET_MAX];//max is 31，广播数据最长31字节
static u8 scan_rsp_data_len;
static u8 scan_rsp_data[ADV_RSP_PACKET_MAX];//max is 31

static char gap_device_name[BT_NAME_LEN_MAX] = "jl_ble_test";
static u8 gap_device_name_len = 0; //名字长度，不包含结束符
static u8 ble_work_state = 0;      //ble 状态变化
static u8 adv_ctrl_en;             //广播

static u8 test_read_write_buf[4];

static void (*app_recieve_callback)(void *priv, void *buf, u16 len) = NULL;
static void (*app_ble_state_callback)(void *priv, ble_state_e state) = NULL;
static void (*ble_resume_send_wakeup)(void) = NULL;
static u32 channel_priv;

int app_send_user_data_check(u16 len);
int app_send_user_data_do(void *priv, u8 *data, u16 len);
int app_send_user_data(u16 handle, u8 *data, u16 len, u8 handle_type);

// Complete Local Name  默认的蓝牙名字

//------------------------------------------------------
//广播参数设置
static void advertisements_setup_init();
static int set_adv_enable(void *priv, u32 en);
static int get_buffer_vaild_len(void *priv);
extern const char *bt_get_local_name();
extern void clr_wdt(void);
extern void sys_auto_shut_down_disable(void);
extern void sys_auto_shut_down_enable(void);
extern u8 get_total_connect_dev(void);

//------------------------------------------------------
//NACS
#if TRANS_ANCS_EN   //0
#define ANCS_SUBEVENT_CLIENT_NOTIFICATION                           0xF1
void ancs_client_init(void);
void ancs_client_register_callback(btstack_packet_handler_t callback);
const char *ancs_client_attribute_name_for_id(int id);
void ancs_set_notification_buffer(u8 *buffer, u16 buffer_size);

//ancs info buffer
#define ANCS_INFO_BUFFER_SIZE  (1024)
static u8 ancs_info_buffer[ANCS_INFO_BUFFER_SIZE];
#endif

//--------------------------------------------------
void soft_turn_on_the_light(void);   //软开灯处理
void soft_rurn_off_lights(void);     //软关灯处理
void Set_Duty(u16 duty1,u16 duty2,u16 duty3);   //设置三个灯的占空比
//------------------------------------------------------
#if TEST_AUDIO_DATA_UPLOAD              //0，测试文件上传，未用功能
static const u8 test_audio_data_file[1024] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9
};


#define AUDIO_ONE_PACKET_LEN  128
static void test_send_audio_data(int init_flag)
{
    static u32 send_pt = 0;
    static u32 start_flag = 0;

    if (!con_handle) {
        return;
    }

    if (init_flag) {
        log_info("audio send init\n");
        send_pt = 0;
        start_flag = 1;
    }

    if (!start_flag) {
        return;
    }

    u32 file_size = sizeof(test_audio_data_file);
    u8 *file_ptr = test_audio_data_file;

    if (send_pt >= file_size) {
        log_info("audio send Complete\n");
        start_flag = 0;
        return;
    }

    u32 send_len = file_size - send_pt;
    if (send_len > AUDIO_ONE_PACKET_LEN) {
        send_len = AUDIO_ONE_PACKET_LEN;
    }

    while (1) {
        if (app_send_user_data_check(send_len)) {
            log_info("audio send %08x\n", send_pt);
            if (app_send_user_data(ATT_CHARACTERISTIC_ae3c_01_VALUE_HANDLE, &file_ptr[send_pt], send_len, ATT_OP_AUTO_READ_CCC)) {
                log_info("audio send fail!\n");
                break;
            } else {
                send_pt += send_len;
            }
        } else {
            break;
        }
    }
}

#endif


static void send_request_connect_parameter(u8 table_index)  //发送请求连接参数
{
    struct conn_update_param_t *param = (void *)&connection_param_table[table_index];//读取连接参数表

//    printf("------------%s-----------------\n", __FUNCTION__);       //打印功能函数名
    log_info("update_request:-%d-%d-%d-%d-\n", param->interval_min, param->interval_max, param->latency, param->timeout);
    if (con_handle) {
        ble_op_conn_param_request(con_handle, param);   //请求连接参数更新
    }
}

static void check_connetion_updata_deal(void)    //检查连接更新处理
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    if (connection_update_enable) { //是否使能参数更新请求,0--disable, 1--enable
        if (connection_update_cnt < CONN_PARAM_TABLE_CNT) { //没有超出创建的内存空间
            send_request_connect_parameter(connection_update_cnt);//connection_update_cnt是当前请求的参数表index
        }
    }
}

static void connection_update_complete_success(u8 *packet)      //连接更新成功
{
    int con_handle, conn_interval, conn_latency, conn_timeout;

//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    con_handle = hci_subevent_le_connection_update_complete_get_connection_handle(packet);  //连接入口
    conn_interval = hci_subevent_le_connection_update_complete_get_conn_interval(packet);   //连接间隔
    conn_latency = hci_subevent_le_connection_update_complete_get_conn_latency(packet);     //连接延迟
    conn_timeout = hci_subevent_le_connection_update_complete_get_supervision_timeout(packet);  //连接超时

    log_info("conn_interval = %d\n", conn_interval);
    log_info("conn_latency = %d\n", conn_latency);
    log_info("conn_timeout = %d\n", conn_timeout);
}


static void set_ble_work_state(ble_state_e state)          //设置BLE工作状态
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    if (state != ble_work_state) {
        log_info("ble_work_st:%x->%x\n", ble_work_state, state);
        ble_work_state = state;
        if (app_ble_state_callback) {
            app_ble_state_callback((void *)channel_priv, state);
        }
    }
}

static ble_state_e get_ble_work_state(void)         //获取BLE工作状态
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    return ble_work_state;
}

static void cbk_sm_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    sm_just_event_t *event = (void *)packet;
    u32 tmp32;

//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {
        case SM_EVENT_JUST_WORKS_REQUEST:
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            log_info("Just Works Confirmed.\n");
            break;
        case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
            log_info_hexdump(packet, size);
            memcpy(&tmp32, event->data, 4);
            log_info("Passkey display: %06u.\n", tmp32);
            break;
        }
        break;
    }
}


#if TEST_SEND_DATA_RATE     //0,测试发送数据的速度
static void server_timer_handler(void)
{
    if (!con_handle) {
        test_data_count = 0;
        test_data_start = 0;
        return;
    }

    log_info("peer_rssi = %d\n", ble_vendor_get_peer_rssi(con_handle));

    if (test_data_count) {
        log_info("\n%d bytes send: %d.%02d KB/s \n", test_data_count, test_data_count / 1000, test_data_count % 1000);
        test_data_count = 0;
    }
}

/*没有调用这个函数*/
static void server_timer_start(void)
{
    if (server_timer_handle) {
        return;
    }

    server_timer_handle  = sys_timer_add(NULL, server_timer_handler, 1000);
}
/*没有调用这个函数*/
static void server_timer_stop(void)
{
    if (server_timer_handle) {
        sys_timeout_del(server_timer_handle);
        server_timer_handle = 0;
    }
}
/*没有调用这个函数*/
void test_data_send_packet(void)            //测试发送数据包
{
    u32 vaild_len = get_buffer_vaild_len(0);//获取发送buffer可写入的数据
    if (!test_data_start) {
        return;
    }

    if (vaild_len) {
        if (!app_send_user_data(TEST_SEND_HANDLE_VAL, (void *)&test_data_count, vaild_len, ATT_OP_AUTO_READ_CCC)) {
            test_data_count += vaild_len;
        }
    }
    clr_wdt();
}
#endif

//发送完成回调，表示可以继续往协议栈发数，用来触发继续发数
static void can_send_now_wakeup(void)
{
    /* putchar('E'); */
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    if (ble_resume_send_wakeup) {      //恢复发送
        ble_resume_send_wakeup();
    }

#if TEST_SEND_DATA_RATE             //测试数据发送速度，未用功能
    test_data_send_packet();
#endif

#if TEST_AUDIO_DATA_UPLOAD         //测试声音数据上传，未用功能
    test_send_audio_data(0);
#endif

}

static void ble_auto_shut_down_enable(u8 enable)    //自动关闭使能
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
#if TCFG_AUTO_SHUT_DOWN_TIME        //0，没有蓝牙连接自动关机时间
    if (enable) {
        if (get_total_connect_dev() == 0) {    //已经没有设备连接
            sys_auto_shut_down_enable();
        }
    } else {
        sys_auto_shut_down_disable();
    }
#endif
}

const char *const phy_result[] = {
    "None",
    "1M",
    "2M",
    "Coded",
};
/*未调用函数*/
static void set_connection_data_length(u16 tx_octets, u16 tx_time)     //设置连接数据长度
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名

    if (con_handle) {
        ble_op_set_data_length(con_handle, tx_octets, tx_time);
    }
}
/*未调用函数*/
static void set_connection_data_phy(u8 tx_phy, u8 rx_phy)
{
    if (0 == con_handle) {
        return;
    }

    u8 all_phys = 0;
    u16 phy_options = CONN_SET_PHY_OPTIONS_S8;

//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    ble_op_set_ext_phy(con_handle, all_phys, tx_phy, rx_phy, phy_options);
}

static void server_profile_start(u16 con_handle)       //开始服务
{
#if BT_FOR_APP_EN       //0，未用功能
    set_app_connect_type(TYPE_BLE);
#endif
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    ble_op_att_send_init(con_handle, att_ram_buffer, ATT_RAM_BUFSIZE, ATT_LOCAL_MTU_SIZE);  //配置ATT发送模块RAM大小
    set_ble_work_state(BLE_ST_CONNECT); //设置BLE工作状态：链路连上
    ble_auto_shut_down_enable(0);   //关闭自动断开

    /* set_connection_data_phy(CONN_SET_CODED_PHY, CONN_SET_CODED_PHY); */
}

_WEAK_
u8 ble_update_get_ready_jump_flag(void)
{
    return 0;
}
/*
 * @section Packet Handler协议栈事件回调处理，主要是连接、断开等事件
 *
 * @text The packet handler is used to:
 *        - stop the counter after a disconnect
 *        - send a notification通知 when the requested请求 ATT_EVENT_CAN_SEND_NOW is received
 */

/* LISTING_START(packetHandler): Packet Handler */
static void cbk_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    int mtu;
    u32 tmp;
    u8 status;
    const char *attribute_name;

//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    log_info("packet_type = %d\n",packet_type);
    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {

        /* case DAEMON_EVENT_HCI_PACKET_SENT: */
        /* break; */
        case ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE:
            log_info("ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE\n");
        case ATT_EVENT_CAN_SEND_NOW:
            can_send_now_wakeup();
            break;

        case HCI_EVENT_LE_META:
            switch (hci_event_le_meta_get_subevent_code(packet)) {
            case HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE:
                status = hci_subevent_le_enhanced_connection_complete_get_status(packet);
                if (status) {
                    log_info("LE_SLAVE CONNECTION FAIL!!! %0x\n", status);
                    set_ble_work_state(BLE_ST_DISCONN); //断开连接
                    break;
                }
                con_handle = hci_subevent_le_enhanced_connection_complete_get_connection_handle(packet);
                log_info("HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE : %0x\n", con_handle);
                log_info("conn_interval = %d\n", hci_subevent_le_enhanced_connection_complete_get_conn_interval(packet));
                log_info("conn_latency = %d\n", hci_subevent_le_enhanced_connection_complete_get_conn_latency(packet));
                log_info("conn_timeout = %d\n", hci_subevent_le_enhanced_connection_complete_get_supervision_timeout(packet));
                server_profile_start(con_handle);
                break;

            case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                log_info("HCI_SUBEVENT_LE_CONNECTION_COMPLETE: %0x\n", con_handle);    //连接完成
                connection_update_complete_success(packet + 8);
                server_profile_start(con_handle);
#if RCSP_BTMATE_EN
#if (defined(BT_CONNECTION_VERIFY) && (0 == BT_CONNECTION_VERIFY))
                JL_rcsp_auth_reset();
#endif
                //rcsp_dev_select(RCSP_BLE);
                rcsp_init();
#endif
                log_info("ble remote rssi= %d\n", ble_vendor_get_peer_rssi(con_handle));   //获取链路对方的信号强度
                break;

            case HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE:
                connection_update_complete_success(packet);
                break;

            case HCI_SUBEVENT_LE_DATA_LENGTH_CHANGE:
                log_info("APP HCI_SUBEVENT_LE_DATA_LENGTH_CHANGE\n");
                /* set_connection_data_phy(CONN_SET_CODED_PHY, CONN_SET_CODED_PHY); */
                break;

            case HCI_SUBEVENT_LE_PHY_UPDATE_COMPLETE:
                log_info("APP HCI_SUBEVENT_LE_PHY_UPDATE %s\n", hci_event_le_meta_get_phy_update_complete_status(packet) ? "Fail" : "Succ");
                log_info("Tx PHY: %s\n", phy_result[hci_event_le_meta_get_phy_update_complete_tx_phy(packet)]);
                log_info("Rx PHY: %s\n", phy_result[hci_event_le_meta_get_phy_update_complete_rx_phy(packet)]);
                break;
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            log_info("HCI_EVENT_DISCONNECTION_COMPLETE: %0x\n", packet[5]); //事件断开完成
#if RCSP_BTMATE_EN
            rcsp_exit();
#endif
            con_handle = 0;
            ble_op_att_send_init(con_handle, 0, 0, 0);
            set_ble_work_state(BLE_ST_DISCONN);

            if (!ble_update_get_ready_jump_flag()) {
                bt_ble_adv_enable(1);
            }
            connection_update_cnt = 0;
#if BT_FOR_APP_EN
            set_app_connect_type(TYPE_NULL);
#endif
            ble_auto_shut_down_enable(1);   //BLE自动断开
            break;

        case ATT_EVENT_MTU_EXCHANGE_COMPLETE:
            mtu = att_event_mtu_exchange_complete_get_MTU(packet) - 3;
            log_info("ATT MTU = %u\n", mtu);
            ble_op_att_set_send_mtu(mtu);
            /* set_connection_data_length(251, 2120); */
            break;

        case HCI_EVENT_VENDOR_REMOTE_TEST:
            log_info("--- HCI_EVENT_VENDOR_REMOTE_TEST\n");
            break;

        case L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE:
            tmp = little_endian_read_16(packet, 4);
            log_info("-update_rsp: %02x\n", tmp);
            if (tmp) {
                connection_update_cnt++;
                log_info("remoter reject!!!\n");
                check_connetion_updata_deal();
            } else {
                connection_update_cnt = CONN_PARAM_TABLE_CNT;
            }
            break;

        case HCI_EVENT_ENCRYPTION_CHANGE:
            log_info("HCI_EVENT_ENCRYPTION_CHANGE= %d\n", packet[2]);
            break;

#if TRANS_ANCS_EN       //未用功能
        case HCI_EVENT_ANCS_META:
            switch (hci_event_ancs_meta_get_subevent_code(packet)) {
            case ANCS_SUBEVENT_CLIENT_NOTIFICATION:
                printf("ANCS_SUBEVENT_CLIENT_NOTIFICATION \n");
                attribute_name = ancs_client_attribute_name_for_id(ancs_subevent_client_notification_get_attribute_id(packet));
                if (!attribute_name) {
                    printf("ancs unknow attribute_id :%d \n", ancs_subevent_client_notification_get_attribute_id(packet));
                    break;
                } else {
                    u16 attribute_strlen = little_endian_read_16(packet, 7);
                    u8 *attribute_str = (void *)little_endian_read_32(packet, 9);
                    printf("Notification: %s - %s \n", attribute_name, attribute_str);
                }
                break;
            default:
                break;
            }

            break;
#endif

        }
        break;
    }
}


/* LISTING_END */

/*
 * @section ATT Read
 *
 * @text The ATT Server handles all reads to constant data. For dynamic data like the custom characteristic, the registered
 * att_read_callback is called. To handle long characteristics and long reads, the att_read_callback is first called
 * with buffer == NULL, to request the total value length. Then it will be called again requesting a chunk of the value.
 * See Listing attRead.
 */

/* LISTING_START(attRead): ATT Read */
//ATT 读事件处理
// ATT Client Read Callback for Dynamic动态 Data
// - if buffer == NULL, don't copy data, just return size of value
// - if buffer != NULL, copy data and return number bytes copied
// @param offset defines start of attribute value
static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint16_t  att_value_len = 0;
    uint16_t handle = att_handle;

//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    log_info("read_callback, handle= 0x%04x,buffer= %08x\n", handle, (u32)buffer);

    switch (handle) {
    case ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE:
        att_value_len = gap_device_name_len;

        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }

        if (buffer) {
            memcpy(buffer, &gap_device_name[offset], buffer_size);
            att_value_len = buffer_size;
            log_info("\n------read gap_name: %s \n", gap_device_name);
        }
        break;
    case ATT_CHARACTERISTIC_fff1_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer) {
            buffer[0] = att_get_ccc_config(handle);
            buffer[1] = 0;
        }
        att_value_len = 2;
        break;
    default:
        break;
    }

    log_info("att_value_len= %d\n", att_value_len);
    return att_value_len;

}
void ble_user_send_version(void)
{
	u8 ver_buff[18];
	ver_buff[0] = 0x01;
	ver_buff[1] = 0x05;
	memcpy(&ver_buff[2],"guan_ZY001_0.0.1", 16);
	app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE,ver_buff,18,ATT_OP_AUTO_READ_CCC);
}
/* LISTING_END */
/*
 * @section ATT Write
 *
 * @text The only valid ATT write in this example is to the Client Characteristic Configuration, which configures notification
 * and indication. If the ATT handle matches the client configuration handle, the new configuration value is stored and used
 * in the heartbeat handler to decide if a new value should be sent. See Listing attWrite.
 */
//ATT 写事件处理
/* LISTING_START(attWrite): ATT Write */
extern  void save_user_data_area3(void);
#include "led_strand_effect.h"

static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    int result = 0;
    u16 tmp16,i;
    u8 temp,temp1;

    u16 handle = att_handle;
    static BaseType_t xLedTaskWoken;

    //    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    log_info("write_callback, handle= 0x%04x,size = %d\n", handle, buffer_size);

    switch (handle) {

    case ATT_CHARACTERISTIC_fff1_01_CLIENT_CONFIGURATION_HANDLE:
        set_ble_work_state(BLE_ST_NOTIFY_IDICATE);
        check_connetion_updata_deal();
        log_info("\n------write ccc:%04x,%02x\n", handle, buffer[0]);
        att_set_ccc_config(handle, buffer[0]);
        break;
    case ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE:
        log_info("\n-fff1_rx(%d):", buffer_size);
        printf_buf(buffer, buffer_size);
        memcpy(&LedCommand,buffer,buffer_size);
        LedCommandLegth = buffer_size;

        if(buffer[0] == 0x01 && buffer[1] == 0x03)  //与APP同步数据
        {
            printf("fc_effect.music.s = %d", fc_effect.music.s);
            extern fc_effect_t fc_effect;//幻彩灯串效果数据
            ble_user_send_version();
            memcpy(Send_buffer,Ble_Addr, 6);
            Send_buffer[6] = 0x2F;
            Send_buffer[7] = 0x02;
            Send_buffer[8] = fc_effect.on_off_flag;
            app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
            os_time_dly(1);
            //-------------流星速度-----------------------------------
            Send_buffer[6] = 0x2F;
            Send_buffer[7] = 0x01;
            Send_buffer[8] = 100-fc_effect.speed;
            app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
            os_time_dly(1);
            //-------------流星周期-----------------------------------
            Send_buffer[6] = 0x2F;
            Send_buffer[7] = 0x03;
            Send_buffer[8] = fc_effect.meteor_period;
            app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
            os_time_dly(1);

            //-------------流星灵敏度-----------------------------------
            Send_buffer[6] = 0x2F;
            Send_buffer[7] = 0x05;
            Send_buffer[8] = fc_effect.music.s;
            app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
            os_time_dly(1);


            #if 0
//-------------------发送开关机状态---------------------------
            Send_buffer[6] = 0x01;
            Send_buffer[7] = 0x01;
            if(led_state.OpenorCloseflag == OPEN_STATE)
                Send_buffer[8] = 0x01;
            else if(led_state.OpenorCloseflag == CLOSE_STATE)
                Send_buffer[8] = 0;
            app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
            os_time_dly(1);
//-------------------发送亮度---------------------------
            Send_buffer[6] = 0x04;
            Send_buffer[7] = 0x03;
            Send_buffer[8] = led_state.ledlight_temp;
            app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
            os_time_dly(1);
//-------------------发送速度---------------------------
            Send_buffer[6] = 0x04;
            Send_buffer[7] = 0x04;
            Send_buffer[8] = led_state.speed;
            app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
            os_time_dly(1);
//-------------------发送工作模式---------------------------
            Send_buffer[6] = 0x04;
            Send_buffer[7] = 0x02;
            if(led_state.running_task == DYNAMIC_TASK)
            {
                Send_buffer[8] = led_state.dynamic_state_flag;    //发送动态模式
                app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
            }
            else if(led_state.running_task == STATIC_TASK)
            {
                Send_buffer[8] = led_state.static_state_flag;     //发送静态模式
                app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
            }
            os_time_dly(1);
//-------------------发送静态RGB模式--------------------------
            Send_buffer[6] = 0x04;
            Send_buffer[7] = 0x01;
            Send_buffer[8] = 0x1e;
            Send_buffer[9] = led_state.R_flag;
            Send_buffer[10] = led_state.G_flag;
            Send_buffer[11] = led_state.B_flag;
            app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,12, ATT_OP_AUTO_READ_CCC);
            os_time_dly(1);
//-------------------发送RGB接口模式--------------------------
            Send_buffer[6] = 0x04;
            Send_buffer[7] = 0x05;
            Send_buffer[8] = led_state.interface_mode;
            app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
            os_time_dly(1);
//-------------------发送闹钟1定时数据--------------------------
            Send_buffer[6] = 0x05;
            Send_buffer[7] = 0x00;
            Send_buffer[8] = alarm_clock[0].hour;
            Send_buffer[9] = alarm_clock[0].minute;
            Send_buffer[10] = alarm_clock[0].on_off;
            Send_buffer[11] = alarm_clock[0].mode;
            app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,12, ATT_OP_AUTO_READ_CCC);
            os_time_dly(1);
//-------------------发送闹钟2定时数据--------------------------
            Send_buffer[6] = 0x05;
            Send_buffer[7] = 0x01;
            Send_buffer[8] = alarm_clock[1].hour;
            Send_buffer[9] = alarm_clock[1].minute;
            Send_buffer[10] = alarm_clock[1].on_off;
            Send_buffer[11] = alarm_clock[1].mode;
            app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,12, ATT_OP_AUTO_READ_CCC);
            os_time_dly(1);
//-------------------发送闹钟3定时数据--------------------------
            Send_buffer[6] = 0x05;
            Send_buffer[7] = 0x02;
            Send_buffer[8] = alarm_clock[2].hour;
            Send_buffer[9] = alarm_clock[2].minute;
            Send_buffer[10] = alarm_clock[2].on_off;
            Send_buffer[11] = alarm_clock[2].mode;
            app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,12, ATT_OP_AUTO_READ_CCC);
            os_time_dly(1);
            #endif
        }
        else
        {
    //---------------------------------接收到开灯命令-----------------------------------
            #if 0
            if(LedCommand[0]==0x01 && LedCommand[1]==0x01  && LedCommand[2]==0x01 && LedCommandLegth==3 && led_state.OpenorCloseflag == CLOSE_STATE)
            {
                memcpy(Send_buffer,Ble_Addr, 6);
                for(i=6;i<(buffer_size+6);i++)
                {
                    Send_buffer[i] = buffer[i-6];
                }
                if (app_send_user_data_check(6))
                {
                    app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,6+buffer_size, ATT_OP_AUTO_READ_CCC);
                }
                soft_turn_on_the_light();
            //    LED_state_write_flash();    //保存LED状态
            }
           
            //---------------------------------接收到时间数据，无需返回应答-----------------------------------
            if(LedCommand[0]==0x06 && LedCommand[1]==0x02)
            {
                time_clock.hour = LedCommand[2];     //更新时间数据
                time_clock.minute = LedCommand[3];
                time_clock.second = LedCommand[4];
                time_clock.week = LedCommand[5];
            }
            //-----------------------------------接收到闹钟数据-----------------------------------------
            if(LedCommand[0]==0x05)
            {
                memcpy(Send_buffer,Ble_Addr, 6);
                for(i=6;i<(buffer_size+6);i++)
                {
                    Send_buffer[i] = buffer[i-6];
                }
                if (app_send_user_data_check(6))
                {
                    app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,6+buffer_size, ATT_OP_AUTO_READ_CCC);
                }
                if(LedCommand[1]==0x00)
                {
                    alarm_clock[0].hour = LedCommand[2];
                    alarm_clock[0].minute = LedCommand[3];
                    alarm_clock[0].on_off = LedCommand[4];
                    alarm_clock[0].mode = LedCommand[5];
                }
                if(LedCommand[1]==0x01)
                {
                    alarm_clock[1].hour = LedCommand[2];
                    alarm_clock[1].minute = LedCommand[3];
                    alarm_clock[1].on_off = LedCommand[4];
                    alarm_clock[1].mode = LedCommand[5];
                }
                if(LedCommand[1]==0x02)
                {
                    alarm_clock[2].hour = LedCommand[2];
                    alarm_clock[2].minute = LedCommand[3];
                    alarm_clock[2].on_off = LedCommand[4];
                    alarm_clock[2].mode = LedCommand[5];
                }
                printf("on_off[0] = %d,hour[0] = %d,minute[0] = %d,mode[0] = %d\n",alarm_clock[0].on_off,alarm_clock[0].hour,alarm_clock[0].minute,alarm_clock[0].mode);
                printf("on_off[1] = %d,hour[1] = %d,minute[1] = %d,mode[1] = %d\n",alarm_clock[1].on_off,alarm_clock[1].hour,alarm_clock[1].minute,alarm_clock[1].mode);
                printf("on_off[2] = %d,hour[2] = %d,minute[2] = %d,mode[2] = %d\n",alarm_clock[2].on_off,alarm_clock[2].hour,alarm_clock[2].minute,alarm_clock[2].mode);
            }
            #endif
            //--------------------------下面所有的操作都需要在开灯状态下操作-----------------------------------
            // if(led_state.OpenorCloseflag == OPEN_STATE)
            #include "led_strip_sys.h"
            extern ON_OFF_FLAG get_on_off_state(void);
             // 流星灯开关
            if(LedCommand[0]==0x2F && LedCommand[1]==0x02)
            {
                if(LedCommand[2] == 1)
                {
                    soft_turn_on_the_light();
                    memcpy(Send_buffer,Ble_Addr, 6);
                    Send_buffer[6] = 0x2F;
                    Send_buffer[7] = 0x02;
                    Send_buffer[8] = 0x01;
                    app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
                }
                else if(LedCommand[2] == 2)
                {
                    soft_rurn_off_lights();
                    memcpy(Send_buffer,Ble_Addr, 6);
                    Send_buffer[6] = 0x2F;
                    Send_buffer[7] = 0x02;
                    Send_buffer[8] = 0x02;
                    app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
                }
                save_user_data_area3();
            }
            if(get_on_off_state() )
            {
                memcpy(Send_buffer,Ble_Addr, 6);
                for(i=6;i<(buffer_size+6);i++)
                {
                    Send_buffer[i] = buffer[i-6];
                }
                if (app_send_user_data_check(6))
                {
                    app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,6+buffer_size, ATT_OP_AUTO_READ_CCC);
                }
                //---------------------------------接收到关灯命令-----------------------------------
                // if(LedCommand[0]==0x01 && LedCommand[1]==0x01 && LedCommand[2]==0 && LedCommandLegth==3)
                // {
                //     soft_rurn_off_lights();
                // //    LED_state_write_flash();    //保存LED状态
                // }
                // 流星模式
                if(LedCommand[0]==0x2F && LedCommand[1]==0x00)
                {
                    extern void set_mereor_mode(u8 m);
                    set_mereor_mode(LedCommand[2]);
                    printf("\n mereor_mode=%d",LedCommand[2]);
                    save_user_data_area3();
                    Send_buffer[6] = 0x2F;
                    Send_buffer[7] = 0x00;
                    Send_buffer[8] = LedCommand[2];
                    app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
                }
                // 流星速度
                if(LedCommand[0]==0x2F && LedCommand[1]==0x01)
                {
                    void set_mereor_speed(u8 s);
                    set_mereor_speed(LedCommand[2]);
                    printf("\n mereor_mode=%d",LedCommand[2]);
                    save_user_data_area3();
                    Send_buffer[6] = 0x2F; 
                    Send_buffer[7] = 0x01;
                    Send_buffer[8] = LedCommand[2];
                    app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
                }
                
                // 流星灯时间间隔
                if(LedCommand[0]==0x2F && LedCommand[1]==0x03)
                {
                    extern void set_meteor_p(u8 p);
                    set_meteor_p(LedCommand[2]);
                    save_user_data_area3();
                    Send_buffer[6] = 0x2F;
                    Send_buffer[7] = 0x03;
                    Send_buffer[8] = LedCommand[2];
                    app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
                }
                // 灵敏度
                if(LedCommand[0]==0x2F && LedCommand[1]==0x05 && LedCommandLegth==3)
                {
                    extern void set_sensitive(u8 s);
                    set_sensitive(LedCommand[2]);
                    save_user_data_area3();
                    Send_buffer[6] = 0x2F;
                    Send_buffer[7] = 0x05;
                    Send_buffer[8] = LedCommand[2];
                    app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
                }
                

             

            }

           
            
        } //end
        break;

    #if RCSP_BTMATE_EN
        case ATT_CHARACTERISTIC_ae02_02_CLIENT_CONFIGURATION_HANDLE:
            ble_op_latency_skip(con_handle, HOLD_LATENCY_CNT_ALL); //
            set_ble_work_state(BLE_ST_NOTIFY_IDICATE);
    #endif
        /* if ((cur_conn_latency == 0) */
        /*     && (connection_update_cnt == CONN_PARAM_TABLE_CNT) */
        /*     && (Peripheral_Preferred_Connection_Parameters[0].latency != 0)) { */
        /*     connection_update_cnt = 0; */
        /* } */
        check_connetion_updata_deal();
        log_info("\n------write ccc:%04x,%02x\n", handle, buffer[0]);
        att_set_ccc_config(handle, buffer[0]);
        break;
    #if RCSP_BTMATE_EN
        case ATT_CHARACTERISTIC_ae01_02_VALUE_HANDLE:
            printf("rcsp_read:%x\n", buffer_size);
            if (app_recieve_callback) {
                app_recieve_callback(0, buffer, buffer_size);
            }
            break;
    #endif
    }
    extern void save_user_data_area3(void);
    save_user_data_area3();
    return 0;
}


/**
 * @brief 向app反馈信息
 * 
 * @param p   数据内容
 * @param len  数据长度
 */
void zd_fb_2_app(u8 *p, u8 len)
{
    uint8_t Send_buffer[50];            //发送缓存
    memcpy(Send_buffer,Ble_Addr, 6);
    memcpy(Send_buffer+6, p, len);
    app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,len+6, ATT_OP_AUTO_READ_CCC);
    // ble_comm_att_send_data(fd_handle, ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer, len+6, ATT_OP_AUTO_READ_CCC);
}





//NOTIFY 和 INDICATE 发送接口，发送前调接口 app_send_user_data_check 检查
int app_send_user_data(u16 handle, u8 *data, u16 len, u8 handle_type)
{
    u32 ret = APP_BLE_NO_ERROR;

//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    if (!con_handle) {
        return APP_BLE_OPERATION_ERROR;
    }

    if (!att_get_ccc_config(handle + 1)) {
        log_info("fail,no write ccc!!!,%04x\n", handle + 1);
        return APP_BLE_NO_WRITE_CCC;
    }

    ret = ble_op_att_send_data(handle, data, len, handle_type);
    if (ret == BLE_BUFFER_FULL) {
        ret = APP_BLE_BUFF_FULL;
    }

    if (ret) {
        log_info("app_send_fail:%d !!!!!!\n", ret);
    }
    return ret;
}

//------------------------------------------------------
//#if RCSP_BTMATE_EN
static u8 tag_in_adv;
//#endif
//配置广播 ADV 数据
static int make_set_adv_data(void)
{
    u8 offset = 0;
    u8 *buf = adv_data;

//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
#if DOUBLE_BT_SAME_MAC
    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_FLAGS, 0x0A, 1);
#else
    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_FLAGS, 0x06, 1);//按(长度 + 类型 + 内容)这样的格,组合填入广播包数据
#endif

    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_16BIT_SERVICE_UUIDS, 0xAF30, 2);
/*
#if RCSP_BTMATE_EN  //0
    u8  tag_len = sizeof(user_tag_string);
    if (tag_len > ADV_RSP_PACKET_MAX - (offset + 2)) {
        tag_in_adv = 0;
    } else {
        offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA, (void *)user_tag_string, tag_len);
        tag_in_adv = 1;
    }
#else*/
    u8 info[7+6];    //客户机型数据,7byte客户信息，6byte蓝牙地址
    info[0] = 'Z';
    info[1] = 'D';
    info[2] = 0;
    info[3] = 0x9d;
    info[4] = 3;
    info[5] = 0x5D;
    info[6] = 0x70;

    le_controller_get_mac(&info[7]);    //获取ble的蓝牙public地址
    //按(长度 + 类型 + 内容)这样的格,组合填入广播包数据
    offset += make_eir_packet_data(&buf[offset],offset,HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA,info,11);
    tag_in_adv = 1;
//#endif

    if (offset > ADV_RSP_PACKET_MAX) {
        puts("***adv_data overflow!!!!!!\n");
        return -1;
    }
    log_info("adv_data(%d):", offset);
    log_info_hexdump(buf, offset);
    adv_data_len = offset;
    ble_op_set_adv_data(offset, buf);//配置广播 Advertising Data内容
    return 0;
}
//配置广播 RESPONE 应答数据
static int make_set_rsp_data(void)
{
    u8 offset = 0;
    u8 *buf = scan_rsp_data;

//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
#if RCSP_BTMATE_EN  //0
    if (!tag_in_adv) {
        u8  tag_len = sizeof(user_tag_string);
        offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA, (void *)user_tag_string, tag_len);
    }
#endif

    u8 name_len = gap_device_name_len;
    u8 vaild_len = ADV_RSP_PACKET_MAX - (offset + 2);
    if (name_len > vaild_len) {
        name_len = vaild_len;
    }
    offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME, (void *)gap_device_name, name_len);

    if (offset > ADV_RSP_PACKET_MAX) {
        puts("***rsp_data overflow!!!!!!\n");
        return -1;
    }

    log_info("rsp_data(%d):", offset);
    log_info_hexdump(buf, offset);
    scan_rsp_data_len = offset;
    ble_op_set_rsp_data(offset, buf);//配置应答数据
    return 0;
}

//广播参数设置
/*
    ADV_IND             //可被连接的、无方向的广播数据
    ADV_DIRECT_IND      //可被连接的、单向广播数据
    ADV_SCAN_IND        //可接受SCAN_REQ请求的、无方向的广播数据
    ADV_NONCONN_IND     //不可被连接的、无方向的广播数据
*/
static void advertisements_setup_init()     //广播设置初始化
{
    uint8_t adv_type = ADV_IND;
    uint8_t adv_channel = ADV_CHANNEL_ALL;
    int   ret = 0;

//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    ble_op_set_adv_param(ADV_INTERVAL_MIN, adv_type, adv_channel);//配置最小广播间隔，可被连接，无方向的广播，37，38，39通道

    ret |= make_set_adv_data(); //配置广播里面的有效负载数据
    ret |= make_set_rsp_data(); //配置应答数据

    if (ret) {
        puts("advertisements_setup_init fail !!!!!!\n");
        return;
    }

}

#define PASSKEY_ENTER_ENABLE      0 //输入passkey使能，可修改passkey
//重设passkey回调函数，在这里可以重新设置passkey
//passkey为6个数字组成，十万位、万位。。。。个位 各表示一个数字 高位不够为0
static void reset_passkey_cb(u32 *key)  //重设密钥
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
#if 1
    u32 newkey = rand32();//获取随机数

    newkey &= 0xfffff;
    if (newkey > 999999) {
        newkey = newkey - 999999; //不能大于999999
    }
    *key = newkey; //小于或等于六位数
    printf("set new_key= %06u\n", *key);
#else
    *key = 123456; //for debug
#endif
}
//sm = security manager，安全管理器
void ble_sm_setup_init(io_capability_t io_type, u8 auth_req, uint8_t min_key_size, u8 security_en)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    //setup SM: Display only
    sm_init();  //安全管理器初始化，库函数
    sm_set_io_capabilities(io_type);
    sm_set_authentication_requirements(auth_req);   //设置身份验证的条件
    sm_set_encryption_key_size_range(min_key_size, 16);//设置密钥大小范围
    sm_set_request_security(security_en);   //设置请求安全措施
    sm_event_callback_set(&cbk_sm_packet_handler);  //安全事件加调

    if (io_type == IO_CAPABILITY_DISPLAY_ONLY) {
        reset_PK_cb_register(reset_passkey_cb); //设置密钥寄存器
    }
}


void ble_profile_init(void)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    printf("ble profile init\n");
    le_device_db_init();    //初始化配对表

#if PASSKEY_ENTER_ENABLE        //0
    ble_sm_setup_init(IO_CAPABILITY_DISPLAY_ONLY, SM_AUTHREQ_MITM_PROTECTION, 7, TCFG_BLE_SECURITY_EN);
#else
    ble_sm_setup_init(IO_CAPABILITY_NO_INPUT_NO_OUTPUT, SM_AUTHREQ_BONDING, 7, TCFG_BLE_SECURITY_EN);   //使能配对加密
#endif

    /* setup ATT server */
    att_server_init(profile_data, att_read_callback, att_write_callback);
    att_server_register_packet_handler(cbk_packet_handler);
    /* gatt_client_register_packet_handler(packet_cbk); */

    // register for HCI events
    hci_event_callback_set(&cbk_packet_handler);
    /* ble_l2cap_register_packet_handler(packet_cbk); */
    /* sm_event_packet_handler_register(packet_cbk); */
    le_l2cap_register_packet_handler(&cbk_packet_handler);

#if TRANS_ANCS_EN       //0
    //setup GATT client
    gatt_client_init();

    //setup ANCS clent
    ancs_client_init();
    ancs_set_notification_buffer(ancs_info_buffer, sizeof(ancs_info_buffer));
    ancs_client_register_callback(&cbk_packet_handler);
#endif

    ble_vendor_set_default_att_mtu(ATT_LOCAL_MTU_SIZE);//配置协议栈ATT默认的MTU 大小
}

#if EXT_ADV_MODE_EN     //0


#define EXT_ADV_NAME                    'J', 'L', '_', 'E', 'X', 'T', '_', 'A', 'D', 'V'
/* #define EXT_ADV_NAME                    "JL_EXT_ADV" */
#define BYTE_LEN(x...)                  sizeof((u8 []) {x})
#define EXT_ADV_DATA                    \
    0x02, 0x01, 0x06, \
    0x03, 0x02, 0xF0, 0xFF, \
    BYTE_LEN(EXT_ADV_NAME) + 1, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, EXT_ADV_NAME

struct ext_advertising_param {
    u8 Advertising_Handle;
    u16 Advertising_Event_Properties;
    u8 Primary_Advertising_Interval_Min[3];
    u8 Primary_Advertising_Interval_Max[3];
    u8 Primary_Advertising_Channel_Map;
    u8 Own_Address_Type;
    u8 Peer_Address_Type;
    u8 Peer_Address[6];
    u8 Advertising_Filter_Policy;
    u8 Advertising_Tx_Power;
    u8 Primary_Advertising_PHY;
    u8 Secondary_Advertising_Max_Skip;
    u8 Secondary_Advertising_PHY;
    u8 Advertising_SID;
    u8 Scan_Request_Notification_Enable;
} _GNU_PACKED_;

struct ext_advertising_data  {
    u8 Advertising_Handle;
    u8 Operation;
    u8 Fragment_Preference;
    u8 Advertising_Data_Length;
    u8 Advertising_Data[BYTE_LEN(EXT_ADV_DATA)];
} _GNU_PACKED_;

struct ext_advertising_enable {
    u8  Enable;
    u8  Number_of_Sets;
    u8  Advertising_Handle;
    u16 Duration;
    u8  Max_Extended_Advertising_Events;
} _GNU_PACKED_;

const struct ext_advertising_param ext_adv_param = {
    .Advertising_Handle = 0,
    .Advertising_Event_Properties = 1,
    .Primary_Advertising_Interval_Min = {30, 0, 0},
    .Primary_Advertising_Interval_Max = {30, 0, 0},
    .Primary_Advertising_Channel_Map = 7,
    .Primary_Advertising_PHY = ADV_SET_1M_PHY,
    .Secondary_Advertising_PHY = ADV_SET_1M_PHY,
};

const struct ext_advertising_data ext_adv_data = {
    .Advertising_Handle = 0,
    .Operation = 3,
    .Fragment_Preference = 0,
    .Advertising_Data_Length = BYTE_LEN(EXT_ADV_DATA),
    .Advertising_Data = EXT_ADV_DATA,
};

const struct ext_advertising_enable ext_adv_enable = {
    .Enable = 1,
    .Number_of_Sets = 1,
    .Advertising_Handle = 0,
    .Duration = 0,
    .Max_Extended_Advertising_Events = 0,
};

const struct ext_advertising_enable ext_adv_disable = {
    .Enable = 0,
    .Number_of_Sets = 1,
    .Advertising_Handle = 0,
    .Duration = 0,
    .Max_Extended_Advertising_Events = 0,
};

#endif /* EXT_ADV_MODE_EN */

static int set_adv_enable(void *priv, u32 en)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    ble_state_e next_state, cur_state;

    if (!adv_ctrl_en) {
        return APP_BLE_OPERATION_ERROR;
    }

    if (con_handle) {
        return APP_BLE_OPERATION_ERROR;
    }

    if (en) {
        next_state = BLE_ST_ADV;    ////设备处于广播状态
    } else {
        next_state = BLE_ST_IDLE;   //关闭广播或扫描状态
    }

    cur_state =  get_ble_work_state();  //获取BLE工作状态
    switch (cur_state) {
    case BLE_ST_ADV:
    case BLE_ST_IDLE:
    case BLE_ST_INIT_OK:
    case BLE_ST_NULL:
    case BLE_ST_DISCONN:
        break;
    default:
        return APP_BLE_OPERATION_ERROR;
        break;
    }

    if (cur_state == next_state) {
        return APP_BLE_NO_ERROR;
    }
    log_info("adv_en:%d\n", en);
    set_ble_work_state(next_state); //设置BLE，使BLE处于广播状态

#if EXT_ADV_MODE_EN //0
    if (en) {
        ble_op_set_ext_adv_param(&ext_adv_param, sizeof(ext_adv_param));

        log_info_hexdump(&ext_adv_data, sizeof(ext_adv_data));
        ble_op_set_ext_adv_data(&ext_adv_data, sizeof(ext_adv_data));

        ble_op_set_ext_adv_enable(&ext_adv_enable, sizeof(ext_adv_enable));
    } else {
        ble_op_set_ext_adv_enable(&ext_adv_disable, sizeof(ext_adv_disable));
    }
#else
    if (en) {
        advertisements_setup_init();    //广播参数设置
    }
    ble_op_adv_enable(en);  //启动BLE广播
#endif /* EXT_ADV_MODE_EN */

    return APP_BLE_NO_ERROR;
}
//断开BLE连接
static int ble_disconnect(void *priv)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    if (con_handle) {
        if (BLE_ST_SEND_DISCONN != get_ble_work_state()) {
            log_info(">>>ble send disconnect\n");
            set_ble_work_state(BLE_ST_SEND_DISCONN);
            ble_op_disconnect(con_handle);  //断开 ble 连接
        } else {
            log_info(">>>ble wait disconnect...\n");
        }
        return APP_BLE_NO_ERROR;
    } else {
        return APP_BLE_OPERATION_ERROR;
    }
}

//获取ATT发送模块cbuffer 可写入数据的长度
static int get_buffer_vaild_len(void *priv)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    u32 vaild_len = 0;
    ble_op_att_get_remain(&vaild_len);
    return vaild_len;
}
//连续发送
int app_send_user_data_do(void *priv, u8 *data, u16 len)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
#if PRINT_DMA_DATA_EN   //0
    if (len < 128) {
        log_info("-le_tx(%d):");
        log_info_hexdump(data, len);
    } else {
        putchar('L');
    }
#endif
    return app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, data, len, ATT_OP_AUTO_READ_CCC);
}
//检查是否可以往协议栈发送数据
int app_send_user_data_check(u16 len)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    u32 buf_space = get_buffer_vaild_len(0);    //获取ATT发送模块cbuffer 可写入数据的长度
    if (len <= buf_space) {
        return 1;
    }
    return 0;
}


static int regiest_wakeup_send(void *priv, void *cbk)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    ble_resume_send_wakeup = cbk;
    return APP_BLE_NO_ERROR;
}

static int regiest_recieve_cbk(void *priv, void *cbk)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    channel_priv = (u32)priv;
    app_recieve_callback = cbk;
    return APP_BLE_NO_ERROR;
}

static int regiest_state_cbk(void *priv, void *cbk)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    channel_priv = (u32)priv;
    app_ble_state_callback = cbk;
    return APP_BLE_NO_ERROR;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


u8 *ble_get_scan_rsp_ptr(u16 *len)  //获取扫描应答
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    if (len) {
        *len = scan_rsp_data_len;
    }
    return scan_rsp_data;
}

u8 *ble_get_adv_data_ptr(u16 *len)  //获取广播数据
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    if (len) {
        *len = adv_data_len;
    }
    return adv_data;
}

u8 *ble_get_gatt_profile_data(u16 *len)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    *len = sizeof(profile_data);
    return (u8 *)profile_data;
}


void bt_ble_adv_enable(u8 enable)   //开启BLE广播
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    set_adv_enable(0, enable);
}

u16 bt_ble_is_connected(void)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    return con_handle;
}

void ble_module_enable(u8 en)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    log_info("mode_en:%d\n", en);
    if (en) {
        adv_ctrl_en = 1;
        bt_ble_adv_enable(1);   //开启BLE广播
    } else {
        if (con_handle) {
            adv_ctrl_en = 0;
            ble_disconnect(NULL);   //断开连接
        } else {
            bt_ble_adv_enable(0);   //停止广播
            adv_ctrl_en = 0;
        }
    }
}


//流控使能 EN: 1-停止收数 or 0-继续收数
int ble_trans_flow_enable(u8 en)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    int ret = -1;
#if ATT_DATA_RECIEVT_FLOW       //流控功能使能,未使用功能
    if (con_handle) {
        att_server_flow_hold(con_handle, en);
        ret = 0;
    }
#endif
    log_info("ble_trans_flow_enable:%d,%d\n", en, ret);
    return ret;
}

//for test
static void timer_trans_flow_test(void)
{
    static u8 sw = 0;
    if (con_handle) {
        sw = !sw;
        ble_trans_flow_enable(sw);
    }
}

static const char ble_ext_name[] = "(BLE)";

void bt_ble_init(void)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    log_info("***** ble_init******\n");
    char *name_p;

#if DOUBLE_BT_SAME_NAME    //双模同名，0
    u8 ext_name_len = 0;
#else
    u8 ext_name_len = sizeof(ble_ext_name) - 1;
#endif

    name_p = bt_get_local_name();   //读取flash里蓝牙名字
    gap_device_name_len = strlen(name_p);
    if (gap_device_name_len > BT_NAME_LEN_MAX - ext_name_len) {
        gap_device_name_len = BT_NAME_LEN_MAX - ext_name_len;
    }

    memcpy(gap_device_name, name_p, gap_device_name_len);

#if DOUBLE_BT_SAME_NAME == 0    //1
    //增加后缀，区分名字
    // memcpy(&gap_device_name[gap_device_name_len], "(BLE)", ext_name_len);
    // gap_device_name_len += ext_name_len;
#endif

    log_info("ble name(%d): %s \n", gap_device_name_len, gap_device_name);
 printf("**************************************************77****************");
#if ATT_DATA_RECIEVT_FLOW   ////流控功能使能，0
    log_info("att_server_flow_enable\n");
    att_server_flow_enable(1);
    /* sys_timer_add(0, timer_trans_flow_test, 3000); */
#endif

    set_ble_work_state(BLE_ST_INIT_OK); //设置蓝牙工作状态
    extern u8 ble_state;
    if(ble_state)
    {
        ble_module_enable(1);       //蓝牙模式使能
    }
    else
    {

        ble_module_enable(0);       //蓝牙模式失能
    }
        
}




void bt_ble_exit(void)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    log_info("***** ble_exit******\n");

    ble_module_enable(0);

#if TEST_SEND_DATA_RATE //0
    server_timer_stop();
#endif

}





void ble_app_disconnect(void)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    ble_disconnect(NULL);
}

#if RCSP_BTMATE_EN
static int rcsp_send_user_data_do(void *priv, u8 *data, u16 len)
{
    log_info("rcsp_tx:%x\n", len);
#if PRINT_DMA_DATA_EN
    if (len < 128) {
        log_info("-dma_tx(%d):");
        log_info_hexdump(data, len);
    } else {
        putchar('L');
    }
#endif
    return app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, data, len, ATT_OP_AUTO_READ_CCC);
}
#endif

#if RCSP_BTMATE_EN
static const struct ble_server_operation_t mi_ble_operation = {
    .adv_enable = set_adv_enable,
    .disconnect = ble_disconnect,
    .get_buffer_vaild = get_buffer_vaild_len,
    .send_data = (void *)rcsp_send_user_data_do,
    .regist_wakeup_send = regiest_wakeup_send,
    .regist_recieve_cbk = regiest_recieve_cbk,
    .regist_state_cbk = regiest_state_cbk,
};
#else
static const struct ble_server_operation_t mi_ble_operation = {
    .adv_enable = set_adv_enable,
    .disconnect = ble_disconnect,
    .get_buffer_vaild = get_buffer_vaild_len,
    .send_data = (void *)app_send_user_data_do,
    .regist_wakeup_send = regiest_wakeup_send,
    .regist_recieve_cbk = regiest_recieve_cbk,
    .regist_state_cbk = regiest_state_cbk,
};
#endif

void ble_get_server_operation_table(struct ble_server_operation_t **interface_pt)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    *interface_pt = (void *)&mi_ble_operation;
}

void ble_server_send_test_key_num(u8 key_num)
{
//    printf("------------%s---------------\n", __FUNCTION__);       //打印功能函数名
    ;
}

#endif


