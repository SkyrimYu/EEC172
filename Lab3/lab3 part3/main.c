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
#include "spi.h"
#include "rom.h"
#include "rom_map.h"
#include "pin.h"
#include "math.h"
#include "string.h"
#include "uart.h"

// Common interface includes
#include "uart_if.h"
#include "timer_if.h"
#include "pin_mux_config.h"
#include "Adafruit_GFX.h"


#define APPLICATION_VERSION     "1.1.1"
#define APP_NAME        "Board to Board Texting"

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
// Color definitions
#define BLACK           0x0000
#define BLUE            0x001F
#define GREEN           0x07E0
#define CYAN            0x07FF
#define RED             0xF800
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

#define SPI_IF_BIT_RATE  100000
#define TR_BUFF_SIZE     100

typedef struct PinSetting {
    unsigned long port;
    unsigned int pin;
} PinSetting;

typedef struct {
    char message[160];
    int index;
} Msg;

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
static char lastkey = '\0';
static int start = 0;
static char keySet[10][4][2] = {{"","","",""},// space for 0
                                {"","","",""}, // nothing for 1
                                {"A","B","C",""}, // 2
                                {"D","E","F",""}, // 3
                                {"G","H","I",""}, // 4
                                {"J","K","L",""}, // 5
                                {"M","N","O",""}, // 6
                                {"P","Q","R","S"},// 7
                                {"T","U","V",""}, // 8
                                {"W","X","Y","Z"} // 9
                        };
static int x = 0, y = 0;
static int keyBuffer[10] = {0,0,0,0,0,0,0,0,0,0};

static PinSetting OC = { .port = GPIOA3_BASE, .pin = 0x80};
static Msg message;

#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


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
    Report("\t\t  CC3200 %s Application \n\r", AppName);
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
}

void deleteChar(int x, int y) {
  fillRect(x, y, 6, 8, 0x0000);
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
    g_ulSamples[1] = TimerValueGet(TIMERA1_BASE, TIMER_A);
    TimerLoadSet(TIMERA1_BASE, TIMER_A, 0xffff);

    //
    // Get the samples and compute the frequency
    //
    g_ulSamples[0] = g_ulSamples[1];

    delta = g_ulSamples[1];
    //Report("Delta: %d\n\r", delta);
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


    i++;
    TimerIntClear(TIMERA1_BASE,TIMER_CAPA_EVENT);


}

static void TimerHandler()
{
    Timer_IF_InterruptClear(TIMERA0_BASE);
    //
    // Turn off the timer
    //
    Timer_IF_Stop(TIMERA0_BASE, TIMER_A);
    // reset the key, so the same key can be enter again
    lastkey = '\0';
    Report("RESET\n\r");
}

