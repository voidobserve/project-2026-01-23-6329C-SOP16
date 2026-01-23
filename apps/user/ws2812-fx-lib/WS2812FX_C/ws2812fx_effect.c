// 利用ws2812fx库，创建效果
#include "WS2812FX.h"
#include "ws2812fx_tool.h"
#include "Adafruit_NeoPixel.h"
#include "led_strand_effect.h"
#include "system/includes.h"

extern  Segment* _seg;
extern  uint16_t _seg_len;
extern Segment_runtime* _seg_rt;




/* ****************项目专属效果******************************* */

#if 0
// 单色灯,做流星效果
uint16_t WS2812FX_mode_comet_1(void)
{
  extern u8 get_effect_p(void);
  // 计时中，模式循环完成

  // printf("\n mode_cycle=%d",fc_effect.mode_cycle);  
  // printf("\n fc_effect.period_cnt=%d",fc_effect.period_cnt);                                                               
  if( (get_effect_p() == 1) && (fc_effect.mode_cycle == 1) )
  {
    return (_seg->speed );
  } 
  WS2812FX_fade_out();
  extern u8 get_custom_index(void);
  u8 offset;
  // if(get_custom_index() < 5)
  // {
  //   offset = get_custom_index()*3+2;
  // }
  // else
  // {
  //   offset = (get_custom_index() - 4)*3;
  // }
  offset = 13;
  if(IS_REVERSE) {
  if(_seg_rt->aux_param == 0)
  {
    // _seg_rt->counter_mode_step = _seg->stop;
    _seg_rt->aux_param = 1;
  }
  if((_seg->stop - _seg->start) >=_seg_rt->counter_mode_step)
  {
    WS2812FX_setPixelColor(_seg->stop - _seg_rt->counter_mode_step, _seg->colors[0]);
    // printf("\n _seg_rt->counter_mode_step=%d",_seg_rt->counter_mode_step);  
  }
  } else {
    if(_seg_rt->counter_mode_step < _seg->stop+1)
    WS2812FX_setPixelColor(_seg->start + _seg_rt->counter_mode_step, _seg->colors[0]);

  }

  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % (_seg_len + offset);
  if(_seg_rt->counter_mode_step == 0)
  {
    SET_CYCLE;
    fc_effect.mode_cycle = 1;
    // printf("\n fc_effect.mode_cycle=%d",fc_effect.mode_cycle);
  } 

  return (_seg->speed );
}

// 两段流星灯，向中心靠拢
uint16_t WS2812FX_mode_comet_2(void)
{
  extern u8 get_effect_p(void);
  // 计时中，模式循环完成
  // printf("\n fc_effect.mode_cycle=%d",fc_effect.mode_cycle);
  // printf("\n fc_effect.period_cnt=%d",fc_effect.period_cnt);

  if( (get_effect_p() == 1) && (fc_effect.mode_cycle == 1) )
  {
    return (_seg->speed );
  } 
  WS2812FX_fade_out();
  extern u8 get_custom_index(void);
  u8 offset;

  offset = 7;

    if(_seg_rt->counter_mode_step < 5)
    {
      WS2812FX_setPixelColor(_seg->start + 5 + _seg_rt->counter_mode_step, _seg->colors[0]);
      
    }

  if(_seg_rt->counter_mode_step < 5)
  WS2812FX_setPixelColor(_seg->start + _seg_rt->counter_mode_step, _seg->colors[0]);

  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % (_seg_len + offset);
  if(_seg_rt->counter_mode_step == 0)
  {
    SET_CYCLE;
    fc_effect.mode_cycle = 1;
    // printf("\n fc_effect.mode_cycle=%d",fc_effect.mode_cycle);
  } 

  return (_seg->speed );
}

//从中心发散
uint16_t WS2812FX_mode_comet_3(void)
{
  extern u8 get_effect_p(void);
  // 计时中，模式循环完成
          
  if( (get_effect_p() == 1) && (fc_effect.mode_cycle == 1) )
  {
    return (_seg->speed );
  } 
  WS2812FX_fade_out();
  extern u8 get_custom_index(void);
  u8 offset;

  offset = 7;

  if(_seg_rt->counter_mode_step < 5)
  {
    WS2812FX_setPixelColor(_seg->stop  - _seg_rt->counter_mode_step -2, _seg->colors[0]);
    // printf("\n _seg_rt->counter_mode_step=%d",_seg_rt->counter_mode_step);  
  }

  if(_seg_rt->counter_mode_step < 5)
  WS2812FX_setPixelColor(_seg->stop / 2 - _seg_rt->counter_mode_step -1, _seg->colors[0]);

  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % (_seg_len + offset);
  if(_seg_rt->counter_mode_step == 0)
  {
    SET_CYCLE;
    fc_effect.mode_cycle = 1;
    // printf("\n fc_effect.mode_cycle=%d",fc_effect.mode_cycle);
  } 

  return (_seg->speed );
}
#endif


// 反向移动某一段
// s起始地址
// e结束地址
// 条件：e>s
void WS2812FX_move_reverse(uint16_t s, uint16_t e)
{
  // uint16_t i;
  // if(s > e) return;
  /* 获取原始颜色，没有进行亮度调整的颜色 */
  uint32_t c = Adafruit_NeoPixel_getOriginPixelColor(s);
  /* 颜色平移一个像素,把后面像素复制到前面 */
  WS2812FX_copyPixels(s, s+1, e - s);

  /* 把第一个颜色，补到最后一个位置 */
  Adafruit_NeoPixel_setPixelColor_raw(e, c);
}

// 正向移动某一段
// s起始地址
// e结束地址
// 条件：e>s
void WS2812FX_move_forward(uint16_t s, uint16_t e)
{
  uint16_t i;

  if(s > e) return;
  uint32_t c = Adafruit_NeoPixel_getOriginPixelColor(e);

  for(i=0;i<e - s;i++)
  {
    WS2812FX_copyPixels(e-i , e - 1-i, 1);
  }

  Adafruit_NeoPixel_setPixelColor_raw(s, c);  
}





/* 指定某个点渐亮 */
uint8_t my_fade_in(uint16_t pos)
{
  uint32_t color;
  int w1 ;
  int r1 ;
  int g1 ;
  int b1 ;

  // for(uint16_t i=_seg->start; i <= _seg->stop; i++) 
  {
    if(IS_REVERSE) //反向流水
    {
      color = Adafruit_NeoPixel_getPixelColor(_seg->stop - pos); 
    }
    else
      color = Adafruit_NeoPixel_getPixelColor(pos);
    w1 = (color >> 24) & 0xff;
    r1 = (color >> 16) & 0xff;
    g1 = (color >>  8) & 0xff;
    b1 =  color        & 0xff;

    w1 = 10 + w1 ; 
    r1 = 10 + r1 ;
    g1 = 10 + g1 ;
    b1 = 10 + b1 ;


    if(w1 > 255) w1 = 255;
    if(r1 > 255) r1 = 255;
    if(g1 > 255) g1 = 255;
    if(b1 > 255) b1 = 255;
    if(IS_REVERSE) //反向流水
    {
      WS2812FX_setPixelColor_rgbw(_seg->stop - pos, r1, g1, b1, w1);
    }
    else
      WS2812FX_setPixelColor_rgbw(pos, r1, g1, b1, w1);

  }
  return b1;
}

//除了x不慢慢灭，其他的是慢慢灭
void fade_out(uint16_t x)
{
  int color;
  int w1 ;
  int r1 ;
  int g1 ;
  int b1 ;
  for(uint16_t i=_seg->start; i < x; i++) 
  {
    if(IS_REVERSE) //反向流水
    {
      color = Adafruit_NeoPixel_getPixelColor(_seg->stop - i); // current color

    }
    else
      color = Adafruit_NeoPixel_getPixelColor(i); // current color
    w1 = (color >> 24) & 0xff;
    r1 = (color >> 16) & 0xff;
    g1 = (color >>  8) & 0xff;
    b1 =  color        & 0xff;


    w1 =  w1-(3-SIZE_OPTION)-1 ;
    r1 =  r1-(3-SIZE_OPTION)-1 ;
    g1 =  g1-(3-SIZE_OPTION)-1 ;
    b1 =  b1-(3-SIZE_OPTION)-1 ;
  
    if(b1 < 0) 
    {
      w1 = 0;
      r1 = 0;
      g1 = 0;
      b1 = 0;
    }
    if(IS_REVERSE) //反向流水
    {
      WS2812FX_setPixelColor_rgbw(_seg->stop - i, r1, g1, b1, w1);
    }
    else
      WS2812FX_setPixelColor_rgbw(i, r1, g1, b1, w1);
    // WS2812FX_setPixelColor(i, color );
  }
}

//100 50 25 12 6 3 2 1 1 0 0 0
#define MAX_RATE 12
uint16_t WS2812FX_mode_comet_1(void)
{


#if 1

  static uint8_t i = 0;
  if( (get_effect_p() == 1) && (fc_effect.mode_cycle == 1) )  //计时中 && 完成一个循环
  {
   
    return (_seg->speed );
  } 


  fade_out(_seg_rt->counter_mode_step);

  if(_seg_rt->aux_param == 0)
  {
    if(my_fade_in( _seg_rt->counter_mode_step) == 255)
    {
      _seg_rt->counter_mode_step++;
      if(_seg_rt->counter_mode_step == _seg_len)
      {
        _seg_rt->aux_param++;
        
      }
    }
  }
  else
  {
    _seg_rt->aux_param ++;
    if(_seg_rt->aux_param > 254)
    {
      _seg_rt->aux_param = 0;
      _seg_rt->counter_mode_step %= _seg_len;

       Adafruit_NeoPixel_clear();
       fc_effect.mode_cycle = 1;
    }
  }

#endif


  return (_seg->speed );

}



// 两段流星灯，正向，反向
uint16_t WS2812FX_mode_comet_2(void)
{
  static uint8_t i = 0;
  uint8_t meteor_len = 12;
  if(_seg_rt->counter_mode_step == 0)   //该判断放在这里，解决效果切换时，立即切换
  {
    i = 0;
    SET_CYCLE;
   
  } 
  if( (get_effect_p() == 1) && (fc_effect.mode_cycle == 1) )  //计时中 && 完成一个循环
  {
    return (_seg->speed );
  } 
  uint32_t r1 = 0, g1 = 0, b1 = 0, w1 = 0;  //必须初始化
  int w = 0, r = 0, g = 0, c = 0, b = 0;    //必须初始化
 
  const uint8_t rate[12] = {100,50,25,12,6,3,2,1,1,0,0,0};  //亮度值 0-100
  
  //灯的颜色
 
  w = (_seg->colors[0] >> 24) & 0xff;
  r = (_seg->colors[0] >> 16) & 0xff;
  g = (_seg->colors[0] >>  8) & 0xff;
  b =  _seg->colors[0]        & 0xff;
            

   
  
  if(IS_REVERSE) //反向流水
  {
    WS2812FX_move_reverse(_seg->start  , _seg->stop - (_seg_len / 2));
    WS2812FX_move_reverse(_seg->start + (_seg_len / 2) , _seg->stop);
  }
  else{
    WS2812FX_move_forward(_seg->start, _seg->stop - (_seg_len / 2)); 
    WS2812FX_move_forward(_seg->start + (_seg_len / 2)+1, _seg->stop ); 


  }
 
  if(i < meteor_len)   //避免数组溢出
  {
    r1 = r * rate[i] / 100;
    g1 = g * rate[i] / 100;
    b1 = b * rate[i] / 100;
    w1 = w * rate[i] / 100;
    i++;
  }

  if(IS_REVERSE) //反向流水
  {
    WS2812FX_setPixelColor_rgbw(_seg->stop, r1, g1, b1, w1);   //反向
    WS2812FX_setPixelColor_rgbw(_seg->stop - (_seg_len / 2) , r1, g1, b1, w1);   //反向
  }
  else{
    WS2812FX_setPixelColor_rgbw(_seg->start, r1, g1, b1, w1);
    WS2812FX_setPixelColor_rgbw(_seg->start + (_seg_len / 2)+1, r1, g1, b1, w1);

  }


  _seg_rt->counter_mode_step++;
  _seg_rt->counter_mode_step %= _seg_len + meteor_len;

  if(_seg_rt->counter_mode_step == 0)
    fc_effect.mode_cycle = 1;
  if(_seg->speed < 10) _seg->speed = 10;
  return (_seg->speed * 50);

}















/******** 流星  ****/


#include "led_strip_drive.h"
uint16_t music_star_sp;
extern u8 music_trigger;
// 采集声音的ADC值
void set_mss(uint16_t s)
{
  if(500 > s)
    music_star_sp = 500 -s;
  else music_star_sp = 10;
  if(music_star_sp < 10) music_star_sp = 10;


}

uint16_t music_mode1(void)
{
  static u8 trg_cnt=0;
  static u8 no_trg_cnt=0;
  extern u8 get_sound_result(void);
  // const u8 rate[12] = {0,1,1,2,2,3,3,4,5,5,6,6};
  const u8 rate[12] = {253,250,240,230,220,200,180,130,100,75,50,0};

  if(get_sound_result())
  {
    uint32_t color = _seg->colors[0];
    int w1 = (color >> 24) & 0xff;
    int r1 = (color >> 16) & 0xff;
    int g1 = (color >>  8) & 0xff;
    int b1 =  color        & 0xff;

    
    WS2812FX_setPixelColor_rgbw( _seg->start+trg_cnt , r1-rate[trg_cnt], g1-rate[trg_cnt], b1-rate[trg_cnt], w1-rate[trg_cnt]);

    // WS2812FX_setPixelColor(_seg->start+trg_cnt , _seg->colors[0] );
    if(trg_cnt < _seg_len)
    {
      trg_cnt++;
      no_trg_cnt = 0;
    }
    else
    {
      // trg_cnt = 0;
    }
  }
  else
  {
    WS2812FX_setPixelColor(_seg->start+trg_cnt , BLACK);
    no_trg_cnt++;
    if(no_trg_cnt>=3)
    {
      no_trg_cnt = 0;
      if(trg_cnt>0)
      {
        trg_cnt--;
      }
    }
  }
  return 300;
}


