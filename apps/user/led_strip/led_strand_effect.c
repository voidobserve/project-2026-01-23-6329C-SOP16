/****************************
@led_strand_effect.c
适用：
产品ID: yxwh27s5
品类：幻彩户外串灯-蓝牙
协议：BLE
负责幻彩灯串效果制作
*****************************/
#include "system/includes.h"
#include "led_strand_effect.h"
#include "WS2812FX.H"
#include "ws2812fx_effect.h"
#include "Adafruit_NeoPixel.H"


extern void printf_buf(u8 *buf, u32 len);
static void static_mode(void);
static void fc_smear_adjust(void);
static void fc_pair_effect(void);
static void ls_scene_effect(void);
void fc_set_style_custom(void);
void custom_effect(void);
static void strand_rainbow(void);
void strand_jump_change(void) ;
void strand_breath(void) ;
void strand_twihkle(void) ;
void strand_flow_water(void) ;
void strand_chas_light(void) ;
void strand_colorful(void) ;
void strand_grandual(void) ;
void set_static_mode(u8 r, u8 g, u8 b);
 void soft_turn_on_the_light(void)  ; 
 void soft_rurn_off_lights(void)  ;



fc_effect_t fc_effect;//幻彩灯串效果数据



// 效果数据初始化
void fc_data_init(void)
{
    u16 num;
    fc_effect.on_off_flag = DEVICE_ON;
    fc_effect.led_num = 15;
    fc_effect.Now_state = ACT_CUSTOM;
    fc_effect.custom_index = 1;
    fc_effect.speed=20;
    fc_effect.meteor_period = 10;
    fc_effect.period_cnt = fc_effect.meteor_period*1000;
    fc_effect.mode_cycle = 1;
    fc_effect.b = 255;
    fc_effect.music.s = 85;


}
// WS2812FX_mode_comet
// WS2812FX_mode_scan
extern uint16_t WS2812FX_mode_comet_1(void) ;

void full_color_init(void)
{
    WS2812FX_init(fc_effect.led_num + 5,NEO_RGB);
    WS2812FX_setBrightness( fc_effect.b );
    if(fc_effect.on_off_flag == DEVICE_ON)
    {
        soft_turn_on_the_light();
    }
    else
    {
        soft_rurn_off_lights();
    }

    set_fc_effect();
}

/**************************************************效果调度函数*****************************************************/

void set_fc_effect(void)
{

    if(fc_effect.on_off_flag == DEVICE_ON)
    {
        fc_effect.period_cnt = 0;
        switch (fc_effect.Now_state)
        {
            case IS_light_scene:
                ls_scene_effect();
                /* code */
                break;
            case ACT_TY_PAIR:
                printf("\n ACT_TY_PAIR");

                // 配对完成，要恢复fc_effect.Now_state
                fc_pair_effect();
                /* code */
                break;
            case ACT_CUSTOM:
                printf("\n ACT_CUSTOM");
                custom_effect();
                /* code */
                break;
            case IS_light_music:
                /* code */
                break;
            case IS_smear_adjust:
                printf("\n IS_smear_adjust");
                fc_smear_adjust();
                break;
            case IS_STATIC:
                static_mode();
                break;
            default:
                break;
        }

    } //if(fc_effect.on_off_flag == DEVICE_ON)
}

// -------------------------------------------------------------------------------------------------工具
void ls_set_colors(uint8_t n, color_t *c)
{
    uint32_t colors[MAX_NUM_COLORS];
    uint8_t i;
    for(i=0; i < n; i++)
    {
        colors[i] = c[i].r << 16 | c[i].g << 8 | c[i].b;
    }
    WS2812FX_setColors(0,colors);
}

/***************************************************自定义效果*****************************************************/


extern uint16_t power_on_effect(void);
extern uint16_t power_off_effect(void);

// 1~4，正向流水效果
// 5~8：反向流水效果
// 9~10：音乐律动
u8 meteor_mode = 0;
// FADE_SLOW：12颗
// FADE_MEDIUM：6颗
// FADE_FAST：5颗灯
// FADE_XFAST:3颗灯
const u8 fade_type[4] =
{
    FADE_XFAST,FADE_FAST,FADE_MEDIUM,FADE_SLOW
};

void set_power_off(void)
{
    fc_effect.custom_index = 1; //关机效果
    fc_set_style_custom(); //自定义效果
    set_fc_effect();
}

