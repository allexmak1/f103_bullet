/**
 * @file flash.c
 * @author Makarov
 * @brief
 * @version 0.2
 * @date 2022-01-19
 *
 * @copyright Copyright (c) 2023
 *
 * ������ ������:   ��������� ������� ���������� ������ ��������� �����,
 *                  ������ ����������� ������ � ���������� ��� ������ ���������� ��������.
 *                  ������ ���������� �� flash.adr � �������� ������� �� ��� ��������
 *                  ������ ���������� � flash.adr � ����������� � ��������� (������ ���� ����� ���� 4� ����������).
 *
 * ����� ������:
 *  0x08007000 - 73FF sector 1 1Kbyte
 *  0x08007400 - 77FF sector 2
 *  0x08007800 - 7BFF sector 3
 *  0x08007�00 - 7FFF sector 4 ���������
 */

#include "flash.h"
#include "logic.h"

extern v_t v;

#define DEFOULT_VAR_1 v.numTreak
#define DEFOULT_VAR_2 v.vol
#define DEFOULT_VAR_3 v.led

#define DEFOULT_VAL_1 1
#define DEFOULT_VAL_2 50
#define DEFOULT_VAL_3 50

#define PAGE_0_START 0x08007000
#define PAGE_3_END 0x08007FF0 // max 0x08008000 - f103c8

#define MAX_SECTOR 4 // ���-�� ��������
#define MAX_PAGE 64  // ���-�� ������� � �������(��� ������� ������� �����)(0-63)
#define LEN_PAGE 4   // ���������� (uint32_t) � ��������; (4 x uint32_t)(0-3)

flash_t flash;

//  �������� ����������
FLASH_STATE_e stateFlashTest;
#define MASS_TEST 40

uint32_t dataWrite[MASS_TEST];
uint32_t dataReadTemp[4];
uint32_t cntErrors = 0;

//----------------------------------------------------------------
// �������� �������
//----------------------------------------------------------------

/**
 * @brief ���� ���������� ������, ������ � ������
 *
 */
void vTestWriteReadFlash()
{
    // HAL_FLASH_Unlock();
    // vClearSector(0);
    // vClearSector(1);
    // vClearSector(2);
    // vClearSector(3);
    // HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 0x08007500, 1);
    // HAL_FLASH_Lock();

    // �����
    // ���������� ������ ������ ��������, ������� ���������� ��������
    stateFlashTest = xStartFlash();

    for (int i = 0; i < MASS_TEST; i++)
    {
        // ������
        // 4 ���������� (uint32_t) � ��������
        dataReadTemp[0] = xReadFlash(0);
        dataReadTemp[1] = xReadFlash(1);
        dataReadTemp[2] = xReadFlash(2);
        dataReadTemp[3] = xReadFlash(3);
        // ������
        // ���������� ����� ��� ��������
        dataWrite[0] = i;
        dataWrite[1] = i;
        dataWrite[2] = i;
        dataWrite[3] = i;
        stateFlashTest = xWriteFlash(&dataWrite[0]);
        HAL_Delay(20);
        // ���������
        if (xReadFlash(0) != i)
        {
            cntErrors++;
        }
    }
    // ������� ������ � ���������� ���������� ��������
    // vResetSettings();
}

/**
 * @brief ����� ������ �� ������� �������� � �������.
 *          ���� ������ ������ �� ����� �������� �� ���������,
 *          ����� ������ ������.
 *
 * @return FLASH_STATE_e
 */
FLASH_STATE_e xStartFlash()
{
    FLASH_STATE_FIND_e stateFind;
    stateFind = xFindFreePage();

    // ���� ������ ������ �� ����� �������� �� ���������
    if (flash.adr == PAGE_0_START)
    {
        DEFOULT_VAR_1 = DEFOULT_VAL_1;
        DEFOULT_VAR_2 = DEFOULT_VAL_2;
        DEFOULT_VAR_3 = DEFOULT_VAL_3;
        flash.isDefoltSettings = 1;
        return OK_DEFOULT;
    }
    // ���� ��� ������ ������, �� �������� ��� ��������� ������ � �������
    if (stateFind == NOT_FIND_PAGE_CLEAR)
    {
        flash.adr = PAGE_0_START;
    }
    // ��������� ������
    DEFOULT_VAR_1 = xReadFlash(0);
    DEFOULT_VAR_2 = xReadFlash(1);
    DEFOULT_VAR_3 = xReadFlash(2);
    return OK;
}

/**
 * @brief ������ �� ����.
 * ������ ��������� ������ � ��������(0-3).
 *
 * @param num  ����� � �������� (0-3)
 * @return uint32_t - ������
 */
uint32_t xReadFlash(int numData)
{
    uint32_t val;
    uint32_t adr;
    if (flash.adr != PAGE_0_START)
    {
        adr = flash.adr - 0x10;
    }
    else
    {
        adr = PAGE_3_END;
    }
    switch (numData)
    {
    case 0:
        break;
    case 1:
        adr += 0x4;
        break;
    case 2:
        adr += 0x8;
        break;
    case 3:
        adr += 0xC;
        break;
    }
    val = FlashRead32(adr);
    return val;
}