#define MAX_RATE 8
// 流星发射，声音触发，不支持连续发射，等上个流星发射完成再发射第二个
uint16_t meteor(void)
{

  static uint8_t i = 0, trg;
  uint32_t r1, g1, b1, w1 ;
  const uint8_t rate[MAX_RATE] = {100,88,75,55,30,10,0,0};
  int w = (_seg->colors[0] >> 24) & 0xff;
  int r = (_seg->colors[0] >> 16) & 0xff;
  int g = (_seg->colors[0] >>  8) & 0xff;
  int b =  _seg->colors[0]        & 0xff;
  extern u8 get_sound_result(void);
  if(get_sound_result())
  {
    trg = 1;
  }

  if(trg)
  {
    WS2812FX_copyPixels(_seg->start+1,_seg->start, _seg_len-1);   //颜色平移一个像素

    r1 = r * rate[i] / 100;
    g1 = g * rate[i] / 100;
    b1 = b * rate[i] / 100;
    w1 = w * rate[i] / 100;

    WS2812FX_setPixelColor_rgbw(_seg->start, r1, g1, b1, w1);
    if(i < MAX_RATE-1)
      i++;
    
    _seg_rt->counter_mode_step++;
    if(_seg_rt->counter_mode_step >= _seg_len+8)
    {
      trg = 0;
      _seg_rt->counter_mode_step = 0;
      i=0;
    }
  }
    

  // if(i>0)
  // i--;
  // else
  // i = MAX_RATE-1;

  return (_seg->speed *10);
}

// 流星发射，声音触发，可以连续发射
uint16_t meteor1(void)
{

  static uint8_t i = 0, trg;
  uint32_t r1, g1, b1, w1 ;
  const uint8_t rate[MAX_RATE] = {100,75,50,25,10,0,0,0};
  int w = (_seg->colors[0] >> 24) & 0xff;
  int r = (_seg->colors[0] >> 16) & 0xff;
  int g = (_seg->colors[0] >>  8) & 0xff;
  int b =  _seg->colors[0]        & 0xff;
  extern u8 get_sound_result(void);
  // if(get_sound_result())
  if(music_trigger)
  {
    music_trigger = 0;
    if(i == MAX_RATE - 1)
    {
      i = 0;
     // printf("\n i=%d",i);
    }
  }

  WS2812FX_copyPixels(_seg->start+1,_seg->start, _seg_len-1);
  r1 = r * rate[i] / 100;
  g1 = g * rate[i] / 100;
  b1 = b * rate[i] / 100;
  w1 = w * rate[i] / 100;
  WS2812FX_setPixelColor_rgbw(_seg->start, r1, g1, b1, w1);
    
  if(i < MAX_RATE-1)
    i++;

  // if(i>0)
  // i--;
  // else
  // i = MAX_RATE-1;

  return (300 );
}





// 流星发射，声音触发，可以连续发射
uint16_t music_meteor3(void)
{

  static uint8_t i = 0, trg;
  uint32_t r1, g1, b1, w1 ;
  const uint8_t rate[MAX_RATE] = {100,75,50,25,10,0,0,0};
  int w = (_seg->colors[0] >> 24) & 0xff;
  int r = (_seg->colors[0] >> 16) & 0xff;
  int g = (_seg->colors[0] >>  8) & 0xff;
  int b =  _seg->colors[0]        & 0xff;
  extern u8 get_sound_result(void);
  // if(get_sound_result())
  if(music_trigger)
  {
    music_trigger = 0;
    if(i == MAX_RATE - 1)
    {
      i = 0;
    }
  }
  // WS2812FX_move_forward(_seg->start, _seg->stop);
  WS2812FX_copyPixels(_seg->start, _seg->start+1, _seg_len-1);
  r1 = r * rate[i] / 100;
  g1 = g * rate[i] / 100;
  b1 = b * rate[i] / 100;
  w1 = w * rate[i] / 100;
  WS2812FX_setPixelColor_rgbw(_seg->stop, r1, g1, b1, w1);


    
    if(i < MAX_RATE-1)
      i++;

  // if(i>0)
  // i--;
  // else
  // i = MAX_RATE-1;

  return (300 );
}

uint16_t music_mode2(void)
{
  static u8 b;

    Adafruit_NeoPixel_fill( WHITE, _seg->start, _seg_len);
  extern u8 get_sound_result(void);

  if(get_sound_result())
  {
    b=255;
    WS2812FX_setBrightness( 255 );
  } 
  else
  {
    // if(b>10)
    //   b-=10;
    // else
      b = 0;
    WS2812FX_setBrightness( b );
  }
  return 10; 
}


// 0:正向堆积
// 1：反向堆积
// 2：单点流水，最后4个点频闪
// 3：2点中间向两边走，逐点
// 4：两边向中间走，堆积
// 5:逐点，两边向中间走
// 6：随机闪
// 7：假频谱
uint8_t music_trg = 0;
uint8_t music_step = 0;
uint8_t step2_flag,change_mode,cycle_t;
uint16_t music_dly;
#define CYCLE_T 3
void cycle_cnt(void)
{
  cycle_t++;
 // printf("cycle_t = %d", cycle_t);
  if(cycle_t >CYCLE_T)
  {
    cycle_t = 0;
    change_mode=1;
    music_step++;
    music_step %=19;
  }
}
// 倒序2个灯逐点流水
void mode1(void)
{
  music_dly = 30;
  Adafruit_NeoPixel_fill(BLACK, _seg->start, _seg_len);
  WS2812FX_setPixelColor( _seg_len -1-_seg_rt->counter_mode_step, WHITE);
  WS2812FX_setPixelColor( _seg_len-_seg_rt->counter_mode_step, WHITE);
  _seg_rt->counter_mode_step += 1;
  _seg_rt->counter_mode_step%=_seg_len;
  if(_seg_rt->counter_mode_step == 0)
  {
    cycle_cnt();
  }
}

// 顺序2个灯逐点流水
void mode2(void)
{
  music_dly = 30;
  Adafruit_NeoPixel_fill(BLACK, _seg->start, _seg_len);
  WS2812FX_setPixelColor( _seg_rt->counter_mode_step, WHITE);
  WS2812FX_setPixelColor( _seg_rt->counter_mode_step+1, WHITE);
  _seg_rt->counter_mode_step += 1;
  _seg_rt->counter_mode_step%=_seg_len;
  if(_seg_rt->counter_mode_step == 0)
  {
    cycle_cnt();
  }
}

//顺序2个点一组，一共2组，第一组从0开始，第二组从一半开始
void mode3(void)
{
  music_dly = 30;
  Adafruit_NeoPixel_fill(BLACK, _seg->start, _seg_len);

  WS2812FX_setPixelColor(_seg_rt->counter_mode_step, WHITE);
  WS2812FX_setPixelColor(_seg_rt->counter_mode_step +1, WHITE);

  WS2812FX_setPixelColor( _seg_len/2 + _seg_rt->counter_mode_step, WHITE);
  WS2812FX_setPixelColor( _seg_len/2 + _seg_rt->counter_mode_step +1, WHITE);
  _seg_rt->counter_mode_step += 1;
  _seg_rt->counter_mode_step%=_seg_len; 
  if(_seg_rt->counter_mode_step == 0)
  {
    cycle_cnt();
  }
}

//倒序2个点一组，一共2组，第一组从0开始，第二组从一半开始
void mode4(void)
{
  music_dly = 30;
  Adafruit_NeoPixel_fill(BLACK, _seg->start, _seg_len);

  WS2812FX_setPixelColor(_seg_len-1 - _seg_rt->counter_mode_step, WHITE);
  WS2812FX_setPixelColor(_seg_len - _seg_rt->counter_mode_step , WHITE);

  WS2812FX_setPixelColor( _seg_len/2 -1- _seg_rt->counter_mode_step, WHITE);
  WS2812FX_setPixelColor( _seg_len/2 - _seg_rt->counter_mode_step , WHITE);
  _seg_rt->counter_mode_step += 1;
  _seg_rt->counter_mode_step%=_seg_len;
  if(_seg_rt->counter_mode_step == 0)
  {
    cycle_cnt();
  }
}
//两边向中间走,逐点
void mode5(void)
{
  Adafruit_NeoPixel_fill(BLACK, _seg->start, _seg_len);
  if(_seg_rt->counter_mode_step < _seg_len/2) 
  {
    
    WS2812FX_setPixelColor(_seg_rt->counter_mode_step, WHITE);
    WS2812FX_setPixelColor(_seg_len-1 - _seg_rt->counter_mode_step, WHITE);
  }
  _seg_rt->counter_mode_step += 1;
  _seg_rt->counter_mode_step%=_seg_len/2;
  if(_seg_rt->counter_mode_step == 0)
  {
    cycle_cnt();
  }
}

//2点中间向两边走，逐点
void mode6(void)
{
  Adafruit_NeoPixel_fill(BLACK, _seg->start, _seg_len);
  if(_seg_rt->counter_mode_step <= _seg_len/2) 
  {
    //反向跑马
    WS2812FX_setPixelColor(_seg_len/2 - _seg_rt->counter_mode_step, WHITE);
    WS2812FX_setPixelColor(_seg_len/2-1 + _seg_rt->counter_mode_step, WHITE);
  }
  _seg_rt->counter_mode_step += 1;
  _seg_rt->counter_mode_step%=_seg_len/2+1;
  if(_seg_rt->counter_mode_step == 0)
  {
    cycle_cnt();
  }
}
//2个点流水，最后4个点频闪
void mode7(void)
{
  if(_seg_rt->counter_mode_step < _seg_len - 4) //正向跑马
  {
    WS2812FX_setPixelColor(_seg_rt->counter_mode_step, WHITE);

    
  }
  if(_seg_rt->counter_mode_step > 1) //清除第3个点
  {
    WS2812FX_setPixelColor(_seg_rt->counter_mode_step - 2, BLACK);
  }
  // if(step2_flag==0) //尾巴闪烁一次，流水下一个点
  _seg_rt->counter_mode_step += 1;
  _seg_rt->counter_mode_step%=_seg_len*2;
  if(_seg_rt->counter_mode_step == 0)
  {
    Adafruit_NeoPixel_fill(BLACK, _seg->start, _seg_len);
    cycle_cnt();
  }

  if(_seg_rt->counter_mode_step >= _seg_len - 4)
  {
    if(step2_flag)
    {
      WS2812FX_setPixelColor(8, WHITE);
      WS2812FX_setPixelColor(10, WHITE);
      WS2812FX_setPixelColor(9, BLACK);
      WS2812FX_setPixelColor(11, BLACK);
    }
    else
    {
      WS2812FX_setPixelColor(8, BLACK);
      WS2812FX_setPixelColor(10, BLACK);
      WS2812FX_setPixelColor(9, WHITE);
      WS2812FX_setPixelColor(11, WHITE);
    }
    step2_flag=~step2_flag;
  }
}

// 假频谱
void mode8(void)
{
    music_dly = 10;
    if(_seg_rt->aux_param == 0) //全亮-》灭4颗
    {
      if(_seg_rt->counter_mode_step > 2)
      {
        WS2812FX_setPixelColor(_seg_rt->counter_mode_step, BLACK);
        WS2812FX_setPixelColor(_seg_len -1 - _seg_rt->counter_mode_step, BLACK);

        _seg_rt->counter_mode_step--;
      }
      else
      {
        _seg_rt->aux_param = 1;
      }
    }
    else if(_seg_rt->aux_param == 1)
    {
      if(_seg_rt->counter_mode_step < _seg_len/2-1)
      {
       
        WS2812FX_setPixelColor(_seg_rt->counter_mode_step, WHITE);
        WS2812FX_setPixelColor(_seg_len -1 -_seg_rt->counter_mode_step, WHITE);

         _seg_rt->counter_mode_step++;
      }
      else
      {
        _seg_rt->aux_param = 2;
      }
    }
    else if(_seg_rt->aux_param == 2) 
    {
      if(_seg_rt->counter_mode_step > 0)
      {
        WS2812FX_setPixelColor(_seg_rt->counter_mode_step, BLACK);
        WS2812FX_setPixelColor(_seg_len -1 -_seg_rt->counter_mode_step, BLACK);

        _seg_rt->counter_mode_step--;
      }
      else
      {
        _seg_rt->aux_param = 3;
      }
    }
    else if(_seg_rt->aux_param == 3) 
    {
      if(_seg_rt->counter_mode_step < _seg_len/2)
      {
        WS2812FX_setPixelColor(_seg_rt->counter_mode_step, WHITE);
        WS2812FX_setPixelColor(_seg_len -1 -_seg_rt->counter_mode_step, WHITE);

        _seg_rt->counter_mode_step++;
      }
      else
      {
        _seg_rt->aux_param = 0;
 
        cycle_cnt();

      }
    }
}

void test(void)
{


  Adafruit_NeoPixel_fill(WHITE, _seg->start, _seg_len);   //全段填黑色，灭灯
}