void change_meteor_mode(void)
{
    fc_effect.custom_index++;
    if(fc_effect.custom_index>0x0a)
    {
        fc_effect.custom_index = 1;
    }
    set_fc_effect();
}





void meteor_len_pul(void)
{
    if(fc_effect.custom_index == 11 || fc_effect.custom_index == 12 || fc_effect.custom_index ==13 || fc_effect.custom_index == 9 || fc_effect.custom_index == 10)
    {
        fc_effect.custom_index = 1;
    }

    if(fc_effect.custom_index > 0 && fc_effect.custom_index < 4)
    {
        fc_effect.custom_index++;
 
    }
    else if(fc_effect.custom_index >= 5 && fc_effect.custom_index < 8 )
    {
        fc_effect.custom_index++;
     
    }
    printf("fc_effect.custom_index = %d", fc_effect.custom_index);
    set_fc_effect();

}

void meteor_len_sub(void)
{
    if(fc_effect.custom_index == 11 || fc_effect.custom_index == 12 || fc_effect.custom_index ==13 || fc_effect.custom_index == 9 || fc_effect.custom_index == 10)
    {
        fc_effect.custom_index = 8;
    }

   if(fc_effect.custom_index > 1  && fc_effect.custom_index < 5)
    {
        fc_effect.custom_index--;

    }
    else if(fc_effect.custom_index > 5 && fc_effect.custom_index < 9 )
    {
        fc_effect.custom_index--;
 
    }

    printf("fc_effect.custom_index = %d", fc_effect.custom_index);


    set_fc_effect();
    
}



void set_mereor_mode(u8 m)
{

    {
        fc_effect.custom_index = m;
        set_fc_effect();
    }
}
void set_mereor_speed(u8 s)
{
    fc_effect.speed = 100-s;
    // if(fc_effect.speed < 10) fc_effect.speed = 10;
    set_fc_effect();
}
u8 get_custom_index(void)
{
    return fc_effect.custom_index;
}

void set_custom_index(u8 m)
{
    if(m<=13)fc_effect.custom_index = m;
    else fc_effect.custom_index = 1;
}

const u8 size_type[4] =
{
    SIZE_SMALL,SIZE_MEDIUM,SIZE_LARGE,SIZE_XLARGE
    //0           
};

