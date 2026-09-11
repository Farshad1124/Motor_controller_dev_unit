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
#include "./RTT/SEGGER_RTT.h"
#include "./BLDC/FOC_Application.h"
volatile int _Cnt;

void LOOP(void * pvParameters );

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
bool Output_sel_pressed = false;
bool Output_sel_last_state = false;
uint8_t LED_count = 0;

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    // RTT terminal 0: value streaming buffer
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);

    FOC_control_config control = 
    {
      .source_voltage = 8,
      .voltage_limit = 5,
      .current_limit = 1,
      .velocity_limit = 100,
      .torque_limit = 0,
      .ADC_gain = 1,
      .motion_mode = velocity_openloop_control,
      .torque_mode = Torque_via_idq,
      .pwm_frequency = 20000,
      .modulation = SinePWM, 
      .current_P = 0.05,
      .current_I = 1.0,
      .current_D = 0.0,
    };

    FOC_motor_config motor = 
    {
      .inductance = 0.0045,
      .pole_pair_count = 7,
      .winding_resistance = 12,
      .KV_rating = 2.8,
    };
    
    FOC_Application_Initialize(&control,&motor);
    xTaskCreate( LOOP, "loop", 2048, NULL, 1, NULL );
   
    vTaskStartScheduler();

    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


void LOOP(void * pvParameters )
{
  float angle = 0;
  FOC_Move(1);

  for(;;)
  {
    if (active_state_check())
    {
      //FOC_Move(angle);
      
      //angle+=1;
      //if (angle > 365)
      //{
      //  angle = 0;
      //}
    }
    vTaskDelay(1000);
  }
}

/*******************************************************************************
 End of File
*/


