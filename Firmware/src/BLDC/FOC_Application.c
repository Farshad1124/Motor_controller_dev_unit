#include "definitions.h"
#include "FOC_PWM.h"
#include "FOC_ADC.h"
#include "FOC_PID.h"
#include "FOC_Application.h"
#include "./RTT/SEGGER_RTT.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include "queue.h"
#include "../BW_filter/BW_filter.h"
#include <math.h>

#define ONE_THIRD (1.0f/3.0f)
#define ONE_SQRT_THREE 0.57735f
#define _SQRT3 1.73205080757f
#define _SQRT3_2 0.86602540378f
#define _RPM_TO_RADS 0.10471975512f
#define _constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define _sign(a) ( ( (a) < 0 )  ?  -1   : ( (a) > 0 ) )
#define _2PI 6.28318530718f

typedef enum 
{
    FOC_idle,
    ADC_Offset_Calibration_setup,
    ADC_Offset_Calibration_run,
    Sensor_alignment,
    Angle_update,
    Move_update,
    FOC_ShutDown,
} FOC_stages;

typedef struct
{
    float iq;
    float id;
} DQCurrent_s;

typedef struct
{
    float i_alpha;
    float i_beta;
} ABCurrent_s;


//-------------------------------------------------------------------------------
//                      local parameter lists
//
// List of parameters that have to be reset after reset:
// Sensor_aligned, OutputEn, FOC_sequence, PID_current_q, PID_currennt_d, PID_velocity
// P_angle, LPF_velocity, LPF_angle
//-------------------------------------------------------------------------------
uint8_t FOC_sequence = FOC_idle;
volatile bool OutputEn = false;
bool Sensor_aligned = false;
float calibration_voltage = 0;
static FOC_control_config control = {0};
static FOC_motor_config motor = {0};

QueueHandle_t FOC_Queue;

FOC_PID* PID_current_q; // parameter determining the q current PID config
FOC_PID* PID_current_d; // parameter determining the d current PID config;
FOC_PID* PID_velocity; // parameter determining the velocity PID configuration
FOC_PID* P_angle;

BWLowPass* LPF_velocity;
BWLowPass* LPF_angle;
BWLowPass* LPF_current_q;
BWLowPass* LPF_current_d;

float shaft_angle = 0;
float shaft_velocity = 0;
//---------------------------------------------------------------------------
//                              Prototype
//---------------------------------------------------------------------------
static void updateVelocityLimit(void);
static void updateCurrentLimit(void);
static void updateVoltageLimit(void);
void FOC_Task(void * pvParameters );
static bool FOC_driver_align(void);
static void current_averaging (float *Ac, float *Bc, float *Cc);
static ABCurrent_s Clark_transform(float a, float b, float c);
static DQCurrent_s Clark_Park_transform (float angle);
static DQCurrent_s Park_transform(ABCurrent_s clark, float angle);
static float estimateBEMF (float vel);
static float angleOpenloop(float target);
static float normalizeAngle(float angle);
static float velocityOpenloop(float target);
//-----------------------------------------------------------------------------------
//                              Public functions
//-----------------------------------------------------------------------------------

void FOC_VelocityLimit_update(float velocity)
{
    FOC_message Mes = {0};
    Mes.type = Change_velocity_limit;
    Mes.value = velocity;
    xQueueSend(FOC_Queue, (void *)&Mes, (TickType_t) 0);
}

void FOC_CurrentLimit_update(float current)
{
    FOC_message Mes = {0};
    Mes.type = Change_current_limit;
    Mes.value = current;
    xQueueSend(FOC_Queue, (void *)&Mes, (TickType_t) 0);
}

void FOC_VoltageLimit_update(float voltage)
{
    FOC_message Mes = {0};
    Mes.type = Change_voltage_limit;
    Mes.value = voltage;
    xQueueSend(FOC_Queue, (void *)&Mes, (TickType_t) 0);
}

void FOC_Move(float target)
{
    FOC_message Mes = {0};
    Mes.type = Change_target;
    Mes.value = target;
    xQueueSend(FOC_Queue, (void *)&Mes, (TickType_t) 0);
}


//---------------------------------------------------------------------------
//                               Private functions
//---------------------------------------------------------------------------
// normalizing radian angle to [0,2PI]
float normalizeAngle(float angle)
{
  float a = fmod(angle, _2PI);
  return a >= 0 ? a : (a + _2PI);
}