void custom_effect(void)
{
extern uint16_t WS2812FX_mode_comet_2(void);
extern uint16_t WS2812FX_mode_comet_3(void);
    fc_effect.period_cnt = 0;

    if( fc_effect.custom_index==1||
        fc_effect.custom_index==2||
        fc_effect.custom_index==3||
        fc_effect.custom_index==4) //流星效果
    {
        WS2812FX_stop();
        // FADE_SLOW：12颗
        // FADE_MEDIUM：6颗
        // FADE_FAST：5颗灯
        // FADE_XFAST:3颗灯
        WS2812FX_setSegment_colorOptions(
            0,                                          //第0段
            0,fc_effect.led_num-1,                      //起始位置，结束位置
            &WS2812FX_mode_comet_1,                     //效果
            WHITE,                                      //颜色，WS2812FX_setColors设置
            fc_effect.speed,                            //速度
            size_type[fc_effect.custom_index-1] | 0);             //选项，这里像素点大小：3 REVERSE决定方向
        WS2812FX_start();
    }
    else if( fc_effect.custom_index==5||
        fc_effect.custom_index==6||
        fc_effect.custom_index==7||
        fc_effect.custom_index==8) //流星效果
    {
        WS2812FX_stop();
        // FADE_SLOW：12颗
        // FADE_MEDIUM：6颗
        // FADE_FAST：5颗灯
        // FADE_XFAST:3颗灯
        WS2812FX_setSegment_colorOptions(
            0,                                          //第0段
            0,fc_effect.led_num-1,                      //起始位置，结束位置
            &WS2812FX_mode_comet_1,                     //效果
            WHITE,                                      //颜色，WS2812FX_setColors设置
            fc_effect.speed,                            //速度
            size_type[fc_effect.custom_index-5] | REVERSE);       //选项，这里像素点大小：3 REVERSE决定方向
        WS2812FX_start();
    }
    else if(fc_effect.custom_index == 9)
    {
        WS2812FX_setSegment_colorOptions(
            0,                                          //第0段
            0,fc_effect.led_num-1,                      //起始位置，结束位置
            &WS2812FX_mode_comet_2,                     //效果
            WHITE,                                      //颜色，WS2812FX_setColors设置
            fc_effect.speed,                            //速度
            size_type[0] | 0);             //选项，这里像素点大小：3 REVERSE决定方向
        WS2812FX_start();
    }
    else if(fc_effect.custom_index == 10)
    {
        WS2812FX_setSegment_colorOptions(
            0,                                          //第0段
            0,fc_effect.led_num-1,                      //起始位置，结束位置
            &WS2812FX_mode_comet_2,                     //效果
            WHITE,                                      //颜色，WS2812FX_setColors设置
            fc_effect.speed,                            //速度
            size_type[0] | REVERSE);             //选项，这里像素点大小：3 REVERSE决定方向
        WS2812FX_start();
    }



    else if(fc_effect.custom_index == 11)
    {
        extern uint16_t music_mode1(void);
        extern uint16_t meteor(void);
        extern uint16_t meteor1(void);


        WS2812FX_setBrightness( 255 );
        WS2812FX_stop();
        // FADE_SLOW：12颗
        // FADE_MEDIUM：6颗
        // FADE_FAST：5颗灯
        // FADE_XFAST:3颗灯
        WS2812FX_setSegment_colorOptions(
            0,                                          //第0段
            0,fc_effect.led_num-1,                      //起始位置，结束位置
            &meteor1,                               //效果
            WHITE,                                    //颜色，WS2812FX_setColors设置
            fc_effect.speed,                          //速度
            0);                                     //选项，这里像素点大小：3 REVERSE决定方向
        WS2812FX_start();
    }
    else if(fc_effect.custom_index ==12)
    {
        extern uint16_t music_meteor3(void);

        WS2812FX_stop();
        // FADE_SLOW：12颗
        // FADE_MEDIUM：6颗
        // FADE_FAST：5颗灯
        // FADE_XFAST:3颗灯
        WS2812FX_setSegment_colorOptions(
            0,                                          //第0段
            0,fc_effect.led_num-1,                      //起始位置，结束位置
            &music_meteor3,                             //效果
            WHITE,                                      //颜色，WS2812FX_setColors设置
            fc_effect.speed,                            //速度
            0);                                         //选项，这里像素点大小：3 REVERSE决定方向
        WS2812FX_start();

    }
    else if(fc_effect.custom_index ==13)
    {
        // extern uint16_t test(void);
        extern uint16_t music_1(void);

        WS2812FX_stop();
        // FADE_SLOW：12颗
        // FADE_MEDIUM：6颗
        // FADE_FAST：5颗灯
        // FADE_XFAST:3颗灯
        WS2812FX_setSegment_colorOptions(
            0,                                          //第0段
            0,fc_effect.led_num-1,                      //起始位置，结束位置
            &music_1,                                   //效果
            WHITE,                                      //颜色，WS2812FX_setColors设置
            fc_effect.speed,                            //速度
            0);                                         //选项，这里像素点大小：3 REVERSE决定方向
        WS2812FX_start();
    }    else if(fc_effect.custom_index ==14)
    {
        extern uint16_t starry_sky_(void);
        WS2812FX_setSegment_colorOptions(
            0,                                      //第0段
            0,fc_effect.led_num-1,                  //起始位置，结束位置
            &starry_sky_,               //效果
            WHITE ,                               //颜色，WS2812FX_setColors设置
            fc_effect.speed,//速度
            size_type[0]);                           //选项，这里像素点大小：3,反向/反向
    

        WS2812FX_start();
    }
   
}

/***************************************************软件关机*****************************************************/
extern u8 music_trigger;
void soft_rurn_off_lights(void) //软关灯处理
{

    // WS2812FX_stop();
    fc_effect.on_off_flag = DEVICE_OFF;
    music_trigger = 0;
    save_user_data_area3();
    printf("soft_rurn_off_light!!\n");

}


/**************************************************软件开机*****************************************************/
 void soft_turn_on_the_light(void)   //软开灯处理
{

    
    fc_effect.on_off_flag = DEVICE_ON;
    save_user_data_area3();
    set_fc_effect();
    WS2812FX_start();
    printf("soft_turn_on_the_light!!\n");

}

ON_OFF_FLAG get_on_off_state(void)
{
    return fc_effect.on_off_flag;
}

void set_on_off_led(u8 on_off)
{
    fc_effect.on_off_flag = on_off;
    printf("\n on_off=%d",on_off);
    if(fc_effect.on_off_flag == DEVICE_ON)
    {
        soft_turn_on_the_light();
    }
    else
    {
        soft_rurn_off_lights();
    }
}

