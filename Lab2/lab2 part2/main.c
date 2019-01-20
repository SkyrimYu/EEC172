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
// Application Name     - I2C 
// Application Overview - The objective of this application is act as an I2C 
//                        diagnostic tool. The demo application is a generic 
//                        implementation that allows the user to communicate 
//                        with any I2C device over the lines. 
// Application Details  -
// http://processors.wiki.ti.com/index.php/CC32xx_I2C_Application
// or
// docs\examples\CC32xx_I2C_Application.pdf
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup i2c_demo
//! @{
//
//*****************************************************************************

// Standard includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "uart.h"
#include "spi.h"

// Common interface includes
#include "uart_if.h"
#include "i2c_if.h"

#include "pin_mux_config.h"
#include "Adafruit_GFX.h"
#include "test.h"


//*****************************************************************************
//                      MACRO DEFINITIONS
//*****************************************************************************
#define APPLICATION_VERSION     "1.1.1"
#define APP_NAME                "CC3200 I2C"
#define UART_PRINT              Report
#define FOREVER                 1
#define CONSOLE                 UARTA0_BASE
#define FAILURE                 -1
#define SUCCESS                 0
#define SPI_IF_BIT_RATE  100000
#define TR_BUFF_SIZE     100
#define OC  0x80
#define RETERR_IF_TRUE(condition) {if(condition) return FAILURE;}
#define RET_IF_ERR(Func)          {int iRetVal = (Func); \
                                   if (SUCCESS != iRetVal) \
                                     return  iRetVal;}

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


//****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS                          
//****************************************************************************

//*****************************************************************************
//
//! Display a prompt for the user to enter command
//!
//! \param  none
//!
//! \return none
//! 
//*****************************************************************************
void 
DisplayPrompt()
{
    UART_PRINT("\n\rcmd#");
}


//*****************************************************************************
//
//! Display the buffer contents over I2C
//!
//! \param  pucDataBuf is the pointer to the data store to be displayed
//! \param  ucLen is the length of the data to be displayed
//!
//! \return none
//! 
//*****************************************************************************
void 
DisplayBuffer(unsigned char *pucDataBuf, unsigned char ucLen)
{
    unsigned char ucBufIndx = 0;
    UART_PRINT("Read contents");
    UART_PRINT("\n\r");
    while(ucBufIndx < ucLen)
    {
        UART_PRINT(" 0x%x, ", pucDataBuf[ucBufIndx]);
        ucBufIndx++;
        if((ucBufIndx % 8) == 0)
        {
            UART_PRINT("\n\r");
        }
    }
    UART_PRINT("\n\r");
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
static void
DisplayBanner(char * AppName)
{

    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t      CC3200 %s Application       \n\r", AppName);
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

//*****************************************************************************
//
//! Main function handling the I2C example
//!
//! \param  None
//!
//! \return None
//! 
//*****************************************************************************
void main()
{
    //
    // Initialize board configurations
    //
    BoardInit();
    
    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();
    
    //
    // Configuring UART
    //
    InitTerm();
    
    //
    // Clearing the Terminal.
    //
    ClearTerm();

    //
    // I2C Init
    //
    I2C_IF_Open(I2C_MASTER_MODE_FST);
    
    //
    // Display the banner followed by the usage description
    //
    DisplayBanner(APP_NAME);

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
    MAP_GPIOPinWrite(GPIOA0_BASE, OC, OC);
    int cx = 64, cy = 64; //  start with middle
    unsigned char x = 0x05, y = 0x03, xVal, yVal;
    Adafruit_Init();
    fillScreen(BLACK);
    fillCircle(cx, cy, 2, CYAN);
    while(FOREVER)
    {
        //
        // Write the register address to be read from.
        // Stop bit implicitly assumed to be 0.
        //
        I2C_IF_Write(0x18, &x, 1, 0);

        //
        // Read the specified length of data
        //
        I2C_IF_Read(0x18, &xVal, 1);

        //
        // Write the register address to be read from.
        // Stop bit implicitly assumed to be 0.
        //
        I2C_IF_Write(0x18, &y, 1, 0);

        //
        // Read the specified length of data
        //
        I2C_IF_Read(0x18, &yVal, 1);

        int xSpeed = (int) xVal;
        int ySpeed = (int) yVal;
        if (xSpeed > 128)
            xSpeed -= 256;
        if (ySpeed > 128)
            ySpeed -= 256;

        fillCircle(cx, cy, 2, BLACK); // clear the circle
        cx += xSpeed * 0.05;
        cx = cx >= 123 ? 123 : cx;
        cx = cx <= 4 ? 4 : cx;
        cy += ySpeed * 0.05;
        cy = cy >= 123 ? 123 : cy;
        cy = cy <= 4 ? 4 : cy;

        fillCircle(cx, cy, 2, CYAN); // draw the new one

    }

    MAP_SPICSDisable(GSPI_BASE);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @
//
//*****************************************************************************


