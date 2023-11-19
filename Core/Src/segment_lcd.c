/**
 ******************************************************************************
 * @file           : segment_lcd.c
 * @brief          : 7-segment LCD driver.
 * @author         : MicroTechnics (microtechnics.ru)
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "segment_lcd.h"
#include <stdio.h>

/* Declarations and definitions ----------------------------------------------*/

static McuPin digitPins[DIGITS_NUM] = {{GPIOB, GPIO_PIN_12}, {GPIOB, GPIO_PIN_13}, {GPIOB, GPIO_PIN_15}, {GPIOB, GPIO_PIN_14}};

static McuPin segmentPins[SEGMENTS_NUM] = {{GPIOA, GPIO_PIN_8}, {GPIOA, GPIO_PIN_9}, {GPIOA, GPIO_PIN_10}, {GPIOA, GPIO_PIN_11}, {GPIOA, GPIO_PIN_12}, {GPIOA, GPIO_PIN_15}, {GPIOB, GPIO_PIN_3}};

static McuPin dotPin = {GPIOB, GPIO_PIN_5};

static uint8_t charactersTable[DIGIT_CHARACTERS_NUM + EXTRA_CHARACTERS_NUM] =
    {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x40, 0x00, 0x7D};

static SEG_LCD_ExtraCharacter extraCharacters[EXTRA_CHARACTERS_NUM] = {{ASCII_MINUS_CODE, DIGIT_CHARACTERS_NUM},
                                                                       {ASCII_SPACE_CODE, DIGIT_CHARACTERS_NUM + 1}};

static uint8_t currentCharacters[DIGITS_NUM] = {0x00, 0x00, 0x00, 0x00};
static uint8_t currentDots[DIGITS_NUM] = {0, 0, 0, 0};
static uint8_t currentDigitIndex = 0;

/* Functions -----------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
SEG_LCD_Result SEG_LCD_WriteString(char *str)
{
  uint8_t currentDigitIndex = 0;

  for (uint8_t i = 0; i < DIGITS_NUM; i++)
  {
    currentCharacters[i] = 0x00;
    currentDots[i] = 0;
  }

  while (*str != '\0')
  {
    if (*str == ASCII_DOT_CODE)
    {
      if (currentDigitIndex > 0)
      {
        currentDots[currentDigitIndex - 1] = 1;
      }
    }
    else
    {
      if ((*str >= ASCII_NUMBER_FIRST_CODE) && (*str <= ASCII_NUMBER_LAST_CODE)) //"0-9"
      {
        uint8_t currentCharacterIndex = (*str - ASCII_NUMBER_FIRST_CODE);
        currentCharacters[currentDigitIndex] = charactersTable[currentCharacterIndex];
        currentDigitIndex++;
      }
      else // добавочные знаки
      {
        uint8_t found = 0;

        for (uint8_t i = 0; i < EXTRA_CHARACTERS_NUM; i++)
        {
          if (*str == extraCharacters[i].asciiCode)
          {
            uint8_t currentCharacterIndex = extraCharacters[i].symbolsTableOffset;
            currentCharacters[currentDigitIndex] = charactersTable[currentCharacterIndex];
            found = 1;
            currentDigitIndex++;
            break;
          }
        }

        if (found == 0)
        {
          return SEG_LCD_ERROR;
        }
      }
    }

    if (currentDigitIndex == DIGITS_NUM)
    {
      break;
    }

    str++;
  }

  if (currentDigitIndex < DIGITS_NUM)
  {
    for (int8_t i = currentDigitIndex - 1; i >= 0; i--)
    {
      currentCharacters[i + (DIGITS_NUM - currentDigitIndex)] = currentCharacters[i];
      currentDots[i + (DIGITS_NUM - currentDigitIndex)] = currentDots[i];
    }

    for (uint8_t i = 0; i < (DIGITS_NUM - currentDigitIndex); i++)
    {
      currentCharacters[i] = 0x00;
      currentDots[i] = 0;
    }
  }

  return SEG_LCD_OK;
}

/*----------------------------------------------------------------------------*/
SEG_LCD_Result SEG_LCD_WriteNumber(float number)
{
  char temp[DIGITS_NUM + 2];
  snprintf(temp, DIGITS_NUM + 2, "%.2f", number);
  SEG_LCD_WriteString(temp);

  return SEG_LCD_OK;
}

