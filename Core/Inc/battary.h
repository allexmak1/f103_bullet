#ifndef __BATARY_H__
#define __BATARY_H__

#include "stm32f1xx_hal.h"

#define BATT_ADC_HAL hadc1
#define BATT_ADC_CAHHEL ADC_CHANNEL_8
#define BATT_ADC_FILTER 1 // используемый фильтр 1 - средн€€ из массива 0 - адаптивна€ с коэф
#define BATT_ADC_LOW 20   // %
#define BAT_ERROR 2500    // adc

#define BATT_ADC_MIN_V 3242 // 3.4 v
#define BATT_ADC_MAX_V 4030 // 4.2 v

typedef enum
{
    B_OK = 0,
    B_ERROR,
    B_LOW,
    B_FULL // не реализованно
} BATTARY_STATE_e;

typedef struct battary
{
    short value;
    BATTARY_STATE_e state;
} battary_t;

#define abs(x) ((x) < 0 ? (-1) * (x) : (x))

battary_t readBattary();
float filterExpAdaptiveK(float newVal);
float filterAdaptiveK(float newVal);
int xGetADCValue(uint32_t Channel);
int map_i(int x, int in_min, int in_max, int out_min, int out_max);
uint32_t xAveragingAdc(uint32_t Channel, uint32_t *element);
#endif