// 随机闪烁
void mode9(void)
{
  music_dly = 10;
  uint8_t size = 1 << SIZE_OPTION;
  Adafruit_NeoPixel_fill(BLACK, _seg->start + _seg_rt->aux_param3, 1);

  _seg_rt->aux_param3 = WS2812FX_random16_lim(_seg_len - 1); // aux_param3 stores the random led index
  Adafruit_NeoPixel_fill(WHITE, _seg->start + _seg_rt->aux_param3, 1);
  _seg_rt->aux_param3 = WS2812FX_random16_lim(_seg_len - 1); // aux_param3 stores the random led index
  Adafruit_NeoPixel_fill(WHITE, _seg->start + _seg_rt->aux_param3, 1);
  _seg_rt->aux_param3 = WS2812FX_random16_lim(_seg_len - 1); // aux_param3 stores the random led index
  Adafruit_NeoPixel_fill(WHITE, _seg->start + _seg_rt->aux_param3, 1);
  _seg_rt->counter_mode_step++;
  if(_seg_rt->counter_mode_step > 100)
  cycle_cnt();

}
// 各种效果的大集合
//有声时，跑大集合，当无声时，跑step 0
uint16_t music_1(void)
{
 // printf("music_step = %d",music_step);
  music_dly = 30;
 // printf("music_dly = %d", music_dly);
  if(music_step == 0)//倒序2个灯逐点流水
  {
    mode1();
  }
  else if(music_step == 1)//顺序2个点一组，一共2组，第一组从0开始，第二组从一半开始
  {
    mode3();
  }
  else if(music_step == 2)//倒序2个点一组，一共2组，第一组从0开始，第二组从一半开始
  {
    mode4();
  }
  else if(music_step == 3)  //两边向中间走,逐点
  {
    mode5();
  }
  else if(music_step == 4) //2点中间向两边走，逐点
  {
    mode6();
  }
  else if(music_step == 5)//顺序2个点一组，一共2组，第一组从0开始，第二组从一半开始
  {
    mode3();
  }
  else if(music_step == 6)//倒序2个点一组，一共2组，第一组从0开始，第二组从一半开始
  {
    mode4();
  }
  if(music_step ==7) //正向流水
  {
 
    if(_seg_rt->counter_mode_step < _seg_len)
    {
      WS2812FX_setPixelColor(_seg_rt->counter_mode_step, WHITE);
    }
    else
    {
      WS2812FX_setPixelColor(_seg_rt->counter_mode_step - _seg_len, BLACK);
    }
		_seg_rt->counter_mode_step++;
    _seg_rt->counter_mode_step%=_seg_len*2;
    if(_seg_rt->counter_mode_step == 0)
    {
      cycle_cnt();
    }
  }
  else if(music_step ==8)//反向流水
  {
    
    if(_seg_rt->counter_mode_step < _seg_len)
    {
      WS2812FX_setPixelColor(_seg->stop - _seg_rt->counter_mode_step, WHITE);
    }
    else
    {
      WS2812FX_setPixelColor(2*_seg_len - _seg_rt->counter_mode_step - 1, BLACK);
    }
    _seg_rt->counter_mode_step++;
    _seg_rt->counter_mode_step%=_seg_len*2;  
    if(_seg_rt->counter_mode_step == 0)
    {
      cycle_cnt();
    }
  }
  else if(music_step == 9)//倒序2个灯逐点流水
  {
    mode1();
  }
  else if(music_step == 10)//顺序2个灯逐点流水
  {
    mode2();
  }
  else if(music_step == 11)
  {
    mode7();
  }
  else if(music_step == 12)  //两边向中间走,逐点
  {
    mode5();
  }
  else if(music_step == 13) //2点中间向两边走，逐点
  {
    mode6();
  }
  else if(music_step == 14) // 假频谱
  {
    mode8();
  }
  else if(music_step == 15) // 随机闪烁
  {
    mode9();
    if(change_mode)
    {
      change_mode = 0;
      Adafruit_NeoPixel_fill(WHITE, _seg->start, _seg_len);
      _seg_rt->counter_mode_step = _seg_len/2;
      _seg_rt->aux_param = 0;
    }
  }
  else if(music_step == 16) // 假频谱
  {
    mode8();
  }
  else if(music_step == 17) // 随机闪烁
  {
    mode9();
  }
  else if(music_step == 18) //顺序2个灯逐点流水
  {
    mode2();
  }
  
  // if(get_sound_result())
  if(music_trigger)  //有声音
  {
    music_trigger = 0;
    music_trg=0;
  }
  else
  {
    if(music_trg < 50)
      music_trg++;
    else
    {
      Adafruit_NeoPixel_fill(BLACK, _seg->start, _seg_len);
      // music_dly = 50000;
      music_step = 0;
      _seg_rt->counter_mode_step = 0;
    }
  }
  //printf("music_dly = %d", music_dly);
  return music_dly * 10;
}



// 开机效果
uint16_t power_on_effect(void)
{
  if(_seg_rt->counter_mode_step)
  {
    Adafruit_NeoPixel_fill( BLACK, _seg->start, _seg_len);
  }
  else
  {
    Adafruit_NeoPixel_fill( GREEN, _seg->start, _seg_len);
  }
  _seg_rt->counter_mode_step =!_seg_rt->counter_mode_step;
  _seg_rt->aux_param++;
  if(_seg_rt->aux_param > 6)
  {
    extern void read_flash_device_status_init(void);
    read_flash_device_status_init();
    set_fc_effect();
  }
  return (500 );
}

uint16_t power_off_effect(void)
{
  if(_seg_rt->counter_mode_step)
  {
    Adafruit_NeoPixel_fill( BLACK, _seg->start, _seg_len);
  }
  else
  {
    Adafruit_NeoPixel_fill( RED, _seg->start, _seg_len);
  }
  _seg_rt->counter_mode_step =!_seg_rt->counter_mode_step;
  _seg_rt->aux_param++;
  if(_seg_rt->aux_param > 4)
  {
    // 硬件关机
    gpio_direction_output(IO_PORTA_08,0);
  }
  return (500 );
}

// 解绑效果
uint16_t unbind_effect(void)
{
  if(_seg_rt->counter_mode_step)
  {
    Adafruit_NeoPixel_fill( WHITE, _seg->start, _seg_len);
  }
  else
  {
    Adafruit_NeoPixel_fill( GRAY, _seg->start, _seg_len);
  }
  _seg_rt->counter_mode_step =!_seg_rt->counter_mode_step;

  return (500 );
}

/*
 * Color wipe function，多种颜色流水效果
 * LEDs are turned on (color1) in sequence, then turned off (color2) in sequence.
 * if (uint8_t rev == true) then LEDs are turned off in reverse order
 * 颜色擦除功能，用color1依次点亮LED,再用color2依次擦除
 * rev = 0:两个颜色同向， =1color2反方向擦除
 * IS_REVERSE:流水方向
 */
uint16_t WS2812FX_multiColor_wipe(uint8_t is_reverse,uint8_t rev)
{
  static uint32_t color[2];

	if(_seg_rt->counter_mode_step == _seg_len)
  {
    // 一个循环后更换颜色
    color[_seg_rt->aux_param] = _seg->colors[_seg_rt->aux_param3];
    _seg_rt->aux_param++;
    _seg_rt->aux_param%=2;
    _seg_rt->aux_param3++;
    _seg_rt->aux_param3%=_seg->c_n;
  }
  if(_seg_rt->counter_mode_step == 0)
  {

    color[_seg_rt->aux_param] = _seg->colors[_seg_rt->aux_param3];
    _seg_rt->aux_param++;
    _seg_rt->aux_param%=2;
    _seg_rt->aux_param3++;
    _seg_rt->aux_param3%=_seg->c_n;
    SET_CYCLE;
  }

  if(_seg_rt->counter_mode_step < _seg_len) {
    uint32_t led_offset = _seg_rt->counter_mode_step;
    if(is_reverse) {
      WS2812FX_setPixelColor(_seg->stop - led_offset, color[0]);
    } else {
      WS2812FX_setPixelColor(_seg->start + led_offset, color[0]);
    }
  } else {
    uint32_t led_offset = _seg_rt->counter_mode_step - _seg_len;
    if((is_reverse && !rev) || (!is_reverse && rev)) {
      WS2812FX_setPixelColor(_seg->stop - led_offset, color[1]);
    } else {
      WS2812FX_setPixelColor(_seg->start + led_offset, color[1]);
    }
  }

	_seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % (_seg_len * 2);

  return (_seg->speed );
}


// 多种颜色流水效果
// 正向流水，颜色同向
uint16_t WS2812FX_mode_multi_forward_same(void)
{
  return WS2812FX_multiColor_wipe(0,0) ;
}


// 多种颜色流水效果
// 反向流水，颜色同向
uint16_t WS2812FX_mode_multi_back_same(void)
{
  return WS2812FX_multiColor_wipe(1,0);
}

/*
在每一个LED颜色随机。并且淡出，淡入
Cycle a rainbow on each LED
 */
uint16_t WS2812FX_mode_fade_each_led(void) {
  if( _seg_rt->counter_mode_step == 0) {
    _seg_rt->counter_mode_step = 0x0f;
    for(uint16_t i=_seg->start; i <= _seg->stop; i++) {
      WS2812FX_setPixelColor(i, WS2812FX_color_wheel(WS2812FX_random8()));

    }
  }
  else{
    WS2812FX_fade_out();
  }
  _seg_rt->counter_mode_step++;
  return (_seg->speed / 8);
}



/*
功能：颜色块跳变效果，多个颜色块组成背景,以块为单位步进做流水,
_seg->c_n:有效颜色数量
SIZE_OPTION：决定颜色块大小
IS_REVERSE:0 反向流水 ；1正向流水，WS2812FX_setOptions(REVERSE)来设置
 */
uint16_t WS2812FX_mode_single_block_scan(void)
{
  uint8_t size = (SIZE_OPTION << 1) + 1;
  uint8_t j;
  uint32_t c;

  if(size > (_seg->stop - _seg->start))
    return 0;
  // size = 5; //debug用，最后删除
  // _seg->c_n = 3;//debug用，最后删除
  _seg_rt->counter_mode_step = 0;
  while(_seg_rt->counter_mode_step  < _seg->stop)
  {
    for(j = 0; j < size; j++)
    {
      if(IS_REVERSE == 0) //反向流水
      {
        WS2812FX_setPixelColor( _seg->start + _seg_rt->counter_mode_step, _seg->colors[_seg_rt->aux_param]);
      }
      else
      {
        WS2812FX_setPixelColor( _seg->stop - _seg_rt->counter_mode_step, _seg->colors[_seg_rt->aux_param]);
      }
      _seg_rt->counter_mode_step++;
      if(_seg_rt->counter_mode_step  > _seg->stop)
      {
        break;
      }
    }
    _seg_rt->aux_param++;
    _seg_rt->aux_param %= _seg->c_n;
  }

    c = _seg->colors[0];
    // 重新开始，对颜色转盘
    for(j=1; j< _seg->c_n; j++)
    {
      // 把后面的颜色提前
      _seg->colors[j-1] = _seg->colors[j];
    }
    _seg->colors[j-1] = c;

  return _seg->speed *size;
}

/*
多段颜色同时流水效果
// SIZE_OPTION：像素点大小
// SIZE_SMALL   1
// SIZE_MEDIUM  3
// SIZE_LARGE   5
// SIZE_XLARGE  7
*/

/*
WS2812FX_setSegment_colorOptions(
        0,                                      //第0段
        0,59,                  //起始位置，结束位置
        &WS2812FX_mode_multi_block_scan,               //效果
        0,                                      //颜色，WS2812FX_setColors设置
        1000,            //速度
        SIZE_MEDIUM | REVERSE);
*/
uint16_t WS2812FX_mode_multi_block_scan(void)
{
  uint8_t size = (SIZE_OPTION << 1) + 1;
  uint8_t j;
  uint16_t i;
  if(size > (_seg->stop - _seg->start))
    return 0;

  /* 构建背景颜色 */
  if(_seg_rt->counter_mode_step == 0)
  {
    while(_seg_rt->counter_mode_step <= _seg->stop)
    {
      for(j = 0; (j < size) && (_seg_rt->counter_mode_step <= _seg->stop); j++)
      {
        WS2812FX_setPixelColor( _seg->start + _seg_rt->counter_mode_step, _seg->colors[_seg_rt->aux_param]);
        _seg_rt->counter_mode_step++;
      }
      _seg_rt->aux_param++;
      _seg_rt->aux_param %= _seg->c_n;
      // _seg_rt->aux_param %= MAX_NUM_COLORS;
    }
  }
  else
  {
    if(IS_REVERSE) //反向流水
    {
      /* 获取原始颜色，没有进行亮度调整的颜色 */

      uint32_t c = Adafruit_NeoPixel_getOriginPixelColor(_seg->start);
      /* 颜色平移一个像素,把后面像素复制到前面 */
      WS2812FX_copyPixels(_seg->start, _seg->start+1, _seg->stop - _seg->start);

      /* 把第一个颜色，补到最后一个位置 */
      Adafruit_NeoPixel_setPixelColor_raw(_seg->stop, c);
    }
    else
    {
      uint32_t c = Adafruit_NeoPixel_getOriginPixelColor(_seg->stop);

      for(i=0;i<_seg->stop - _seg->start;i++)
      {
        WS2812FX_copyPixels(_seg->stop-i , _seg->stop - 1-i, 1);
      }

      Adafruit_NeoPixel_setPixelColor_raw(_seg->start, c);

    }


  }

  return _seg->speed ;
}




/*
 * Fades the LEDs between mutil colors
 */
