#include "definitions.h"     
#include "FOC_PWM.h"
#include <math.h>

// ================================================
// 					Global parameters
PhaseState phase_state[3]; // phase state (active / disabled)
float dc_a; // currently set duty cycle on phaseA
float dc_b; // currently set duty cycle on phaseB
float dc_c; // currently set duty cycle on phaseC

float voltage_limit = 0;
float voltage_power_supply = 0;
uint32_t pwm_resolution = 0;

uint8_t TCC_CHANNEL_COUNT[3];


//===================================================
// 					Macros
#define _constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define _UNUSED(v) (void) (v)
#define GetTCNumber( x ) ( (x) >> 8 )
#define GetTCChannelNumber( x ) ( (x) & 0xff )


//========================================================================================
//									Global function
//========================================================================================
/**
 * @brief Set the phase state
 * @brief actually changing the state is only done on the next call to setPwm
 * @param sa 
 * @param sb 
 * @param sc 
 */
void setPhaseState(PhaseState sa, PhaseState sb, PhaseState sc) {
	phase_state[0] = sa;
  	phase_state[1] = sb;
  	phase_state[2] = sc;
}

/**
 * @brief 
 * 
 * @param pwm_frequency - frequency in hertz - if applicable
 * @param dead_zone  duty cycle protection zone [0, 1] - both low and high side low - if applicable
 * @param pinA_h pinA high-side bldc driver
 * @param pinA_l pinA low-side bldc driver
 * @param pinB_h pinA high-side bldc driver
 * @param pinB_l pinA low-side bldc driver
 * @param pinC_h pinA high-side bldc driver
 * @param pinC_l pinA low-side bldc driver
 *
 * @return True if config good, false if failed
 */
bool configure_FOC_PWM(long pwm_frequency, float dead_zone, const int pinA_h, const int pinA_l,  const int pinB_h, const int pinB_l, const int pinC_h, const int pinC_l)
{

    return true;
}

/**
 * @brief 
 * 
 * @param params 
 * @return true 
 * @return false 
 */
