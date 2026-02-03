#include "system/app_core.h"
#include "system/includes.h"
#include "server/server_core.h"
#include "app_config.h"
#include "app_action.h"
#include "os/os_api.h"
#include "btcontroller_config.h"
#include "btctrler/btctrler_task.h"
#include "config/config_transport.h"
#include "btstack/avctp_user.h"
#include "btstack/btstack_task.h"
#include "rcsp_bluetooth.h"
#include "rcsp_user_update.h"
#include "app_charge.h"
#include "app_chargestore.h"
#include "app_power_manage.h"

#include "app_main.h"
#include "led_strand_effect.h"
#include "app_feedback.h"

extern fc_effect_t fc_effect; // 幻彩灯串效果数据
//******************************************************************************//
//
//
//
//
//******************************************************************************//
#define UART_CBUF_SIZE 0x100
#define UART_FRAM_SIZE 0x100 // 单次数据包最大值(每次cbuf缓存达到fram或串口收到一次数据, 就会起一次中断)

static u8 devBuffer_static[UART_CBUF_SIZE] __attribute__((aligned(4))); // dev DMA memory

const uart_bus_t *uart_bus_a;
extern void printf_buf(u8 *buf, u32 len);
extern void ble_send_user_data(u8 *buffer, u8 buffer_size);
u16 uart_a_send_api(uint8_t *data, u16 len);
u8 uart_rx_flag = 1;
u8 _rx_buf[300];
u8 _rx_len = 0;

// 串口接受处理
void uart_rx_handler_a(void)
{
    u8 len;
    printf("uart_rx_handler_a\n");
    if (uart_bus_a)
    {
        // uart_bus_a->write("aa",2);

        len = uart_bus_a->read(_rx_buf, UART_FRAM_SIZE, 0); // 全部收齐
        _rx_len = len;
        if (len > UART_FRAM_SIZE)
        {
            len = 0;
            printf("uart overflow\n");
        }

        uart_rx_flag = 1;
    }
}

static void user_uart_isr_cb(void *ut_bus, u32 status)
{
    struct sys_event e;

    if (status == UT_RX || status == UT_RX_OT)
    {
#if FLOW_CONTROL
        uart1_flow_ctl_rts_suspend();
#endif
        e.type = SYS_DEVICE_EVENT;
        e.arg = (void *)DEVICE_EVENT_FROM_BOARD_UART;
        e.u.dev.event = 0;
        e.u.dev.value = 0;
        sys_event_notify(&e);
    }
}

int ct_uart_init_a(u32 baud)
{
    struct uart_platform_data_t u_arg = {0};
    u_arg.tx_pin = IO_PORTA_07;
    u_arg.rx_pin = IO_PORTB_06;

    u_arg.rx_cbuf = devBuffer_static;
    u_arg.rx_cbuf_size = UART_CBUF_SIZE;
    u_arg.frame_length = UART_FRAM_SIZE;
    u_arg.rx_timeout = 5; // ms
    u_arg.isr_cbfun = user_uart_isr_cb;
    u_arg.baud = baud;
    u_arg.is_9bit = 0;

    uart_bus_a = uart_dev_open(&u_arg);

    if (uart_bus_a != NULL)
    {
        printf("uart_dev_open() success\n");
        return 1;
    }
    return 0;
}

#if 0
void tuya_UT1_putbyte(char a)
{
    if (JL_UART1->CON0 & BIT(0)) {
        JL_UART1->BUF = a;
        __asm__ volatile("csync");
        while ((JL_UART1->CON0 & BIT(15)) == 0);
        JL_UART1->CON0 |= BIT(13);
    }
}
#endif
/*
u16 uart_dev_test_send_api(uint8_t *data, u16 len)
{
    int i = 0;
    while (len--) {
        tuya_UT1_putbyte(data[i++]);
    }
    return len;
}
*/
u16 uart_a_send_api(uint8_t *data, u16 len)
{
    uart_bus_a->write(data, len);
    return len;
}

void test_uart_a(void)
{
    u8 buf[10] = {0};
    u8 i = 0;

    static u8 count = 0;
    if (count++ > 100)
    {
        count = 0;
        for (i = 0; i < 10; i++)
        {
            buf[i] = i;
        }
        uart_a_send_api(buf, 10);
        printf("uart_a_send_api\n");
    }
}

void uart_key_handle(void)
{
    if (uart_rx_flag == 1)
    {
        uart_rx_flag = 0;

        printf_buf(_rx_buf, _rx_len);

        if (_rx_buf[0] == 0xAA && _rx_buf[1] == 0xFA)
        {

            if (_rx_buf[2] == 0x14)
            {
                // 打开流星
                soft_turn_on_the_light(); // 软开灯
                app_feedback_meteor_power_status(fc_effect.on_off_flag);
            }
            else if (_rx_buf[2] == 0x15)
            {
                // 关闭流星
                soft_rurn_off_lights(); // 软开灯
                app_feedback_meteor_power_status(fc_effect.on_off_flag);
            }
            else if (_rx_buf[2] == 0x16)
            {
                // 流星调快点
                extern void ls_speed_plus(void);
                ls_speed_plus();

                u8 app_feedback_speed = 100 - fc_effect.speed;
                app_feedback_meteor_speed(app_feedback_speed);
            }
            else if (_rx_buf[2] == 0x17)
            {
                // 流星调慢点
                extern void ls_speed_sub(void);
                ls_speed_sub();

                u8 app_feedback_speed = 100 - fc_effect.speed;
                app_feedback_meteor_speed(app_feedback_speed);
            }
            else if (_rx_buf[2] == 0x18)
            {
                // 流星调长点
                void len_put(void);
                len_put();
            }
            else if (_rx_buf[2] == 0x19)
            {
                // 流星调短点
                void len_sub(void);
                len_sub();
            }
            else if (_rx_buf[2] == 0x1a)
            {
                // 间隔调快点
                void meteor_p_sub(void);
                meteor_p_sub();

                app_feedback_meteor_interval(fc_effect.meteor_period);
            }
            else if (_rx_buf[2] == 0x1b)
            {
                // 间隔调慢点
                void meteor_p_pul(void);
                meteor_p_pul();

                app_feedback_meteor_interval(fc_effect.meteor_period);
            }
            else if (_rx_buf[2] == 0x1c)
            {
                // 流星调最快
                void max_speed(void);
                max_speed();

                u8 app_feedback_speed = 100 - fc_effect.speed;
                app_feedback_meteor_speed(app_feedback_speed);
            }
            else if (_rx_buf[2] == 0x1d)
            {
                // 流星调最慢
                void min_speed(void);
                min_speed();

                u8 app_feedback_speed = 100 - fc_effect.speed;
                app_feedback_meteor_speed(app_feedback_speed);
            }
            else if (_rx_buf[2] == 0x1e)
            {
                // 随机切换流星
                random_effect();
            }
            else if (_rx_buf[2] == 0x1f)
            {
                // 切换普通流星
                set_mereor_mode(1);
            }
            else if (_rx_buf[2] == 0x20)
            {
                // 切换劳斯流星
            }
            else if (_rx_buf[2] == 0x21)
            {
                // 切换闪电
                set_mereor_mode(14);
            }

            save_user_data_area3();
        }

        memset(_rx_buf, 0, sizeof(_rx_buf));
        _rx_len = 0;
    }
}

#if 0
bool uart_dev_test_init_api(void)
{
    bool ret;

    ret = ct_uart_init(115200);
    return ret;
}
#endif
