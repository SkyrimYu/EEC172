//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     - Blinky
// Application Overview - The objective of this application is to showcase the 
//                        GPIO control using Driverlib api calls. The LEDs 
//                        connected to the GPIOs on the LP are used to indicate 
//                        the GPIO output. The GPIOs are driven high-low 
//                        periodically in order to turn on-off the LEDs.
// Application Details  -
// http://processors.wiki.ti.com/index.php/CC32xx_Blinky_Application
// or
// docs\examples\CC32xx_Blinky_Application.pdf
//
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup blinky
//! @{
//
//****************************************************************************

// Standard includes
#include <stdio.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "gpio.h"
#include "utils.h"
#include "uart.h"

// Common interface includes
#include "gpio_if.h"
#include "uart_if.h"

#include "pin_mux_config.h"

#define APPLICATION_VERSION     "1.1.1"

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES                           
//*****************************************************************************
void LEDBlinkyRoutine();
static void BoardInit(void);

//*****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS                         
//*****************************************************************************

//*****************************************************************************
//
//! Configures the pins as GPIOs and peroidically toggles the lines
//!
//! \param None
//! 
//! This function  
//!    1. Configures 3 lines connected to LEDs as GPIO
//!    2. Sets up the GPIO pins as output
//!    3. Periodically toggles each LED one by one by toggling the GPIO line
//!
//! \return None
//
//*****************************************************************************
void AllLEDOFF()
{
    GPIOPinWrite(GPIOA1_BASE, 0x2, 0); // red off
    GPIOPinWrite(GPIOA1_BASE, 0x4, 0); // orange off
    GPIOPinWrite(GPIOA1_BASE, 0x8, 0); // green off
}

void
LEDCountRoutine()
{
    //
    // Toggle the lines initially to turn off the LEDs.
    // The values driven are as required by the LEDs on the LP.
    //
    AllLEDOFF();

    // 001
    MAP_UtilsDelay(8000000);
    GPIOPinWrite(GPIOA1_BASE, 0x2, 0x2); // red on
    // 010
    MAP_UtilsDelay(8000000);
    GPIOPinWrite(GPIOA1_BASE, 0x4, 0x4); // orange on
    GPIOPinWrite(GPIOA1_BASE, 0x2, 0); // red off
    // 011
    MAP_UtilsDelay(8000000);
    GPIOPinWrite(GPIOA1_BASE, 0x2, 0x2); // red on
    // 100
    MAP_UtilsDelay(8000000);
    GPIOPinWrite(GPIOA1_BASE, 0x2, 0); // red off
    GPIOPinWrite(GPIOA1_BASE, 0x4, 0); // orange off
    GPIOPinWrite(GPIOA1_BASE, 0x8, 0x8); // green on
    // 101
    MAP_UtilsDelay(8000000);
    GPIOPinWrite(GPIOA1_BASE, 0x2, 0x2); // red on
    // 110
    MAP_UtilsDelay(8000000);
    GPIOPinWrite(GPIOA1_BASE, 0x4, 0x4); // orange on
    GPIOPinWrite(GPIOA1_BASE, 0x2, 0); // red off
    // 111
    MAP_UtilsDelay(8000000);
    GPIOPinWrite(GPIOA1_BASE, 0x2, 0x2); // red on

}

void
LEDBlinkyRoutine()
{
    //
    // Toggle the lines initially to turn off the LEDs.
    // The values driven are as required by the LEDs on the LP.
    //
    AllLEDOFF();

    MAP_UtilsDelay(8000000);
    GPIOPinWrite(GPIOA1_BASE, 0x2, 0x2); // red on
    MAP_UtilsDelay(8000000);
    GPIOPinWrite(GPIOA1_BASE, 0x2, 0); // red off
    MAP_UtilsDelay(8000000);
    GPIOPinWrite(GPIOA1_BASE, 0x4, 0x4); // orange on
    MAP_UtilsDelay(8000000);
    GPIOPinWrite(GPIOA1_BASE, 0x4, 0); // orange off
    MAP_UtilsDelay(8000000);
    GPIOPinWrite(GPIOA1_BASE, 0x8, 0x8); // green on
    MAP_UtilsDelay(8000000);
    GPIOPinWrite(GPIOA1_BASE, 0x8, 0); // green off
}

void SetP18(int val)
{
    if (val)
        GPIOPinWrite(GPIOA3_BASE, 0x10, 0x10);
    else
        GPIOPinWrite(GPIOA3_BASE, 0x10, 0);
}
//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}
//****************************************************************************
//
//! Main function
//!
//! \param none
//! 
//! This function  
//!    1. Invokes the LEDBlinkyTask
//!
//! \return None.
//
//****************************************************************************
int
main()
{
    //
    // Initialize Board configurations
    //
    BoardInit();
    
    //
    // Power on the corresponding GPIO port B for 9,10,11.
    // Set up the GPIO lines to mode 0 (GPIO)
    //
    PinMuxConfig();
    GPIO_IF_LedConfigure(LED1|LED2|LED3);

    GPIO_IF_LedOff(MCU_ALL_LED_IND);
    

    //
    // Initializing the Terminal.
    //
    InitTerm();
    //
    // Clearing the Terminal.
    //
    ClearTerm();

    Message("\t\t*************************************************\n\r");
    Message("\t\tCC3200 GPIO Application\n\r");
    Message("\t\t*************************************************\n\r");
    Message("\t\t****************************************************\n\r") ;
    Message("\t\tPush SW3 to start LED binary counting\n\r");
    Message("\t\tPush SW2 to blink LEDs on and off\n\r");
    Message("\t\t****************************************************\n\r");
    Message("\n\n\n\r");
    //
    // Start the LEDBlinkyRoutine
    //
    int state = 0; // 0 = initialize state
    while(1) {
        int sw2 = GPIOPinRead(GPIOA2_BASE, 0x40);
        int sw3 = GPIOPinRead(GPIOA1_BASE, 0x20);

        if (sw2) {
            if (state == 0 || state == 3) {
                // just switch show message
                Message("SW2 pressed\n\r");
            }
            LEDBlinkyRoutine();
            SetP18(1);
            state = 2;
        }
        if (sw3) {
            if (state == 0 || state == 2) {
                // just switch show message
                Message("SW3 pressed\n\r");
            }
            LEDCountRoutine();
            SetP18(0);
            state = 3;
        }
    }

    return 0;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