void OUTPUT_SEL_EIC_Handler(uintptr_t context)
{
    if (FOC_sequence == FOC_idle)
    {
        FOC_sequence = ADC_Offset_Calibration_setup;
    }
    else if (FOC_sequence != FOC_idle)
    {
        FOC_sequence = FOC_ShutDown;
    }
}

// Update limit values in controllers when changed
void updateVelocityLimit(void) 
{
  if(control.motion_mode != position_nocascade) 
    P_angle->limit = fabs(control.velocity_limit); // if angle control but no velocity cascade, limit the angle controller by the velocity limit
}

// Update limit values in controllers when changed
void updateCurrentLimit(void) 
{
  if(control.torque_mode != Torque_via_voltage) {
    // if current control
    PID_velocity->limit = control.current_limit;
    if(control.motion_mode == position_nocascade) 
      // if angle control but no velocity cascade, limit the angle controller by the current limit
      P_angle->limit = control.current_limit;
  }
}

// Update limit values in controllers when changed
// PID values and limits
void updateVoltageLimit(void) 
{
  PID_current_q->limit = control.voltage_limit;
  PID_current_d->limit = control.voltage_limit;
  if(control.torque_mode == Torque_via_voltage) {
    // if voltage control
    PID_velocity->limit = control.voltage_limit;
    if(control.motion_mode == position_nocascade) 
      // if angle control but no velocity cascade, limit the angle controller by the voltage limit
      P_angle->limit = control.voltage_limit;
  }
}

// Function (iterative) generating open loop movement towards the target angle
// - target_angle - rad
// it uses voltage_limit and velocity_limit variables
float angleOpenloop(float target)
{
    // calculate the necessary angle to move from current position towards target angle
    // with maximal velocity (velocity_limit)
    // TODO sensor precision: this calculation is not numerically precise. The angle can grow to the point
    //                        where small position changes are no longer captured by the precision of floats
    //                        when the total position is large.
    if(fabs( target - shaft_angle ) > fabs(control.velocity_limit*0.001) && fabs(control.velocity_limit) > 0){
      shaft_angle += _sign(target - shaft_angle) * fabs(control.velocity_limit)*0.001;
      shaft_velocity = control.velocity_limit;
    }else{
      shaft_angle = target;
      shaft_velocity = 0;
    }

    if (control.torque_mode == Torque_via_voltage)
        return control.voltage_limit;
    else
        return control.current_limit;
}

// Function (iterative) generating open loop movement for target velocity
// - target_velocity - rad/s
// it uses voltage_limit variable
float velocityOpenloop(float target)
{
  // calculate the necessary angle to achieve target velocity
  shaft_angle = normalizeAngle(shaft_angle + target*0.001);
  // for display purposes
  shaft_velocity = target;

  if (control.torque_mode == Torque_via_voltage)
    return control.voltage_limit;
  else
    return control.voltage_limit;
}

//---------------------------------------------------------------------------
//                          Private task
//---------------------------------------------------------------------------
void FOC_Application_Initialize(FOC_control_config* c, FOC_motor_config* m)
{
    memcpy((void*)&control,   (void*)c,   sizeof(FOC_control_config));
    memcpy((void*)&motor,   (void*)m,     sizeof(FOC_motor_config));

    //LPF_velocity = create_bw_low_pass_filter(2,(float)1000,DEF_VELOCITY_CUTOFF);
    //LPF_angle = create_bw_low_pass_filter(2,(float)1000,DEF_VELOCITY_CUTOFF);
    //LPF_angle = create_bw_low_pass_filter(2,(float)1000,DEF_VELOCITY_CUTOFF);
    LPF_current_q = create_bw_low_pass_filter(2,(float)1000,DEF_VELOCITY_CUTOFF);
    LPF_current_d = create_bw_low_pass_filter(2,(float)1000,DEF_VELOCITY_CUTOFF);

    FOC_PWM_Initialize(control.voltage_limit,control.source_voltage,control.pwm_frequency);
    FOC_ADC_Initialize(control.ADC_gain,control.pwm_frequency);

    if (control.current_P == -1)
    {
        control.current_P = DEF_PID_CURR_P;
    }

    if (control.current_I == -1)
    {
        control.current_I = DEF_PID_CURR_I;
    } 

    if (control.current_D == -1)
    {
        control.current_D = DEF_PID_CURR_D;
    } 

    // configure PID
    PID_current_q = FOC_PID_create(control.current_P,control.current_I,control.current_D,DEF_POWER_SUPPLY,DEF_PID_CURR_RAMP);
    PID_current_d = FOC_PID_create(control.current_P,control.current_I,control.current_D,DEF_POWER_SUPPLY,DEF_PID_CURR_RAMP);
    PID_velocity = FOC_PID_create(DEF_PID_VEL_P,DEF_PID_VEL_I,DEF_PID_VEL_D,DEF_PID_VEL_LIMIT,DEF_PID_VEL_RAMP);
    P_angle = FOC_PID_create(DEF_P_ANGLE_P,0,0,DEF_VEL_LIM,0);

    BLDC_EN_Toggle();
    
    xTaskCreate( FOC_Task, "FOC Application", 2048, NULL, 1, NULL );
    FOC_Queue = xQueueCreate( 5, sizeof(FOC_message) );
    EIC_CallbackRegister(EIC_PIN_8,OUTPUT_SEL_EIC_Handler, 0);
    calibration_voltage = control.voltage_limit/2;

    updateCurrentLimit();
    updateVoltageLimit();
    updateVelocityLimit();

    if (control.motion_mode == position_openloop_control 
        || control.motion_mode == velocity_openloop_control)
    {
        control.sensor_direction = CW;
    }
}


