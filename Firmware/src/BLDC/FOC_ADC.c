#include <stddef.h>       
#include <stdbool.h>      
#include <stdlib.h>       
#include "definitions.h"  
#include "FOC_ADC.h"
#include "FOC_utilities.h"

//===================================================
//                  private configs
#define OFFSET_SAMPLES    128u  //< No of samples for offset calculation 


//====================================================
//                  private parameters

typedef struct{
    float32_t uBus_f;
    uint16_t IaADC_16;
    uint16_t IbADC_16;
    uint16_t uBus_16;
    float32_t iaOffset;
    float32_t ibOffset;
    float32_t Ia_f;
    float32_t Ib_f;
    float32_t Ic_f;
}_FOC_ADC_READ;

static _FOC_ADC_READ ADC_READ = {0};

static uint16_t Vol_State = 0;
static float adcToVoltsFactor = 0.000806; // ADC_VOLTAGE / ADC_RESOLUTION
static float32_t adcToCurrentFactor = (float32_t)(0.5);
static uintptr_t dummyForMisra;
static FOC_ADC_status status = 0; 

// Current calibration parameters
static uint16_t Current_Offset_counter = 0;
static uint32_t iaOffsetBuffer= 0;
static uint32_t ibOffsetBuffer= 0;

//===================================================
//                  Private prototypes 
static void FOC_ADC_PhaseCurrentChannelSelect(void);
static void FOC_ADC_Enable(void);
static void FOC_ADC_Disable(void);
static void FOC_ADC_PhaseACurrentGet(void);
static void FOC_ADC_PhaseBCurrentGet(void);
static void FOC_ADC_DCBusGet(void);
static void FOC_ADC_ConversionStart(void);
static void FOC_ADC_HardwareTriggerRenable(void);
static void FOC_ADC_InterruptClear(void);
static void FOC_ADC_InterruptDisable(void);
static void FOC_ADC_InterruptEnable(void);
static void FOC_ADC_CurrentOffsetCalculation(void);
volatile static void FOC_ADC_FinishedIsr(ADC_STATUS status, uintptr_t context);
static void FOC_ADC_CurrentCalculation(void);

//=================================================
//                  private functions

/**
 * @brief Enable ADC peripheral
 *
 * @details
 * Enable ADC peripheral
 */
void FOC_ADC_Enable(void)
{
    ADC0_Enable();
}

/**
 * @brief Disable ADC peripheral
 *
 * @details
 * Disable ADC peripheral
 */
void FOC_ADC_Disable(void)
{
    ADC0_Disable();
}

/**
 * @brief Get Phase A current from ADC peripheral.
 *
 * @details
 * Get analog signals from ADC peripheral.

 * @param[out]
 */
void FOC_ADC_PhaseACurrentGet(void)
{
    ADC_READ.IaADC_16 = ADC0_ConversionResultGet();
}

/**
 * @brief Get Phase B current from ADC peripheral.
 *
 * @details
 * Get Phase B current from ADC peripheral.

 * @param[out]
 */
void FOC_ADC_PhaseBCurrentGet(void)
{
    ADC_READ.IbADC_16 =  ADC0_ConversionResultGet();
}

/**
 * @brief Get DC link voltage from ADC peripheral.
 *
 * @details
 * Get DC link voltage from ADC peripheral.

 * @param[out]
 */
void FOC_ADC_DCBusGet(void)
{
    /** Get ADC value for DC bus voltage */
    ADC_READ.uBus_16 = ADC0_ConversionResultGet();
}


/**
 * @brief ADC conversion complete interrupt callback function
 *
 * @details
 * ADC conversion complete interrupt callback function
 */
void FOC_ADC_CallBackRegister( ADC_CALLBACK callback, uintptr_t context )
{
    ADC0_CallbackRegister( callback, context);
}

/**
 * @brief Attaching pins to ADC channel
 * 
 */
void FOC_ADC_PhaseCurrentChannelSelect(void)
{
    //phase A attached to AIN9
    ADC0_ChannelSelect(ADC_POSINPUT_AIN9,ADC_NEGINPUT_GND);
    
    //phase B attached to AIN10
    ADC0_ChannelSelect(ADC_POSINPUT_AIN10,ADC_NEGINPUT_GND);

    //phase C attached to AIN10
    ADC0_ChannelSelect(ADC_POSINPUT_AIN11,ADC_NEGINPUT_GND);
}

/**
 * @brief Start ADC conversion
 *
 * @details
 * Start ADC conversion
 */
void FOC_ADC_ConversionStart(void)
{
    // Enable software  trigger
    ADC0_InterruptsClear(ADC_STATUS_MASK);
    ADC0_InterruptsDisable(ADC_STATUS_RESRDY);
    ADC0_ConversionStart();
}

/**
 * @brief Re-enable ADC conversion from PWM event source
 *
 * @details
 * Re-enable ADC conversion from PWM event source
 */
void FOC_ADC_HardwareTriggerRenable(void)
{
    /* Re-enable hardware trigger */
    ADC0_InterruptsClear(ADC_STATUS_MASK);
    ADC0_InterruptsEnable(ADC_STATUS_RESRDY);
}

/**
 * @brief Clear ADC interrupt flag
 *
 * @details
 * Clear ADC interrupt flag
 */
void FOC_ADC_InterruptClear(void)
{
    ADC0_InterruptsClear(ADC_STATUS_MASK);
}

/**
 * @brief ADC interrupt disable
 *
 * @details
 * ADC interrupt disable
 */
void FOC_ADC_InterruptDisable(void)
{
    ADC0_InterruptsDisable(ADC_STATUS_RESRDY);
}

/**
 * @brief ADC interrupt enable
 *
 * @details
 * ADC interrupt enable
 */
