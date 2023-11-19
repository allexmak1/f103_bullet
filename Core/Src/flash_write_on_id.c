/**
 * @file flash.c
 * @author Makarov
 * @brief
 * @version 0.3
 * @date 2022-01-19
 *
 * @copyright Copyright (c) 2023
 *
 * ������ ������:   ��������� ������� ���������� ������ ��������� ����� (����� � �����),
 *                  ������ ����������� ������ � ���������� ��� ������ ���������� ��������.
 *                  ������ ���������� �� flash.adr � �������� ������� �� ��� ��������, ��������� "A5" � ������� ����� ������ (0-255)
 *                  ������ ���������� � flash.adr � ����������� � ���������.
 *
 * ����� ������:
 *  �������� exel
 *
 */

#include "flash_write_on_id.h"
#include "logic.h"

extern v_t v;

#define DEFOULT_VAR_MAX 1
#define DEFOULT_VAR_1 v.numTreak

// �������� ������ � ������� xStartFlash()

#define LEN_PAGE 4 // �����  ������ ������ ��� ������ (� ������) (A5 00 FF FF)

#define START_ADR 0x08007000 // ��������� �����
#define MAX_ADR 0x08007FFC   // ������������ ����� ��������� ������ ������ � ������

flash_t flash;
uint16_t defoultData[DEFOULT_VAR_MAX];



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

    // ������ �� �������
    defoultData[0] = 1; // 1


    // ���� ������ ������ �� ����� �������� �� ���������
    if (flash.adr == START_ADR)
    {
        DEFOULT_VAR_1 = defoultData[0];

        flash.isNotClearBank = 1;
        return OK_DEFOULT;
    }
    // ���� ��� ������ ������, �� �������� ��� ��������� ������ � �������
    if (stateFind == NOT_FIND_PAGE_CLEAR)
    {
        flash.adr = START_ADR;
    }
    // ��������� ������
    DEFOULT_VAR_1 = xReadFlash(0);

    return OK;
}

/**
 * @brief ������ �� ����.
 * ������ ��������� ������ � ��������.
 *
 * @param num  ����� � �������� (0-3)
 * @return uint32_t - ������
 */
uint16_t xReadFlash(uint8_t numData)
{
    FLASH_STATE_FIND_e stateFind;
    uint16_t val = 0;
    flash.adrTemp = flash.adr;
    stateFind = xFindAdrValue(numData);
    if (stateFind == FIND_ADR)
    {
        val = (FlashRead32(flash.adrTemp) >> 16) & 0xFFFF;
    }
    else
    {
        // ���� ���� ��� ��������� �� �� ������
        // �������� �� ���������
        if (numData >= DEFOULT_VAR_MAX)
            return val;
        val = defoultData[numData];
    }
    return val;
}

/**
 * @brief ��������� ������:
 *          ���������� ������ ������ ������������ ������, �� �����:
 *          8 ��� - �5 (������������� �������������)
 *          8 ��� - 0xFF (����� ����������)(0-255)
 *          16 ��� - 0xFFFF (������)(0-65535)
 * @param num ����� ������ (0-255)
 * @param data ������ 16 ���
 * @return FLASH_STATE_e
 */
FLASH_STATE_e xWriteFlash(uint8_t num, uint16_t data)
{
    FLASH_STATE_e state = OK;
    uint8_t tempData[DEFOULT_VAR_MAX];
    HAL_FLASH_Unlock();
    // ������� ������ ���� ��������� �� ��������� ������
    if (flash.adr == START_ADR)
    {
        if (flash.isNotClearBank == 0)
        {
            // ���������� ������
            flash.adr = MAX_ADR + LEN_PAGE;
            for (uint8_t i = 0; i < DEFOULT_VAR_MAX; i++)
            {
                tempData[i] = xReadFlash(i);
            }
            vClearSector(0);
            vClearSector(1);
            vClearSector(2);
            vClearSector(3);
            // ����������
            flash.adr = START_ADR;
            for (uint8_t i = 0; i < DEFOULT_VAR_MAX; i++)
            {
                xWriteData(i, tempData[i]);
            }
        }
        else
        {
            // ������ ���� ��������� �� ���������
            flash.isNotClearBank = 0;
        }
    }
    state = xWriteData(num, data);
    HAL_FLASH_Lock();
    return state;
}