/**************************************************静态模式*****************************************************/
void set_static_mode(u8 r, u8 g, u8 b)
{
    fc_effect.Now_state = IS_STATIC;
    fc_effect.rgb.r = r;
    fc_effect.rgb.g = g;
    fc_effect.rgb.b = b;
    set_fc_effect();
}

static void static_mode(void)
{
    WS2812FX_stop();
    WS2812FX_setSegment_colorOptions(
        0,                                      //第0段
        0,fc_effect.led_num-1,                  //起始位置，结束位置
        &WS2812FX_mode_static,                  //效果
        0,                                      //颜色，WS2812FX_setColors设置
        1000,                                   //速度
        0);                            //选项，这里像素点大小：1
    WS2812FX_set_coloQty(0,fc_effect.dream_scene.c_n);
    ls_set_colors(1, &fc_effect.rgb);
    WS2812FX_start();
}
/******************************************************************
 * 函数：更新涂抹效果数据
 * 形参1：tool       油桶、画笔、橡皮擦
 * 形参2：colour     hsv颜色
 * 形参3：led_place  灯点位置（0~47）
 * 返回：无
 *
 * 注：若选择IS_drum油桶，led_place参数无效
 *     若选择IS_eraser橡皮擦，colour参数无效，内部将colour设为黑色
 *****************************************************************/
void effect_smear_adjust_updata(smear_tool_e tool, hsv_t *colour,unsigned short *led_place)
{
    unsigned char num = 0;
    unsigned char max;

    //更新为涂抹功能状态
    fc_effect.Now_state = IS_smear_adjust;

    //更新工具
    fc_effect.smear_adjust.smear_tool = tool;
    printf("fc_effect.smear_adjust.smear_tool = %d\r\n",(uint8_t)fc_effect.smear_adjust.smear_tool);
    printf("\r\n");

    //清除rgb[0~n]数据
    // memset(fc_effect.smear_adjust.rgb, 0, sizeof(fc_effect.smear_adjust.rgb));

    /*HSV转换RGB*/
    if(fc_effect.smear_adjust.smear_tool == IS_drum) //油桶
    {
      m_hsv_to_rgb(&fc_effect.smear_adjust.rgb[0].r,
                   &fc_effect.smear_adjust.rgb[0].g,
                   &fc_effect.smear_adjust.rgb[0].b,
                   colour->h_val,
                   colour->s_val,
                   colour->v_val);
      max = fc_effect.led_num;
      for(num = 1; num < max; ++num)
      {
        fc_effect.smear_adjust.rgb[num].r = fc_effect.smear_adjust.rgb[0].r;
        fc_effect.smear_adjust.rgb[num].g = fc_effect.smear_adjust.rgb[0].g;
        fc_effect.smear_adjust.rgb[num].b = fc_effect.smear_adjust.rgb[0].b;

        // printf("fc_effect.smear_adjust.rgb[%d].r = %d\r\n", num,fc_effect.smear_adjust.rgb[num].r);
        // printf("fc_effect.smear_adjust.rgb[%d].g = %d\r\n", num,fc_effect.smear_adjust.rgb[num].g);
        // printf("fc_effect.smear_adjust.rgb[%d].b = %d\r\n", num,fc_effect.smear_adjust.rgb[num].b);
        // printf("\r\n");
      }
    }
    else if((fc_effect.smear_adjust.smear_tool == IS_pen) ||   //画笔
            (fc_effect.smear_adjust.smear_tool == IS_eraser))  //橡皮擦
    {
        m_hsv_to_rgb(&fc_effect.smear_adjust.rgb[*led_place].r,
                     &fc_effect.smear_adjust.rgb[*led_place].g,
                     &fc_effect.smear_adjust.rgb[*led_place].b,
                     colour->h_val,
                     colour->s_val,
                     colour->v_val);

        // printf("fc_effect.smear_adjust.rgb[%d].r = %d\r\n", *led_place, fc_effect.smear_adjust.rgb[dp_draw_tool.led_place].r);
        // printf("fc_effect.smear_adjust.rgb[%d].g = %d\r\n", *led_place, fc_effect.smear_adjust.rgb[dp_draw_tool.led_place].g);
        // printf("fc_effect.smear_adjust.rgb[%d].b = %d\r\n", *led_place, fc_effect.smear_adjust.rgb[dp_draw_tool.led_place].b);
        // printf("\r\n");
    }
}
/*----------------------------------涂抹模式----------------------------------*/
extern  Segment* _seg;
extern  uint16_t _seg_len;
extern Segment_runtime* _seg_rt;