/*----------------------------------------------------------------------------*/
static void SetOutput(McuPin output, uint8_t state)
{
  HAL_GPIO_WritePin(output.port, output.pin, (GPIO_PinState)state);
}

/*----------------------------------------------------------------------------*/
static void SetSegmentPins(uint8_t characterCode)
{
  for (uint8_t i = 0; i < SEGMENTS_NUM; i++)
  {
    uint8_t bit = (characterCode >> i) & 0x01;

    if (bit == 1)
    {
      SetOutput(segmentPins[i], SEGMENT_PIN_ACTIVE);
    }
    else
    {
      SetOutput(segmentPins[i], !SEGMENT_PIN_ACTIVE);
    }
  }
}

/*----------------------------------------------------------------------------*/
void SEG_LCD_Process()
{
  for (uint8_t i = 0; i < DIGITS_NUM; i++)
  {
    SetOutput(digitPins[i], !DIGIT_PIN_ACTIVE);
  }

  SetSegmentPins(currentCharacters[currentDigitIndex]);

  if (currentDots[currentDigitIndex] == 1)
  {
    SetOutput(dotPin, SEGMENT_PIN_ACTIVE);
  }
  else
  {
    SetOutput(dotPin, !SEGMENT_PIN_ACTIVE);
  }

  SetOutput(digitPins[currentDigitIndex], DIGIT_PIN_ACTIVE);

  currentDigitIndex++;
  if (currentDigitIndex == DIGITS_NUM)
  {
    currentDigitIndex = 0;
  }
}

/*----------------------------------------------------------------------------*/
/**
 * @brief индикация первого двойного числа.
 *
 * @param num (0-99)
 * @return SEG_LCD_Result
 */
SEG_LCD_Result SEG_LCD_one_num(uint8_t num)
{
  uint8_t currentCharacterIndex;
  if ((num > 99))
  {
    return SEG_LCD_ERROR;
  }
  // обнуление
  currentCharacters[3] = 0x00;
  currentCharacters[2] = 0x00;

  // выводим
  currentCharacterIndex = num % 10;
  currentCharacters[3] = charactersTable[currentCharacterIndex];
  currentCharacterIndex = num / 10;
  currentCharacters[2] = charactersTable[currentCharacterIndex];
  return SEG_LCD_OK;
}

/**
 * @brief индикация второго двойного числа.
 *
 * @param num (0-99)
 * @return SEG_LCD_Result
 */
SEG_LCD_Result SEG_LCD_two_num(uint8_t num)
{
  uint8_t currentCharacterIndex;
  if ((num > 99))
  {
    return SEG_LCD_ERROR;
  }
  // обнуление
  currentCharacters[1] = 0x00;
  currentCharacters[0] = 0x00;

  // выводим
  currentCharacterIndex = num % 10;
  currentCharacters[1] = charactersTable[currentCharacterIndex];
  // currentCharacterIndex = num / 10;
  // currentCharacters[0] = charactersTable[currentCharacterIndex];
  return SEG_LCD_OK;
}

/**
 * @brief индикация двоеточия
 *
 * @param set
 * @return SEG_LCD_Result
 */
SEG_LCD_Result SEG_LCD_colon(short set)
{
  if (set)
  {
    currentDots[0] = 0; // центр точка
    currentDots[1] = 1; // двоеточие
    // currentDots[2] = 1; // не работает
    // currentDots[3] = 1; // не работает
  }
  else
  {
    currentDots[1] = 0;
  }
  return SEG_LCD_OK;
}

/**
 * @brief индикация двоеточия
 *
 * @param set
 *    @arg SIMBOL_NONE: "пусто"
 *    @arg SIMBOL_B:    "Б"
 *    @arg SIMBOL_T:    "Т"
 * @return SEG_LCD_Result
 */
SEG_LCD_Result SEG_LCD_simbol(int position, uint8_t data)
{
  currentCharacters[position] = charactersTable[data];
  return SEG_LCD_OK;
}