bool FOC_PWM_Initialize(float vlim, float vsup, uint32_t pwm_frequency)
{
	voltage_limit = vlim;
	voltage_power_supply = vsup;

	// set phase state to disabled
	phase_state[0] = PHASE_OFF;
	phase_state[1] = PHASE_OFF;
	phase_state[2] = PHASE_OFF;	

	// set zero to PWM
	dc_a = dc_b = dc_c = 0;	

	/* Configure TCC0 prescaler */
	TCC0_REGS->TCC_CTRLA &= ~TCC_CTRLA_ENABLE_Msk;	// disable TCC 
	while((TCC0_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_ENABLE_Msk) == TCC_SYNCBUSY_ENABLE_Msk)
	{
		/* Wait for sync */
	}

	/* DSTOP PWM. Master (low-side) output n is HIGH while
     * COUNT < CC[n]; DTI generates the complementary high side. */
	TCC0_REGS->TCC_WAVE =  TCC_WAVE_WAVEGEN_DSTOP
							| TCC_WAVE_POL(0xF)
							| TCC_WAVE_SWAP1_Msk 
							| TCC_WAVE_SWAP2_Msk
							| TCC_WAVE_SWAP3_Msk;
	while (TCC0_REGS->TCC_SYNCBUSY != 0U)
    {
        /* Wait for sync */
    }

	/* Dead time in GCLK_TCC0 clock cycles (the DTI counter runs at the TCC's
     * own peripheral clock, i.e. GCLK_TCC0_FREQ_HZ, independent of PRESCALER).
     * dead_time_ticks = DEAD_TIME_NS * GCLK_TCC0_FREQ_HZ / 1e9, clamped 0-255. */
	uint8_t DT_TICKS = (uint8_t)(DEFAULT_PWM_DEAD_ZONE_TIME * (float)(DPLL_FREQ/1000000u));
	TCC0_REGS->TCC_WEXCTRL =  TCC_WEXCTRL_OTMX(0)
							| TCC_WEXCTRL_DTLS(DT_TICKS)
							| TCC_WEXCTRL_DTHS(DT_TICKS)
							| TCC_WEXCTRL_DTIEN1_Msk   /* phase C: WO1 -> WO5 */
							| TCC_WEXCTRL_DTIEN2_Msk   /* phase B: WO2 -> WO6 */
							| TCC_WEXCTRL_DTIEN3_Msk;  /* phase A: WO3 -> WO7 */
	while (TCC0_REGS->TCC_SYNCBUSY != 0U)
    {
        /* Wait for sync */
    }		

 	/* Period: single-slope, freq = GCLK_TCC0 / (prescaler * (PER + 1)) */
 	uint32_t per = ((DPLL_FREQ / (PWM_PRESCALER_DIV * 2 * pwm_frequency)));
	TCC0_REGS->TCC_PER = TCC_PER_PER(per);
 	while (TCC0_REGS->TCC_SYNCBUSY != 0U)
    {
        /* Wait for sync */
    }

	/* Start all three phases at 0% duty (outputs off) until the FOC loop
     * starts writing real values. */
	TCC0_REGS->TCC_CC[PWM_PHASE_A] = 0;
	TCC0_REGS->TCC_CC[PWM_PHASE_B] = 0;
	TCC0_REGS->TCC_CC[PWM_PHASE_C] = 0;
    while (TCC0_REGS->TCC_SYNCBUSY != 0U)
    {
        /* Wait for sync */
    }
	
    //TCC0_REGS->TCC_CTRLA |= TCC_CTRLA_ENABLE_Msk;
	//while((TCC0_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_ENABLE_Msk) != TCC_SYNCBUSY_ENABLE_Msk)
	//{
	//	/* Wait for sync */
	//}
	
	pwm_resolution = (DPLL_FREQ/2) / pwm_frequency;
	if (pwm_resolution>MAX_PWM_RESOLUTION) 
		pwm_resolution = MAX_PWM_RESOLUTION;
	if (pwm_resolution<MIN_PWM_RESOLUTION) 
		pwm_resolution = MIN_PWM_RESOLUTION;

	return 1;
}

/**
 * @brief enable motor drive
 * 
 */
void FOC_PWM_Enable()
{
    // todo: set enable pin
    
    // set phase state enabled
    setPhaseState(PHASE_ON, PHASE_ON, PHASE_ON);
    // set zero to PWM
    FOC_PWM_Set(0, 0, 0);
}

/**
 * @brief disable motor drive
 */
void FOC_PWM_Disable ()
{

	setPhaseState(PHASE_OFF, PHASE_OFF, PHASE_OFF);
	FOC_PWM_Set(0, 0, 0);

	// todo: clear enable pin
}

void FOC_PWM_Set(float Ua, float Ub, float Uc)
{
	// limit the voltage in driver
	Ua = _constrain(Ua, 0, voltage_limit);
	Ub = _constrain(Ub, 0, voltage_limit);
	Uc = _constrain(Uc, 0, voltage_limit);
	
	// calculate duty cycle
	// limited in [0,1]
	dc_a = _constrain(Ua / voltage_power_supply, 0.0f , 1.0f );
	dc_b = _constrain(Ub / voltage_power_supply, 0.0f , 1.0f );
	dc_c = _constrain(Uc / voltage_power_supply, 0.0f , 1.0f );

	
	TCC0_REGS->TCC_CCBUF[PWM_PHASE_A] =  (uint32_t)((float)(pwm_resolution-1) * dc_a); 
	TCC0_REGS->TCC_CCBUF[PWM_PHASE_B] =  (uint32_t)((float)(pwm_resolution-1) * dc_b); 
	TCC0_REGS->TCC_CCBUF[PWM_PHASE_C] =  (uint32_t)((float)(pwm_resolution-1) * dc_c); 

	while (TCC0_REGS->TCC_SYNCBUSY != 0U)
    {
        /* Wait for sync */
    }
}




