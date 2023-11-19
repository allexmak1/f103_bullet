#include "main.h"
#include "logic.h"
#include "GPIO.h"

#include "battary.h"
#include <stdio.h>
#include "segment_lcd.h"
#include "flash_write_on_id.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
v_t v;
int isStart;
tim_t tim;
battary_t battary;

void init()
{
  v.rezhim = 1;
  v.step = 0;
  v.numTreak = 1;
  xStartFlash();
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  SEG_LCD_WriteNumber(9888);
  SEG_LCD_colon(1);
  tim.ms3 = 0;
  v.ledSpeed = LED_SPEED_ALWAYS;
}

void loop()
{
  if (tim.ms2 >= 1) // 1ms
  {
    tim.ms2 = 0;
    readAllInputGpio();
    if(isStart)
      trigerReadAllInputGpio();
    SEG_LCD_Process();
    vLedProcess(v.ledSpeed);
    battary = readBattary();
  }
  setAllOutput();
  //======================================================
  switch (v.rezhim)
  {
  case 0:
    // ======================================================= wait =====
    v.step = 0;
    break;
  case 1:
    // ======================================================= Start =====
    switch (v.step)
    {
    case 0:
      if (tim.ms3 >= 3000)
      {
        v.step++;
        setOutput(OUT_GO);
      }
      break;
    case 1:
      if (tim.ms3 >= 4000)
      {
        vNomberTrack(0);
        v.rezhim = 0;
        v.ledSpeed = LED_SPEED_LOW;
        isStart = 1;
        v.step = 0;
      }
      break;
    }
    break;
  case 2:
    // ======================================================= go =====
    if (tim.htim1 >= 1000)
    {
      tim.htim1 = 0;
      switch (v.stepGo)
      {
      case 0:
        setOutput(OUT_GO);
        v.timeMin = 0;
        v.timeSec = 0;
        v.ledSpeed = LED_SPEED_MEDIUM;
        tim.ms3 = 0;
        v.isSec = 1;
        v.stepGo++;
        break;
      case 1:
        if (tim.ms3 >= DELAY_START)
        {
          v.stepGo++;
        }
        break;
      case 2:
        v.timeSec = maxMin(++v.timeSec, 0, 59);
        if (v.timeSec == 0)
          v.timeMin = maxMin(++v.timeMin, 0, 3);
        if (v.timeMin == 3)
        {
          v.stepGo++;
          tim.ms3 = 0;
        }
        break;
      case 3:
        if (tim.ms3 >= 4000)
        {
          v.go = 0;
          v.timeMin = 0;
          v.timeSec = 0;
          v.stepGo = 0;
          v.isSec = 0;
          v.ledSpeed = LED_SPEED_LOW;
          HAL_TIM_Base_Stop_IT(&htim1);
          setOutput(OUT_NEXT);
          setOutput(OUT_GO);
          v.numTreak = maxMin(++v.numTreak, 1, 18);
          xWriteFlash(0, (uint8_t)v.numTreak);
          vNomberTrack(0);
        }
        break;
      }
      SEG_LCD_one_num(v.timeSec);
      SEG_LCD_two_num(v.timeMin);
    }

    if (v.go)
    {
      if (tim.ms4 >= 500)
      {
        tim.ms4 = 0;
        v.isSec = v.isSec ? 0 : 1;
        SEG_LCD_colon(v.isSec);
      }
    }

    break;
  case 3:
    // ======================================================= number track =====
    switch (v.step)
    {
    case 0:
      tim.ms3 = 0;
      setOutput(OUT_NEXT);
      setOutput(OUT_GO);
      v.numTreak = maxMin(++v.numTreak, 1, 18);
      xWriteFlash(0, (uint8_t)v.numTreak);
      vNomberTrack(1);
      v.ledSpeed = LED_SPEED_HIGHT;
      v.step++;
      break;
    case 1:
      if (tim.ms3 > (2500))
      {
        vNomberTrack(0);
        v.ledSpeed = LED_SPEED_LOW;
        v.rezhim = 0;
        v.step = 0;
      }
      break;
    }
    break;
  case 4:
    // ======================================================= battary =====
    switch (v.step)
    {
    case 0:
      SEG_LCD_simbol(0, V_SPASE);
      SEG_LCD_simbol(1, V_BATT);
      SEG_LCD_colon(1);
      v.ledSpeed = LED_SPEED_HIGHT;
      tim.ms3 = 0;
      v.step++;
      SEG_LCD_one_num(battary.value);
      break;
    case 1:
      if (tim.ms3 > 4000)
      {
        vNomberTrack(0);
        v.ledSpeed = LED_SPEED_LOW;
        v.rezhim = 0;
        v.step = 0;
      }
      break;
    }
    break;
  }
}

short maxMin(short var, short min, short max)
{
  if (var > max)
    var = min;
  if (var < min)
    var = min;
  return var;
}

void vSetPwmLed(int value)
{
  if (value >= 50000)
    value = 49999;
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, value);
}

void vLedProcess(int time)
{
  switch (time)
  {
  case LED_SPEED_NONE:
    v.ledValue = 0;
    break;
  case LED_SPEED_ALWAYS:
    v.ledValue = PWM_LED_PRESCALER;
    break;
  default:
    if (tim.ms1 >= time)
    {
      tim.ms1 = 0;
      if (!v.ledIs)
      {
        v.ledValue += 1;
        if (v.ledValue >= PWM_LED_PRESCALER)
        {
          v.ledValue = PWM_LED_PRESCALER;
          v.ledIs = !v.ledIs;
        }
      }
      else
      {
        v.ledValue -= 1;
        if (v.ledValue <= 0)
        {
          v.ledValue = 0;
          v.ledIs = !v.ledIs;
        }
      }
    }
    break;
  }
  vSetPwmLed(v.ledValue);
}

void vNomberTrack(int next)
{
  SEG_LCD_simbol(1, (v.numTreak / 10));
  SEG_LCD_simbol(2, (v.numTreak % 10));
  SEG_LCD_colon(0);
  if (next)
  {
    SEG_LCD_simbol(0, V_SPASE);
    SEG_LCD_simbol(3, V_SPASE);
  }
  else
  {
    SEG_LCD_simbol(0, V_MINUS);
    SEG_LCD_simbol(3, V_MINUS);
  }
}

// trigger button
// ===== next =====
void vIsM()
{
  if (v.go == 0){
    v.rezhim = 3;
    v.step = 0;
  }
  else
    vIsGo();
}
// ===== -- =====
void vIsMin()
{
  setOutput(OUT_VOL_M);
//v.ledSpeed = LED_SPEED_HIGHT;// придумать возврат
}
// ===== ++ =====
void vIsPlu()
{
  setOutput(OUT_VOL_P);
//v.ledSpeed = LED_SPEED_HIGHT;
}
// ===== go =====
void vIsGo()
{
  if (v.go)
  {
    v.go = 0;
    SEG_LCD_colon(1);
    HAL_TIM_Base_Stop_IT(&htim1);
    setOutput(OUT_NEXT);
    setOutput(OUT_GO);
    v.ledSpeed = LED_SPEED_LOW;
    v.numTreak = maxMin(++v.numTreak, 1, 18);
    xWriteFlash(0, (uint8_t)v.numTreak);
    vNomberTrack(0);
    v.rezhim = 0;
  }
  else
  {
    v.go = 1;
    tim.ms4 = 10000;
    v.isSec = 1;
    HAL_TIM_Base_Start_IT(&htim1);
    v.rezhim = 2;
    v.step = 0;
  }
  tim.htim1 = 0;
  v.stepGo = 0;
}