void FOC_ADC_InterruptEnable(void)
{
    ADC0_InterruptsEnable(ADC_STATUS_RESRDY);
}

/**
 * @brief Motor control application calibration ISR
 *
 * @details Interrupt service routine for motor control application calibration.
 *
 * @param[in] status ADC status information
 * @param[in/out] context Interrupt context
 * @param[out] None
 *
 * @return None
 */
void FOC_ADC_CalibrationIsr(ADC_STATUS status, uintptr_t context)
{
    FOC_ADC_status e_returnStatus;

    // ADC end of conversion interrupt generation for FOC control
    FOC_ADC_InterruptDisable();
    FOC_ADC_InterruptClear();

    // Read phase currents
    FOC_ADC_PhaseACurrentGet();
    FOC_ADC_PhaseBCurrentGet();

    // Phase current offset measurement
    FOC_ADC_CurrentOffsetCalculation();

    // Current sense amplifiers offset calculation
    if( StdReturn_Complete == e_returnStatus )
    {
        FOC_ADC_CallBackRegister((ADC_CALLBACK)FOC_ADC_FinishedIsr, (uintptr_t)dummyForMisra );
    }
    else
    {
        /** For MISRA Compliance */
    }


     /** ADC end of conversion interrupt generation for FOC control */
    FOC_ADC_InterruptClear();
    FOC_ADC_InterruptEnable();
}

/**
 * @brief Function to calculate the current sensor offset.
 *
 * Calculates the offset for the current sensors to ensure accurate current measurements.
 * 
 * @return None
 */
void FOC_ADC_CurrentOffsetCalculation(void)
{

    status = StdReturn_Progress;

    // Read input ports
    int16_t iaADCInput = (int16_t)ADC_READ.IaADC_16;
    int16_t ibADCInput = (int16_t)ADC_READ.IbADC_16;

    if (Current_Offset_counter < OFFSET_SAMPLES)
    {
        iaOffsetBuffer += (uint32_t)iaADCInput;
        ibOffsetBuffer += (uint32_t)ibADCInput;
        Current_Offset_counter++;
    }
    else
    {
        ADC_READ.iaOffset = (float32_t)( (float32_t)iaOffsetBuffer/ (float32_t)OFFSET_SAMPLES );
        ADC_READ.ibOffset = (float32_t)( (float32_t)ibOffsetBuffer/ (float32_t)OFFSET_SAMPLES );

        /**Set ADC Calibration Done Flag */
        status = StdReturn_Complete;
    }
}

/**
 * @brief ADC finished ISR
 *
 * @details Interrupt service routine for ADC finished tasks.
 *
 * @param[in] status ADC status information
 * @param[in/out] context Interrupt context
 * @param[out] None
 *
 * @return None
 */
volatile void FOC_ADC_FinishedIsr(ADC_STATUS status, uintptr_t context )
{
    /** ADC interrupt disable  */
    FOC_ADC_InterruptDisable();
    FOC_ADC_InterruptClear();

    /** Read phase currents  */
    FOC_ADC_PhaseACurrentGet();
    FOC_ADC_PhaseBCurrentGet();

    /** Set Potentiometer and bus voltage channels */
    //mcHalI_PotentiometerChannelSelect();
    //mcHalI_DcLinkVoltageChannelSelect();

    /** Start software conversion */
    FOC_ADC_ConversionStart();

    /** Current calculation */
    FOC_ADC_CurrentCalculation();

    /** Bus voltage calculation */
    //mcVolI_VoltageCalculation( &mcVolI_ModuleData_gds );

    /** Read DC bus voltage */
    //mcHalI_DcLinkVoltageGet();

    /** Read potentiometer input */
    //mcHalI_PotentiometerInputGet();

    /** Set phase A and phase B current channels */
    FOC_ADC_PhaseCurrentChannelSelect();

    /** Re-enable hardware trigger for ADC channels */
    FOC_ADC_HardwareTriggerRenable();

    /** ADC interrupt clear  */
    FOC_ADC_InterruptClear();
    FOC_ADC_InterruptEnable();
}

void FOC_ADC_CurrentCalculation(void)
{
    /** Read input ports */
    int16_t iaADCInput = (int16_t)ADC_READ.IaADC_16;
    int16_t ibADCInput = (int16_t)ADC_READ.IbADC_16;

    /** Phase A current measurement */
    ADC_READ.Ia_f = (ADC_READ.iaOffset - (float32_t)iaADCInput) * adcToCurrentFactor;

    /** Phase B current measurement */
    ADC_READ.Ib_f = (ADC_READ.ibOffset - (float32_t)ibADCInput) * adcToCurrentFactor;

    /** Phase C current calculation */
    ADC_READ.Ic_f = -ADC_READ.Ia_f - ADC_READ.Ib_f ;
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
void FOC_ADC_initialize (void)
{
    // ADC interrupt disable 
    FOC_ADC_InterruptDisable();
    FOC_ADC_InterruptClear();

    // Enable ADC interrupt for field oriented control
    FOC_ADC_CallBackRegister((ADC_CALLBACK)FOC_ADC_CalibrationIsr, (uintptr_t)dummyForMisra);
    FOC_ADC_InterruptEnable( );

    // Enable ADC module
    FOC_ADC_Enable();

    // Enable interrupt for fault detection
    //mcHalI_PwmCallbackRegister( (TCC_CALLBACK)mcAppI_OverCurrentReactionIsr, (uintptr_t)dummyForMisra );

    FOC_ADC_PhaseCurrentChannelSelect();
    FOC_ADC_ConversionStart();
}


//void FOC_ADC_VoltageCalculation(void)
//{
//    ADC_READ.uBus_f = (float32_t)
//}