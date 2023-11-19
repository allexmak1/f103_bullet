/**
 * @file flash.c
 * @author Makarov
 * @brief
 * @version 0.3
 * @date 2022-01-19
 *
 * @copyright Copyright (c) 2023
 *
 * логика работы:   Стартовая функция определяет первый свободный адрес (поиск с конца),
 *                  чтение предидущего адреса в переменные или чтение дефолтовых значений.
 *                  Чтение происходит от flash.adr в обратную сторону на шаг страницы, проверяет "A5" и смотрит номер ячейки (0-255)
 *                  Запись происходит в flash.adr с последующей её итерацией.
 *
 * сетка памяти:
 *  смотреть exel
 *
 */

#include "flash_write_on_id.h"
#include "logic.h"

extern v_t v;

#define DEFOULT_VAR_MAX 1
#define DEFOULT_VAR_1 v.numTreak

// значения данных в функции xStartFlash()

#define LEN_PAGE 4 // длина  ячейки памяти для записи (в байтах) (A5 00 FF FF)

#define START_ADR 0x08007000 // стартовый адрес
#define MAX_ADR 0x08007FFC   // максимальный адрес последней ячейки записи в памяти

flash_t flash;
uint16_t defoultData[DEFOULT_VAR_MAX];



/**
 * @brief Поиск первой не занятой страницы и сектора.
 *          Если память пустая то берем значения по умолчанию,
 *          иначе читаем данные.
 *
 * @return FLASH_STATE_e
 */
FLASH_STATE_e xStartFlash()
{
    FLASH_STATE_FIND_e stateFind;
    stateFind = xFindFreePage();

    // данные по дефолту
    defoultData[0] = 1; // 1


    // если память пустая то берем значения по умолчанию
    if (flash.adr == START_ADR)
    {
        DEFOULT_VAR_1 = defoultData[0];

        flash.isNotClearBank = 1;
        return OK_DEFOULT;
    }
    // если вся память занята, то возможно это последняя ячейка с данными
    if (stateFind == NOT_FIND_PAGE_CLEAR)
    {
        flash.adr = START_ADR;
    }
    // считываем данные
    DEFOULT_VAR_1 = xReadFlash(0);

    return OK;
}

/**
 * @brief Чтение из флеш.
 * Чтение выбранной ячейки в странице.
 *
 * @param num  номер в странице (0-3)
 * @return uint32_t - данные
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
        // сюда если все правельно то не зайдем
        // значения по умолчанию
        if (numData >= DEFOULT_VAR_MAX)
            return val;
        val = defoultData[numData];
    }
    return val;
}

/**
 * @brief процедура записи:
 *          записываем подряд кратно определенной длинне, по схеме:
 *          8 бит - А5 (фиксированный индетификатор)
 *          8 бит - 0xFF (номер переменной)(0-255)
 *          16 бит - 0xFFFF (данные)(0-65535)
 * @param num номер записи (0-255)
 * @param data данные 16 бит
 * @return FLASH_STATE_e
 */
FLASH_STATE_e xWriteFlash(uint8_t num, uint16_t data)
{
    FLASH_STATE_e state = OK;
    uint8_t tempData[DEFOULT_VAR_MAX];
    HAL_FLASH_Unlock();
    // затирка памяти если находимся на начальном адресе
    if (flash.adr == START_ADR)
    {
        if (flash.isNotClearBank == 0)
        {
            // запоминаем данные
            flash.adr = MAX_ADR + LEN_PAGE;
            for (uint8_t i = 0; i < DEFOULT_VAR_MAX; i++)
            {
                tempData[i] = xReadFlash(i);
            }
            vClearSector(0);
            vClearSector(1);
            vClearSector(2);
            vClearSector(3);
            // записываем
            flash.adr = START_ADR;
            for (uint8_t i = 0; i < DEFOULT_VAR_MAX; i++)
            {
                xWriteData(i, tempData[i]);
            }
        }
        else
        {
            // данные были записанны по умолчанию
            flash.isNotClearBank = 0;
        }
    }
    state = xWriteData(num, data);
    HAL_FLASH_Lock();
    return state;
}

/**
 * @brief сброс всех параметров возвращение к дефолтовым
 *          при этом стираем память
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
    // запись стартового идентификатора и номера переменной
    dataWrite = 0xA5 + (num << 8);
    // запись данных
    dataWrite += (data << 16);
    statHal = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash.adr, dataWrite);
    // HAL не записал (ячейка не пустая)(не всегда выдает ошибку)
    if (statHal != HAL_OK)
    {
        flash.cntError++;
        state = ERR;
    }
    // инкрементируем адрес
    if (flash.adr != MAX_ADR)
    {
        flash.adr += LEN_PAGE;
    }
    else
        flash.adr = START_ADR;

    return state;
}

/**
 * @brief поиск первой свободной страницы + проверяет всю страницу
 *          результат записывает в структуру flash. Поиск идет с конца.
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
        //  смотрим внутри страницы
        if (FlashRead32(tempAdr) != 0xFFFFFFFF)
        {
            flash.adr = tempAdr + LEN_PAGE;
            return FIND_PAGE_CLEAR;
        }
    }
    // вся память занята
    return NOT_FIND_PAGE_CLEAR;
}

/**
 * @brief поиск в обратную сторону по метке и номеру
 *              результат в flash.adrTemp
 *
 * @param num
 * @return FLASH_STATE_FIND_e
 */
FLASH_STATE_FIND_e xFindAdrValue(uint8_t num)
{
    FLASH_STATE_FIND_e state;
    uint8_t valKey, valNum;
    // вернулись назад
    if (flash.adrTemp != START_ADR)
    {
        flash.adrTemp -= LEN_PAGE;
    }
    else
    {
        // ошибка не найденно ни чего
        return NOT_FIND_ADR;
    }
    // проверили ключ
    valKey = FlashRead32(flash.adrTemp) & 0xFF;
    if (valKey == 0xA5)
    {
        // проверили номер
        valNum = (FlashRead32(flash.adrTemp) >> 8) & 0xFF;
        if (valNum == num)
        {
            // нашли
            return FIND_ADR;
        }
    }
    state = xFindAdrValue(num);
    return state;
}

/**
 * @brief стирает выбранный сектор
 *
 * @param numSector номер сектора (0-3)
 */
void vClearSector(int numSector)
{
    int adr = vGetNumSector(numSector);
    FLASH_EraseInitTypeDef ef;            // Объявляю структуру, необходимую для функции стирания страницы
    ef.TypeErase = FLASH_TYPEERASE_PAGES; // Стирать постранично
    ef.PageAddress = adr;                 // Адрес страницы для стирания
    ef.NbPages = 1;                       // Число страниц = 1
    uint32_t temp;                        // Временная переменная для результата стирания (не использую)
    HAL_FLASHEx_Erase(&ef, &temp);        // Вызов функции стирания
}

/**
 * @brief
 *
 * @param numSector  номер сектора (0-3)
 * @return uint32_t - адрес первой страице в секторе
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
 * @brief чтение 32 бит
 *
 * @param address  (0x08007000)
 * @return uint32_t - данные
 */
uint32_t FlashRead32(uint32_t address)
{
    return (*(__IO uint32_t *)address);
}
