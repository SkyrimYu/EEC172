//*****************************************************************************
//
// Application Name     - Lab 2
// Teresa Li and Kevin Ren
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
// Color definitions
#define BLACK           0x0000
#define BLUE            0x001F
#define GREEN           0x07E0
#define CYAN            0x07FF
#define RED             0xF800
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

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
    int lastx = 64, lasty = 64;
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

        cx += xSpeed * 0.05;
        cx = cx >= 123 ? 123 : cx;
        cx = cx <= 4 ? 4 : cx;
        cy += ySpeed * 0.05;
        cy = cy >= 123 ? 123 : cy;
        cy = cy <= 4 ? 4 : cy;

        //  move if coordinates changed
        if (lastx != cx || lasty != cy) {
            fillCircle(lastx, lasty, 2, BLACK); // clear the circle
            fillCircle(cx, cy, 2, CYAN); // draw the new one
        }

        lastx = cx;
        lasty = cy;

    }

    MAP_SPICSDisable(GSPI_BASE);
}