#if 0
uint16_t WS2812FX_mode_mutil_fade(void)
{
  uint8_t size = 1 << SIZE_OPTION;
  uint16_t j;
  uint32_t color, color1,color0;
  int lum = _seg_rt->counter_mode_step;

  size = 5; //调试用
  _seg->c_n = 3; //调试用
  if(size > (_seg->stop - _seg->start))
    return 0;

  if(lum > 255) lum = 511 - lum; // lum = 0 -> 255 -> 0
  _seg_rt->aux_param = 0;
  _seg_rt->aux_param2 = 0;
  // if(_seg_rt->aux_param2 == 0)
  {
    while(_seg_rt->aux_param2  < _seg->stop)
    {
      color0 = _seg->colors[_seg_rt->aux_param];
      _seg_rt->aux_param++;
      _seg_rt->aux_param %= _seg->c_n;
      color1 = _seg->colors[_seg_rt->aux_param];
      _seg_rt->aux_param++;
      _seg_rt->aux_param %= _seg->c_n;
      color = WS2812FX_color_blend(color1, color0, lum);
      for(j = 0; j < size; j++)
      {
        WS2812FX_setPixelColor( _seg->start + _seg_rt->aux_param2, \
                                color);
        _seg_rt->aux_param2++;
        if(_seg_rt->aux_param2  > _seg->stop)
        {
          break;
        }
      }
    }
  }




  // uint32_t color = WS2812FX_color_blend(_seg->colors[1], _seg->colors[0], lum);
  // Adafruit_NeoPixel_fill(color, _seg->start, _seg_len);

  // 此时颜色停留在color1,把color0颜色变换
  if(_seg_rt->counter_mode_step==0)
  {

  }

  _seg_rt->counter_mode_step++;

  // 此时颜色停留在color1
  if(_seg_rt->counter_mode_step > 511) {
    _seg_rt->counter_mode_step = 0;
    SET_CYCLE;
  }
  // 此时颜色停留再color0
  if(_seg_rt->counter_mode_step == 256)
  {

  }
  return (_seg->speed / 32);
}


#endif

// 多段颜色，同时在做渐变效果，每段效果把_seg->colors轮转
// SIZE_OPTION：像素点大小
// SIZE_SMALL   1
// SIZE_MEDIUM  3
// SIZE_LARGE   5
// SIZE_XLARGE  7
// WS2812FX_set_coloQty()设置颜色数量
uint16_t WS2812FX_mode_mutil_fade(void)
{
  uint8_t size =  (SIZE_OPTION<<1) + 1;
  uint16_t j;
  uint8_t cnt0=0,cnt1 = 1;
  uint32_t color, color1,color0;
  static uint32_t c1[MAX_NUM_COLORS];
  int lum = _seg_rt->counter_mode_step;


  if(size > (_seg->stop - _seg->start) && size== 0)
    return 0;

  if(lum > 255) lum = 511 - lum; // lum = 0 -> 255 -> 0
  _seg_rt->aux_param = 0;
  _seg_rt->aux_param2 = 0;

  if(_seg_rt->aux_param3 == 0)
  {
    _seg_rt->aux_param3 = 1;
    memcpy(c1, _seg->colors ,MAX_NUM_COLORS * 4);

  }

    while(_seg_rt->aux_param2  < _seg->stop)
    {
      color0 = _seg->colors[cnt0];
      cnt0++;
      cnt0 %= _seg->c_n;
      color1 = c1[cnt1];
      cnt1++;
      cnt1 %= _seg->c_n;
      color = WS2812FX_color_blend(color1, color0, lum);
      for(j = 0; j < size; j++)
      {
        WS2812FX_setPixelColor( _seg->start + _seg_rt->aux_param2, \
                                color);
        _seg_rt->aux_param2++;
        if(_seg_rt->aux_param2  > _seg->stop)
        {
          break;
        }
      }
    }

  _seg_rt->counter_mode_step+=4;

  // 此时颜色停留在color1
  if(_seg_rt->counter_mode_step > 511) {
    _seg_rt->counter_mode_step = 0;
    // color0的颜色池左移1

    SET_CYCLE;
  }

  // 此时颜色停留在color1,把color0颜色变换,color0向左转盘
  if(_seg_rt->counter_mode_step==0)
  {
    uint32_t c_tmp;
    c_tmp = _seg->colors[0];

    memmove(_seg->colors, _seg->colors + 1, (_seg->c_n-1)*4);

    _seg->colors[_seg->c_n - 1] = c_tmp;
  }

  // 此时颜色停留再color0
  if(_seg_rt->counter_mode_step == 256)
  {
    // color1的颜色池左移1
    uint32_t c_tmp;
    c_tmp = c1[0];
    memmove(&c1[0], &c1[1], (_seg->c_n-1)*4);


    c1[_seg->c_n - 1] = c_tmp;
  }
  return (_seg->speed / 32);
}



// 多段颜色构成背景色，做呼吸效果
// SIZE_OPTION：像素点大小
// SIZE_SMALL   1
// SIZE_MEDIUM  3
// SIZE_LARGE   5
// SIZE_XLARGE  7
// WS2812FX_set_coloQty()设置颜色数量

uint16_t WS2812FX_mode_mutil_breath(void)
{
  uint8_t size = (SIZE_OPTION << 1) + 1;
  uint8_t j;
  uint16_t lum = _seg_rt->aux_param3;
  uint32_t color;

  if(lum>255)
  {
    lum = 511 - lum;
  }

  if(size > (_seg->stop - _seg->start))
    return 0;
  _seg_rt->counter_mode_step = 0;
  _seg_rt->aux_param = 0;
  while(_seg_rt->counter_mode_step <= _seg->stop)
  {
    for(j = 0; (j < size) && (_seg_rt->counter_mode_step <= _seg->stop); j++)
    {
      color =  WS2812FX_color_blend( _seg->colors[_seg_rt->aux_param], 0, lum);
      WS2812FX_setPixelColor( _seg->start + _seg_rt->counter_mode_step, color);
      _seg_rt->counter_mode_step++;
    }
    _seg_rt->aux_param++;
    _seg_rt->aux_param %= _seg->c_n;
  }

  _seg_rt->aux_param3+=4;
  _seg_rt->aux_param3 %= 511;

  return _seg->speed / 4;
}


// 多段颜色构成背景色，做闪烁
// SIZE_OPTION：像素点大小
// SIZE_SMALL   1
// SIZE_MEDIUM  3
// SIZE_LARGE   5
// SIZE_XLARGE  7
// WS2812FX_set_coloQty()设置颜色数量

uint16_t WS2812FX_mode_mutil_twihkle(void)
{
  uint8_t size = (SIZE_OPTION << 1) + 1;
  uint8_t j;
  size = 5;
  _seg->c_n = 3;
  if(size > (_seg->stop - _seg->start))
    return 0;

  _seg_rt->counter_mode_step = 0;
  _seg_rt->aux_param = 0;
  if(_seg_rt->aux_param3)
  {
      while(_seg_rt->counter_mode_step <= _seg->stop)
    {
      for(j = 0; (j < size) && (_seg_rt->counter_mode_step <= _seg->stop); j++)
      {
        WS2812FX_setPixelColor( _seg->start + _seg_rt->counter_mode_step, _seg->colors[_seg_rt->aux_param]);
        _seg_rt->counter_mode_step++;
      }
      _seg_rt->aux_param++;
      _seg_rt->aux_param %= _seg->c_n;
    }
  }
  else
  {
    Adafruit_NeoPixel_fill( BLACK, _seg->start, _seg_len);

  }

  _seg_rt->aux_param3 =!_seg_rt->aux_param3;


  return _seg->speed;
}






/* -------------------------------ws2812fx自带效果----------------------------------- */

/* #define	FX_MODE_STATIC			0		//静态-无闪烁。只是普通的老式静电灯。
#define	FX_MODE_BLINK			1		//眨眼-正常眨眼。50%的开/关时间。
#define	FX_MODE_BREATH			2		//呼吸-进行众所周知的i-设备的“备用呼吸”。固定速度。
#define	FX_MODE_COLOR_WIPE			3		//颜色擦除-在每个LED灯亮起后点亮所有LED灯。然后按顺序关闭它们。重复
#define	FX_MODE_COLOR_WIPE_INV			4		//颜色擦除反转-与颜色擦除相同，只是交换开/关颜色。
#define	FX_MODE_COLOR_WIPE_REV			5		//颜色擦除反转-在每个LED点亮后点亮所有LED。然后按相反的顺序关闭它们。重复
#define	FX_MODE_COLOR_WIPE_REV_INV			6		//颜色擦除反转-与颜色擦除反转相同，除了交换开/关颜色。
#define	FX_MODE_COLOR_WIPE_RANDOM			7		//颜色擦除随机-将所有LED依次切换为随机颜色。然后用另一种颜色重新开始。
#define	FX_MODE_RANDOM_COLOR			8		//随机颜色-以一种随机颜色点亮所有LED。然后将它们切换到下一个随机颜色。
#define	FX_MODE_SINGLE_DYNAMIC			9		//单一动态-以随机颜色点亮每个LED。将一个随机LED逐个更改为随机颜色。
#define	FX_MODE_MULTI_DYNAMIC			10		//多动态-以随机颜色点亮每个LED。将所有LED同时更改为新的随机颜色。
#define	FX_MODE_RAINBOW			11		//彩虹-通过彩虹同时循环所有LED。
#define	FX_MODE_RAINBOW_CYCLE			12		//彩虹循环-在整个LED串上循环彩虹。
#define	FX_MODE_SCAN			13		//扫描-来回运行单个像素。
#define	FX_MODE_DUAL_SCAN			14		//双扫描-在相反方向来回运行两个像素。
#define	FX_MODE_FADE			15		//淡入淡出-使LED灯再次淡入淡出。
#define	FX_MODE_THEATER_CHASE			16		//剧场追逐——剧场风格的爬行灯。灵感来自Adafruit的例子。
#define	FX_MODE_THEATER_CHASE_RAINBOW			17		//剧院追逐彩虹-剧院风格的彩虹效果爬行灯。灵感来自Adafruit的例子。
#define	FX_MODE_RUNNING_LIGHTS			18		//运行灯光-运行灯光效果与平滑正弦过渡。
#define	FX_MODE_TWINKLE			19		//闪烁-闪烁几个LED灯，重置，重复。
#define	FX_MODE_TWINKLE_RANDOM			20		//闪烁随机-以随机颜色闪烁多个LED，打开、重置、重复。
#define	FX_MODE_TWINKLE_FADE			21		//闪烁淡出-闪烁几个LED，淡出。
#define	FX_MODE_TWINKLE_FADE_RANDOM			22		//闪烁淡出随机-以随机颜色闪烁多个LED，然后淡出。
#define	FX_MODE_SPARKLE			23		//闪烁-每次闪烁一个LED。
#define	FX_MODE_FLASH_SPARKLE			24		//闪烁-以选定颜色点亮所有LED。随机闪烁单个白色像素。
#define	FX_MODE_HYPER_SPARKLE			25		//超级火花-像闪光火花。用更多的闪光。
#define	FX_MODE_STROBE			26		//频闪-经典的频闪效果。
#define	FX_MODE_STROBE_RAINBOW			27		//频闪彩虹-经典的频闪效果。骑自行车穿越彩虹。
#define	FX_MODE_MULTI_STROBE			28		//多重选通-具有不同选通计数和暂停的选通效果，由速度设置控制。
#define	FX_MODE_BLINK_RAINBOW			29		//闪烁彩虹-经典的闪烁效果。骑自行车穿越彩虹。
#define	FX_MODE_CHASE_WHITE			30		//追逐白色——白色上的颜色。
#define	FX_MODE_CHASE_COLOR			31		//追逐颜色-白色在颜色上奔跑。
#define	FX_MODE_CHASE_RANDOM			32		//Chase Random-白色跑步，然后是随机颜色。
#define	FX_MODE_CHASE_RAINBOW			33		//追逐彩虹——白色在彩虹上奔跑。
#define	FX_MODE_CHASE_FLASH			34		//Chase Flash（追逐闪光）-在彩色屏幕上运行的白色闪光。
#define	FX_MODE_CHASE_FLASH_RANDOM			35		//Chase Flash Random（随机闪烁）：白色闪烁，然后是随机颜色。
#define	FX_MODE_CHASE_RAINBOW_WHITE			36		//追逐彩虹白色-彩虹在白色上奔跑。
#define	FX_MODE_CHASE_BLACKOUT			37		//Chase Blackout-黑色在颜色上运行。
#define	FX_MODE_CHASE_BLACKOUT_RAINBOW			38		//追逐遮光彩虹-黑色在彩虹上奔跑。
#define	FX_MODE_COLOR_SWEEP_RANDOM			39		//颜色扫描随机-从条带开始和结束交替引入的随机颜色。
#define	FX_MODE_RUNNING_COLOR			40		//运行颜色-交替运行颜色/白色像素。
#define	FX_MODE_RUNNING_RED_BLUE			41		//运行红蓝-交替运行红/蓝像素。
#define	FX_MODE_RUNNING_RANDOM			42		//随机运行-随机彩色像素运行。
#define	FX_MODE_LARSON_SCANNER			43		//拉森扫描仪-K.I.T.T。
#define	FX_MODE_COMET			44		//彗星——从一端发射彗星。
#define	FX_MODE_FIREWORKS			45		//烟花-烟花火花。
#define	FX_MODE_FIREWORKS_RANDOM			46		//烟花随机-随机彩色烟花火花。
#define	FX_MODE_MERRY_CHRISTMAS			47		//圣诞快乐-绿色/红色像素交替运行。
#define	FX_MODE_FIRE_FLICKER			48		//火焰闪烁-火焰闪烁效果。就像在刺骨的风中。
#define	FX_MODE_FIRE_FLICKER_SOFT			49		//火焰闪烁（软）-火焰闪烁效果。跑得更慢/更柔和。
#define	FX_MODE_FIRE_FLICKER_INTENSE			50		//火焰闪烁（强烈）-火焰闪烁效果。更多颜色范围。
#define	FX_MODE_CIRCUS_COMBUSTUS			51		//Circus Combustitus-交替运行白/红/黑像素。
#define	FX_MODE_HALLOWEEN			52		//万圣节-交替运行橙色/紫色像素。
#define	FX_MODE_BICOLOR_CHASE			53		//双色追逐-背景色上运行的两个LED。
#define	FX_MODE_TRICOLOR_CHASE			54		//三色追逐-交替运行三色像素。
#define	FX_MODE_CUSTOM			55  // keep this for backward compatiblity		//闪烁福克斯-灯光随机淡入淡出。
#define	FX_MODE_CUSTOM_0			55  // custom modes need to go at the end		//通过63。自定义-最多八个用户创建的自定义效果。 */
/*
 * Random flickering.
 */
