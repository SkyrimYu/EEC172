// Standard includes
#include <string.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_ints.h"
#include "spi.h"
#include "rom.h"
#include "rom_map.h"
#include "utils.h"
#include "prcm.h"
#include "uart.h"
#include "gpio.h"
#include "interrupt.h"
#include "timer.h"

// Common interface includes
#include "Adafruit_GFX.h"
#include "uart_if.h"
#include "timer_if.h"
#include "pin_mux_config.h"

#define SPI_IF_BIT_RATE  400000
#define TR_BUFF_SIZE     100

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

#define BUTTON_ZERO     11
#define BUTTON_ONE      1
#define BUTTON_TWO      2
#define BUTTON_THREE    3
#define BUTTON_FOUR     4
#define BUTTON_FIVE     5
#define BUTTON_SIX      6
#define BUTTON_SEVEN    7
#define BUTTON_EIGHT    8
#define BUTTON_NINE     9
#define BUTTON_STAR     10
#define BUTTON_POUND    12

#define BLACK           0x0000
#define BLUE            0x001F
#define GREEN           0x07E0
#define CYAN            0x07FF
#define RED             0xF800
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

#define SAMPLE_SPACE 410
long int coeff_array[7] = {31548, 31281, 30951, 30556, 29144, 28361, 27409};
long int power_all[7];
unsigned long A0TICK = (80000000 / 16000);
unsigned short isSampling;
unsigned short isProcessing;
unsigned short sample_num;
signed long sample_buffer[SAMPLE_SPACE];
int new_digit = 0;
int num;

typedef struct PinSetting {
    unsigned long port;
    unsigned int pin;
} PinSetting;

typedef struct {
    char message[160];
    int index;
} Msg;

typedef struct {
    int x, y;
} Coordinate;

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
static char lastkey = '\0';
static int start = 0;

static char keySet[10][4][2] = {{" ","","",""},// space for 0
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
static int keyBuffer[10] = {0,0,0,0,0,0,0,0,0,0};

static PinSetting OC = {.port = GPIOA3_BASE, .pin = 0x80};
static Coordinate top = {.x = 0, .y = 0};
static Coordinate bot = {.x = 0, .y = 120};
static Msg message;
static Msg rmsg;


//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************



//*****************************************************************************

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


unsigned short readADC(void)
{
    unsigned char data0;
    unsigned char data1;
    GPIOPinWrite(GPIOA1_BASE, 0x1, 0x00);
    MAP_SPITransfer(GSPI_BASE, 0, &data0, 0x1, SPI_CS_ENABLE);
    MAP_SPITransfer(GSPI_BASE, 0, &data1, 0x1, SPI_CS_DISABLE);
    GPIOPinWrite(GPIOA1_BASE, 0x1, 0x1);
    unsigned short data = 0x1f & data0;
    data = (data << 5) | ((0xf8 & data1) >> 3);

    return data;
}


static void
TimerA0IntHandler(void)
{
    unsigned long ulStatus;
    ulStatus = MAP_TimerIntStatus(TIMERA0_BASE, true);
    MAP_TimerIntClear(TIMERA0_BASE, ulStatus);

    sample_num++;
    isSampling = 1;

    if (sample_num == SAMPLE_SPACE)
        isProcessing = 1;
}



long int goertzel(long int coeff)
{
    //initialize variables to be used in the function
    int Q, Q_prev, Q_prev2,i;
    long prod1,prod2,prod3,power;

    Q_prev = 0;         //set delay element1 Q_prev as zero
    Q_prev2 = 0;        //set delay element2 Q_prev2 as zero
    power=0;            //set power as zero

    for (i=0; i<SAMPLE_SPACE; i++) // loop SAMPLE_SPACE times and calculate Q, Q_prev, Q_prev2 at each iteration
        {
            Q = (sample_buffer[i]) + ((coeff* Q_prev)>>14) - (Q_prev2); // >>14 used as the coeff was used in Q15 format
            Q_prev2 = Q_prev;                                    // shuffle delay elements
            Q_prev = Q;
        }

    //calculate the three products used to calculate power
    prod1=((long) Q_prev*Q_prev);
    prod2=((long) Q_prev2*Q_prev2);
    prod3=((long) Q_prev *coeff)>>14;
    prod3=(prod3 * Q_prev2);

    power = ((prod1+prod2-prod3))>>8; //calculate power using the three products and scale the result down

    return power;
}

signed char decode(void) // post_test() function from the Github example
{
    //initialize variables to be used in the function
    int max_power,i, row, col;

    // find the maximum power in the row frequencies and the row number
    max_power=0;            //initialize max_power=0
    for(i=0;i<4;i++) {      //loop 4 times from 0>3 (the indecies of the rows)
        if (power_all[i] > max_power) { //if power of the current row frequency > max_power
            max_power=power_all[i];     //set max_power as the current row frequency
            row=i;                      //update row number
        }
    }

    // find the maximum power in the column frequencies and the column number
    max_power=0;            //initialize max_power=0
    for(i=4;i<7;i++) {      //loop 3 times from 4>7 (the indecies of the columns)
        if (power_all[i] > max_power) { //if power of the current column frequency > max_power
            max_power=power_all[i];     //set max_power as the current column frequency
            col=i;                      //update column number
        }
    }

    if(power_all[col]<=40000 && power_all[row]<=20000){
        new_digit = 1;
    }
    if((power_all[col]>400000 && power_all[row]>400000) && (new_digit == 1)) {
        new_digit = 0;
        return (3*row + col - 3);
    }

    return -1;
}



static void SPI_Init(void)
{
    // Enable the SPI module clock
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);

    // Reset the peripheral
    MAP_PRCMPeripheralReset(PRCM_GSPI);

    // Reset SPI
    MAP_SPIReset(GSPI_BASE);

    // Configure SPI interface
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                           SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                           (SPI_SW_CTRL_CS |
                           SPI_4PIN_MODE |
                           SPI_TURBO_OFF |
                           SPI_CS_ACTIVEHIGH |
                           SPI_WL_8));

    MAP_SPIEnable(GSPI_BASE);
}