/*******************************************
灯串涂抹效果实现方法
 ******************************************/
static uint16_t ls_smear_adjust_effect(void)
{
  unsigned char num;
  unsigned char max = fc_effect.led_num;
  if(max >= _seg_len) max = _seg_len;
  for(num = 0; num < max; ++num)
  {
      WS2812FX_setPixelColor_rgb(num,
        fc_effect.smear_adjust.rgb[num].r,
        fc_effect.smear_adjust.rgb[num].g,
        fc_effect.smear_adjust.rgb[num].b);
  }
  return _seg->speed;
}

static void fc_smear_adjust(void)
{
    WS2812FX_stop();
    WS2812FX_setSegment_colorOptions(0,0,fc_effect.led_num-1,&ls_smear_adjust_effect,BLUE,100,0);
    WS2812FX_start();
}

/*----------------------------------涂鸦配网效果----------------------------------*/
static void fc_pair_effect(void)
{
    extern uint16_t unbind_effect(void);
    WS2812FX_stop();
    WS2812FX_setSegment_colorOptions(0,0,fc_effect.led_num-1,&unbind_effect,0,0,0);
    WS2812FX_start();
}

/*----------------------------------情景效果实现----------------------------------*/
static void ls_scene_effect(void)
{
    printf("\n fc_effect.dream_scene.change_type=%d",fc_effect.dream_scene.change_type);

    switch (fc_effect.dream_scene.change_type)
    {

        case IS_SCENE_RAINBOW:      //彩虹

            strand_rainbow();
            // printf("\n IS_SCENE_RAINBOW");
        break;
        case IS_SCENE_JUMP_CHANGE://跳变模式
            strand_jump_change();
            // printf("\n IS_SCENE_JUMP_CHANGE");
            break;

        case IS_SCENE_BRAETH://呼吸模式
            strand_breath();
            break;

        case IS_SCENE_TWIHKLE://闪烁模式
            strand_twihkle();
            printf("\n IS_SCENE_TWIHKLE");
            break;

        case IS_SCENE_FLOW_WATER://流水模式
            strand_flow_water();
            printf("\n IS_SCENE_FLOW_WATER");
            break;

        case IS_SCENE_CHAS_LIGHT://追光模式
            strand_chas_light();
            printf("\n IS_SCENE_CHAS_LIGHT");
            break;

        case IS_SCENE_COLORFUL://炫彩模式
            strand_colorful();
            break;

        case IS_SCENE_GRADUAL_CHANGE://渐变模式
            strand_grandual();
            printf("\n IS_SCENE_GRADUAL_CHANGE");
            break;
        default:
            break;
    }
}


/*----------------------------------彩虹模式----------------------------------*/
static void strand_rainbow(void)
{
    WS2812FX_stop();
    // printf("\n fc_effect.dream_scene.c_n=%d",fc_effect.dream_scene.c_n);
    // printf("\n fc_effect.led_num=%d",fc_effect.led_num);
    // printf("\n fc_effect.dream_scene.speed=%d",fc_effect.dream_scene.speed);
    // printf("\n fc_effect.dream_scene.rgb");
    // printf_buf(fc_effect.dream_scene.rgb, fc_effect.dream_scene.c_n*sizeof(color_t));
    // printf("\n fc_effect.dream_scene.direction=%d",fc_effect.dream_scene.direction);

    WS2812FX_setSegment_colorOptions(
        0,                                      //第0段
        0,fc_effect.led_num-1,                  //起始位置，结束位置
        &WS2812FX_mode_mutil_fade,               //效果
        0,                                      //颜色，WS2812FX_setColors设置
        fc_effect.dream_scene.speed,            //速度
        SIZE_SMALL);                            //选项，这里像素点大小：1

    WS2812FX_set_coloQty(0,fc_effect.dream_scene.c_n);
    ls_set_colors(fc_effect.dream_scene.c_n, &fc_effect.dream_scene.rgb);

    WS2812FX_start();
}