uint16_t WS2812FX_mode_fire_flicker(void) {
  return WS2812FX_fire_flicker(3);
}

/*
* Random flickering, less intensity.
*/
uint16_t WS2812FX_mode_fire_flicker_soft(void) {
  return WS2812FX_fire_flicker(6);
}

/*
* Random flickering, more intensity.
*/
uint16_t WS2812FX_mode_fire_flicker_intense(void) {
  return WS2812FX_fire_flicker(1);
}


/*
 * Random colored firework sparks.
 */
uint16_t WS2812FX_mode_fireworks_random(void) {
  return WS2812FX_fireworks(WS2812FX_color_wheel(WS2812FX_random8()));
}


/*
 * Firework sparks.
 */
uint16_t WS2812FX_mode_fireworks(void) {

  #if 0
  uint32_t color = BLACK;
  do { // randomly choose a non-BLACK color from the colors array
    color = _seg->colors[WS2812FX_random8_lim(MAX_NUM_COLORS)];
  } while (color == BLACK);
  return WS2812FX_fireworks(color);

#else 

uint32_t color = BLACK;
do { // randomly choose a non-BLACK color from the colors array
  color = WHITE;
} while (color == BLACK);
return WS2812FX_fireworks(color) ;
#endif

}

/*
 * Firing comets from one end.
 */
uint16_t WS2812FX_mode_comet(void) {
  WS2812FX_fade_out();

  if(IS_REVERSE) {
    WS2812FX_setPixelColor(_seg->stop - _seg_rt->counter_mode_step, _seg->colors[0]);
  } else {
    WS2812FX_setPixelColor(_seg->start + _seg_rt->counter_mode_step, _seg->colors[0]);
  }

  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % _seg_len;
  if(_seg_rt->counter_mode_step == 0) SET_CYCLE;

  return (_seg->speed / _seg_len);
}


/*
 * K.I.T.T.
 */
uint16_t WS2812FX_mode_larson_scanner(void) {
  WS2812FX_fade_out();

  if(_seg_rt->counter_mode_step < _seg_len) {
    if(IS_REVERSE) {
      WS2812FX_setPixelColor(_seg->stop - _seg_rt->counter_mode_step, _seg->colors[0]);
    } else {
      WS2812FX_setPixelColor(_seg->start + _seg_rt->counter_mode_step, _seg->colors[0]);
    }
  } else {
    uint16_t index = (_seg_len * 2) - _seg_rt->counter_mode_step - 2;
    if(IS_REVERSE) {
      WS2812FX_setPixelColor(_seg->stop - index, _seg->colors[0]);
    } else {
      WS2812FX_setPixelColor(_seg->start + index, _seg->colors[0]);
    }
  }

  _seg_rt->counter_mode_step++;
  if(_seg_rt->counter_mode_step >= (uint16_t)((_seg_len * 2) - 2)) {
    _seg_rt->counter_mode_step = 0;
    SET_CYCLE;
  }

  return (_seg->speed / (_seg_len * 2));
}


/*
 * Random colored pixels running.
 */
uint16_t WS2812FX_mode_running_random(void) {
  uint8_t size = 2 << SIZE_OPTION;
  if((_seg_rt->counter_mode_step) % size == 0) {
    _seg_rt->aux_param = WS2812FX_get_random_wheel_index(_seg_rt->aux_param);
  }

  uint32_t color = WS2812FX_color_wheel(_seg_rt->aux_param);

  return WS2812FX_running(color, color);
}


/*
 * Alternating color/white pixels running.
 */
uint16_t WS2812FX_mode_running_color(void) {
  return WS2812FX_running(_seg->colors[0], _seg->colors[1]);
}


/*
 * Alternating red/blue pixels running.
 */
uint16_t WS2812FX_mode_running_red_blue(void) {
  return WS2812FX_running(RED, BLUE);
}


/*
 * Alternating red/green pixels running.
 */
uint16_t WS2812FX_mode_merry_christmas(void) {
  return WS2812FX_running(RED, GREEN);
}

/*
 * Alternating orange/purple pixels running.
 */
uint16_t WS2812FX_mode_halloween(void) {
  return WS2812FX_running(PURPLE, ORANGE);
}

/*
 * White flashes running on _color.
 */
uint16_t WS2812FX_mode_chase_flash(void) {
  return WS2812FX_chase_flash(_seg->colors[0], WHITE);
}


/*
 * White flashes running, followed by random color.
 */
uint16_t WS2812FX_mode_chase_flash_random(void) {
  return WS2812FX_chase_flash(WS2812FX_color_wheel(_seg_rt->aux_param), WHITE);
}



/*
 * White running on rainbow.
 白色点流水效果，背景为彩虹色，才会说也跟随白色点变换花样
 */
uint16_t WS2812FX_mode_chase_rainbow(void) {
  uint8_t color_sep = 256 / _seg_len;
  uint8_t color_index = _seg_rt->counter_mode_call & 0xFF;
  uint32_t color = WS2812FX_color_wheel(((_seg_rt->counter_mode_step * color_sep) + color_index) & 0xFF);

  return WS2812FX_chase(color, WHITE, WHITE);
}


/*
 * Black running on rainbow.
 */
uint16_t WS2812FX_mode_chase_blackout_rainbow(void) {
  uint8_t color_sep = 256 / _seg_len;
  uint8_t color_index = _seg_rt->counter_mode_call & 0xFF;
  uint32_t color = WS2812FX_color_wheel(((_seg_rt->counter_mode_step * color_sep) + color_index) & 0xFF);

  return WS2812FX_chase(color, BLACK, BLACK);
}


/*
 * White running followed by random color.
 */
uint16_t WS2812FX_mode_chase_random(void) {
  if(_seg_rt->counter_mode_step == 0) {
    _seg_rt->aux_param = WS2812FX_get_random_wheel_index(_seg_rt->aux_param);
  }
  return WS2812FX_chase(WS2812FX_color_wheel(_seg_rt->aux_param), WHITE, WHITE);
}


/*
 * Rainbow running on white.
 */
uint16_t WS2812FX_mode_chase_rainbow_white(void) {
  uint16_t n = _seg_rt->counter_mode_step;
  uint16_t m = (_seg_rt->counter_mode_step + 1) % _seg_len;
  uint32_t color2 = WS2812FX_color_wheel(((n * 256 / _seg_len) + (_seg_rt->counter_mode_call & 0xFF)) & 0xFF);
  uint32_t color3 = WS2812FX_color_wheel(((m * 256 / _seg_len) + (_seg_rt->counter_mode_call & 0xFF)) & 0xFF);

  return WS2812FX_chase(WHITE, color2, color3);
}

/*
 * Bicolor chase mode
 */
uint16_t WS2812FX_mode_bicolor_chase(void) {
  return WS2812FX_chase(_seg->colors[0], _seg->colors[1], _seg->colors[2]);
}


/*
 * White running on _color.
 */
uint16_t WS2812FX_mode_chase_color(void) {
  return WS2812FX_chase(_seg->colors[0], WHITE, WHITE);
}


/*
 * Black running on _color.
 */
uint16_t WS2812FX_mode_chase_blackout(void) {
  return WS2812FX_chase(_seg->colors[0], BLACK, BLACK);
}


/*
 * _color running on white.
 */
uint16_t WS2812FX_mode_chase_white(void) {
  return WS2812FX_chase(WHITE, _seg->colors[0], _seg->colors[0]);
}


/*
 * Strobe effect with different strobe count and pause, controlled by speed.
  颜色爆闪
 */
uint16_t WS2812FX_mode_multi_strobe(void) {
  Adafruit_NeoPixel_fill(_seg->colors[1], _seg->start, _seg_len);

  uint16_t delay = 200 + ((9 - (_seg->speed % 10)) * 100);
  uint16_t count = 2 * ((_seg->speed / 100) + 1);
  if(_seg_rt->counter_mode_step < count) {
    if((_seg_rt->counter_mode_step & 1) == 0) {
      Adafruit_NeoPixel_fill(_seg->colors[0], _seg->start, _seg_len);
      delay = 20;
    } else {
      delay = 50;
    }
  }

  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % (count + 1);
  if(_seg_rt->counter_mode_step == 0) SET_CYCLE;
  return delay;
}

/*
 * Blinks one LED at a time.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX_mode_sparkle(void) {
  return WS2812FX_sparkle(_seg->colors[1], _seg->colors[0]);
}


/*
 * Lights all LEDs in the color. Flashes white pixels randomly.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX_mode_flash_sparkle(void) {
  return WS2812FX_sparkle(_seg->colors[0], WHITE);
}


/*
 * Like flash sparkle. With more flash.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX_mode_hyper_sparkle(void) {
  Adafruit_NeoPixel_fill(_seg->colors[0], _seg->start, _seg_len);

  uint8_t size = 1 << SIZE_OPTION;
  for(uint8_t i=0; i<8; i++) {
    Adafruit_NeoPixel_fill(WHITE, _seg->start + WS2812FX_random16_lim(_seg_len - size), size);
  }

  SET_CYCLE;
  return (_seg->speed / 32);
}


/*
 * Blink several LEDs on, fading out.
 */
uint16_t WS2812FX_mode_twinkle_fade(void) {
  return WS2812FX_twinkle_fade(_seg->colors[0]);
}


/*
 * Blink several LEDs in random colors on, fading out.
 */
uint16_t WS2812FX_mode_twinkle_fade_random(void) {
  return WS2812FX_twinkle_fade(WS2812FX_color_wheel(WS2812FX_random8()));
}


// 所有LED当前颜色淡出，弹出最终颜色为_seg->colors[1]
uint16_t WS2812FX_mode_fade_single(void)
{

  WS2812FX_fade_out();
  return (_seg->speed / 8);
}

/*
 * Blink several LEDs on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX_mode_twinkle(void) {
  return WS2812FX_twinkle(_seg->colors[0], _seg->colors[1]);
}

/*
 * Blink several LEDs in random colors on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX_mode_twinkle_random(void) {
  return WS2812FX_twinkle(WS2812FX_color_wheel(WS2812FX_random8()), _seg->colors[1]);
}


/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX_mode_theater_chase_rainbow(void) {
  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) & 0xFF;
  uint32_t color = WS2812FX_color_wheel(_seg_rt->counter_mode_step);
  return WS2812FX_tricolor_chase(color, _seg->colors[1], _seg->colors[1]);
}


/*
 * Running lights effect with smooth sine transition.
 */
uint16_t WS2812FX_mode_running_lights(void) {
  uint8_t size = 1 << SIZE_OPTION;
  uint8_t sineIncr = max(1, (256 / _seg_len) * size);
  for(uint16_t i=0; i < _seg_len; i++) {
    int lum = (int)Adafruit_NeoPixel_sine8(((i + _seg_rt->counter_mode_step) * sineIncr));
    uint32_t color = WS2812FX_color_blend(_seg->colors[0], _seg->colors[1], lum);
    if(IS_REVERSE) {
      WS2812FX_setPixelColor(_seg->start + i, color);
    } else {
      WS2812FX_setPixelColor(_seg->stop - i,  color);
    }
  }
  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % 256;
  if(_seg_rt->counter_mode_step == 0) SET_CYCLE;
  return (_seg->speed / _seg_len);
}



/*
 * Tricolor chase mode
 */
uint16_t WS2812FX_mode_tricolor_chase(void) {
  return WS2812FX_tricolor_chase(_seg->colors[0], _seg->colors[1], _seg->colors[2]);
}


/*
 * Alternating white/red/black pixels running.
 */
uint16_t WS2812FX_mode_circus_combustus(void) {
  return WS2812FX_tricolor_chase(RED, WHITE, BLACK);
}


/*
 * Theatre-style crawling lights.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX_mode_theater_chase(void) {
  return WS2812FX_tricolor_chase(_seg->colors[0], _seg->colors[1], _seg->colors[1]);
}

/*
 * Cycles a rainbow over the entire string of LEDs.
 彩虹颜色流水效果
 */
uint16_t WS2812FX_mode_rainbow_cycle(void) {
  for(uint16_t i=0; i < _seg_len; i++) {
	  uint32_t color = WS2812FX_color_wheel(((i * 256 / _seg_len) + _seg_rt->counter_mode_step) & 0xFF);
    WS2812FX_setPixelColor(_seg->stop - i, color);
  }

  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) & 0xFF;

  if(_seg_rt->counter_mode_step == 0) SET_CYCLE;

  return (_seg->speed / 256);
}

/*
 * Cycles all LEDs at once through a rainbow.
 */
uint16_t WS2812FX_mode_rainbow(void) {
  uint32_t color = WS2812FX_color_wheel(_seg_rt->counter_mode_step);
  Adafruit_NeoPixel_fill(color, _seg->start, _seg_len);

  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) & 0xFF;

  if(_seg_rt->counter_mode_step == 0)  SET_CYCLE;

  return (_seg->speed / 256);
}

/*
 * Runs a block of pixels back and forth.
 来回运动像素块
 */
uint16_t WS2812FX_mode_scan(void) {
  return WS2812FX_scan(_seg->colors[0], _seg->colors[1], false);
}


/*
 * Runs two blocks of pixels back and forth in opposite directions.
 */
uint16_t WS2812FX_mode_dual_scan(void) {
  return WS2812FX_scan(_seg->colors[0], _seg->colors[1], true);
}


/*
 * Fades the LEDs between two colors
 */
