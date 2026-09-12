#ifndef AS5045_H
#define AS5045_H

#include <stdint.h>
#include <stdbool.h>

typedef union
{
    uint32_t raw;

    struct
    {
        uint8_t DC        : 4;
        uint16_t position : 12;
        bool OCF          : 1;
        bool COF          : 1;
        bool LIN          : 1;
        bool Mag_INC      : 1;
        bool Mag_DEC      : 1;
        bool Even_PAR     : 1;
        //uint16_t Parity   : 13;
    } bits;

} _AS5045;



void AS5045_ENC_Initialize();
bool AS5045_ENC_Read(float *angle);
bool AS5045_ENC_Request();

#endif