/*----------------------------------跳变模式----------------------------------*/
void strand_jump_change(void)
{
    WS2812FX_stop();
    WS2812FX_setSegment_colorOptions(
        0,                                      //第0段
        0,fc_effect.led_num-1,                  //起始位置，结束位置
        &WS2812FX_mode_single_block_scan,       //效果
        0,                                      //颜色，WS2812FX_setColors设置
        fc_effect.dream_scene.speed,            //速度
        SIZE_MEDIUM);                           //选项，这里像素点大小：3

    WS2812FX_set_coloQty(0,fc_effect.dream_scene.c_n);
    ls_set_colors(fc_effect.dream_scene.c_n, &fc_effect.dream_scene.rgb);

    WS2812FX_start();
}

/*----------------------------------呼吸模式----------------------------------*/
void strand_breath(void)
{
    WS2812FX_stop();
    WS2812FX_setSegment_colorOptions(
        0,                                      //第0段
        0,fc_effect.led_num-1,                  //起始位置，结束位置
        &WS2812FX_mode_mutil_breath,            //效果
        0,                                      //颜色，WS2812FX_setColors设置
        fc_effect.dream_scene.speed,            //速度
        SIZE_MEDIUM);                           //选项，这里像素点大小：3

    WS2812FX_set_coloQty(0,fc_effect.dream_scene.c_n);
    ls_set_colors(fc_effect.dream_scene.c_n, &fc_effect.dream_scene.rgb);

    WS2812FX_start();
}

/*----------------------------------闪烁模式----------------------------------*/
void strand_twihkle(void)
{
    uint8_t option;
    WS2812FX_stop();
    WS2812FX_setSegment_colorOptions(
        0,                                      //第0段
        0,fc_effect.led_num-1,                  //起始位置，结束位置
        &WS2812FX_mode_mutil_twihkle,           //效果
        0,                                      //颜色，WS2812FX_setColors设置
        fc_effect.dream_scene.speed,            //速度
        SIZE_SMALL);                            //选项，这里像素点大小：1
    WS2812FX_set_coloQty(0,fc_effect.dream_scene.c_n);
    ls_set_colors(fc_effect.dream_scene.c_n, &fc_effect.dream_scene.rgb);

    WS2812FX_start();

}

/*----------------------------------流水模式----------------------------------*/
void strand_flow_water(void)
{
    uint8_t option;
    // 正向
    if(fc_effect.dream_scene.direction == IS_forward)
    {
        option = SIZE_MEDIUM | 0;
    }
    else{
        option = SIZE_MEDIUM | REVERSE;
    }

    WS2812FX_stop();
    WS2812FX_setSegment_colorOptions(
        0,                                      //第0段
        0,fc_effect.led_num-1,                  //起始位置，结束位置
        &WS2812FX_mode_multi_block_scan,        //效果
        0,                                      //颜色，WS2812FX_setColors设置
        fc_effect.dream_scene.speed,            //速度
        option);                                //选项，这里像素点大小：3,反向/反向
    WS2812FX_set_coloQty(0,fc_effect.dream_scene.c_n);
    ls_set_colors(fc_effect.dream_scene.c_n, &fc_effect.dream_scene.rgb);

    WS2812FX_start();
}

/*----------------------------------追光模式----------------------------------*/
void strand_chas_light(void)
{
    printf("\n fc_effect.dream_scene.c_n=%d",fc_effect.dream_scene.c_n);
    printf("\n fc_effect.led_num=%d",fc_effect.led_num);
    printf("\n fc_effect.dream_scene.speed=%d",fc_effect.dream_scene.speed);
    printf("\n fc_effect.dream_scene.rgb");
    printf_buf(fc_effect.dream_scene.rgb, fc_effect.dream_scene.c_n*sizeof(color_t));
    printf("\n fc_effect.dream_scene.direction=%d",fc_effect.dream_scene.direction);
    WS2812FX_stop();
    // 正向
    if(fc_effect.dream_scene.direction == IS_forward)
    {
        WS2812FX_setSegment_colorOptions(
            0,                                      //第0段
            0,fc_effect.led_num-1,                  //起始位置，结束位置
            &WS2812FX_mode_multi_forward_same,        //效果
            0,                                      //颜色，WS2812FX_setColors设置
            fc_effect.dream_scene.speed,            //速度
            0);                                     //选项
    }
    else
    {
        WS2812FX_setSegment_colorOptions(
            0,                                      //第0段
            0,fc_effect.led_num-1,                  //起始位置，结束位置
            &WS2812FX_mode_multi_back_same,        //效果
            0,                                      //颜色，WS2812FX_setColors设置
            fc_effect.dream_scene.speed,            //速度
            0);
    }
    WS2812FX_set_coloQty(0,fc_effect.dream_scene.c_n);
    ls_set_colors(fc_effect.dream_scene.c_n, &fc_effect.dream_scene.rgb);

    WS2812FX_start();
}

