#include "FOC_PID.h"
#include <stdlib.h>

#define _constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))


FOC_PID* FOC_PID_create (float P, float I, float D, float limit, float output_ramp)
{
    FOC_PID* PID = (FOC_PID*) calloc(1,sizeof(FOC_PID));

    PID->P = P;
    PID->I = I;
    PID->D = D;
    PID->limit = limit;
    PID->output_ramp = output_ramp;

    PID->error_prev = 0;
    PID->output_prev = 0;
    PID->timestamp_prev = 0;

    return PID;
}

float FOC_PID_calculate (FOC_PID* PID, float error)
{
    float proportional = PID->P * error;

    // Tustin transform of the integral part
    // u_ik = u_ik_1  + I*Ts/2*(ek + ek_1)
    float integral = PID->integral_prev + PID->I * PID_dt * (error + PID->error_prev);
    integral = _constrain(integral, -(PID->limit), PID->limit);

    float derivative = PID->D * (error - PID->error_prev)/PID_dt;

    float output = proportional + integral + derivative;
    output = _constrain(output, -(PID->limit), PID->limit);

    if (PID->output_ramp > 0)
    {
        float output_rate = (output - PID->output_prev)/PID_dt;
        if (output_rate > PID->output_ramp)
        {
            output = PID->output_prev + PID->output_ramp*PID_dt;
        }
        else if (output_rate < PID->output_ramp)
        {
            output = PID->output_prev - PID->output_ramp*PID_dt;
        }
    }

    PID->integral_prev = integral;
    PID->output_prev = output;
    PID->error_prev = error;

    return output;
}

void FOC_PID_reset(FOC_PID* PID)
{
    PID->integral_prev = 0;
    PID->output_prev = 0;
    PID->error_prev = 0;
}