void FOC_Task(void * pvParameters )
{
    static FOC_message message = {0};

    float electrical_angle = 0;
    
    float current_sp = 0;
    float target = 0;
    DQCurrent_s feed_forward_current = {0};
    DQCurrent_s feed_forward_voltage = {0};
    DQCurrent_s current = {0};
    DQCurrent_s voltage = {0};
    
    float voltage_bemf = 0;


    for(;;)
    {
        // Queue checking for state and settings change
        if( xQueueReceive(FOC_Queue, &(message), ( TickType_t ) 0 ) == pdPASS )
        {
            switch (message.type) 
            {
                case Change_current_limit:
                    control.current_limit = message.value;
                    updateCurrentLimit();
                break;

                case Change_velocity_limit:
                    control.velocity_limit = message.value;
                    updateVelocityLimit();
                break;

                case Change_voltage_limit:
                    control.voltage_limit = message.value;
                    updateVoltageLimit();
                break;

                case Change_target:
                    target = message.value;
                break;

                default:
                break;
            }
        }

        // initialization sequence -----------------------------------------------------------
        switch (FOC_sequence)
        {
            case FOC_idle: // do nothing wait for button interrupt trigger
            break;

            case ADC_Offset_Calibration_setup: // Prepare the PWM for ADC calibration
                TCC0_PWMStart();
                FOC_PWM_Set(calibration_voltage,calibration_voltage,calibration_voltage);

                if (ADC_Calibration_check() == false)
                {
                    SEGGER_RTT_WriteString(0, "[FOC] [ADC] Starting calibration\n");
                    FOC_sequence = ADC_Offset_Calibration_run;
                }
                else
                {
                    SEGGER_RTT_WriteString(0, "[FOC] Starting temp\n");
                    FOC_sequence = Sensor_alignment;
                }

            break;

            case ADC_Offset_Calibration_run: // execute the ADC calbration for one round at a time
                if (FOC_ADC_Calibration())
                {
                    SEGGER_RTT_WriteString(0, "[FOC] [ADC] Calibration done\n");
                    FOC_PWM_Set(0,0,0);
                    //FOC_sequence = Sensor_alignment;
                    FOC_sequence = Angle_update;
                }
            break;

            case Sensor_alignment: // perform sensor alignment, skip if already aligned
                if ((control.motion_mode == position_openloop_control 
                    || control.motion_mode == velocity_openloop_control)
                    && (Sensor_aligned == false))
                {
                    if (FOC_driver_align() == false)
                    {
                        FOC_sequence = FOC_ShutDown;
                    }
                    else
                    {
                        Sensor_aligned = true;
                        FOC_sequence = Angle_update;
                    }
                }
            break;

            case FOC_ShutDown:
                SEGGER_RTT_WriteString(0, "[FOC] Shutting down\n");
                FOC_PWM_Set(0,0,0);
                TCC0_PWMStop();
                FOC_sequence = FOC_idle;
            break;

            default:
            break;
        }

        
        if (active_state_check() == false)
        {
            goto SKIP_MOVE;
        }

        // ===================================================================================
        // Sensor angle update ---------------------------------------------------------------
        // ===================================================================================
        switch(control.motion_mode)
        {
            case position_openloop_control: // angle control in open loop
                current_sp = angleOpenloop(target); 
            break;
            case velocity_openloop_control:
                // this function updates the shaft_angle and shaft_velocity
                // returns the voltage or current that is to be set to the motor (depending on torque control mode)
                // returned values correspond to the voltage_limit and current_limit
                current_sp = velocityOpenloop(target); 
            break;
        
            default:
            break;
        }

        // ==================================================================================
        // Motion mode calculation ----------------------------------------------------------
        // ==================================================================================
        switch (control.motion_mode)
        {
            case position_openloop_control: // calculate the open loop electirical angle
            case velocity_openloop_control:
                
                electrical_angle = shaft_angle *  motor.pole_pair_count;
            break;

            default:    // TODO: add other sensor measurment method
            break;
        }

        // ===================================================================================
        // Torque selection sequence ---------------------------------------------------------
        // ====================================================================================
        switch (control.torque_mode)
        {
            case Torque_via_idq:
                // constrain current setpoint
                current_sp = _constrain(current_sp, -control.current_limit, control.current_limit) + feed_forward_current.iq;
                // read dq currents
                current = Clark_Park_transform(electrical_angle);
                // filter values
                //current.iq = LPF_current_q(current.iq);
                current.iq = bw_low_pass(LPF_current_q,current.iq);
                current.id = bw_low_pass(LPF_current_d,current.id);

                // calculate the phase voltages
                voltage.iq = FOC_PID_calculate(PID_current_q, current_sp - current.iq);
                voltage.id = FOC_PID_calculate(PID_current_d, feed_forward_current.id - current.id);

                // d voltage decoupling
                voltage.id = _constrain( voltage.id - current_sp*shaft_velocity*motor.pole_pair_count*motor.inductance, -control.voltage_limit, control.voltage_limit);

                // q voltage decoupling
                voltage.iq = _constrain( voltage.iq - current.id*shaft_velocity*motor.pole_pair_count*motor.inductance, -control.voltage_limit, control.voltage_limit);

                // add feed forward
                voltage.iq += feed_forward_voltage.iq;
                voltage.id += feed_forward_voltage.id;
            break;

            // TODO: add voltage and estimate_current torque modes

            default:
            break;
        }

        // ===================================================================================
        // Motor modulation sequence ---------------------------------------------------------
        // ===================================================================================
        switch (control.modulation)
        {
            case SinePWM:
            case SpaceVectorPWM:
                // Inverse Park + Clarke transformation
                float ca = cos(electrical_angle);
                float sa = sin(electrical_angle);
                float Ualpha = ca * voltage.id - sa * voltage.iq;  // -sin(angle) * Uq;
                float Ubeta = sa * voltage.id + ca * voltage.iq;   //  cos(angle) * Uq;

                // Clarke transform
                float Ua = Ualpha;
                float Ub = -0.5f * Ualpha + _SQRT3_2 * Ubeta;
                float Uc = -0.5f * Ualpha - _SQRT3_2 * Ubeta;

                // centre moduclation
                float center = control.voltage_limit/2;
                Ua += center;
                Ub += center;
                Uc += center;

                FOC_PWM_Set(Ua,Ub,Uc);
            break;

            default:
            break;
        }


SKIP_MOVE:
        vTaskDelay(1);
    }
}


