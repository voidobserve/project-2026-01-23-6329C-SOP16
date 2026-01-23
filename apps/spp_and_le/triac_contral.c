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

#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
#include "jl_kws/jl_kws_api.h"
#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */

#define LOG_TAG_CONST       APP
#define LOG_TAG             "[APP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


static const u32 timer_div[] = {
    /*0000*/    1,
    /*0001*/    4,
    /*0010*/    16,
    /*0011*/    64,
    /*0100*/    2,
    /*0101*/    8,
    /*0110*/    32,
    /*0111*/    128,
    /*1000*/    256,
    /*1001*/    4 * 256,
    /*1010*/    16 * 256,
    /*1011*/    64 * 256,
    /*1100*/    2 * 256,
    /*1101*/    8 * 256,
    /*1110*/    32 * 256,
    /*1111*/    128 * 256,
};

u32 GC_Total_Count = 0xA0000000;
u16 GC_High_Count = 0;
u16 GC_Low_Count = 0;

u8  GC_RGB_List = 0;
u8  GC_List_Step = 0;
u32 GC_Red = 0xFFFFFFFF;
u32 GC_Gre = 0xFFFFFFFF;
u32 GC_Blu = 0xFFFFFFFF;
u32 GC_Buff[3] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};

u16 Test_RGB = 0;

extern void GCLight_Falling_isr();
___interrupt
void GCLight_Leading_isr()
{
    JL_TIMER3->CON |= BIT(14);
    // GC_Low_Count = JL_TIMER3->PRD;
    JL_TIMER3->CON &= ~(BIT(1) | BIT(0));
    JL_TIMER3->CNT = 0;
    request_irq(IRQ_TIME3_IDX, 5, GCLight_Falling_isr, 0);
    JL_TIMER3->CON |= (BIT(1) | BIT(0));

    GC_Total_Count = JL_TIMER0->CNT;
    JL_TIMER0->CNT = 0;

    JL_TIMER2->CON &= ~BIT(0);
    JL_PORTA->OUT &= ~BIT(0);   //r
    JL_PORTA->OUT &= ~BIT(1);   //g
    JL_PORTB->OUT &= ~BIT(10);  //b
}

___interrupt
void GCLight_Falling_isr()
{
    JL_TIMER3->CON |= BIT(14);
    // GC_High_Count = JL_TIMER3->PRD;
    JL_TIMER3->CON &= ~(BIT(1) | BIT(0));
    JL_TIMER3->CNT = 0;
    request_irq(IRQ_TIME3_IDX, 5, GCLight_Leading_isr, 0);
    JL_TIMER3->CON |= (BIT(1));

    JL_TIMER2->CNT = 0;
    // JL_TIMER2->PRD = GC_Red;
    GC_List_Step = 0;
    JL_TIMER2->PRD = GC_Buff[0];
    JL_TIMER2->CON |= BIT(0);

    JL_TIMER0->CNT = 0;
}

___interrupt
AT_VOLATILE_RAM_CODE
static void GCLight_Control_isr()
{
    JL_TIMER2->CON |= BIT(14);
    JL_TIMER2->CON &= ~BIT(0);
    GC_List_Step++;
    if (GC_List_Step == 1) {
        if (GC_RGB_List < 2) {
            JL_PORTA->OUT |= BIT(0);   //r
        } else if (GC_RGB_List < 4) {
            JL_PORTA->OUT |= BIT(1);   //g
        } else {
            JL_PORTB->OUT |= BIT(10);   //b
        }
        if (GC_Buff[1] == 0) {
            GC_List_Step++;
            goto __list_2;
        }
        JL_TIMER2->CNT = 0;
        JL_TIMER2->PRD = GC_Buff[1];
        JL_TIMER2->CON |= BIT(0);
    } else if (GC_List_Step == 2) {
__list_2:
        if (GC_RGB_List == 2 || GC_RGB_List == 4) {
            JL_PORTA->OUT |= BIT(0);   //r
        } else if (GC_RGB_List == 0 || GC_RGB_List == 5) {
            JL_PORTA->OUT |= BIT(1);   //g
        } else {
            JL_PORTB->OUT |= BIT(10);   //b
        }
        if (GC_Buff[2] == 0) {
            GC_List_Step++;
            goto __list_3;
        }
        JL_TIMER2->CNT = 0;
        JL_TIMER2->PRD = GC_Buff[2];
        JL_TIMER2->CON |= BIT(0);
    } else if (GC_List_Step == 3) {
__list_3:
        if (GC_RGB_List == 3 || GC_RGB_List == 5) {
            JL_PORTA->OUT |= BIT(0);   //r
        } else if (GC_RGB_List == 1 || GC_RGB_List == 4) {
            JL_PORTA->OUT |= BIT(1);   //g
        } else {
            JL_PORTB->OUT |= BIT(10);   //b
        }
    }
}

___interrupt
AT_VOLATILE_RAM_CODE
static void GCLight_Sum_isr()
{
    JL_TIMER0->CON |= BIT(14);
    // GC_Low_Count++;
}