static void ResetButton()
{
    Timer_IF_InterruptClear(TIMERA1_BASE);
    //
    // Turn off the timer
    //
    TimerDisable(TIMERA1_BASE, TIMER_A);
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
         top.x -= 6;
         deleteChar(top.x, top.y);
         setCursor(top.x, top.y);
     }
     if (numKeys == 3) {
         switch (keyBuffer[keyIndex]) {
         case 0:
             Outstr(keySet[keyIndex][keyBuffer[keyIndex]]);
             message.message[message.index] = keySet[keyIndex][keyBuffer[keyIndex]][0];
             break;
         case 1:
         case 2:
             top.x -= 6;
             deleteChar(top.x, top.y);
             setCursor(top.x, top.y);
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
             top.x -= 6;
             deleteChar(top.x, top.y);
             setCursor(top.x, top.y);
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

static void UARTIntHandler()
{
    TimerIntClear(TIMERA2_BASE, TIMER_A);
    TimerDisable(TIMERA2_BASE, TIMER_A);
    UARTIntClear(UARTA1_BASE, UART_INT_RX);
    Report("Interrupt\n\r");
    char tmp = UARTCharGet(UARTA1_BASE);
    rmsg.message[rmsg.index++] = tmp;
    TimerEnable(TIMERA2_BASE, TIMER_A);
}

static void PrintBottom()
{
    TimerIntClear(TIMERA2_BASE, TIMER_A);
    TimerDisable(TIMERA2_BASE, TIMER_A);
    Report("End\n\r");
    rmsg.message[rmsg.index] = '\0';
    Report("%s\n\r", rmsg.message);
    setCursor(bot.x, bot.y);
    fillRect(bot.x, bot.y, 128, 8, BLACK);
    Outstr(rmsg.message);
    rmsg.message[0] = '\0';
    rmsg.index = 0;
}

static void Timer_Init(void)
{
    unsigned long ulStatus;
    // Init Timer A0
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_TIMERA0);
    MAP_TimerConfigure(TIMERA0_BASE, TIMER_CFG_PERIODIC);
    MAP_TimerLoadSet(TIMERA0_BASE, TIMER_A, A0TICK);
    MAP_TimerIntRegister(TIMERA0_BASE, TIMER_A, TimerA0IntHandler);
    ulStatus = MAP_TimerIntStatus(TIMERA0_BASE, false);
    MAP_TimerIntClear(TIMERA0_BASE, ulStatus);

    // Configuring the timers
    //
    Timer_IF_Init(PRCM_TIMERA1, TIMERA1_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);
    Timer_IF_Init(PRCM_TIMERA2, TIMERA2_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);

    //
    // Setup the interrupts for the timer timeouts.
    //
    Timer_IF_IntSetup(TIMERA1_BASE, TIMER_A, ResetButton);
    Timer_IF_IntSetup(TIMERA2_BASE, TIMER_A, PrintBottom);


    TimerLoadSet(TIMERA1_BASE, TIMER_A, MILLISECONDS_TO_TICKS(1000));
    TimerLoadSet(TIMERA2_BASE, TIMER_A, MILLISECONDS_TO_TICKS(1000));

}


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
    fillRect(x, y, 6, 8, BLACK);
}


