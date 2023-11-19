/**
 * @file flash.c
 * @author Makarov
 * @brief
 * @version 0.2
 * @date 2022-01-19
 *
 * @copyright Copyright (c) 2023
 *
 * логика работы:   Стартовая функция определяет первый свободный адрес,
 *                  чтение предидущего адреса в переменные или чтение дефолтовых значений.
 *                  Чтение происходит от flash.adr в обратную сторону на шаг страницы
 *                  Запись происходит в flash.adr с последующей её итерацией (запись идет сразу всех 4х переменных).
 *
 * сетка памяти:
 *  0x08007000 - 73FF sector 1 1Kbyte
 *  0x08007400 - 77FF sector 2
 *  0x08007800 - 7BFF sector 3
 *  0x08007С00 - 7FFF sector 4 последний
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

#define MAX_SECTOR 4 // кол-во секторов
#define MAX_PAGE 64  // кол-во страниц в секторе(при затирки стираем сектр)(0-63)
#define LEN_PAGE 4   // переменных (uint32_t) в странице; (4 x uint32_t)(0-3)

flash_t flash;

//  тестовые переменные
FLASH_STATE_e stateFlashTest;
#define MASS_TEST 40

uint32_t dataWrite[MASS_TEST];
uint32_t dataReadTemp[4];
uint32_t cntErrors = 0;

//----------------------------------------------------------------
// Основные функции
//----------------------------------------------------------------

/**
 * @brief тест стартового чтения, записи и чтения
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

    // старт
    // запоминаем первую пустую страницу, выводим дефолтовые значения
    stateFlashTest = xStartFlash();

    for (int i = 0; i < MASS_TEST; i++)
    {
        // чтение
        // 4 переменных (uint32_t) в странице
        dataReadTemp[0] = xReadFlash(0);
        dataReadTemp[1] = xReadFlash(1);
        dataReadTemp[2] = xReadFlash(2);
        dataReadTemp[3] = xReadFlash(3);
        // запись
        // записываем сразу всю страницу
        dataWrite[0] = i;
        dataWrite[1] = i;
        dataWrite[2] = i;
        dataWrite[3] = i;
        stateFlashTest = xWriteFlash(&dataWrite[0]);
        HAL_Delay(20);
        // сравнение
        if (xReadFlash(0) != i)
        {
            cntErrors++;
        }
    }
    // затирка памяти и применение дефолтовых значений
    // vResetSettings();
}

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

    // если память пустая то берем значения по умолчанию
    if (flash.adr == PAGE_0_START)
    {
        DEFOULT_VAR_1 = DEFOULT_VAL_1;
        DEFOULT_VAR_2 = DEFOULT_VAL_2;
        DEFOULT_VAR_3 = DEFOULT_VAL_3;
        flash.isDefoltSettings = 1;
        return OK_DEFOULT;
    }
    // если вся память занята, то возможно это последняя ячейка с данными
    if (stateFind == NOT_FIND_PAGE_CLEAR)
    {
        flash.adr = PAGE_0_START;
    }
    // считываем данные
    DEFOULT_VAR_1 = xReadFlash(0);
    DEFOULT_VAR_2 = xReadFlash(1);
    DEFOULT_VAR_3 = xReadFlash(2);
    return OK;
}

/**
 * @brief Чтение из флеш.
 * Чтение выбранной ячейки в странице(0-3).
 *
 * @param num  номер в странице (0-3)
 * @return uint32_t - данные
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
 * @brief Процедура записи.
 * Запись проходит сразу всей страницы
 *
 * @param data указатель, 4 шт по 32 бита
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
    // процедура записи
    for (int i = 0; i < LEN_PAGE; i++)
    {
        stat = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash.adr + (i * 4), data[i]);
        // HAL не записал (ячейка не пустая)(не всегда выдает ошибку)
        if (stat != HAL_OK)
        {
            flash.cntError++;
            cntErrWrite++;
            break;
        }
    }
    HAL_FLASH_Lock();
    // инкриментируем адрес на другую страницу
    flash.adr += 0x10;
    if (flash.adr >= (PAGE_3_END + 0x10))
        flash.adr = PAGE_0_START;

    // проверяем что записали
    if (xReadFlash(0) != data[0])
    {
        flash.cntError++;
        cntErrWrite++;
    }
    // если прочисал все сектора, так и не смог записать
    if (flash.cntError >= (MAX_SECTOR * MAX_PAGE))
    {
        flash.cntError = 0;
        return ERR;
    }

    // рекурсия
    if (cntErrWrite > 0)
    {
        cntErrWrite = 0;
        xWriteFlash(data);
    }
    flash.cntError = 0;
    return OK;
}

/**
 * @brief Сброс всех параметров возвращение к дефолтовым,
 *          при этом стираем память. Если первая чистая страница не первая то выводим ошибку.
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
// Внутренние функции
//----------------------------------------------------------------

/**
 * @brief поиск первой свободной страницы + проверяет всю страницу
 *          результат записывает в структуру flash.
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
        // смотрим внутри страницы
        isFindSector = 0;
        for (int i = 0; i < LEN_PAGE; i++)
        {
            if (FlashRead32(flash.adr + (i * 4)) == 0xFFFFFFFF)
            {
                isFindSector++;
            }
        }
        if (isFindSector == 4)
        { // вся страница чистая
            return FIND_PAGE_CLEAR;
            // здесь бы доделать проверку остальной части сектора
        }
    }
    // вся память занята
    return NOT_FIND_PAGE_CLEAR;
}

/**
 * @brief стирает выбранный сектр
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
 * @brief чтение 32 бит
 *
 * @param address  (0x08007000)
 * @return uint32_t - данные
 */
uint32_t FlashRead32(uint32_t address)
{
    return (*(__IO uint32_t *)address);
}
