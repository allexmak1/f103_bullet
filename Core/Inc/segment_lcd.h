/**
  ******************************************************************************
  * @file           : segment_lcd.h
  * @brief          : 7-segment LCD driver.
  * @author         : MicroTechnics (microtechnics.ru)
  https://microtechnics.ru/stm32-i-semisegmentnyj-indikator-dinamicheskaya-indikacziya/
  ******************************************************************************
  */

#ifndef SEG_LCD_H
#define SEG_LCD_H

/* Includes ------------------------------------------------------------------*/

#include "stm32f1xx_hal.h"

/* Declarations and definitions ----------------------------------------------*/

#define DIGITS_NUM 4
#define SEGMENTS_NUM 7

#define SEGMENT_PIN_ACTIVE 0
#define DIGIT_PIN_ACTIVE !SEGMENT_PIN_ACTIVE

#define DIGIT_CHARACTERS_NUM 11
#define EXTRA_CHARACTERS_NUM 2

#define ASCII_NUMBER_FIRST_CODE 0x30
#define ASCII_NUMBER_LAST_CODE 0x39

#define ASCII_MINUS_CODE 0x2D // "-"
#define ASCII_SPACE_CODE 0x20 // " "

#define ASCII_DOT_CODE 0x2E

typedef enum
{
  SEG_LCD_OK,
  SEG_LCD_ERROR
} SEG_LCD_Result;

typedef struct SEG_LCD_ExtraCharacter
{
  uint8_t asciiCode;
  uint8_t symbolsTableOffset;
} SEG_LCD_ExtraCharacter;

typedef struct McuPin
{
  GPIO_TypeDef *port;
  uint16_t pin;
} McuPin;

//typedef enum
//{
//  SIMBOL_NONE = 0,
//  SIMBOL_B,
//  SIMBOL_T
//} SEG_LCD_simbol_e;

typedef enum
{
  V_0 = 0,
  V_1,
  V_2,
  V_3,
  V_4,
  V_5,
  V_6,
  V_7,
  V_8,
  V_9,
  V_MINUS,
  V_SPASE,
  V_BATT
} SEG_LCD_simbol_e;

/* Functions -----------------------------------------------------------------*/

extern void SEG_LCD_Process();
extern SEG_LCD_Result SEG_LCD_WriteNumber(float number);
extern SEG_LCD_Result SEG_LCD_WriteString(char *str);

SEG_LCD_Result SEG_LCD_one_num(uint8_t num);
SEG_LCD_Result SEG_LCD_two_num(uint8_t num);
SEG_LCD_Result SEG_LCD_colon(short set);
SEG_LCD_Result SEG_LCD_simbol(int position, uint8_t data);
#endif // #ifndef SEG_LCD_H