bool FOC_driver_align(void)
{
    char rtt_buffer_current[100]; 
    float zero = control.voltage_limit/2;
    float Ac, Bc, Cc = 0;

    // -----------set phase A active and phases B and C down
    // 300 ms of ramping
    SEGGER_RTT_WriteString(0, "[FOC] [Align] Powering Phase A\n");
    for(int i=0; i < 100; i++){
        FOC_PWM_Set(DEF_VOLTAGE_SENSOR_ALIGN/100.0f*((float)i)+zero,zero,zero);
        vTaskDelay(3);
    }
    vTaskDelay(500);

    // get the average phase current
    current_averaging(&Ac,&Bc,&Cc);
    FOC_PWM_Set(zero,zero,zero);
    sprintf(rtt_buffer_current,"[FOC] [Align] IA=%.3f, IB=%.3f, IC=%.3f\n",Ac,Bc,Cc);
    SEGGER_RTT_WriteString(0, rtt_buffer_current);

    if((fabs(Ac) < 0.1f) && (fabs(Bc) < 0.1f) && (fabs(Cc) < 0.1f)){
        SEGGER_RTT_WriteString(0,"[FOC] [Align] [Error] too low current, rise voltage!");
        return false; // measurement current too low
    }

    if (Ac < 0)
    {
        FOC_ADC_set_phaseA_gain(-1);
    }

    // -------------set phase B active and phases A and C down
    // 300 ms of ramping
    SEGGER_RTT_WriteString(0, "[FOC] [Align] Powering Phase B\n");
    for(int i=0; i < 100; i++){
        FOC_PWM_Set(zero, DEF_VOLTAGE_SENSOR_ALIGN/100.0f*((float)i)+zero,zero);
        vTaskDelay(3);
    }
    vTaskDelay(500);

    // get the average phase current
    current_averaging(&Ac,&Bc,&Cc);
    FOC_PWM_Set(zero,zero,zero);
    sprintf(rtt_buffer_current,"[FOC] [Align] IA=%.3f, IB=%.3f, IC=%.3f\n",Ac,Bc,Cc);
    SEGGER_RTT_WriteString(0, rtt_buffer_current);

    if((fabs(Ac) < 0.1f) && (fabs(Bc) < 0.1f) && (fabs(Cc) < 0.1f)){
        SEGGER_RTT_WriteString(0,"[FOC] [Align] [Error] too low current, rise voltage!");
        return false; // measurement current too low
    }

    if (Bc < 0)
    {
        FOC_ADC_set_phaseB_gain(-1);
    }

    // --------------set phase C active and phases A and B down
    // 300 ms of ramping
    SEGGER_RTT_WriteString(0, "[FOC] [Align] Powering Phase C\n");
    for(int i=0; i < 100; i++){
        FOC_PWM_Set(zero, zero,DEF_VOLTAGE_SENSOR_ALIGN/100.0f*((float)i)+zero);
        vTaskDelay(3);
    }
    vTaskDelay(500);

    // get the average phase current
    current_averaging(&Ac,&Bc,&Cc);
    FOC_PWM_Set(zero,zero,zero);
    sprintf(rtt_buffer_current,"[FOC] [Align] IA=%.3f, IB=%.3f, IC=%.3f\n",Ac,Bc,Cc);
    SEGGER_RTT_WriteString(0, rtt_buffer_current);

    if((fabs(Ac) < 0.1f) && (fabs(Bc) < 0.1f) && (fabs(Cc) < 0.1f)){
        SEGGER_RTT_WriteString(0,"[FOC] [Align] [Error] too low current, rise voltage!");
        return false; // measurement current too low
    }

    if (Cc < 0)
    {
        FOC_ADC_set_phaseC_gain(-1);
    }

    return true;
}

