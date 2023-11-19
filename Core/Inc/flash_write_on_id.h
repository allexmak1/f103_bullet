#ifndef FLASH_H
#define FLASH_H

#include "stm32f1xx_hal.h"

// структура дл¤ записи
typedef struct
{
    uint32_t adr;         // адрес текущей пустой ¤чейки пам¤ти
    uint32_t adrTemp;     // адрес текущей найденой ¤чейки через фильтр (ј5) и номер переменной
    short isNotClearBank; // флаг игорировани¤ стирани¤ пам¤ти
    int cntError;         // счетчик ошибок неудачной записи
} flash_t;

typedef enum
{
    OK = 0,     // операци¤ успешна
    OK_DEFOULT, // значени¤ по умолчанию
    ERR,        // HAL не записал (¤чейка не пуста¤)
} FLASH_STATE_e;

typedef enum
{
    FIND_PAGE_CLEAR = 0, // найдена пуста¤ строка
    NOT_FIND_PAGE_CLEAR, // не найдена пуста¤ строка
    FIND_ADR,            // найдена переменна¤
    NOT_FIND_ADR         // не найдена переменна¤
} FLASH_STATE_FIND_e;

//----------------------------------------------------------------
// пользовательские функции
void vTestWriteReadFlash();
FLASH_STATE_e xStartFlash();
uint16_t xReadFlash(uint8_t numData);
FLASH_STATE_e xWriteFlash(uint8_t num, uint16_t data);
FLASH_STATE_e vResetSettings();
//----------------------------------------------------------------
// внутренние функции
FLASH_STATE_e xWriteData(uint8_t num, uint16_t data);
FLASH_STATE_FIND_e xFindFreePage();
FLASH_STATE_FIND_e xFindAdrValue(uint8_t num);

void vClearSector(int numSector);
uint32_t vGetNumSector(int numSector);
uint32_t FlashRead32(uint32_t address);
#endif