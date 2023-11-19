#ifndef __GPIO_H__
#define __GPIO_H__

#include "stm32f1xx_hal.h"

#define IN_M_PORT GPIOA
#define IN_M_PIN GPIO_PIN_3 // M
#define IN_MIN_PORT GPIOA
#define IN_MIN_PIN GPIO_PIN_2 // -
#define IN_PLU_PORT GPIOA
#define IN_PLU_PIN GPIO_PIN_1 // +
#define IN_GO_PORT GPIOA
#define IN_GO_PIN GPIO_PIN_0 // >

#define OUT_PR_PORT GPIOA
#define OUT_PR_PIN GPIO_PIN_6 // Prev/V--
#define OUT_P_PORT GPIOA
#define OUT_P_PIN GPIO_PIN_4 // P/P/Mode
#define OUT_NE_PORT GPIOA
#define OUT_NE_PIN GPIO_PIN_5 // Next/V++
#define OUT_REPEAT_PORT GPIOA
#define OUT_REPEAT_PIN GPIO_PIN_7 // Repeat

#define TIM_SHOT_PRESS 80
#define TIM_LONG_PRESS 750

typedef enum
{
  IN_M = 0,
  IN_MIN,
  IN_PLU,
  IN_GO
} INPUT_e;

typedef enum
{
  NONE = 0,
  OUT_GO,
  OUT_VOL_P,
  OUT_VOL_M,
  OUT_NEXT,
  OUT_PREV,
  OUT_REPEAT,
  OUT_MODE
} OUTPUT_e;

void readAllInputGpio();
uint8_t readStateGpio(INPUT_e InputX);
void setAllOutput();
void setOutput(OUTPUT_e OutputX);
void upravlenieNazatiem(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint16_t tempCntOutputX, INPUT_e input, uint16_t delay);
void trigerReadAllInputGpio();
#endif