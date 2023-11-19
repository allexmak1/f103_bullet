#ifndef FLASH_H
#define FLASH_H

#include "stm32f1xx_hal.h"

// структура для записи
typedef struct
{
    uint32_t adr;
    short isDefoltSettings;
    int cntError; // счетчик ошибок неудачной записи
} flash_t;

typedef enum
{
    OK = 0,                // операция успешна
    OK_DEFOULT,            // значения по умолчанию
    ERR,                   // HAL не записал (ячейка не пустая)
    ERR_NOT_FIND_FREE_ADR, // не найден свободный сектр после затирки памяти
    FIND_FREE_ADR          // адрес найден
} FLASH_STATE_e;

typedef enum
{
    FIND_PAGE_CLEAR = 0, // найдена пустая строка
    NOT_FIND_PAGE_CLEAR, // не найдена пустая строка

} FLASH_STATE_FIND_e;

FLASH_STATE_e xStartFlash();
FLASH_STATE_e xWriteFlash(uint32_t *data);
uint32_t xReadFlash(int num);

void vTestWriteReadFlash();

FLASH_STATE_FIND_e xFindFreePage();
FLASH_STATE_e xFindFreePageAndClear();
void vClearSector(int numSector);
uint32_t vGetNumSector(int numSector);
uint32_t FlashRead32(uint32_t address);
#endif