int main() {
    // Initializations
    BoardInit();
    PinMuxConfig();
    SPI_Init();
    Timer_Init();
    //Enable and set up the UARTA1
    MAP_UARTConfigSetExpClk(UARTA1_BASE,MAP_PRCMPeripheralClockGet(PRCM_UARTA1),
                             UART_BAUD_RATE,
                             (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
    MAP_UARTIntEnable(UARTA1_BASE, UART_INT_RX);
    MAP_UARTIntRegister(UARTA1_BASE, UARTIntHandler);
    MAP_UARTFIFODisable(UARTA1_BASE);
    ClearTerm();
    InitTerm();
    DisplayBanner("Lab 4");
    Adafruit_Init();
    fillScreen(BLACK);

    // variable setups
    unsigned long ulStatus;
    int i;

    isSampling = 0;
    isProcessing = 0;
    sample_num = 0;
    message.index = 0;

    MAP_TimerIntEnable(TIMERA0_BASE, TIMER_TIMA_TIMEOUT);
    MAP_TimerEnable(TIMERA0_BASE, TIMER_A);

    while(1) {
        if (isSampling == 1) {
            isSampling = 0;
            sample_buffer[sample_num-1] = ((signed long) readADC()) - 372;
        }
        if (isProcessing == 1) {
            // disable sampling timer
            MAP_TimerDisable(TIMERA0_BASE, TIMER_A);
            MAP_TimerIntDisable(TIMERA0_BASE, TIMER_TIMA_TIMEOUT);
            ulStatus = MAP_TimerIntStatus(TIMERA0_BASE, false);
            MAP_TimerIntClear(TIMERA0_BASE, ulStatus);
            //TimerDisable(TIMERA2_BASE, TIMER_A);

            // reset state variables
            isProcessing = 0;
            sample_num = 0;

            for (i = 0; i < 7; i++)
                power_all[i] = goertzel(coeff_array[i]); // call goertzel to calculate the power at each frequency and store it in the power_all array

            int num = decode();

            if (num > 0) {
                setCursor(top.x, top.y);
                TimerLoadSet(TIMERA1_BASE, TIMER_A, MILLISECONDS_TO_TICKS(1000));
                TimerEnable(TIMERA1_BASE, TIMER_TIMA_TIMEOUT);
                switch (num) {
                case BUTTON_ZERO:
                    Outstr(keySet[0][0]);
                    message.message[++message.index] = ' ';
                    lastkey = '0';
                    Report("%d\n\r", num);
                    break;
                case BUTTON_ONE:
                    Outstr(keySet[1][0]);
                    lastkey = '1';
                    Report("%d\n\r", num);
                    break;
                case BUTTON_TWO:
                    Process('2', 2, 3);
                    Report("%d\n\r", num);
                    break;
                case BUTTON_THREE:
                    Process('3', 3, 3);
                    Report("%d\n\r", num);
                    break;
                case BUTTON_FOUR:
                    Process('4', 4, 3);
                    Report("%d\n\r", num);
                    break;
                case BUTTON_FIVE:
                    Process('5', 5, 3);
                    Report("%d\n\r", num);
                    break;
                case BUTTON_SIX:
                    Process('6', 6, 3);
                    Report("%d\n\r", num);
                    break;
                case BUTTON_SEVEN:
                    Process('7', 7, 4);
                    Report("%d\n\r", num);
                    break;
                case BUTTON_EIGHT:
                    Process('8', 8, 3);
                    Report("%d\n\r", num);
                    break;
                case BUTTON_NINE:
                    Process('9', 9, 4);
                    Report("%d\n\r", num);
                    break;
                case BUTTON_STAR:
                    lastkey = '*';
                    top.x -= 6;
                    deleteChar(top.x, top.y);
                    top.x -= 6;
                    message.message[message.index--] = '\0';
                    Report("%d\n\r", num);
                    break;
                case BUTTON_POUND:
                    lastkey = '#';
                    Report("%d\n\r", num);
                    // print message for now
                    message.message[++message.index] = '\0';
                    Report("message: %s\n\r", message.message);
                    int index;
                    for (index = 0; index < message.index; index++) {
                        top.x -= 6;
                        deleteChar(top.x, top.y);
                        UARTCharPut(UARTA1_BASE, message.message[index]);
                    }
                    message.index = 0;
                    message.message[message.index] = '\0';

                    TimerDisable(TIMERA1_BASE, TIMER_A);
                    start = 0;
                    top.x -= 6;
                    break;
                    break;
                default:
                    lastkey = '\0';
                    top.x -= 6;
                    break;
                }
                 top.x += 6;
                 if (top.x > 122) {
                     top.x = 0;
                     top.y += 8;
                 }
            }

            // Re-enable sampling timer
            MAP_TimerLoadSet(TIMERA0_BASE, TIMER_A, A0TICK);
            MAP_TimerIntEnable(TIMERA0_BASE, TIMER_A);
            MAP_TimerEnable(TIMERA0_BASE, TIMER_TIMA_TIMEOUT);
        }
    }
    return 0;
}
