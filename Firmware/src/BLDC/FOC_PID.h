#ifndef FOC_PID_H
#define FOC_PID_H

typedef struct
{
    float P,I,D;
    float output_ramp;  // Maximum speed of change of the output value
    float limit; 
    float error_prev;
    float output_prev;
    unsigned long timestamp_prev;
    float integral_prev;
} FOC_PID;

#define PID_dt (float)(0.001/2)

FOC_PID* FOC_PID_create (float P, float I, float D, float limit, float output_ramp);
float FOC_PID_calculate (FOC_PID* PID, float error);
void FOC_PID_reset(FOC_PID* PID);

#endif