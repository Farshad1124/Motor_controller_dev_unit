#include <stddef.h>       
#include <stdbool.h>      
#include <stdlib.h>       
#include "definitions.h"  
#include "FOC_ADC.h"
#include "../BW_filter/BW_filter.h"

//===================================================
//                  private macros
#define OFFSET_SAMPLES    128u  //< No of samples for offset calculation 
#define ADC_PhaseA ADC_POSINPUT_AIN9
#define ADC_PhaseB ADC_POSINPUT_AIN10
#define ADC_PhaseC ADC_POSINPUT_AIN11

#define Vref (float)(3.3)
#define HalfOfVref (float)(Vref/2)

#define CALC_IA(ia_sensed, ib_sensed, ic_sensed) \
    (1.001152f * (ia_sensed) - 0.003375f * (ib_sensed) - 0.003103f * (ic_sensed))
#define CALC_IB(ia_sensed, ib_sensed, ic_sensed) \
    (0.002369f * (ia_sensed) - 1.000665f * (ib_sensed) - 0.019126f * (ic_sensed))
#define CALC_IC(ia_sensed, ib_sensed, ic_sensed) \
    (0.001234f * (ia_sensed) - 0.001595f * (ib_sensed) - 0.998166f * (ic_sensed))
//====================================================
//                  private parameters

typedef struct{
    bool Offset_calibrated;
    uint16_t VaADC_16;
    uint16_t VbADC_16;
    uint16_t VcADC_16;
    float iaOffset;
    float ibOffset;
    float icOffset;
    float Ia;
    float Ib;
    float Ic;
    float Va;
    float Vb;
    float Vc;
    float amp_gain;
    float PhaseA_gain;
    float PhaseB_gain;
    float PhaseC_gain;
    int calibration_count;
}_FOC_ADC_READ;

static _FOC_ADC_READ ADC_READ = {0};

static float adcToVoltsFactor = 0.000806; // ADC_VOLTAGE / ADC_RESOLUTION
static float adcToCurrentFactor = (float)(0.5);
static uintptr_t dummyForMisra;
static uint8_t Current_PhaseNO = Phase_A;

// Current calibration parameters
static uint16_t Current_Offset_counter = 0;
static uint32_t iaOffsetBuffer= 0;
static uint32_t ibOffsetBuffer= 0;
static float *temp_calib;
static volatile bool temp_calib_freed = false;
BWLowPass* IA_filter;
BWLowPass* IB_filter;
BWLowPass* IC_filter;

//===================================================
//                  Private prototypes 
static void FOC_ADC_PhaseA_ChannelSelect(void);
static void FOC_ADC_PhaseB_ChannelSelect(void);
static void FOC_ADC_PhaseC_ChannelSelect(void);
static void FOC_ADC_CurrentOffsetCalculation(void);
static void FOC_ADC_CurrentCalculation(void);
void ADC_ISR(ADC_STATUS status, uintptr_t context );

//=================================================
//                  private functions

static inline float adc_raw_to_volts(uint16_t raw, float v_ref)
{
    return ((float)raw / 4096.0f) * v_ref;   // 4096 = 2^12
}

static inline float adc_volts_to_current(float v, float v_ref)
{
    return ((v - v_ref) / ADC_READ.amp_gain); 
}


void FOC_ADC_PhaseA_ChannelSelect(void)
{
    //phase A attached to AIN9
    ADC0_ChannelSelect(ADC_PhaseA,ADC_NEGINPUT_GND);
}

void FOC_ADC_PhaseB_ChannelSelect(void)
{
    //phase B attached to AIN10
    ADC0_ChannelSelect(ADC_PhaseB,ADC_NEGINPUT_GND);
}

void FOC_ADC_PhaseC_ChannelSelect(void)
{
    //phase C attached to AIN11
    ADC0_ChannelSelect(ADC_PhaseC,ADC_NEGINPUT_GND);
}