void Process(char key, int keyIndex, int numKeys)
{
    if (lastkey != key) {
         keyBuffer[keyIndex] = 0;
         if (start)
             message.index++;
         else
             start = 1;
     }
     if (keyBuffer[keyIndex] == numKeys) {
         keyBuffer[keyIndex] = 0;
         x -= 6;
         deleteChar(x, y);
         setCursor(x, y);
     }
     if (numKeys == 3) {
         switch (keyBuffer[keyIndex]) {
         case 0:
             Outstr(keySet[keyIndex][keyBuffer[keyIndex]]);
             message.message[message.index] = keySet[keyIndex][keyBuffer[keyIndex]][0];
             break;
         case 1:
         case 2:
             x -= 6;
             deleteChar(x, y);
             setCursor(x, y);
             Outstr(keySet[keyIndex][keyBuffer[keyIndex]]);
             message.message[message.index] = keySet[keyIndex][keyBuffer[keyIndex]][0];
             break;
         default:
             break;
         }
     }
     else {
         switch (keyBuffer[keyIndex]) {
         case 0:
             Outstr(keySet[keyIndex][keyBuffer[keyIndex]]);
             message.message[message.index] = keySet[keyIndex][keyBuffer[keyIndex]][0];
             break;
         case 1:
         case 2:
         case 3:
             x -= 6;
             deleteChar(x, y);
             setCursor(x, y);
             Outstr(keySet[keyIndex][keyBuffer[keyIndex]]);
             message.message[message.index] = keySet[keyIndex][keyBuffer[keyIndex]][0];
             break;
         default:
             break;
         }
     }
     keyBuffer[keyIndex]++;
     lastkey = key;
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
    message.index = 0;
    BoardInit();
    PinMuxConfig();

    MAP_PinConfigSet(PIN_02,PIN_TYPE_STD_PU,PIN_STRENGTH_6MA);
    MAP_IntPrioritySet(INT_TIMERA1A, INT_PRIORITY_LVL_1);
    MAP_TimerIntRegister(TIMERA1_BASE,TIMER_A,TimerIntHandler);
    MAP_TimerConfigure(TIMERA1_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME));
    MAP_TimerControlEvent(TIMERA1_BASE,TIMER_A,TIMER_EVENT_NEG_EDGE);
    MAP_TimerLoadSet(TIMERA1_BASE,TIMER_A,0xffff);
    MAP_TimerIntEnable(TIMERA1_BASE,TIMER_CAPA_EVENT);
    MAP_TimerEnable(TIMERA1_BASE,TIMER_A);


    // Configuring the timers
    //
    Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);

    //
    // Setup the interrupts for the timer timeouts.
    //
    Timer_IF_IntSetup(TIMERA0_BASE, TIMER_A, TimerHandler);

    ClearTerm();
    InitTerm();
    DisplayBanner(APP_NAME);

    //Enable and set up the UARTA1
    MAP_UARTEnable(UARTA1_BASE);
    MAP_UARTFIFOEnable(UARTA1_BASE);
    MAP_UARTConfigSetExpClk(UARTA1_BASE,MAP_PRCMPeripheralClockGet(PRCM_UARTA1),
                             UART_BAUD_RATE,
                             (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));


    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);

    //
    // Reset the peripheral
    //
    MAP_PRCMPeripheralReset(PRCM_GSPI);
    MAP_SPIReset(GSPI_BASE);

    //
    // Configure SPI interface
    //
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVEHIGH |
                     SPI_WL_8));

    //
    // Enable SPI for communication
    //
    MAP_SPIEnable(GSPI_BASE);
    MAP_SPICSEnable(GSPI_BASE); // Enables chip select
    MAP_GPIOPinWrite(OC.port, OC.pin, OC.pin);
    Adafruit_Init();
    fillScreen(BLACK);

    while (1) {
        if(!(detected && i >= 16)){
            continue;
        }

        sum = 0;
        for(k = 0; k < 16; k++){
            sum += (int)number[k]*pow(2, (15-k));
        }

        Timer_IF_Start(TIMERA0_BASE, TIMER_A, 1500);

        setCursor(x, y);

        switch(sum){
            case(BUTTON_ZERO):
                Outstr(" ");
                message.message[++message.index] = ' ';
                lastkey = '0';
                break;
            case(BUTTON_ONE):
                Report("1");
                lastkey = '1';
                break;
            case(BUTTON_TWO):
                Process('2', 2, 3);
                break;
            case(BUTTON_THREE):
                Process('3', 3, 3);
                break;
            case(BUTTON_FOUR):
                Process('4', 4, 3);
                break;
            case(BUTTON_FIVE):
                Process('5', 5, 3);
                break;
            case(BUTTON_SIX):
                Process('6', 6, 3);
                break;
            case(BUTTON_SEVEN):
                Process('7', 7, 4);
                break;
            case(BUTTON_EIGHT):
                Process('8', 8, 3);
                break;
            case(BUTTON_NINE):
                Process('9', 9, 4);
                break;
            case(BUTTON_LAST):
                Report("Delete\n\r");
                x -= 6;
                deleteChar(x, y);
                x -= 6;
                lastkey = 'd';
                message.message[message.index--] = '\0';
                break;
            case(BUTTON_MUTE):
                Report("Enter\n\r");
                lastkey = 'e';
                // print message for now
                message.message[++message.index] = '\0';
                Report("message: %s\n\r", message.message);
                int index;
                for (index = 0; index < message.index; index++) {
                    x -= 6;
                    deleteChar(x, y);
                }
                message.index = 0;
                message.message[message.index] = '\0';
                start = 0;
                x -= 6;
                break;
            default:
                Report("Unknown code %d\n\r", sum);
                x -= 6;
                lastkey = 'u';
                break;
        }
        Report("Pressed\n\r");
        i = 0;
        detected = 0;
        x += 6;
        Report("x: %d, y: %d\n\r", x, y);
        if (x > 122) {
            x = 0;
            y += 8;
        }
    }

    MAP_SPICSDisable(GSPI_BASE);
}
