#include "GPIO.h"
#include "logic.h"
#include "stm32f1xx_hal.h"

static uint16_t vhod[5] = {0};
static uint16_t comand[8] = {0};
extern v_t v;
extern tim_t tim;
uint8_t cntButton;
uint8_t isM;
uint8_t isMLong;
int cntOutputX;
uint16_t tempCntOutputX = 0;
/*******************************************************************************
*  @brief   readAllGpio - считывание состояний всех портов,
*                         с антидребезгом (16 циклов)
*  @param   none
*  @retval  none
*******************************************************************************/
void readAllInputGpio()
{
  vhod[0] = vhod[0] << 1;
  vhod[0] |= HAL_GPIO_ReadPin(IN_M_PORT, IN_M_PIN);
  vhod[1] = vhod[1] << 1;
  vhod[1] |= HAL_GPIO_ReadPin(IN_MIN_PORT, IN_MIN_PIN);
  vhod[2] = vhod[2] << 1;
  vhod[2] |= HAL_GPIO_ReadPin(IN_PLU_PORT, IN_PLU_PIN);
  vhod[3] = vhod[3] << 1;
  vhod[3] |= HAL_GPIO_ReadPin(IN_GO_PORT, IN_GO_PIN);
}

void trigerReadAllInputGpio(){ 
  if (v.Busy && !v.BusyTime)
  {
    //////////////
    // long push
    switch (v.mButtonRezim){
    case 0: // читаем
      if (readStateGpio(IN_M)){
        isMLong = 1;
        if (tim.ms5 > 2000)
        {
          v.mButtonRezim = 1; 
          isMLong = 0;
        }
      }else{
        if (tim.ms5 > 100 && isMLong)
        {
          v.mButtonRezim = 2; 
        }
        isMLong = 0;
        tim.ms5 = 0;
      }
      break;
    case 1: // долгое "B"
      v.rezhim = 4;
      v.step = 0;
      v.mButtonRezim = 3;
      break;
    case 2: // короткое "next"
      v.Busy = 0;
      vIsM();
      v.mButtonRezim = 3;
      break;
    case 3: //ждем отжатия
      if (!readStateGpio(IN_M)){
        v.mButtonRezim = 0;
        tim.ms5 = 0;
      }
      break;
    }
    
      
  // triger push
  
    // if (readStateGpio(IN_M))
    // {
    //   v.Busy = 0;
    //   vIsM();
    // }
    if (readStateGpio(IN_MIN))
    {
      v.Busy = 0;
      vIsMin();
    }
    if (readStateGpio(IN_PLU))
    {
      v.Busy = 0;
      vIsPlu();
    }
    if (readStateGpio(IN_GO))
    {
      v.Busy = 0;
      vIsGo();
    }
  }
  else
  {
    if (!readStateGpio(IN_M) && !readStateGpio(IN_MIN) && !readStateGpio(IN_PLU) && !readStateGpio(IN_GO))
    {
      v.Busy = 1;
    }
  }
}
/*******************************************************************************
*  @brief   readStateGpio - вывод состояния порта
*  @param   InputX - номер порта, берем из INPUT_e
*  @retval  состояние порта с фильтром
*               0 - неактивен
*               1 - активен
*******************************************************************************/
uint8_t readStateGpio(INPUT_e InputX)
{
  if (vhod[InputX] == 0x00) // 0xFFFF
    return 1;
  else
    return 0;
}

/*******************************************************************************
*  @brief   setAllOutput - логика нажатия (в цикл)
*  @param   none
*  @retval  none
*******************************************************************************/
void setAllOutput()
{
  
  for (int i = 0; i < 8; i++)
  {
    if(comand[i] != 0){
      tempCntOutputX = i;
      break;
    }
  }
  if(tempCntOutputX == 0){
    cntOutputX = 0;
  }
  
  if (comand[tempCntOutputX] == OUT_GO)
    upravlenieNazatiem(OUT_P_PORT, OUT_P_PIN, tempCntOutputX, IN_GO, TIM_SHOT_PRESS);
  else if(comand[tempCntOutputX] == OUT_VOL_P)
    upravlenieNazatiem(OUT_NE_PORT, OUT_NE_PIN, tempCntOutputX, IN_PLU, TIM_LONG_PRESS);
  else if(comand[tempCntOutputX] == OUT_VOL_M)
    upravlenieNazatiem(OUT_PR_PORT, OUT_PR_PIN, tempCntOutputX, IN_MIN, TIM_LONG_PRESS);
  else if(comand[tempCntOutputX] == OUT_NEXT)
    upravlenieNazatiem(OUT_NE_PORT, OUT_NE_PIN, tempCntOutputX, IN_M, TIM_SHOT_PRESS);
  else if(comand[tempCntOutputX] == OUT_PREV)
    upravlenieNazatiem(OUT_PR_PORT, OUT_PR_PIN, tempCntOutputX, IN_M, TIM_SHOT_PRESS);
  else if(comand[tempCntOutputX] == OUT_REPEAT)
    ;// upravlenieNazatiem(OUT_REPEAT_PORT, tempCntOutputX, OUT_REPEAT, 1, TIM_SHOT_PRESS); // ?
  else if(comand[tempCntOutputX] == OUT_MODE)
    ;// upravlenieNazatiem(OUT_P_PORT, OUT_P_PIN, tempCntOutputX, 1, TIM_LONG_PRESS); // ?
}

/*******************************************************************************
*  @brief   setOutput - исполнение команды
*  @param   OUTPUT_e - название команды
*  @retval  none
*******************************************************************************/
void setOutput(OUTPUT_e OutputX)
{
  comand[cntOutputX] = OutputX;
  if(cntOutputX < 8)
    cntOutputX++;
}

/*******************************************************************************
*  @brief   upravlenieNazatiem - исполнение команды
*  @param   GPIOx - порт выхода
*  @param   GPIO_Pin - пин выхода
*  @param   tempCntOutputX - сбрасываемый флаг
*  @param   input - пин нажатой кнопки
*  @param   tim - задержка нажатия
*  @retval  none
*******************************************************************************/
void upravlenieNazatiem(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint16_t tempCntOutputX, INPUT_e input, uint16_t delay)
{
  switch (cntButton)
  {
  case 0:
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
    v.BusyTime = 1;
    tim.ticBusyCmd = 0;
    cntButton = 2;
    break;
  case 2:
    if (tim.ticBusyCmd >= delay)
    {
      HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
      tim.ticBusyCmd = 0;
      cntButton = 3;
    }
    break;
  case 3:
    if (tim.ticBusyCmd >= delay)
    {
      if (tim.ticBusyCmd >= 1500)
      {
        cntButton = 4;
      }
    }
    break;
  case 4:
    if (readStateGpio(input) == 0)
    {
      comand[tempCntOutputX] = NONE;
      tim.ticBusyCmd = 0;
      v.BusyTime = 0;
      cntButton = 0;
    }
    break;
  }
}