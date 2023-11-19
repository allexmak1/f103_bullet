#ifndef LOGIC_H
#define LOGIC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32f1xx_hal.h"

#define DELAY_START 4500 // ms

// скорость мигания
#define PWM_LED_PRESCALER 99
#define LED_SPEED_NONE 0
#define LED_SPEED_LOW 22
#define LED_SPEED_MEDIUM 10
#define LED_SPEED_HIGHT 2
#define LED_SPEED_ALWAYS 1


    typedef struct
    {
        int ms1; // таймер шим для светодиода
        int ms2; // таймер для опроса
        int ms3; // для go
        int ms4; // для двоеточия
        int ms5; // для долгово нажатия
        int htim1;
        int ticBusyCmd;
    } tim_t;

    typedef struct
    {
        short go;     // флаг старта
        short rezhim; // режим работы

        short timeMin;  // счетчик минут
        short timeSec;  // счетчик секунд
        short numTreak; // номер трека
        short battery;  // текущий заряд

        short isSec;    // мигалка двоеточия
        short Busy;     // флаг занятости по не отжатию
        short BusyTime; // флаг зантости по времени
        short stepGo;   // шаги в режиме go
        short step;     // шаг для каждого режима
        int ledSpeed;   // скорость изменения мигания светодиода
        int ledValue;   // значение шим
        short ledIs;    // внутренний флаг для переключения направления мигания
        short mButtonRezim;
    } v_t;

    void init();
    void loop();
    short maxMin(short var, short min, short max);
    void vSetPwmLed(int value);
    void vLedProcess(int time);
    void vIsM();
    void vIsMin();
    void vIsPlu();
    void vIsGo();
    void vNomberTrack(int next);

#ifdef __cplusplus
}
#endif

#endif
