/*********************************************************************************************
    *   Filename        : led_music.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  :

    *
*********************************************************************************************/
#include "system/app_core.h"
#include "system/includes.h"

#include "app_config.h"
#include "app_action.h"

u32 triac_turn_on_timer=10;
u32 count_val=0;


/****************************************************************************************
**名称:捕获器3中断服务程序
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
___interrupt
AT_VOLATILE_RAM_CODE
void capture_cycle_timer_isr(void)
{
//    printf("triac_turn_on_timer = %d\n",triac_turn_on_timer);

    JL_PORTB->OUT &= ~ BIT(7);

    count_val = JL_TIMER3->PRD;
    JL_TIMER3->CNT = 0;			    //清计数值
    JL_TIMER3->PRD = 0;
    JL_TIMER3->CON |= (1<<14);      //清除中断标志

    JL_TIMER0->CNT = 0;
    JL_TIMER0->PRD = count_val*(1-triac_turn_on_timer/100);
    JL_TIMER0->CON |= (0b01 << 0);	//启动定时器
    JL_TIMER0->CON |= (1<<14);      //清除定时器中断标志
}

/****************************************************************************************
**名称:捕获器3初始化
**功能:下降沿捕获
**说明:
**备注:
**日期:
*****************************************************************************************/
void capture_cycle_time_init(void)
{
    u32 u_clk = 24000000;

    gpio_set_die(IO_PORTB_06, 1);
	gpio_set_direction(IO_PORTB_06, 1);
	gpio_set_pull_up(IO_PORTB_06,1);
    gpio_set_pull_down(IO_PORTB_06, 0);

    //初始化timer
    JL_TIMER3->CON = 0;
    JL_TIMER3->CON |= (0b110 << 10);					//时钟源选择STD_24M时钟源
    JL_TIMER3->CON |= (0b0001 << 4);					//时钟源再4分频
    JL_TIMER3->CNT = 0;									//清计数值
    JL_TIMER3->PRD = 0;				                    //清计数值
    JL_TIMER3->CON |= (0b11 << 0);						//下降沿捕获模式
    JL_TIMER3->CON |= (1<<14);                          //清除中断标志

    request_irq(IRQ_TIME3_IDX, 1, capture_cycle_timer_isr, 0);  //定义中断服务程序
    gpio_set_fun_input_port(IO_PORTB_06, PFI_TMR3_CAP); //PB6作捕获脚
}

/****************************************************************************************
**名称:定时器0中断服务程序
**功能:
**说明:
**备注:
**日期:
*****************************************************************************************/
___interrupt
AT_VOLATILE_RAM_CODE
void triac_turn_on_timer_isr(void)
{
//    printf("triac_turn_on\n");

    JL_PORTB->OUT |= BIT(7);

    JL_TIMER0->CON |= (1<<14);                          //清除定时器中断标志
    JL_TIMER0->CON &= 0xfffc;                           //暂停计数模式
    JL_TIMER0->PRD = 0;
}
/****************************************************************************************
**名称:定时器0初始化
**功能:用于定时计数
**说明:
**备注:
**日期:
*****************************************************************************************/
void buty_time(void)
{
    //初始化timer
    JL_TIMER0->CON = 0;
    JL_TIMER0->CON |= (0b110 << 10);					//时钟源选择STD_24M时钟源
    JL_TIMER0->CON |= (0b0001 << 4);					//时钟源再4分频
//    JL_TIMER0->CON &= 0xfffc;                           //暂停计数模式
    JL_TIMER0->CON |= (1<<14);                          //清除定时器中断标志
    JL_TIMER0->CNT = 0;                                 //

    request_irq(IRQ_TIME0_IDX, 1, triac_turn_on_timer_isr, 0);
}