/*----------------------------------炫彩模式----------------------------------*/
void strand_colorful(void)
{
    uint8_t option;
    WS2812FX_stop();
    WS2812FX_setSegment_colorOptions(
        0,                                      //第0段
        0,fc_effect.led_num-1,                  //起始位置，结束位置
        &WS2812FX_mode_multi_block_scan,        //效果
        0,                                      //颜色，WS2812FX_setColors设置
        fc_effect.dream_scene.speed,            //速度
        SIZE_SMALL);                            //选项，这里像素点大小：1
    WS2812FX_set_coloQty(0,fc_effect.dream_scene.c_n);
    ls_set_colors(fc_effect.dream_scene.c_n, &fc_effect.dream_scene.rgb);

    WS2812FX_start();
}

/*----------------------------------渐变模式----------------------------------*/
void strand_grandual(void)
{
    WS2812FX_stop();
    WS2812FX_setSegment_colorOptions(
        0,                                      //第0段
        0,fc_effect.led_num-1,                  //起始位置，结束位置
        &WS2812FX_mode_mutil_fade,              //效果
        0,                                      //颜色，WS2812FX_setColors设置
        fc_effect.dream_scene.speed,            //速度
        SIZE_MEDIUM);                                //选项，这里像素点大小：3,反向/反向

    WS2812FX_set_coloQty(0,fc_effect.dream_scene.c_n);
    ls_set_colors(fc_effect.dream_scene.c_n, &fc_effect.dream_scene.rgb);

    WS2812FX_start();
}

/*----------------------------------API----------------------------------*/

// --------------------------------------速度
const uint16_t speed_map[]=
{
    30,
    100,
    150,
    200,
    250,
    300
};
u8 speed_index=0;


// s:1%-100%
void ls_set_speed(uint8_t s)
{
    fc_effect.dream_scene.speed =  500 - (500 * s / 100);
}

void ls_speed_plus(void)
{
    if(fc_effect.speed > 10)
    {
        fc_effect.speed -= 10;
    }
    if(fc_effect.speed < 10)
    {
        fc_effect.speed = 1;
    }

    printf("\n fc_effect.speed=%d",fc_effect.speed);
    set_fc_effect();
}

void ls_speed_sub(void)
{
    if(fc_effect.speed < 100)
    {
        fc_effect.speed += 10;
    }
    if(fc_effect.speed > 100)
    {
        fc_effect.speed = 100;
    }
    printf("\n fc_effect.speed=%d",fc_effect.speed);
    set_fc_effect();
}

void max_speed(void)
{
    fc_effect.speed = 1;
    set_fc_effect();
}

void min_speed(void)
{
    fc_effect.speed = 100;
    set_fc_effect();
}



void random_effect(void)
{
    fc_effect.custom_index = WS2812FX_random8_lim(11);
    if(fc_effect.custom_index == 0)fc_effect.custom_index = 1;
    printf("\n fc_effect.custom_index=%d",fc_effect.custom_index);
    set_fc_effect();
}




void len_put(void)
{
    if( fc_effect.custom_index == 1) {
        fc_effect.custom_index = 2;
    } else if( fc_effect.custom_index == 2) {
        fc_effect.custom_index = 3; 
    } else if( fc_effect.custom_index == 3) {
        fc_effect.custom_index = 4; 
    } else if( fc_effect.custom_index == 5) {
        fc_effect.custom_index = 6; 
    } else if( fc_effect.custom_index == 6) {
        fc_effect.custom_index = 7; 
    } else if( fc_effect.custom_index == 7) {
        fc_effect.custom_index = 8; 
    }
    set_fc_effect();
}

void len_sub(void)
{
    if( fc_effect.custom_index == 4) {
        fc_effect.custom_index = 3;
    } else if( fc_effect.custom_index == 3) {
        fc_effect.custom_index = 2; 
    } else if( fc_effect.custom_index == 2) {
        fc_effect.custom_index = 1; 
    } else if( fc_effect.custom_index == 8) {
        fc_effect.custom_index = 7; 
    } else if( fc_effect.custom_index == 7) {
        fc_effect.custom_index = 6; 
    } else if( fc_effect.custom_index == 6) {
        fc_effect.custom_index = 5; 
    }
    set_fc_effect();
}


