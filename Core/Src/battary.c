#include "logic.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_adc.h"
#include "battary.h"

// view zarad akb
// 3.4v-?
// 3.6v-3200
// 4.2v-4000

// доделать
// заряжен - пока не думал как сделать, нужно померит ацп во время заряда

// взять из проекта DHT

#if BATT_ADC_FILTER
#define BATT_MASS_BUFF_ADC 16 // размер массива (max 16)
int bufferAdc[BATT_MASS_BUFF_ADC];
int cntAdc;
int rezVib; // закинуть вфункцию
#else
#define BATT_ADC_K 0.9 // коэф 0.0-1.0
#endif

extern ADC_HandleTypeDef BATT_ADC_HAL;

/*******************************************************************************
 *  @brief   readBattary - чтение данных (в цикл)
 *  @param   none
 *  @retval  результат выводит структуру battary_t
 *******************************************************************************/
battary_t readBattary()
{
  battary_t battary;
  battary.state = B_OK;
  int newAdc;
  int temp = 0;
  newAdc = (uint32_t)xGetADCValue(BATT_ADC_CAHHEL);
#if BATT_ADC_FILTER
  bufferAdc[cntAdc] = newAdc;
  cntAdc = cntAdc >= (BATT_MASS_BUFF_ADC - 1) ? 0 : cntAdc + 1;
  // фильтр растянутая выборка
  for (uint32_t i = 0; i < BATT_MASS_BUFF_ADC; i++)
  {
    temp += bufferAdc[i];
  }
  rezVib = (int)((float)temp / BATT_MASS_BUFF_ADC);
  battary.value = map_i((uint32_t)rezVib, BATT_ADC_MIN_V, BATT_ADC_MAX_V, 0, 100);
#else
  rezVib = (int)filterExpAdaptiveK((float)newAdc);
  battary.value = map_i((uint32_t)rezVib, BATT_ADC_MIN_V, BATT_ADC_MAX_V, 0, 100);
#endif
  // низкий заряд
  if (battary.value <= BATT_ADC_LOW)
  {
    battary.state = B_LOW;
  }

  // очень низкий заряд выводит ошибку
  if (rezVib <= BAT_ERROR)
  {
    battary.value = 0;
    battary.state = B_ERROR;
  }
  // заряжен - пока не думал как сделать, нужно померить ацп во время заряда
  return battary;
}

#if !BATT_ADC_FILTER
// фильтр с экспиденциальным адаптивным коэфициентом
float filterExpAdaptiveK(float newVal)
{
  static float filVal = 0;
  filVal += (newVal - filVal) * BATT_ADC_K;
  return filVal;
}

// среднее с адаптивным коэф
float filterAdaptiveK(float newVal)
{
  static float filVal = 0;
  float k = 0;
  if (abs(newVal - filVal))
    k = 0.9;
  else
    k = 0.03;
  filVal += (newVal - filVal) * k;
  return filVal;
}
#endif

// данные с датчика
int xGetADCValue(uint32_t Channel)
{
  int val = 0;
  ADC_ChannelConfTypeDef sConfig = {0};
  sConfig.Channel = Channel;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_41CYCLES_5;
  HAL_ADC_ConfigChannel(&BATT_ADC_HAL, &sConfig);

  HAL_ADC_Start(&BATT_ADC_HAL);
  HAL_ADC_PollForConversion(&BATT_ADC_HAL, 100000);

  val = HAL_ADC_GetValue(&BATT_ADC_HAL);

  HAL_ADC_Stop(&BATT_ADC_HAL);
  return val; // / Count;
}

// масштабирование
int map_i(int x, int in_min, int in_max, int out_min, int out_max)
{
  if (x > in_max)
    x = in_max;
  if (x < in_min)
    x = in_min;
  int temp = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  return temp;
}

// дополнительное усреднение (не используется)
// среднее из массива и заполнение его
uint32_t xAveragingAdc(uint32_t Channel, uint32_t *element)
{
  uint64_t result = element[0];
  for (int i = 1; i < BATT_MASS_BUFF_ADC; i++)
  {
    result += element[i];
    element[i - 1] = element[i];
  }
  element[BATT_MASS_BUFF_ADC - 1] = (uint32_t)xGetADCValue(Channel);
  result /= BATT_MASS_BUFF_ADC;
  return result;
}