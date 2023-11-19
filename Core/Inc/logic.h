#ifndef LOGIC_H
#define LOGIC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32f1xx_hal.h"

#define DELAY_START 4500 // ms

// �������� �������
#define PWM_LED_PRESCALER 99
#define LED_SPEED_NONE 0
#define LED_SPEED_LOW 22
#define LED_SPEED_MEDIUM 10
#define LED_SPEED_HIGHT 2
#define LED_SPEED_ALWAYS 1


    typedef struct
    {
        int ms1; // ������ ��� ��� ����������
        int ms2; // ������ ��� ������
        int ms3; // ��� go
        int ms4; // ��� ���������
        int ms5; // ��� ������� �������
        int htim1;
        int ticBusyCmd;
    } tim_t;

    typedef struct
    {
        short go;     // ���� ������
        short rezhim; // ����� ������

        short timeMin;  // ������� �����
        short timeSec;  // ������� ������
        short numTreak; // ����� �����
        short battery;  // ������� �����

        short isSec;    // ������� ���������
        short Busy;     // ���� ��������� �� �� �������
        short BusyTime; // ���� �������� �� �������
        short stepGo;   // ���� � ������ go
        short step;     // ��� ��� ������� ������
        int ledSpeed;   // �������� ��������� ������� ����������
        int ledValue;   // �������� ���
        short ledIs;    // ���������� ���� ��� ������������ ����������� �������
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