// --------------------------------------播放
// 继续播放
void ls_play(void)
{
    WS2812FX_play();
}
// 暂停
void ls_pause(void)
{
    WS2812FX_pause();
}

// --------------------------------------流星灯周期
void set_meteor_p(u8 p)
{
    if(p >= 2 && p <= 20)
    {
        fc_effect.meteor_period = p;
        fc_effect.period_cnt = 0;
    }
}


void meteor_p_pul(void)
{
    if(fc_effect.meteor_period < 20)
    {
        fc_effect.meteor_period++;
    }
    set_fc_effect();
    printf("fc_effect.meteor_period = %d", fc_effect.meteor_period);
}

void meteor_p_sub(void)
{
     if(fc_effect.meteor_period > 2)
    {
        fc_effect.meteor_period--;
    }
    set_fc_effect();
    printf("fc_effect.meteor_period = %d", fc_effect.meteor_period);
}




// 时间递减
// sub:减数，ms
void meteor_period_sub(void)
{

    if(fc_effect.period_cnt > 1)
    {
        fc_effect.period_cnt -= 1;
        // printf("\n fc_effect.period_cnt=%d",fc_effect.period_cnt);
    }
    else{
        fc_effect.period_cnt = 0;
        if(fc_effect.mode_cycle)    //模式循环完成，更新
        {
            fc_effect.period_cnt = fc_effect.meteor_period*1000;
            fc_effect.mode_cycle = 0;
        }
    }
}


void set_sensitive(u8 s)
{
    fc_effect.music.s = s;
    printf("\n sound.sensitive=%d",fc_effect.music.s);

}


// 0:计时完成
// 1：计时中
u8 get_effect_p(void)
{
    if(fc_effect.period_cnt > 0) return 1;
    else return 0;
}

/* *********************************样式 */
// 自定义样式
void fc_set_style_custom(void)
{
    fc_effect.Now_state = ACT_CUSTOM;
}



// 涂鸦配对样式
void fc_set_style_ty_pair(void)
{
    fc_effect.Now_state = ACT_TY_PAIR;
}


void set_bright(u8 b)
{
    // if(b == 0) b = 10;
    fc_effect.b = 255*b/100;

    WS2812FX_setBrightness(fc_effect.b);
}



void bright_plus(void)
{
    if(fc_effect.b < 255 - 20)
    {
        fc_effect.b += 20;
    }
    else
    {
        fc_effect.b = 255;
       
    }
    printf("fc_effect.b = %d", fc_effect.b);
    WS2812FX_setBrightness(fc_effect.b);
    set_fc_effect();

}

void bright_sub(void)
{

    if(fc_effect.b > 40)
    {
        fc_effect.b -= 20;
    }
    else
    {
        fc_effect.b = 40;
       
    }
    printf("fc_effect.b = %d", fc_effect.b);

    WS2812FX_setBrightness(fc_effect.b);
    set_fc_effect();

}


void meteor_reset(void)
{
    fc_effect.b = 255;
    WS2812FX_setBrightness(fc_effect.b);
    fc_effect.meteor_period = 10;
    fc_effect.custom_index = 7;
    fc_effect.speed=20;
    set_fc_effect();
}


/*---------------------------------------------  向app反馈信息 ---------------*/
extern void zd_fb_2_app(u8 *p, u8 len);
void fc_meteor_p(void)
{
    uint8_t Send_buffer[6]; 
    Send_buffer[0] = 0x2F;
    Send_buffer[1] = 0x03;
    Send_buffer[2] = fc_effect.meteor_period ;// 
    extern void zd_fb_2_app(u8 *p, u8 len);
    zd_fb_2_app(Send_buffer, 3);
}

void fc_meteor_speed(void)
{
    uint8_t Send_buffer[6]; 
    Send_buffer[0] = 0x2F;
    Send_buffer[1] = 0x01;
    Send_buffer[2] = 100 - fc_effect.speed ;// 
    extern void zd_fb_2_app(u8 *p, u8 len);
    zd_fb_2_app(Send_buffer, 3);
}
/*--------------------------------------API-----------------------------------*/
// 触发提示效果，白光闪烁
void run_white_tips(void)
{
    extern uint16_t white_tips(void);
    WS2812FX_setSegment_colorOptions(0,0,fc_effect.led_num-1,&white_tips,0,3000,0);
    WS2812FX_start();
}