/**
 * @brief ����� ���� ���������� ����������� � ����������
 *          ��� ���� ������� ������
 *
 * @return FLASH_STATE_e
 */
FLASH_STATE_e vResetSettings()
{
    flash.adr = START_ADR;
    HAL_FLASH_Unlock();
    vClearSector(0);
    vClearSector(1);
    vClearSector(2);
    vClearSector(3);
    HAL_FLASH_Lock();
    xFindFreePage();
    if (flash.adr != START_ADR)
    {
        return ERR;
    }
    DEFOULT_VAR_1 = defoultData[0];

    flash.isNotClearBank = 1;
    return OK_DEFOULT;
}

//----------------------------------------------------------------
FLASH_STATE_e xWriteData(uint8_t num, uint16_t data)
{
    FLASH_STATE_e state = OK;
    HAL_StatusTypeDef statHal;
    uint32_t dataWrite;
    // ������ ���������� �������������� � ������ ����������
    dataWrite = 0xA5 + (num << 8);
    // ������ ������
    dataWrite += (data << 16);
    statHal = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash.adr, dataWrite);
    // HAL �� ������� (������ �� ������)(�� ������ ������ ������)
    if (statHal != HAL_OK)
    {
        flash.cntError++;
        state = ERR;
    }
    // �������������� �����
    if (flash.adr != MAX_ADR)
    {
        flash.adr += LEN_PAGE;
    }
    else
        flash.adr = START_ADR;

    return state;
}

/**
 * @brief ����� ������ ��������� �������� + ��������� ��� ��������
 *          ��������� ���������� � ��������� flash. ����� ���� � �����.
 *
 * @return FLASH_STATE_FIND_e
 */
FLASH_STATE_FIND_e xFindFreePage()
{
    uint32_t tempAdr;
    flash.adr = START_ADR;
    int maxfor = (MAX_ADR - START_ADR) / LEN_PAGE;
    for (int i = maxfor; i >= 0; i--)
    {
        tempAdr = START_ADR + (i * LEN_PAGE);
        //  ������� ������ ��������
        if (FlashRead32(tempAdr) != 0xFFFFFFFF)
        {
            flash.adr = tempAdr + LEN_PAGE;
            return FIND_PAGE_CLEAR;
        }
    }
    // ��� ������ ������
    return NOT_FIND_PAGE_CLEAR;
}

/**
 * @brief ����� � �������� ������� �� ����� � ������
 *              ��������� � flash.adrTemp
 *
 * @param num
 * @return FLASH_STATE_FIND_e
 */
FLASH_STATE_FIND_e xFindAdrValue(uint8_t num)
{
    FLASH_STATE_FIND_e state;
    uint8_t valKey, valNum;
    // ��������� �����
    if (flash.adrTemp != START_ADR)
    {
        flash.adrTemp -= LEN_PAGE;
    }
    else
    {
        // ������ �� �������� �� ����
        return NOT_FIND_ADR;
    }
    // ��������� ����
    valKey = FlashRead32(flash.adrTemp) & 0xFF;
    if (valKey == 0xA5)
    {
        // ��������� �����
        valNum = (FlashRead32(flash.adrTemp) >> 8) & 0xFF;
        if (valNum == num)
        {
            // �����
            return FIND_ADR;
        }
    }
    state = xFindAdrValue(num);
    return state;
}

/**
 * @brief ������� ��������� ������
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
        adr = START_ADR;
    else if (numSector == 1)
        adr = START_ADR + 0x400;
    else if (numSector == 2)
        adr = START_ADR + 0x800;
    else if (numSector == 3)
        adr = START_ADR + 0xC00;
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