uint16_t WS2812FX_mode_fade(void) {
  int lum = _seg_rt->counter_mode_step;
  if(lum > 255) lum = 511 - lum; // lum = 0 -> 255 -> 0

  uint32_t color = WS2812FX_color_blend(_seg->colors[1], _seg->colors[0], lum);
  Adafruit_NeoPixel_fill(color, _seg->start, _seg_len);

  _seg_rt->counter_mode_step += 4;
  if(_seg_rt->counter_mode_step > 511) {
    _seg_rt->counter_mode_step = 0;
    SET_CYCLE;
  }
  return (_seg->speed / 128);
}

/*
 * Does the "standby-breathing" of well known i-Devices. Fixed Speed.
 * Use mode "fade" if you like to have something similar with a different speed.
 * _seg->colors[1]，和_seg->colors[0]渐变，若_seg->colors[1]为很色就是呼吸功能
 * lum最小值决定两种颜色混合最小比例。典型值15，若为红色呼吸，LED最暗到15
 */
uint16_t WS2812FX_mode_breath(void) {
  int lum = _seg_rt->counter_mode_step;
  if(lum > 255) lum = 511 - lum; // lum = 15 -> 255 -> 15

  uint16_t delay;
  if(lum == 15) delay = 970; // 970 pause before each breath
  else if(lum <=  25) delay = 38; // 19
  else if(lum <=  50) delay = 36; // 18
  else if(lum <=  75) delay = 28; // 14
  else if(lum <= 100) delay = 20; // 10
  else if(lum <= 125) delay = 14; // 7
  else if(lum <= 150) delay = 11; // 5
  else delay = 10; // 4

  uint32_t color =  WS2812FX_color_blend(_seg->colors[1], _seg->colors[0], lum);
  Adafruit_NeoPixel_fill(color, _seg->start, _seg_len);

  _seg_rt->counter_mode_step += 2;
  if(_seg_rt->counter_mode_step > (512-15)) {
    _seg_rt->counter_mode_step = 15;
    SET_CYCLE;
  }
  return delay;
}


/*
 * Lights every LED in a random color. Changes all LED at the same time
 * to new random colors.
 * 每次以随机颜色变换所有LED
 */
uint16_t WS2812FX_mode_multi_dynamic(void) {
  for(uint16_t i=_seg->start; i <= _seg->stop; i++) {
    WS2812FX_setPixelColor(i, WS2812FX_color_wheel(WS2812FX_random8()));
  }
  SET_CYCLE;
  return _seg->speed;
}



/*
 * Lights all LEDs in one random color up. Then switches them
 * to the next random color.
 * 彩虹跳变
 */
uint16_t WS2812FX_mode_random_color(void) {
  _seg_rt->aux_param = WS2812FX_get_random_wheel_index(_seg_rt->aux_param); // aux_param will store our random color wheel index
  uint32_t color = WS2812FX_color_wheel(_seg_rt->aux_param);
  Adafruit_NeoPixel_fill(color, _seg->start, _seg_len);
  SET_CYCLE;
  return _seg->speed;
}


/*
 * Lights every LED in a random color. Changes one random LED after the other
 * to another random color.
 * 以随机颜色点亮每个LED。依次更改一个随机LED
 * 另一种随机颜色。
 */
uint16_t WS2812FX_mode_single_dynamic(void) {
  if(_seg_rt->counter_mode_call == 0) {
    for(uint16_t i=_seg->start; i <= _seg->stop; i++) {
      WS2812FX_setPixelColor(i, WS2812FX_color_wheel(WS2812FX_random8()));
    }
  }

  WS2812FX_setPixelColor(_seg->start + WS2812FX_random16_lim(_seg_len), WS2812FX_color_wheel(WS2812FX_random8()));
  SET_CYCLE;
  return _seg->speed;
}


/*
 * Turns all LEDs after each other to a random color.
 * Then starts over with another color.
 * 彩虹随机颜色依次流水
 */
uint16_t WS2812FX_mode_color_wipe_random(void) {
  if(_seg_rt->counter_mode_step % _seg_len == 0) { // aux_param will store our random color wheel index
    _seg_rt->aux_param = WS2812FX_get_random_wheel_index(_seg_rt->aux_param);
  }
  uint32_t color = WS2812FX_color_wheel(_seg_rt->aux_param);
  return WS2812FX_color_wipe(color, color, false) * 2;
}


/*
 * Random color introduced alternating from start and end of strip.
 彩虹颜色往返流水，每次到达起点/终点，变换颜色
 */
uint16_t WS2812FX_mode_color_sweep_random(void) {
  if(_seg_rt->counter_mode_step % _seg_len == 0) { // aux_param will store our random color wheel index
    _seg_rt->aux_param = WS2812FX_get_random_wheel_index(_seg_rt->aux_param);
  }
  uint32_t color = WS2812FX_color_wheel(_seg_rt->aux_param);
  return WS2812FX_color_wipe(color, color, true) * 2;
}


/*
 * Lights all LEDs one after another.
 */
uint16_t WS2812FX_mode_color_wipe(void) {
  return WS2812FX_color_wipe(_seg->colors[0], _seg->colors[1], false);
}

uint16_t WS2812FX_mode_color_wipe_inv(void) {
  return WS2812FX_color_wipe(_seg->colors[1], _seg->colors[0], false);
}

uint16_t WS2812FX_mode_color_wipe_rev(void) {
  return WS2812FX_color_wipe(_seg->colors[0], _seg->colors[1], true);
}

uint16_t WS2812FX_mode_color_wipe_rev_inv(void) {
  return WS2812FX_color_wipe(_seg->colors[1], _seg->colors[0], true);
}


/*
 * Normal blinking. 50% on/off time.
 */
uint16_t WS2812FX_mode_blink(void) {
  return WS2812FX_blink(_seg->colors[0], _seg->colors[1], false);
}


/*
 * Classic Blink effect. Cycling through the rainbow.
 彩虹颜色和_seg->colors[1]交替闪烁，彩虹颜色一直在变换
 */
uint16_t WS2812FX_mode_blink_rainbow(void) {
  return WS2812FX_blink(WS2812FX_color_wheel(_seg_rt->counter_mode_call & 0xFF), _seg->colors[1], false);
}


/*
 * Classic Strobe effect.
 两个颜色爆闪，_seg->colors[1]下突然爆闪一下_seg->colors[0]，
_seg->colors[0]时间很多眼睛都没察觉
 */
uint16_t WS2812FX_mode_strobe(void) {
  return WS2812FX_blink(_seg->colors[0], _seg->colors[1], true);
}


/*
 * Classic Strobe effect. Cycling through the rainbow.
 彩虹色爆闪
 */
uint16_t WS2812FX_mode_strobe_rainbow(void) {
  return WS2812FX_blink(WS2812FX_color_wheel(_seg_rt->counter_mode_call & 0xFF), _seg->colors[1], true);
}

/*
 * No blinking. Just plain old static light.
 */
uint16_t WS2812FX_mode_static(void) {
  Adafruit_NeoPixel_fill(_seg->colors[0], _seg->start, _seg_len);
  SET_CYCLE;
  return _seg->speed;
}



// 提示效果,白光闪烁
uint16_t white_tips(void)
{
printf("white_tips\r\n");
  if(_seg_rt->counter_mode_step)
  {
    Adafruit_NeoPixel_fill( GRAY, _seg->start, _seg_len);
  }
  else
  {
    Adafruit_NeoPixel_fill( 0, _seg->start, _seg_len);
  }
  _seg_rt->counter_mode_step =!_seg_rt->counter_mode_step;
  _seg_rt->aux_param++;
  if(_seg_rt->aux_param > 3)
  {
    _seg_rt->aux_param = 0;

    extern void read_flash_device_status_init(void);
    read_flash_device_status_init();
    printf("fc_effect.Now_state = %d", fc_effect.Now_state);
    set_fc_effect();
  }
  return (3000 );
}

