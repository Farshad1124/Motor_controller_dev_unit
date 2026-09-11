#ifndef FOC_PWM_H
#define FOC_PWM_H

#include <stdbool.h>


typedef enum  : uint8_t {
  PHASE_OFF = 0, // both sides of the phase are off
  PHASE_ON = 1,  // both sides of the phase are driven with PWM, dead time is applied in 6-PWM mode
  PHASE_HI = 2,  // only the high side of the phase is driven with PWM (6-PWM mode only)
  PHASE_LO = 3,  // only the low side of the phase is driven with PWM (6-PWM mode only)
}PhaseState;

typedef enum {
    PWM_PHASE_C = 1,   /* CC1 -> WO1 (CL, pin41) direct; WO5 (CH, pin45) auto */
    PWM_PHASE_B = 2,   /* CC2 -> WO2 (BL, pin42) direct; WO6 (BH, pin46) auto */
    PWM_PHASE_A = 3,   /* CC3 -> WO3 (AL, pin43) direct; WO7 (AH, pin47) auto */
} pwm_phase_t;


#define DPLL_FREQ 60000000u
#define PWM_CLOCK_NUM 3
#define PWM_PRESCALER_DIV 1
#define ATSAMD51_TCC0_GCLK_ID 25  // based on how MCC is setup

// Defaults 
#define PWM_RESOLUTION 1000
#define DEFAULT_PWM_FREQUENCY_HZ 24000
#define DEFAULT_PWM_DEAD_ZONE_TIME (float)(0.03)  // useconds
#define DEFAULT_PINA_L 43
#define DEFAULT_PINA_H 47
#define DEFAULT_PINB_L 42
#define DEFAULT_PINB_H 46
#define DEFAULT_PINC_L 41
#define DEFAULT_PINC_H 45

// arbitrary maximum. On SAMD51 with 120MHz clock this means 2kHz minimum pwm frequency
#define MAX_PWM_RESOLUTION 30000

// lets not go too low - 400 with clock speed of 120MHz on SAMD51 means 150kHz maximum PWM frequency...
//						 400 with 48MHz clock on SAMD21 means 60kHz maximum PWM frequency...
#define MIN_PWM_RESOLUTION 400 


// Functions
bool FOC_PWM_Initialize(float vlim, float vsup, uint32_t pwm_frequency);
void FOC_PWM_Disable ();
void FOC_PWM_Enable();
void FOC_PWM_Set(float Ua, float Ub, float Uc);
void setPhaseState(PhaseState sa, PhaseState sb, PhaseState sc);

#endif