void current_averaging (float *Ac, float *Bc, float *Cc)
{
    FOC_ADC_Get_Current(Ac, Bc, Cc);
    for (int i = 0; i<100; i++)
    {
        float Ac1, Bc1, Cc1 = 0;
        FOC_ADC_Get_Current(&Ac1, &Bc1, &Cc1);
        *Ac = *Ac * 0.6f + 0.4f * Ac1;
        *Bc = *Bc * 0.6f + 0.4f * Bc1;
        *Cc = *Cc * 0.6f + 0.4f * Cc1;
        vTaskDelay(3);
    }
}

DQCurrent_s Clark_Park_transform (float angle)
{
    float A, B, C = 0;
    FOC_ADC_Get_Current(&A, &B, &C);

    // clark transform
    ABCurrent_s ABcurrent = Clark_transform(A,B,C);

    // park transform
    DQCurrent_s IQcurrent = Park_transform(ABcurrent, angle);

}

ABCurrent_s Clark_transform(float a, float b, float c)
{
    float ioffset = ONE_THIRD * (a+b+c);
    ABCurrent_s clark = {0};
    float _a = a - ioffset;
    float _b = b - ioffset;
    clark.i_alpha = _a;
    clark.i_beta = ONE_SQRT_THREE * (_a + 2 * _b);
}

DQCurrent_s Park_transform(ABCurrent_s clark, float angle)
{
    float sa = sinf(angle);
    float ca = cosf(angle);
    DQCurrent_s park = {0};
    park.id = clark.i_alpha * ca + clark.i_beta * sa;
    park.iq = -clark.i_alpha * sa + clark.i_beta * ca;

    return park;
}

float estimateBEMF (float vel)
{
    // TODO: implement Sliding-mode observers (SMO)

    // bemf constant is approximately 1/KV rating
    // V_bemf = K_bemf * velocity
    return vel/(motor.KV_rating*_SQRT3)/_RPM_TO_RADS;
}

bool active_state_check (void)
{
    if (FOC_sequence == FOC_idle 
        || FOC_sequence == ADC_Offset_Calibration_setup 
        || FOC_sequence == ADC_Offset_Calibration_run 
        || FOC_sequence == FOC_ShutDown
        || FOC_sequence == Sensor_alignment)
    {
        return false;
    }

    return true;
}