const uint8_t led[266][16] = {
  {0,	0,	0,	0,	55,	0,	0,	0,	0,	0,	0,	0,	0,	55,	0,	0},
  {2,	0,	0,	2,	50,	0,	0,	0,	0,	0,	0,	2,	0,	50,	0,	2},
  {4,	0,	0,	4,	45,	0,	0,	0,	0,	0,	0,	4,	0,	45,	0,	4},
  {6,	0,	0,	6,	40,	0,	0,	0,	0,	0,	0,	6,	0,	40,	0,	6},
  {8,	0,	0,	8,	35,	0,	0,	0,	0,	0,	0,	8,	0,	35,	0,	8},
  {10,	0,	0,	10,	30,	0,	0,	0,	0,	0,	0,	10,	0,	30,	0,	10},
  {12,	0,	0,	12,	25,	0,	0,	0,	0,	0,	0,	12,	0,	25,	0,	12},
  {14,	0,	0,	14,	20,	0,	0,	0,	0,	0,	0,	14,	0,	20,	0,	14},
  {16,	0,	0,	16,	15,	0,	0,	0,	0,	0,	0,	16,	0,	15,	0,	16},
  {18,	0,	0,	18,	10,	0,	0,	0,	0,	0,	0,	18,	0,	10,	0,	18},
  {20,	0,	0,	20,	5,	0,	0,	0,	0,	0,	0,	20,	0,	5,	0,	20},
  {22,	0,	0,	22,	0,	0,	0,	0,	0,	0,	0,	22,	0,	0,	0,	22},
  {24,	0,	0,	24,	0,	0,	0,	0,	0,	0,	0,	24,	0,	0,	0,	24},
  {26,	0,	0,	26,	0,	0,	0,	0,	0,	0,	0,	26,	0,	0,	0,	26},
  {28,	0,	0,	28,	0,	0,	0,	0,	0,	0,	0,	28,	0,	0,	0,	28},
  {30,	0,	0,	30,	0,	0,	0,	0,	0,	0,	0,	30,	0,	0,	0,	30},
  {32,	0,	0,	32,	0,	0,	0,	0,	0,	0,	0,	32,	0,	0,	0,	32},
  {34,	0,	0,	34,	0,	0,	0,	0,	0,	0,	0,	34,	0,	0,	0,	34},
  {36,	0,	0,	36,	0,	0,	0,	0,	0,	0,	0,	36,	0,	0,	0,	36},
  {38,	0,	0,	38,	0,	0,	0,	0,	0,	0,	0,	38,	0,	0,	0,	38},
  {40,	0,	0,	40,	0,	0,	0,	0,	0,	0,	0,	40,	0,	0,	0,	40},
  {42,	0,	0,	42,	0,	0,	0,	0,	0,	0,	0,	42,	0,	0,	0,	42},
  {44,	0,	0,	44,	0,	0,	0,	0,	0,	0,	0,	44,	0,	0,	0,	44},
  {46,	0,	0,	46,	0,	0,	0,	0,	0,	0,	0,	46,	0,	0,	0,	46},
  {48,	0,	0,	48,	0,	0,	0,	0,	0,	0,	0,	48,	0,	0,	0,	48},
  {50,	0,	0,	50,	0,	0,	0,	0,	0,	0,	0,	50,	0,	0,	0,	50},
  {52,	0,	0,	52,	0,	0,	0,	0,	0,	0,	0,	52,	0,	0,	0,	52},
  {54,	0,	0,	54,	0,	0,	0,	0,	0,	0,	0,	54,	0,	0,	0,	54},
  {56,	0,	0,	56,	0,	0,	0,	0,	0,	0,	0,	56,	0,	0,	0,	56},
  {58,	0,	0,	58,	0,	0,	0,	0,	0,	0,	0,	58,	0,	0,	0,	58},
  {60,	0,	0,	60,	0,	0,	0,	0,	0,	0,	0,	60,	0,	0,	0,	60},
  {62,	0,	0,	62,	0,	0,	0,	0,	0,	0,	0,	62,	0,	0,	0,	62},
  {64,	0,	0,	64,	0,	0,	0,	0,	0,	0,	0,	64,	0,	0,	0,	64},
  {66,	2,	0,	66,	0,	0,	0,	0,	0,	0,	0,	66,	0,	0,	0,	66},
  {68,	4,	0,	68,	0,	0,	0,	0,	0,	0,	0,	68,	0,	0,	0,	68},
  {70,	6,	0,	70,	0,	0,	0,	0,	0,	0,	0,	70,	0,	0,	0,	70},
  {72,	8,	0,	72,	0,	0,	0,	0,	0,	0,	0,	72,	0,	0,	0,	72},
  {74,	10,	0,	74,	0,	0,	0,	0,	0,	0,	0,	74,	0,	0,	0,	74},
  {76,	12,	0,	76,	0,	0,	0,	0,	0,	0,	0,	76,	0,	0,	0,	76},
  {78,	14,	0,	78,	0,	0,	0,	0,	0,	0,	0,	78,	0,	0,	0,	78},
  {80,	16,	0,	80,	0,	0,	0,	0,	0,	0,	0,	80,	0,	0,	0,	80},
  {82,	18,	0,	82,	0,	0,	0,	0,	0,	0,	0,	82,	0,	0,	0,	82},
  {84,	20,	0,	84,	0,	0,	0,	0,	0,	0,	0,	84,	0,	0,	0,	84},
  {86,	22,	0,	86,	0,	0,	0,	0,	0,	0,	0,	86,	0,	0,	0,	86},
  {88,	24,	0,	88,	0,	0,	0,	0,	0,	0,	0,	88,	0,	0,	0,	88},
  {90,	26,	0,	90,	0,	0,	0,	0,	0,	0,	0,	90,	0,	0,	0,	90},
  {92,	28,	0,	92,	0,	0,	0,	0,	0,	0,	0,	92,	0,	0,	0,	92},
  {94,	30,	0,	94,	0,	0,	0,	0,	0,	0,	0,	94,	0,	0,	0,	94},
  {96,	32,	0,	96,	0,	0,	0,	0,	0,	0,	0,	96,	0,	0,	0,	96},
  {98,	34,	0,	98,	0,	0,	0,	0,	0,	0,	0,	98,	0,	0,	0,	98},
  {100,	36,	0,	100,	0,	0,	2,	0,	0,	0,	0,	100,	0,	0,	0,	100},
  {100,	38,	0,	100,	0,	0,	4,	0,	0,	0,	0,	100,	0,	0,	0,	98},
  {100,	40,	0,	100,	0,	0,	6,	0,	0,	0,	0,	100,	0,	0,	0,	96},
  {100,	42,	0,	100,	0,	0,	8,	0,	0,	0,	0,	100,	0,	0,	0,	94},
  {100,	44,	0,	100,	0,	0,	10,	0,	0,	0,	0,	100,	0,	0,	0,	92},
  {100,	46,	0,	100,	0,	0,	12,	0,	0,	0,	0,	100,	0,	0,	0,	90},
  {100,	48,	0,	100,	0,	0,	14,	0,	0,	0,	0,	100,	0,	0,	0,	88},
  {100,	50,	0,	100,	0,	0,	16,	0,	0,	0,	0,	100,	0,	0,	0,	86},
  {100,	52,	0,	100,	0,	0,	18,	0,	0,	0,	0,	100,	0,	0,	0,	84},
  {100,	54,	0,	100,	0,	0,	20,	0,	0,	0,	0,	100,	0,	0,	0,	82},
  {100,	56,	0,	100,	0,	0,	22,	0,	0,	0,	0,	100,	0,	0,	0,	80},
  {100,	58,	0,	100,	0,	0,	24,	0,	0,	0,	0,	100,	0,	0,	0,	78},
  {100,	60,	0,	98,	0,	0,	26,	0,	0,	0,	0,	98,	0,	0,	0,	76},
  {98,	62,	0,	96,	0,	0,	28,	0,	0,	0,	0,	96,	0,	0,	0,	74},
  {96,	64,	0,	94,	0,	0,	30,	0,	0,	0,	0,	94,	0,	0,	0,	72},
  {94,	66,	0,	92,	0,	0,	32,	0,	0,	0,	0,	92,	0,	0,	0,	70},
  {92,	68,	0,	90,	0,	0,	34,	0,	0,	0,	0,	90,	0,	0,	0,	68},
  {90,	70,	0,	88,	0,	0,	36,	0,	0,	0,	0,	88,	0,	0,	0,	66},
  {88,	72,	0,	86,	0,	0,	38,	0,	0,	0,	0,	86,	0,	0,	0,	64},
  {86,	74,	0,	84,	0,	0,	40,	0,	0,	0,	0,	84,	0,	0,	0,	62},
  {84,	76,	0,	82,	0,	0,	42,	0,	0,	0,	0,	82,	0,	0,	0,	60},
  {82,	78,	0,	80,	0,	0,	44,	0,	0,	0,	0,	80,	0,	0,	0,	58},
  {80,	80,	0,	78,	0,	0,	46,	0,	0,	0,	0,	78,	0,	0,	0,	56},
  {78,	82,	0,	76,	0,	0,	48,	0,	0,	0,	0,	76,	0,	0,	0,	54},
  {76,	84,	0,	74,	0,	0,	50,	0,	0,	0,	0,	74,	0,	0,	0,	52},
  {74,	86,	0,	72,	0,	0,	52,	0,	0,	0,	0,	72,	0,	0,	0,	50},
  {72,	88,	0,	70,	0,	0,	54,			0,	0,	70,	0,	0,	0,	48},
  {70,	90,	0,	68,	0,	0,	56,			0,	0,	68,	0,	0,	0,	46},
  {68,	92,	0,	66,	0,	0,	58,			0,	0,	66,	0,	0,	0,	44},
  {66,	94,	0,	64,	0,	0,	60,			0,	0,	64,	0,	0,	0,	42},
  {64,	96,	0,	62,	0,	0,	62,			0,	0,	62,	0,	0,	0,	40},
  {62,	98,	0,	60,	0,	0,	64,			0,	0,	60,	0,	0,	0,	38},
  {60,	100,	2,	58,	0,	0,	66,			0,	0,	58,	0,	0,	0,	36},
  {58,	100,	4,	56,	0,	0,	68,			0,	0,	56,	0,	2,	0,	34},
  {56,	100,	6,	54,	0,	0,	70,			0,	0,	54,	0,	4,	0,	32},
  {54,	100,	8,	52,	0,	0,	72,			0,	0,	52,	0,	6,	0,	30},
  {52,	100,	10,	50,	0,	0,	74,			0,	0,	50,	0,	8,	0,	28},
  {50,	100,	12,	48,	0,	0,	76,			0,	0,	48,	0,	10,	0,	26},
  {48,	100,	14,	46,	0,	0,	78,			0,	0,	46,	0,	12,	0,	24},
  {46,	100,	16,	44,	2,	0,	80,			0,	0,	44,	0,	14,	0,	22},
  {44,	100,	18,	42,	4,	0,	82,			0,	0,	42,	0,	16,	0,	20},
  {42,	100,	20,	40,	6,	0,	84,			0,	0,	40,	0,	18,	0,	18},
  {40,	100,	22,	38,	8,	0,	86,			0,	0,	38,	0,	20,	0,	16},
  {38,	100,	24,	36,	10,	0,	88,			0,	0,	36,	0,	22,	0,	14},
  {36,	98,	26,	34,	12,	0,	90,			0,	0,	34,	0,	24,	0,	12},
  {34,	96,	28,	32,	14,	0,	92,			0,	0,	32,	0,	26,	0,	10},
  {32,	94,	30,	30,	16,	0,	94,			0,	0,	30,	0,	28,	0,	8},
  {30,	92,	32,	28,	18,	0,	96,			0,	0,	28,	0,	30,	0,	6},
  {28,	90,	34,	26,	20,	0,	98,			0,	0,	26,	0,	32,	0,	4},
  {26,	88,	36,	24,	22,	0,	100,			0,	0,	24,	0,	34,	0,	2},
  {24,	86,	38,	22,	24,	0,	100,			0,	0,	22,	0,	36,	0,	0},
  {22,	84,	40,	20,	26,	0,	100,			0,	0,	20,	0,	38,	0,	0},
  {20,	82,	42,	18,	28,	0,	100,			0,	0,	18,	0,	40,	0,	0},
  {18,	80,	44,	16,	30,	0,	100,			0,	0,	16,	0,	42,	0,	0},
  {16,	78,	46,	14,	32,	0,	100,			0,	0,	14,	0,	44,	0,	0},
  {14,	76,	48,	12,	34,	0,	100,			0,	0,	12,	0,	46,	0,	0},
  {12,	74,	50,	10,	36,	0,	100,			0,	0,	10,	0,	48,	0,	0},
  {10,	72,	52,	8,	38,	0,	100,			0,	0,	8,	0,	50,	0,	0},
  {8,	70,	54,	6,	40,	0,	100,			0,	0,	6,	0,	52,	0,	0},
  {6,	68,	56,	4,	42,	0,	100,			0,	0,	4,	0,	54,	0,	0},
  {4,	66,	58,	2,	44,	0,	100,			0,	0,	2,	0,	56,	0,	0},
  {2,	64,	60,	0,	46,	0,	98,			0,	0,	0,	0,	58,	0,	0},
  {0,	62,	62,	0,	48,	2,	96,			0,	0,	0,	0,	60,	0,	0},
  {0,	60,	64,	0,	50,	4,	94,			0,	0,	0,	2,	62,	2,	0},
  {0,	58,	66,	0,	52,	6,	92,			0,	0,	0,	4,	64,	4,	0},
  {0,	56,	68,	0,	54,	8,	90,			0,	0,	0,	6,	66,	6,	0},
  {0,	54,	70,	0,	56,	10,	88,			0,	0,	0,	8,	68,	8,	0},
  {0,	52,	72,	0,	58,	12,	86,			0,	0,	0,	10,	70,	10,	0},
  {0,	50,	74,	0,	60,	14,	84,			0,	0,	0,	12,	72,	12,	0},
  {0,	48,	76,	0,	62,	16,	82,			0,	0,	0,	14,	74,	14,	0},
  {0,	46,	78,	0,	64,	18,	80,			0,	0,	0,	16,	76,	16,	0},
  {0,	44,	80,	0,	66,	20,	78,			0,	0,	0,	18,	78,	18,	0},
  {0,	42,	82,	0,	68,	22,	76,			0,	0,	0,	20,	80,	20,	0},
  {0,	40,	84,	0,	70,	24,	74,			0,	0,	0,	22,	82,	22,	0},
  {0,	38,	86,	0,	72,	26,	72,	2,	0,	0,	0,	0,	24,	84,	24,	0},
  {0,	36,	88,	0,	74,	28,	70,	4,	0,	0,	0,	0,	26,	86,	26,	0},
  {0,	34,	90,	0,	76,	30,	68,	6,	0,	0,	0,	0,	28,	88,	28,	0},
  {0,	32,	92,	0,	78,	32,	66,	8,	0,	0,	0,	0,	30,	90,	30,	0},
  {0,	30,	94,	0,	80,	34,	64,	10,	0,	0,	0,	0,	32,	92,	32,	0},
  {0,	28,	96,	0,	82,	36,	62,	12,	0,	0,	0,	0,	34,	94,	34,	0},
  {0,	26,	98,	0,	84,	38,	60,	14,	0,	0,	0,	0,	36,	96,	36,	0},
  {0,	24,	100,	0,	86,	40,	58,	16,	2,	0,	0,	0,	38,	98,	38,	0},
  {0,	22,	100,	0,	88,	42,	56,	18,	4,	0,	0,	0,	40,	100,	40,	0},
  {0,	20,	100,	0,	90,	44,	54,	20,	6,	0,	0,	0,	42,	100,	42,	0},
  {0,	18,	100,	0,	92,	46,	52,	22,	8,	0,	0,	0,	44,	100,	44,	0},
  {0,	16,	100,	0,	94,	48,	50,	24,	10,	0,	0,	0,	46,	100,	46,	0},
  {0,	14,	100,	0,	96,	50,	48,	26,	12,	0,	0,	0,	48,	100,	48,	0},
  {0,	12,	100,	0,	98,	52,	46,	28,	14,	0,	0,	0,	50,	100,	50,	0},
  {0,	10,	100,	0,	100,	54,	44,	30,	16,	0,	0,	0,	52,	100,	52,	0},
  {0,	8,	100,	0,	100,	56,	42,	32,	18,	0,	0,	0,	54,	100,	54,	0},
  {0,	6,	100,	0,	100,	58,	40,	34,	20,	2,	0,	0,	56,	100,	56,	0},
  {0,	4,	100,	0,	100,	60,	38,	36,	22,	4,	0,	0,	58,	100,	58,	0},
  {0,	2,	100,	0,	100,	62,	36,	38,	24,	6,	0,	0,	60,	100,	60,	0},
  {0,	0,	98,	0,	100,	64,	34,	40,	26,	8,	0,	0,	62,	100,	62,	0},
  {0,	0,	96,	0,	100,	66,	32,	42,	28,	10,	0,	0,	64,	98,	64,	0},
  {0,	0,	94,	0,	100,	68,	30,	44,	30,	12,	0,	0,	66,	96,	66,	0},
  {0,	0,	92,	0,	100,	70,	28,	46,	32,	14,	0,	0,	68,	94,	68,	0},
  {0,	0,	90,	0,	100,	72,	26,	48,	34,	16,	0,	0,	70,	92,	70,	0},
  {0,	0,	88,	0,	100,	74,	24,	50,	36,	18,	0,	0,	72,	90,	72,	0},
  {0,	0,	86,	0,	100,	76,	22,	52,	38,	20,	0,	0,	74,	88,	74,	0},
  {0,	0,	84,	0,	98,	78,	20,	54,	40,	22,	0,	0,	76,	86,	76,	0},
  {0,	0,	82,	0,	96,	80,	18,	56,	42,	24,	0,	0,	78,	84,	78,	0},
  {0,	0,	80,	0,	94,	82,	16,	58,	44,	26,	0,	0,	80,	82,	80,	0},
  {0,	0,	78,	0,	92,	84,	14,	60,	46,	28,	0,	0,	82,	80,	82,	0},
  {0,	0,	76,	0,	90,	86,	12,	62,	48,	30,	0,	0,	84,	78,	84,	0},
  {0,	0,	74,	0,	88,	88,	10,	64,	50,	32,	0,	0,	86,	76,	86,	0},
  {0,	0,	72,	0,	86,	90,	8,	66,	52,	34,	2,	0,	88,	74,	88,	0},
  {0,	0,	70,	0,	84,	92,	6,	68,	54,	36,	4,	0,	90,	72,	90,	0},
  {0,	0,	68,	0,	82,	94,	4,	70,	56,	38,	6,	0,	92,	70,	92,	0},
  {0,	0,	66,	0,	80,	96,	2,	72,	58,	40,	8,	0,	94,	68,	94,	0},
  {0,	0,	64,	0,	78,	98,	0,	74,	60,	42,	10,	0,	96,	66,	96,	0},
  {0,	0,	62,	0,	76,	100,	0,	76,	62,	44,	12,	0,	98,	64,	98,	0},
  {0,	0,	60,	0,	74,	100,	0,	78,	64,	46,	14,	0,	100,	62,	100,	0},
  {0,	0,	58,	0,	72,	100,	0,	80,	66,	48,	16,	0,	100,	60,	100,	0},
  {0,	0,	56,	0,	70,	100,	0,	82,	68,	50,	18,	0,	100,	58,	100,	0},
  {0,	0,	54,	0,	68,	100,	0,	84,	70,	52,	20,	0,	100,	56,	100,	0},
  {0,	0,	52,	0,	66,	100,	0,	86,	72,	54,	22,	0,	100,	54,	100,	0},
  {0,	0,	50,	0,	64,	100,	0,	88,	74,	56,	24,	0,	100,	52,	100,	0},
  {0,	0,	48,	0,	62,	100,	0,	90,	76,	58,	26,	0,	100,	50,	100,	0},
  {0,	0,	46,	0,	60,	100,	0,	92,	78,	60,	28,	0,	100,	48,	100,	0},
  {0,	0,	44,	0,	58,	100,	0,	94,	80,	62,	30,	0,	100,	46,	100,	0},
  {0,	0,	42,	0,	56,	100,	0,	96,	82,	64,	32,	0,	100,	44,	100,	0},
  {0,	0,	40,	0,	54,	100,	0,	98,	84,	66,	34,	0,	100,	42,	100,	0},
  {0,	0,	38,	0,	52,	98,	0,	100,	86,	68,	36,	0,	100,	40,	100,	0},
  {0,	0,	36,	0,	50,	96,	0,	100,	88,	70,	38,	0,	98,	38,	98,	0},
  {0,	0,	34,	0,	48,	94,	0,	100,	90,	72,	40,	0,	96,	36,	96,	0},
  {0,	0,	32,	0,	46,	92,	0,	100,	92,	74,	42,	0,	94,	34,	94,	0},
  {0,	0,	30,	0,	44,	90,	0,	100,	94,	76,	44,	0,	92,	32,	92,	0},
  {0,	0,	28,	0,	42,	88,	0,	100,	96,	78,	46,	0,	90,	30,	90,	0},
  {0,	0,	26,	0,	40,	86,	0,	100,	98,	80,	48,	0,	88,	28,	88,	0},
  {0,	0,	24,	0,	38,	84,	0,	100,	100,	82,	50,	0,	86,	26,	86,	0},
  {0,	0,	22,	0,	36,	82,	0,	100,	100,	84,	52,	0,	84,	24,	84,	0},
  {0,	0,	20,	0,	34,	80,	0,	100,	100,	86,	54,	0,	82,	22,	82,	0},
  {0,	0,	18,	0,	32,	78,	0,	100,	100,	88,	56,	0,	80,	20,	80,	0},
  {0,	0,	16,	0,	30,	76,	0,	100,	100,	90,	58,	0,	78,	18,	78,	0},
  {0,	0,	14,	0,	28,	74,	0,	98,	100,	92,	60,	0,	76,	16,	76,	0},
  {0,	0,	12,	0,	26,	72,	0,	96,	100,	94,	62,	0,	74,	14,	74,	0},
  {0,	2,	10,	0,	24,	70,	0,	94,	100,	96,	64,	0,	72,	12,	72,	0},
  {0,	4,	8,	0,	22,	68,	0,	92,	100,	98,	66,	0,	70,	10,	70,	0},
  {0,	6,	6,	0,	20,	66,	0,	90,	100,	100,	68,	0,	68,	8,	68,	0},
  {0,	8,	4,	0,	18,	64,	0,	88,	100,	100,	70,	0,	66,	6,	66,	0},
  {0,	10,	2,	0,	16,	62,	0,	86,	100,	100,	72,	0,	64,	4,	64,	0},
  {0,	12,	0,	0,	14,	60,	0,	84,	98,	100,	74,	0,	62,	2,	62,	0},
  {0,	14,	0,	0,	12,	58,	0,	82,	96,	100,	76,	0,	60,	0,	60,	0},
  {0,	16,	0,	0,	10,	56,	0,	80,	94,	100,	78,	0,	58,	0,	58,	0},
  {0,	18,	0,	0,	8,	54,	0,	78,	92,	100,	80,	0,	56,	0,	56,	0},
  {0,	20,	0,	0,	6,	52,	0,	76,	90,	100,	82,	0,	54,	0,	54,	0},
  {0,	22,	0,	0,	4,	50,	0,	74,	88,	100,	84,	0,	52,	0,	52,	0},
  {0,	24,	0,	0,	2,	48,	0,	72,	86,	100,	86,	0,	50,	0,	50,	0},
  {0,	26,	0,	0,	0,	46,	0,	70,	84,	100,	88,	0,	48,	0,	48,	0},
  {0,	28,	0,	0,	0,	44,	0,	68,	82,	100,	90,	0,	46,	0,	46,	0},
  {0,	30,	0,	0,	0,	42,	0,	66,	80,	98,	92,	0,	44,	0,	44,	0},
  {0,	32,	0,	0,	0,	40,	0,	64,	78,	96,	94,	0,	42,	0,	42,	0},
  {0,	34,	0,	0,	0,	38,	0,	62,	76,	94,	96,	0,	40,	0,	40,	0},
  {0,	36,	0,	0,	0,	36,	0,	60,	74,	92,	98,	0,	38,	0,	38,	0},
  {0,	38,	0,	0,	0,	34,	0,	58,	72,	90,	100,	0,	36,	0,	36,	0},
  {0,	40,	0,	0,	0,	32,	0,	56,	70,	88,	100,	0,	34,	0,	34,	0},
  {0,	42,	0,	0,	0,	30,	0,	54,	68,	86,	100,	0,	32,	0,	32,	0},
  {0,	44,	0,	0,	0,	28,	0,	52,	66,	84,	100,	0,	30,	0,	30,	0},
  {0,	46,	0,	0,	0,	26,	0,	50,	64,	82,	100,	0,	28,	0,	28,	0},
  {0,	48,	0,	0,	0,	24,	0,	48,	62,	80,	100,	0,	26,	0,	26,	0},
  {0,	50,	0,	0,	0,	22,	0,	46,	60,	78,	100,	0,	24,	0,	24,	0},
  {0,	52,	0,	0,	0,	20,	0,	44,	58,	76,	100,	0,	22,	0,	22,	0},
  {0,	54,	0,	0,	0,	18,	0,	42,	56,	74,	100,	0,	20,	0,	20,	0},
  {0,	56,	0,	0,	0,	16,	0,	40,	54,	72,	100,	0,	18,	0,	18,	0},
  {0,	58,	0,	0,	0,	14,	0,	38,	52,	70,	100,	0,	16,	0,	16,	0},
  {0,	60,	0,	0,	0,	12,	0,	36,	50,	68,	100,	0,	14,	0,	14,	0},
  {0,	62,	0,	0,	0,	10,	0,	34,	48,	66,	98,	0,	12,	0,	12,	0},
  {0,	64,	0,	0,	0,	8,	0,	32,	46,	64,	96,	0,	10,	0,	10,	0},
  {0,	66,	0,	0,	0,	6,	0,	30,	44,	62,	94,	0,	8,	0,	8,	0},
  {0,	68,	0,	0,	0,	4,	0,	28,	42,	60,	92,	0,	6,	0,	6,	0},
  {0,	70,	0,	0,	0,	2,	0,	26,	40,	58,	90,	0,	4,	0,	4,	0},
  {0,	72,	0,	0,	0,	0,	0,	24,	38,	56,	88,	0,	2,	0,	2,	0},
  {0,	74,	0,	0,	0,	0,	0,	22,	36,	54,	86,	0,	0,	0,	0,	0},
  {0,	76,	0,	0,	0,	0,	0,	20,	34,	52,	84,	0,	0,	0,	0,	0},
  {0,	78,	0,	0,	0,	0,	0,	18,	32,	50,	82,	0,	0,	0,	0,	0},
  {0,	80,	0,	0,	0,	0,	0,	16,	30,	48,	80,	0,	0,	0,	0,	0},
  {0,	82,	0,	0,	0,	0,	0,	14,	28,	46,	78,	0,	0,	0,	0,	0},
  {0,	84,	0,	0,	0,	0,	0,	12,	26,	44,	76,	0,	0,	0,	0,	0},
  {0,	86,	0,	0,	0,	0,	0,	10,	24,	42,	74,	0,	0,	0,	0,	0},
  {0,	88,	0,	0,	0,	0,	0,	8,	22,	40,	72,	0,	0,	0,	0,	0},
  {0,	90,	0,	0,	0,	0,	0,	6,	20,	38,	70,	0,	0,	0,	0,	0},
  {0,	92,	0,	0,	0,	0,	0,	4,	18,	36,	68,	0,	0,	0,	0,	0},
  {0,	94,	0,	0,	0,	0,	0,	2,	16,	34,	66,	0,	0,	0,	0,	0},
  {0,	96,	0,	0,	0,	0,	0,	0,	14,	32,	64,	0,	0,	0,	0,	0},
  {0,	98,	0,	0,	0,	0,	0,	0,	12,	30,	62,	0,	0,	0,	0,	0},
  {0,	100,	0,	0,	0,	0,	0,	0,	10,	28,	60,	0,	0,	0,	0,	0},
  {0,	100,	0,	0,	0,	0,	0,	0,	8,	26,	58,	0,	0,	0,	0,	0},
  {0,	100,	0,	0,	0,	0,	0,	0,	6,	24,	56,	0,	0,	0,	0,	0},
  {0,	98,	0,	0,	0,	0,	0,	0,	4,	22,	54,	0,	0,	0,	0,	0},
  {0,	96,	0,	0,	0,	0,	0,	0,	2,	20,	52,	0,	0,	0,	0,	0},
  {0,	94,	0,	0,	0,	0,	0,	90,	0,	18,	50,	0,	0,	0,	0,	0},
  {0,	92,	0,	0,	0,	0,	0,	95,	0,	16,	48,	0,	0,	0,	0,	0},
  {0,	90,	0,	0,	0,	0,	0,	100,	0,	14,	46,	0,	0,	0,	0,	0},
  {0,	88,	0,	0,	0,	0,	0,	100,	0,	12,	44,	0,	0,	0,	0,	0},
  {0,	86,	0,	0,	0,	0,	0,	100,	0,	10,	42,	0,	0,	0,	0,	0},
  {0,	84,	0,	0,	0,	0,	0,	95,	0,	8,	40,	0,	0,	0,	0,	0},
  {0,	82,	0,	0,	0,	0,	0,	90,	0,	6,	38,	0,	0,	0,	0,	0},
  {0,	80,	0,	0,	0,	0,	0,	85,	0,	4,	36,	0,	0,	0,	0,	0},
  {0,	78,	0,	0,	0,	0,	0,	80,	0,	2,	34,	0,	0,	0,	0,	0},
  {0,	76,	0,	0,	0,	0,	0,	75,	0,	0,	32,	0,	0,	0,	0,	0},
  {0,	74,	0,	0,	0,	0,	0,	70,	0,	0,	30,	0,	0,	0,	0,	0},
  {0,	72,	0,	0,	0,	0,	0,	65,	0,	0,	28,	0,	0,	0,	0,	0},
  {0,	70,	0,	0,	0,	0,	0,	60,	0,	0,	26,	0,	0,	0,	0,	0},
  {0,	68,	0,	0,	0,	0,	0,	55,	0,	0,	24,	0,	0,	0,	0,	0},
  {0,	66,	0,	0,	5,	0,	0,	50,	0,	0,	22,	5,	0,	5,	0,	0},
  {0,	64,	0,	0,	10,	0,	0,	45,	0,	0,	20,	10,	0,	10,	0,	0},
  {0,	62,	0,	0,	15,	0,	0,	40,	0,	0,	18,	15,	0,	15,	0,	0},
  {0,	60,	0,	0,	20,	0,	0,	35,	0,	0,	16,	20,	0,	20,	0,	0},
  {0,	58,	0,	0,	25,	0,	0,	30,	0,	0,	14,	25,	0,	25,	0,	0},
  {0,	56,	0,	0,	30,	0,	0,	25,	0,	0,	12,	30,	0,	30,	0,	0},
  {0,	54,	0,	0,	35,	0,	0,	20,	0,	0,	10,	35,	0,	35,	0,	0},
  {0,	52,	0,	0,	40,	0,	0,	15,	0,	0,	8,	40,	0,	40,	0,	0},
  {0,	50,	0,	0,	45,	0,	0,	10,	0,	0,	6,	45,	0,	45,	0,	0},
  {0,	48,	0,	0,	50,	0,	0,	5,	0,	0,	4,	50,	0,	50,	0,	0},
  {0,	46,	0,	0,	55,	0,	0,	0,	0,	0,	2,	55,	0,	55,	0,	0},
  
  };

