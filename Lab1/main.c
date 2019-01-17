//*****************************************************************************
//
// EEC 172 Lab #1
// Authors: Kevin Ren, Teresa Li
//
//*****************************************************************************

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
//! Turn off all the LEDS
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
void
AllLEDOFF()
{
    GPIOPinWrite(GPIOA1_BASE, 0x2, 0); // red off
    GPIOPinWrite(GPIOA1_BASE, 0x4, 0); // orange off
    GPIOPinWrite(GPIOA1_BASE, 0x8, 0); // green off
}

//*****************************************************************************
//
//! Perform the binary counting routine 
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
void
LEDCountRoutine()
{
    //
    // Toggle the lines initially to turn off the LEDs.
    // The values driven are as required by the LEDs on the LP.
    //

    // 000
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
    GPIOPinWrite(GPIOA1_BASE, 0x4, 0x4); // orange on
    GPIOPinWrite(GPIOA1_BASE, 0x8, 0x8); // green on
    MAP_UtilsDelay(8000000);
    AllLEDOFF();
}

//*****************************************************************************
//
//! Set P18 high or low
//!
//! \param val: indicate setting high or low
//!
//! \return None
//
//*****************************************************************************
void
SetP18(int val)
{
    if (val)
        GPIOPinWrite(GPIOA3_BASE, 0x10, 0x10);
    else
        GPIOPinWrite(GPIOA3_BASE, 0x10, 0);
}
//*****************************************************************************
//
//! Board Initialization & Configuration
//! \param  None
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
//!    1. Invokes the LEDBlinky and set P18 high when switch 2 pressed
//!    2. Invokes the LED Binary Counting Routine and set P18 low when switch 3 
//!       pressed
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

    int state = 0; // 0 = initialize state
    while(1)
    {
        int sw2 = GPIOPinRead(GPIOA2_BASE, 0x40);
        int sw3 = GPIOPinRead(GPIOA1_BASE, 0x20);

        if ((sw2 & 0x40) != 0) // switch 2 pushed
        {
            if (state == 0 || state == 3)
            {
                // just switch, show message
                Message("SW2 pressed\n\r");
            }
            LEDBlinkyRoutine();
            SetP18(1); // set high
            state = 2;
        }
        else if ((sw3 & 0x20) != 0) // switch 3 pushed
        {
            if (state == 0 || state == 2) {
                // just switch, show message
                Message("SW3 pressed\n\r");
            }
            LEDCountRoutine();
            SetP18(0); // set low
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