void FOC_ADC_CurrentCalculation(void)
{
    ADC_READ.Va = adc_raw_to_volts(ADC_READ.VaADC_16,Vref);
    ADC_READ.Vb = adc_raw_to_volts(ADC_READ.VbADC_16,Vref);
    ADC_READ.Vc = adc_raw_to_volts(ADC_READ.VcADC_16,Vref);

    float Ia = adc_volts_to_current(ADC_READ.Va,HalfOfVref);
    float Ib = adc_volts_to_current(ADC_READ.Vb,HalfOfVref);
    float Ic = adc_volts_to_current(ADC_READ.Vc,HalfOfVref);

    // Current measurement
    //ADC_READ.Ia = bw_low_pass(IA_filter,CALC_IA(Ia,Ib,Ic)) - ADC_READ.iaOffset;
    //ADC_READ.Ib = bw_low_pass(IB_filter,CALC_IB(Ia,Ib,Ic)) - ADC_READ.ibOffset;
    //ADC_READ.Ic = bw_low_pass(IC_filter,CALC_IC(Ia,Ib,Ic)) - ADC_READ.icOffset;
    ADC_READ.Ia = (CALC_IA(Ia,Ib,Ic) * ADC_READ.PhaseA_gain) - ADC_READ.iaOffset;
    ADC_READ.Ib = (CALC_IB(Ia,Ib,Ic) * ADC_READ.PhaseB_gain) - ADC_READ.ibOffset;
    ADC_READ.Ic = (CALC_IC(Ia,Ib,Ic) * ADC_READ.PhaseC_gain) - ADC_READ.icOffset;
}

void ADC_ISR(ADC_STATUS status, uintptr_t context)
{
    switch (Current_PhaseNO)
    {
        case Phase_A:
            ADC_READ.VaADC_16 = ADC0_ConversionResultGet();
            FOC_ADC_PhaseB_ChannelSelect();
            Current_PhaseNO = Phase_B;
        break;

        case Phase_B:
            ADC_READ.VbADC_16 = ADC0_ConversionResultGet();
            FOC_ADC_PhaseC_ChannelSelect();
            Current_PhaseNO = Phase_C;
        break;

        case Phase_C:
            ADC_READ.VcADC_16 = ADC0_ConversionResultGet();
            FOC_ADC_PhaseA_ChannelSelect();
            Current_PhaseNO = Phase_A;
        break;
    }
}


//=================================================
//                  Global functions

/**
 * @brief Application initialization
 *
 * @details Initializes the application.
 *
 * @param[in] None
 * @param[in/out] None
 * @param[out] None
 *
 * @return None
 */