uint16_t starry_sky_()
{
  

  if(_seg_rt->aux_param3 == 0)
  {
    Adafruit_NeoPixel_fill(BLACK, _seg->start, _seg_len);
  }
  if(_seg_rt->counter_mode_step == 0)
  {
    if(_seg_rt->aux_param3 < 255)
      _seg_rt->aux_param3++;
    else
     _seg_rt->aux_param3 = 0;
  }

  int led_r   = (_seg->colors[0] >> 16) & 0xff;
  int led_g = (_seg->colors[0] >>  8) & 0xff;
  int led_b =  _seg->colors[0]        & 0xff;
// printf("led_r = %d ,led_g = %d ,led_b = %d",led_r,led_g,led_b);

  for(int deng = 0; deng < 16; deng++)
  {
    // printf("led[%d][%d] = %d" , _seg_rt->counter_mode_step, deng, led[_seg_rt->counter_mode_step][deng]);
    int _r = (led_r * led[_seg_rt->counter_mode_step][deng]) / 100;
    int _g = (led_g * led[_seg_rt->counter_mode_step][deng]) / 100;
    int _b = (led_b * led[_seg_rt->counter_mode_step][deng]) / 100;
    // printf("deng = %d, _r = %d , _g  = %d, _b = %d",deng, _r , _g , _b);
    WS2812FX_setPixelColor_rgb(deng, _r , _g , _b);
  }
   
 _seg_rt->counter_mode_step++;
 _seg_rt->counter_mode_step %= 266;
  return _seg->speed * 2;
}



