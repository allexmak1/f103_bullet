#ifndef FLASH_H
#define FLASH_H

#include "stm32f1xx_hal.h"

// ��������� ��� ������
typedef struct
{
    uint32_t adr;
    short isDefoltSettings;
    int cntError; // ������� ������ ��������� ������
} flash_t;

typedef enum
{
    OK = 0,                // �������� �������
    OK_DEFOULT,            // �������� �� ���������
    ERR,                   // HAL �� ������� (������ �� ������)
    ERR_NOT_FIND_FREE_ADR, // �� ������ ��������� ����� ����� ������� ������
    FIND_FREE_ADR          // ����� ������
} FLASH_STATE_e;

typedef enum
{
    FIND_PAGE_CLEAR = 0, // ������� ������ ������
    NOT_FIND_PAGE_CLEAR, // �� ������� ������ ������

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