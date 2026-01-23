
#include "ws2812_bsp.h"
#include "debug.h"
#include "my_effect.h"
#include "led_strip_sys.h"
#include "led_strand_effect.h"


extern fc_effect_t fc_effect;//幻彩灯串效果数据
static unsigned long tick_ms;
void ws281x_init()
{

}
extern uint8_t           rOffset;    ///< Red index within each 3- or 4-byte pixel
extern uint8_t           gOffset;    ///< Index of green byte
extern uint8_t           bOffset;    ///< Index of blue byte
u8 buf[20*3];
void ws281x_show(unsigned char *pixels_pattern, unsigned short pattern_size)
{
    // printf_buf(pixels_pattern,pattern_size);
    unsigned short i,j=0,k=0;
    
    u8 r,g,b;

  unsigned short ii = 0;
    for(i=0; i< pattern_size/3; i++)
    {
         *(buf+i) = *(pixels_pattern + i*3);

    }

    for(i=0; i< pattern_size/3; i++)
    {
        if(j==0)
        {
            r = *(buf+i);
        }
        else if(j==1)
        {
            g = *(buf+i);
        }
        else if(j==2)
        {
            b = *(buf+i);
        }
        j++;
        if(j==3)
        {
            j=0;
            *(buf + rOffset + (i-2)) = r;
            *(buf + gOffset + (i-2)) = g;
            *(buf + bOffset + (i-2)) = b;
        }
    }
  
if( fc_effect.on_off_flag == DEVICE_OFF)
{
    memset(buf,0,sizeof(buf));
}

// printf_buf(buf,pattern_size);
   
    extern void ledc_send_rgbbuf_isr(u8 index, u8 *rgbbuf, u32 buf_len, u16 again_cnt);
    ledc_send_rgbbuf_isr(0, buf, pattern_size/3, 0);
 
  

}

// 周期10ms
unsigned long HAL_GetTick(void)
{
    return tick_ms;
}

// 每10ms调用一次
void run_tick_per_10ms(void)
{
    tick_ms+=10;
}




