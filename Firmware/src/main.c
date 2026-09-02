/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "FreeRTOS.h"
#include "task.h"
//#include "BLDC/FOC_PWM.h"
#include "mc_application.h"
#include "./RTT/SEGGER_RTT.h"
volatile int _Cnt;

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
bool Output_sel_pressed = false;
bool Output_sel_last_state = false;
uint8_t LED_count = 0;


void RTT_Initialize(void);

/**
 * @brief 
 * 
 * @param context 
 */
void OUTPUT_SEL_EIC_Handler(uintptr_t context)
{

  switch (LED_count)
  {
    case 0:
    LED_control_Toggle();
    Servo_EN_Clear();
    LED_count++;
    break;

    case 1:
    LED_control_Toggle();
    BLDC_EN_Toggle();
    LED_count++;
    break;

    case 2:
    BLDC_EN_Toggle();
    PM_EN_Toggle();
    LED_count++;
    break;

    case 3: 
    PM_EN_Toggle();
    SM_EN_Toggle();
    LED_count++;
    break;

    case 4: 
    SM_EN_Toggle();
    Servo_EN_Toggle();
    LED_count = 0;
    break;
  }
}


void BLDC_motor_test( void *pvParameters )
{
  uint8_t BLDC_mode = 0;
  while(1)
  {
    switch (BLDC_mode)
    {
      case 0: // wait for the power /enable singal
      if (LED_count == 2)
      {
        mcAppI_ApplicationInit();
        BLDC_mode++;
      }
      break;

      case 1:
        mcAppI_NonISRTasks();
        //PWM_FOC_Set(6,6,6);
        BLDC_mode++;
      break;

      case 2: 
      if (LED_count != 2)
      {
        //PWM_FOC_Disable ();
        BLDC_mode= 1;
      }
      break;
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}



int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
    //RTT_Initialize();
    //PWM_FOC_Initialize(5,8,20000);


    EIC_CallbackRegister(EIC_PIN_8,OUTPUT_SEL_EIC_Handler, 0);
    xTaskCreate( BLDC_motor_test, "BLDC_test", 1024, NULL, 1, NULL );
   

    vTaskStartScheduler();

    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}



void RTT_Initialize(void)
{
  SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);

  SEGGER_RTT_WriteString(0, "SEGGER Real-Time-Terminal Sample\r\n\r\n");
  SEGGER_RTT_WriteString(0, "###### Testing SEGGER_printf() ######\r\n");

  SEGGER_RTT_printf(0, "printf Test: %%c,         'S' : %c.\r\n", 'S');
  SEGGER_RTT_printf(0, "printf Test: %%5c,        'E' : %5c.\r\n", 'E');
  SEGGER_RTT_printf(0, "printf Test: %%-5c,       'G' : %-5c.\r\n", 'G');
  SEGGER_RTT_printf(0, "printf Test: %%5.3c,      'G' : %-5c.\r\n", 'G');
  SEGGER_RTT_printf(0, "printf Test: %%.3c,       'E' : %-5c.\r\n", 'E');
  SEGGER_RTT_printf(0, "printf Test: %%c,         'R' : %c.\r\n", 'R');

  SEGGER_RTT_printf(0, "printf Test: %%s,      \"RTT\" : %s.\r\n", "RTT");
  SEGGER_RTT_printf(0, "printf Test: %%s, \"RTT\\r\\nRocks.\" : %s.\r\n", "RTT\r\nRocks.");

  SEGGER_RTT_printf(0, "printf Test: %%u,       12345 : %u.\r\n", 12345);
  SEGGER_RTT_printf(0, "printf Test: %%+u,      12345 : %+u.\r\n", 12345);
  SEGGER_RTT_printf(0, "printf Test: %%.3u,     12345 : %.3u.\r\n", 12345);
  SEGGER_RTT_printf(0, "printf Test: %%.6u,     12345 : %.6u.\r\n", 12345);
  SEGGER_RTT_printf(0, "printf Test: %%6.3u,    12345 : %6.3u.\r\n", 12345);
  SEGGER_RTT_printf(0, "printf Test: %%8.6u,    12345 : %8.6u.\r\n", 12345);
  SEGGER_RTT_printf(0, "printf Test: %%08u,     12345 : %08u.\r\n", 12345);
  SEGGER_RTT_printf(0, "printf Test: %%08.6u,   12345 : %08.6u.\r\n", 12345);
  SEGGER_RTT_printf(0, "printf Test: %%0u,      12345 : %0u.\r\n", 12345);
  SEGGER_RTT_printf(0, "printf Test: %%-.6u,    12345 : %-.6u.\r\n", 12345);
  SEGGER_RTT_printf(0, "printf Test: %%-6.3u,   12345 : %-6.3u.\r\n", 12345);
  SEGGER_RTT_printf(0, "printf Test: %%-8.6u,   12345 : %-8.6u.\r\n", 12345);
  SEGGER_RTT_printf(0, "printf Test: %%-08u,    12345 : %-08u.\r\n", 12345);
  SEGGER_RTT_printf(0, "printf Test: %%-08.6u,  12345 : %-08.6u.\r\n", 12345);
  SEGGER_RTT_printf(0, "printf Test: %%-0u,     12345 : %-0u.\r\n", 12345);

  SEGGER_RTT_printf(0, "printf Test: %%u,      -12345 : %u.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%+u,     -12345 : %+u.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%.3u,    -12345 : %.3u.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%.6u,    -12345 : %.6u.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%6.3u,   -12345 : %6.3u.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%8.6u,   -12345 : %8.6u.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%08u,    -12345 : %08u.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%08.6u,  -12345 : %08.6u.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%0u,     -12345 : %0u.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%-.6u,   -12345 : %-.6u.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%-6.3u,  -12345 : %-6.3u.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%-8.6u,  -12345 : %-8.6u.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%-08u,   -12345 : %-08u.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%-08.6u, -12345 : %-08.6u.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%-0u,    -12345 : %-0u.\r\n", -12345);

  SEGGER_RTT_printf(0, "printf Test: %%d,      -12345 : %d.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%+d,     -12345 : %+d.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%.3d,    -12345 : %.3d.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%.6d,    -12345 : %.6d.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%6.3d,   -12345 : %6.3d.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%8.6d,   -12345 : %8.6d.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%08d,    -12345 : %08d.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%08.6d,  -12345 : %08.6d.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%0d,     -12345 : %0d.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%-.6d,   -12345 : %-.6d.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%-6.3d,  -12345 : %-6.3d.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%-8.6d,  -12345 : %-8.6d.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%-08d,   -12345 : %-08d.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%-08.6d, -12345 : %-08.6d.\r\n", -12345);
  SEGGER_RTT_printf(0, "printf Test: %%-0d,    -12345 : %-0d.\r\n", -12345);

  SEGGER_RTT_printf(0, "printf Test: %%x,      0x1234ABC : %x.\r\n", 0x1234ABC);
  SEGGER_RTT_printf(0, "printf Test: %%+x,     0x1234ABC : %+x.\r\n", 0x1234ABC);
  SEGGER_RTT_printf(0, "printf Test: %%.3x,    0x1234ABC : %.3x.\r\n", 0x1234ABC);
  SEGGER_RTT_printf(0, "printf Test: %%.6x,    0x1234ABC : %.6x.\r\n", 0x1234ABC);
  SEGGER_RTT_printf(0, "printf Test: %%6.3x,   0x1234ABC : %6.3x.\r\n", 0x1234ABC);
  SEGGER_RTT_printf(0, "printf Test: %%8.6x,   0x1234ABC : %8.6x.\r\n", 0x1234ABC);
  SEGGER_RTT_printf(0, "printf Test: %%08x,    0x1234ABC : %08x.\r\n", 0x1234ABC);
  SEGGER_RTT_printf(0, "printf Test: %%08.6x,  0x1234ABC : %08.6x.\r\n", 0x1234ABC);
  SEGGER_RTT_printf(0, "printf Test: %%0x,     0x1234ABC : %0x.\r\n", 0x1234ABC);
  SEGGER_RTT_printf(0, "printf Test: %%-.6x,   0x1234ABC : %-.6x.\r\n", 0x1234ABC);
  SEGGER_RTT_printf(0, "printf Test: %%-6.3x,  0x1234ABC : %-6.3x.\r\n", 0x1234ABC);
  SEGGER_RTT_printf(0, "printf Test: %%-8.6x,  0x1234ABC : %-8.6x.\r\n", 0x1234ABC);
  SEGGER_RTT_printf(0, "printf Test: %%-08x,   0x1234ABC : %-08x.\r\n", 0x1234ABC);
  SEGGER_RTT_printf(0, "printf Test: %%-08.6x, 0x1234ABC : %-08.6x.\r\n", 0x1234ABC);
  SEGGER_RTT_printf(0, "printf Test: %%-0x,    0x1234ABC : %-0x.\r\n", 0x1234ABC);

  SEGGER_RTT_printf(0, "printf Test: %%p,      &_Cnt      : %p.\r\n", &_Cnt);

  SEGGER_RTT_WriteString(0, "###### SEGGER_printf() Tests done. ######\r\n");
  do {
    _Cnt++;
  } while (1);

}
/*******************************************************************************
 End of File
*/