void FOC_ADC_Initialize(float gain, uint32_t frequency)
{
    ADC0_Disable();

    ADC_READ.amp_gain = gain;
    ADC_READ.PhaseA_gain = 1;
    ADC_READ.PhaseB_gain = 1;
    ADC_READ.PhaseC_gain = 1;

    ADC0_CallbackRegister((ADC_CALLBACK) ADC_ISR, (uintptr_t)dummyForMisra);

    // resolution and operation mode     
    ADC0_REGS->ADC_CTRLA = ADC_CTRLA_PRESCALER_DIV4;
    ADC0_REGS->ADC_SAMPCTRL = (uint8_t)ADC_SAMPCTRL_SAMPLEN(3UL);
    ADC0_REGS->ADC_REFCTRL = ADC_REFCTRL_REFSEL_INTVCC1 | ADC_REFCTRL_REFCOMP_Msk; // Set reference voltage to VDD
    ADC0_ChannelSelect(ADC_POSINPUT_AIN11, ADC_NEGINPUT_GND);
    ADC0_REGS->ADC_CTRLB = ADC_CTRLB_RESSEL_12BIT | ADC_CTRLB_WINMODE(0U);

    // Enable ADC interrupt 
    ADC0_REGS->ADC_INTFLAG = ADC_INTFLAG_Msk;
    ADC0_REGS->ADC_INTENSET = ADC_INTENSET_RESRDY_Msk;
    while(ADC0_REGS->ADC_SYNCBUSY != 0U)
    {
        /* Wait for Synchronization */
    }
    
    NVIC_EnableIRQ(ADC0_RESRDY_IRQn);

    // Event channel user configuration
    EVSYS_REGS->EVSYS_USER[EVENT_ID_USER_ADC0_START] = EVSYS_USER_CHANNEL(((uint32_t)ADC_EVSYS_CHANNEL + 1U));

    // Event channel 0 configuration
    EVSYS_REGS->CHANNEL[ADC_EVSYS_CHANNEL].EVSYS_CHANNEL = EVSYS_CHANNEL_EVGEN(EVENT_ID_GEN_TCC0_OVF) 
                                                         | EVSYS_CHANNEL_PATH(EVSYS_CHANNEL_PATH_ASYNCHRONOUS_Val) 
                                                         | EVSYS_CHANNEL_EDGSEL(EVSYS_CHANNEL_EDGSEL_RISING_EDGE_Val);

    // configure the event system
    TCC0_PWMStop();
    TCC0_REGS->TCC_EVCTRL = TCC_EVCTRL_OVFEO_Msk; // PWM OVFEO Overflow/Underflow Event Output Enable
    while (TCC0_REGS->TCC_SYNCBUSY != 0U)
    {
        /* Wait for sync */
    }

    // configure BW filter
    IA_filter = create_bw_low_pass_filter(2,(float)1000,ADC_cutoff_frequency);
    IB_filter = create_bw_low_pass_filter(2,(float)1000,ADC_cutoff_frequency);
    IC_filter = create_bw_low_pass_filter(2,(float)1000,ADC_cutoff_frequency);

    ADC0_Enable();
    ADC0_ConversionStart(); 
}

void FOC_ADC_Get_Current(float* A, float* B, float* C)
{
    FOC_ADC_CurrentCalculation();
    *A = ADC_READ.Ia;
    *B = ADC_READ.Ib;
    *C = ADC_READ.Ic;
}

void FOC_ADC_Get_Voltage(float* A, float* B, float* C)
{
    FOC_ADC_CurrentCalculation();
    *A = ADC_READ.Va;
    *B = ADC_READ.Vb;
    *C = ADC_READ.Vc;
}

/**
 * @brief Perform ADC calibration on each call untill state is changed
 * 
 * @return true 
 * @return false 
 */
bool FOC_ADC_Calibration(void)
{
    if (ADC_READ.Offset_calibrated)
    {
        if (temp_calib_freed == false)
        {
            free(temp_calib);
            temp_calib_freed = true;
        }
        return true;
    }

    if (ADC_READ.calibration_count == 0)
    {
        temp_calib = (float *)malloc(3 * sizeof(float));
        temp_calib[Phase_A] = 0;
        temp_calib[Phase_B] = 0;
        temp_calib[Phase_C] = 0;
    }

    if (ADC_READ.calibration_count < calibration_rounds)
    {
        ADC_READ.calibration_count++;
        FOC_ADC_CurrentCalculation();
        temp_calib[Phase_A] += ADC_READ.Ia;
        temp_calib[Phase_B] += ADC_READ.Ib;
        temp_calib[Phase_C] += ADC_READ.Ic;
    }
    else 
    {
        ADC_READ.iaOffset = temp_calib[Phase_A]/calibration_rounds;
        ADC_READ.ibOffset = temp_calib[Phase_B]/calibration_rounds;
        ADC_READ.icOffset = temp_calib[Phase_C]/calibration_rounds;
        ADC_READ.Offset_calibrated = true;
    }

    return false;
}

bool ADC_Calibration_check(void)
{
    return ADC_READ.Offset_calibrated;
}

void FOC_ADC_set_phaseA_gain(float gain)
{
    ADC_READ.PhaseA_gain = gain;
}

void FOC_ADC_set_phaseB_gain(float gain)
{
    ADC_READ.PhaseB_gain = gain;
}

void FOC_ADC_set_phaseC_gain(float gain)
{
    ADC_READ.PhaseC_gain = gain;
}