/**
 * @brief ��������� ������.
 * ������ �������� ����� ���� ��������
 *
 * @param data ���������, 4 �� �� 32 ����
 * @return FLASH_STATE_e
 */
FLASH_STATE_e xWriteFlash(uint32_t *data)
{
    HAL_StatusTypeDef stat;
    short cntErrWrite = 0;
    HAL_FLASH_Unlock();
    if (flash.adr == PAGE_0_START)
    {
        if (flash.isDefoltSettings == 0)
        {
            vClearSector(0);
            vClearSector(1);
            vClearSector(2);
            vClearSector(3);
        }
        else
            flash.isDefoltSettings = 0;
    }
    // ��������� ������
    for (int i = 0; i < LEN_PAGE; i++)
    {
        stat = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash.adr + (i * 4), data[i]);
        // HAL �� ������� (������ �� ������)(�� ������ ������ ������)
        if (stat != HAL_OK)
        {
            flash.cntError++;
            cntErrWrite++;
            break;
        }
    }
    HAL_FLASH_Lock();
    // �������������� ����� �� ������ ��������
    flash.adr += 0x10;
    if (flash.adr >= (PAGE_3_END + 0x10))
        flash.adr = PAGE_0_START;

    // ��������� ��� ��������
    if (xReadFlash(0) != data[0])
    {
        flash.cntError++;
        cntErrWrite++;
    }
    // ���� �������� ��� �������, ��� � �� ���� ��������
    if (flash.cntError >= (MAX_SECTOR * MAX_PAGE))
    {
        flash.cntError = 0;
        return ERR;
    }

    // ��������
    if (cntErrWrite > 0)
    {
        cntErrWrite = 0;
        xWriteFlash(data);
    }
    flash.cntError = 0;
    return OK;
}

/**
 * @brief ����� ���� ���������� ����������� � ����������,
 *          ��� ���� ������� ������. ���� ������ ������ �������� �� ������ �� ������� ������.
 *
 * @return FLASH_STATE_e
 */
FLASH_STATE_e vResetSettings()
{
    flash.adr = PAGE_0_START;
    HAL_FLASH_Unlock();
    vClearSector(0);
    vClearSector(1);
    vClearSector(2);
    vClearSector(3);
    HAL_FLASH_Lock();

    DEFOULT_VAR_1 = DEFOULT_VAL_1;
    DEFOULT_VAR_2 = DEFOULT_VAL_2;
    DEFOULT_VAR_3 = DEFOULT_VAL_3;
    flash.isDefoltSettings = 1;

    xFindFreePage();
    if (flash.adr != PAGE_0_START)
    {
        return ERR;
    }
    return OK_DEFOULT;
}
//----------------------------------------------------------------
// ���������� �������
//----------------------------------------------------------------

/**
 * @brief ����� ������ ��������� �������� + ��������� ��� ��������
 *          ��������� ���������� � ��������� flash.
 *
 * @return FLASH_STATE_FIND_e
 */
FLASH_STATE_FIND_e xFindFreePage()
{
    flash.adr = 0;
    short isFindSector;
    // uint32_t adrTempStart;
    int maxPage = MAX_SECTOR * MAX_PAGE;
    for (int i = 0; i < maxPage; i++)
    {
        flash.adr = PAGE_0_START + (i * 16);
        // ������� ������ ��������
        isFindSector = 0;
        for (int i = 0; i < LEN_PAGE; i++)
        {
            if (FlashRead32(flash.adr + (i * 4)) == 0xFFFFFFFF)
            {
                isFindSector++;
            }
        }
        if (isFindSector == 4)
        { // ��� �������� ������
            return FIND_PAGE_CLEAR;
            // ����� �� �������� �������� ��������� ����� �������
        }
    }
    // ��� ������ ������
    return NOT_FIND_PAGE_CLEAR;
}

/**
 * @brief ������� ��������� �����
 *
 * @param numSector ����� ������� (0-3)
 */
void vClearSector(int numSector)
{
    int adr = vGetNumSector(numSector);
    FLASH_EraseInitTypeDef ef;            // �������� ���������, ����������� ��� ������� �������� ��������
    ef.TypeErase = FLASH_TYPEERASE_PAGES; // ������� �����������
    ef.PageAddress = adr;                 // ����� �������� ��� ��������
    ef.NbPages = 1;                       // ����� ������� = 1
    uint32_t temp;                        // ��������� ���������� ��� ���������� �������� (�� ���������)
    HAL_FLASHEx_Erase(&ef, &temp);        // ����� ������� ��������
}

/**
 * @brief
 *
 * @param numSector  ����� ������� (0-3)
 * @return uint32_t - ����� ������ ������� � �������
 */
uint32_t vGetNumSector(int numSector)
{
    uint32_t adr;
    if (numSector == 0)
        adr = PAGE_0_START;
    else if (numSector == 1)
        adr = PAGE_0_START + 0x400;
    else if (numSector == 2)
        adr = PAGE_0_START + 0x800;
    else if (numSector == 3)
        adr = PAGE_0_START + 0xC00;
    return adr;
}

/**
 * @brief ������ 32 ���
 *
 * @param address  (0x08007000)
 * @return uint32_t - ������
 */
uint32_t FlashRead32(uint32_t address)
{
    return (*(__IO uint32_t *)address);
}
