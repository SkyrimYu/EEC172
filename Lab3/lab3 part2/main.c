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
// Application Name     - Timer Count Capture
// Application Overview - This application showcases Timer's count capture 
//                        feature to measure frequency of an external signal.
// Application Details  -
// http://processors.wiki.ti.com/index.php/CC32xx_Timer_Count_Capture_Application
// or
// docs\examples\CC32xx_Timer_Count_Capture_Application.pdf
//
//*****************************************************************************

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "interrupt.h"
#include "prcm.h"
#include "gpio.h"
#include "utils.h"
#include "timer.h"
#include "rom.h"
#include "rom_map.h"
#include "pin.h"
#include "math.h"
#include "string.h"

// Common interface includes
#include "uart_if.h"

#include "pin_mux_config.h"


#define APPLICATION_VERSION     "1.1.1"
#define APP_NAME        "Decoding IR Transmissions"

#define BUTTON_ZERO     255
#define BUTTON_ONE      32895
#define BUTTON_TWO      16575
#define BUTTON_THREE    49215
#define BUTTON_FOUR     8415
#define BUTTON_FIVE     41055
#define BUTTON_SIX      24735
#define BUTTON_SEVEN    57375
#define BUTTON_EIGHT    4335
#define BUTTON_NINE     36975
#define BUTTON_LAST     765
#define BUTTON_MUTE     2295


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
static unsigned long g_ulSamples[2];
static int buffer[100];
static int number[100];
static int value = 1;
static int i = 0;
static int detected = 0;
static int delta = 0;
#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


int sameArray(int a[], int b[]) {
    int index;
    for (index = 0; index < 16; index++) {
        if (a[index] != b[index]) {
            return 0;
        }
    }
    return 1;
}

void findPattern()
{
    int pattern[16] = {0,0,0,0,
                       0,0,1,0,
                       1,1,1,1,
                       1,1,0,1};
    int index, find = 0;
    for (index = 0; index < i-16; index++) {
        if (sameArray(buffer+index, pattern)) {
            find = 1;
            break;
        }
    }
    if (find) {
    int j, k;
    for (k = 0, j = index + 16; j < i; j++, k++) {
        number[k] = buffer[j];
    }
    detected = 1;
    //Report("k: %d\n", k);
    i = k;
}
}


//*****************************************************************************
//
//! Timer interrupt handler
//
//*****************************************************************************
static void TimerIntHandler()
{

    //
    // Clear the interrupt
    //
    g_ulSamples[1] = MAP_TimerValueGet(TIMERA1_BASE,TIMER_A);
    TimerLoadSet(TIMERA1_BASE, TIMER_A,0xffff);

    //
    // Get the samples and compute the frequency
    //
    g_ulSamples[0] = g_ulSamples[1];

    delta = g_ulSamples[1];

    if(delta > 40000)
        value = 0;
    else
        value = 1;

    if (i >= 16 && !detected) {
        findPattern();
    }
    if (!detected)
        buffer[i] = value;
    else
        number[i] = value;
    //if (detected)
        //Report("i%d: %d\n\r", i, value);

    i++;
    MAP_TimerIntClear(TIMERA1_BASE,TIMER_CAPA_EVENT);


}

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void
DisplayBanner(char * AppName)
{

    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t\t  CC3200 %s Application       \n\r", AppName);
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
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
void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs) || defined(gcc)
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


//*****************************************************************************
//
//! Main  Function
//
//*****************************************************************************
int main()
{
    int k = 0;
    int sum = 0;
    BoardInit();
    PinMuxConfig();
    InitTerm();
    DisplayBanner(APP_NAME);


    MAP_PinConfigSet(PIN_02,PIN_TYPE_STD_PU,PIN_STRENGTH_6MA);
    MAP_TimerIntRegister(TIMERA1_BASE,TIMER_A,TimerIntHandler);
    MAP_TimerConfigure(TIMERA1_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME));
    MAP_TimerControlEvent(TIMERA1_BASE,TIMER_A,TIMER_EVENT_NEG_EDGE);
    MAP_TimerLoadSet(TIMERA1_BASE,TIMER_A,0xffff);
    MAP_TimerIntEnable(TIMERA1_BASE,TIMER_CAPA_EVENT);
    MAP_TimerEnable(TIMERA1_BASE,TIMER_A);

    while (1) {
        if(!(detected && i >= 16)){
            continue;
        }

        sum = 0;
        for(k = 0; k < 16; k++){
            sum += (int)number[k]*pow(2, (15-k));
        }

        Report("You pressed: ");
        switch(sum){
        case(BUTTON_ZERO):
            Report("0");
            break;
        case(BUTTON_ONE):
            Report("1");
            break;
        case(BUTTON_TWO):
            Report("2");
            break;
        case(BUTTON_THREE):
            Report("3");
            break;
        case(BUTTON_FOUR):
            Report("4");
            break;
        case(BUTTON_FIVE):
            Report("5");
            break;
        case(BUTTON_SIX):
            Report("6");
            break;
        case(BUTTON_SEVEN):
            Report("7");
            break;
        case(BUTTON_EIGHT):
            Report("8");
            break;
        case(BUTTON_NINE):
            Report("9");
            break;
        case(BUTTON_LAST):
            Report("LAST");
            break;
        case(BUTTON_MUTE):
            Report("MUTE");
            break;
        default:
            Report("Unknown code %d", sum);
            break;
        }
        Report("\n\r");
        i = 0;
        detected = 0;
    }


}
