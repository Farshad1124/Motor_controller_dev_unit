#ifndef FOC_ADC_H
#define FOC_ADC_H

#include <stdbool.h>


//=========================================================
//                      Config parameters


/*! \brief Enumeration for standard return status codes */
typedef enum {
    e_StdReturn_Progress, /*!< Operation in progress */
    e_StdReturn_Failed,   /*!< Operation failed */
    e_StdReturn_Success,   /*!< Operation successful */
    e_StdReturn_Complete, /*!< Operation successful */
    e_StdReturn_Pending,  /*!< Operation pending */
    e_StdReturn_Error,  /*!< Operation error */
    e_StdReturn_Timeout,  /*!< Operation timeout */
    e_StdReturn_Invalid,  /*!< Operation invalid */
} FOC_ADC_status;

typedef enum {
    Phase_A,
    Phase_B,
    Phase_C
} FOC_ADC_phase;

#define ADC_EVSYS_CHANNEL 0
#define calibration_rounds 2000

// current sense low pass filter
#define ADC_cutoff_frequency (float)(200)

void FOC_ADC_Initialize(float gain, uint32_t frequency);
void FOC_ADC_Get_Current(float* A, float* B, float* C);
void FOC_ADC_Get_Voltage(float* A, float* B, float* C);
bool FOC_ADC_Calibration(void);
bool ADC_Calibration_check(void);
void FOC_ADC_set_phaseA_gain(float gain);
void FOC_ADC_set_phaseB_gain(float gain);
void FOC_ADC_set_phaseC_gain(float gain);

#endif