void GCLight_ZeroDetect(void)
{
    // // JL_PORTB->DIE |= BIT(9); JL_PORTB->DIR |= BIT(9); JL_PORTB->PU  |= BIT(9);      //zero detect
    JL_PORTA->DIE |= BIT(0); JL_PORTA->DIR &= ~BIT(0); JL_PORTA->OUT &= ~BIT(0);    //r
    JL_PORTA->DIE |= BIT(1); JL_PORTA->DIR &= ~BIT(1); JL_PORTA->OUT &= ~BIT(1);    //g
    JL_PORTB->DIE |= BIT(10); JL_PORTB->DIR &= ~BIT(10); JL_PORTB->OUT &= ~BIT(10); //b

    gpio_irflt_in(25);
    gpio_set_direction(25, 1);
    gpio_set_die(25, 1);
    gpio_set_pull_up(25, 1);

    SFR(JL_IOMAP->CON0, 8, 3, 7);
    request_irq(IRQ_TIME3_IDX, 5, GCLight_Leading_isr, 0);

    JL_IR->RFLT_CON = 0;
    /* JL_IR->RFLT_CON |= BIT(7);		//256 div */
    /* JL_IR->RFLT_CON |= BIT(3);		//osc 24m */
    JL_IR->RFLT_CON |= BIT(7) | BIT(4);		//512 div
    JL_IR->RFLT_CON |= BIT(3) | BIT(2);		//PLL_48m（兼容省晶振）
    JL_IR->RFLT_CON |= BIT(0);		//irflt enable

    #define APP_TIMER_CLK           12000000L //pll12m
    #define TIMER_UNIT_MS           1
    #define MAX_TIME_CNT 0x07ff //分频准确范围，更具实际情况调整
    #define MIN_TIME_CNT 0x0030
    u32 clk;
    u32 prd_cnt;
    u8 index;

    for (index = 0; index < (sizeof(timer_div) / sizeof(timer_div[0])); index++) {
        prd_cnt = TIMER_UNIT_MS * (APP_TIMER_CLK / 1200) / timer_div[index];
        if (prd_cnt > MIN_TIME_CNT && prd_cnt < MAX_TIME_CNT) {
            break;
        }
    }

    /* IR_TIME_REG->CON = ((index << 4) | BIT(3) | BIT(1) | BIT(0));//选择osc时钟 */
    JL_IOMAP->CON1 |= BIT(27);//这里已选了timer3,时钟源选io信号里的pll_12m,不是所有的timer都可选pll,修改请看文档
    JL_TIMER3->CON = ((index << 4) | BIT(2) | BIT(1));
    extern int GCtimer2_init();
    GCtimer2_init();
}

int GCtimer2_init()
{
    u32 prd_cnt;
    u8 index;
    for (index = 0; index < (sizeof(timer_div) / sizeof(timer_div[0])); index++) {
        prd_cnt = TIMER_UNIT_MS * (clk_get("timer") / 50) / timer_div[index];
        if (prd_cnt > 0x100 && prd_cnt < 0x7fff) {
            break;
        }
    }
    JL_TIMER2->CNT = 0;
    request_irq(IRQ_TIME2_IDX, 1, GCLight_Control_isr, 0);
    JL_TIMER2->CON = ((index << 4) | BIT(3));

    JL_TIMER0->CNT = 0;
    JL_TIMER0->PRD = 0xFFFFFFFF;
    request_irq(IRQ_TIME0_IDX, 1, GCLight_Sum_isr, 0);
    JL_TIMER0->CON = ((index << 4) | BIT(3));
    JL_TIMER0->CON |= BIT(0);
    return 0;
}

void GCLight_PWM_SetDuty(u16 RedDuty, u16 GreDuty, u16 BluDuty)
{
    if (RedDuty > 10000) RedDuty = 10000;
    if (GreDuty > 10000) GreDuty = 10000;
    if (BluDuty > 10000) BluDuty = 10000;
    u32 Tmp = GC_Total_Count;
    if (RedDuty)
        GC_Red = Tmp - (Tmp * RedDuty / 10000);
    else
        GC_Red = 0xFFFFFFFF;
    if (GreDuty)
        GC_Gre = Tmp - (Tmp * GreDuty / 10000);
    else
        GC_Gre = 0xFFFFFFFF;
    if (BluDuty)
        GC_Blu = Tmp - (Tmp * BluDuty / 10000);
    else
        GC_Blu = 0xFFFFFFFF;
    if (GC_Red <= GC_Gre && GC_Red <= GC_Blu)
    {
        GC_Buff[0] = GC_Red;
        if (GC_Gre <= GC_Blu)
        {
            GC_Buff[1] = GC_Gre - GC_Red;
            GC_Buff[2] = GC_Blu - GC_Gre - GC_Red;
            GC_RGB_List = 0;
        }
        else
        {
            GC_Buff[1] = GC_Blu - GC_Red;
            GC_Buff[2] = GC_Gre - GC_Blu - GC_Red;
            GC_RGB_List = 1;
        }
    }
    else if (GC_Gre <= GC_Red && GC_Gre <= GC_Blu)
    {
        GC_Buff[0] = GC_Gre;
        if (GC_Red <= GC_Blu)
        {
            GC_Buff[1] = GC_Red - GC_Gre;
            GC_Buff[2] = GC_Blu - GC_Red - GC_Gre;
            GC_RGB_List = 2;
        }
        else
        {
            GC_Buff[1] = GC_Blu - GC_Gre;
            GC_Buff[2] = GC_Red - GC_Blu - GC_Gre;
            GC_RGB_List = 3;
        }
    }
    else if (GC_Blu <= GC_Gre && GC_Blu <= GC_Red)
    {
        GC_Buff[0] = GC_Blu;
        if (GC_Red <= GC_Gre)
        {
            GC_Buff[1] = GC_Red - GC_Blu;
            GC_Buff[2] = GC_Gre - GC_Red - GC_Blu;
            GC_RGB_List = 4;
        }
        else
        {
            GC_Buff[1] = GC_Gre - GC_Blu;
            GC_Buff[2] = GC_Red - GC_Gre - GC_Blu;
            GC_RGB_List = 5;
        }
    }
}
