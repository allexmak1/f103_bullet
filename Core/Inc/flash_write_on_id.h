#ifndef FLASH_H
#define FLASH_H

#include "stm32f1xx_hal.h"

// ��������� �� ������
typedef struct
{
    uint32_t adr;         // ����� ������� ������ ������ �����
    uint32_t adrTemp;     // ����� ������� �������� ������ ����� ������ (�5) � ����� ����������
    short isNotClearBank; // ���� ����������� ������� �����
    int cntError;         // ������� ������ ��������� ������
} flash_t;

typedef enum
{
    OK = 0,     // ������� �������
    OK_DEFOULT, // ������� �� ���������
    ERR,        // HAL �� ������� (������ �� �����)
} FLASH_STATE_e;

typedef enum
{
    FIND_PAGE_CLEAR = 0, // ������� ����� ������
    NOT_FIND_PAGE_CLEAR, // �� ������� ����� ������
    FIND_ADR,            // ������� ���������
    NOT_FIND_ADR         // �� ������� ���������
} FLASH_STATE_FIND_e;

//----------------------------------------------------------------
// ���������������� �������
void vTestWriteReadFlash();
FLASH_STATE_e xStartFlash();
uint16_t xReadFlash(uint8_t numData);
FLASH_STATE_e xWriteFlash(uint8_t num, uint16_t data);
FLASH_STATE_e vResetSettings();
//----------------------------------------------------------------
// ���������� �������
FLASH_STATE_e xWriteData(uint8_t num, uint16_t data);
FLASH_STATE_FIND_e xFindFreePage();
FLASH_STATE_FIND_e xFindAdrValue(uint8_t num);

void vClearSector(int numSector);
uint32_t vGetNumSector(int numSector);
uint32_t FlashRead32(uint32_t address);
#endif