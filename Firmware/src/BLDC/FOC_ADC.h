#ifndef FOC_ADC_H
#define FOC_ADC_H


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



#endif



