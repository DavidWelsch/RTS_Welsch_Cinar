/* ========================================
 *
 * Copyright Ralf S. Mayer, 2017
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
#include <stdio.h>
#include <string.h>     // eg. strtok
#include <stdlib.h>     // eg. atoi, atol, atof
#include <unistd.h>
// own includes
#include "swsign.h"
#include "help.h"
#include "etCount.h"    // execution time counter

int gj;                 // a global var
float gf = 0.0;
double gd = 0.0;

/**
 * Control the User LED on board
 * @param sts 0: off, 1: on
 */
inline void usrLED( uint8 sts ) {
    Pin_USR_LED_Write( sts );
}

/**
 * \def USERLED_ONOFF
 * switch user LED on or off
 * @param x  1: on, 0: off
 */
#define USERLED_ONOFF(x) ( Pin_USR_LED_Write(x) )

// +++ globals +++
char buffer[128];       //!< buffer (global) for sprintf
uint8 _verboseFlag = 1; //!< verbosity flag

uint8 flagUART_RX = 0;          //!< flag (global) itr has occured
static char cRx = 0;            //!< caharcter from UART, visible within main.c

/** 
 * UART RX interrupt service routine.
 *
 * Collect a characters from UART into a global input variable.
 * @see cRx
 */
CY_ISR( MyIsrUartRX ) {
    cRx = UART_GetChar();     // copy char
}


int TestMethod1(int a, int b, int c){
    return (a+b+c+1+2+3);
}

int TestMethod2(int a, int b, int c, int d, int e, int f){
    return (a+b+c+d+e+f);
}
void TestMethod3(){
    ;
}

void BCETWCET(){
    uint32 max = 0, min = 2000000000, sum = 0;
    float avg = 0;
    uint32 cnt;
    for (int i = 0; i < 10000; i++) {
        CNT_START_RES;  // reset counter
        int x = 1;
        CNT_LAP_STOP_32( cnt );   // get counter value, stop counter
        if (cnt > max)
            max = cnt;
        if (cnt < min)
            min = cnt;
        
        sum = sum + cnt;
    }
    avg = sum / 10000;
    sprintf( buffer, "Store: min: %lu max: %lu avg: %f \n\r", min, max, avg );
    UART_PutString( buffer );
    for (int i = 0; i < 10000; i++) {
        int j = 50;
        CNT_START_RES;  // reset counter
        j = j*3;
        CNT_LAP_STOP_32( cnt );   // get counter value, stop counter
        if (cnt > max)
            max = cnt;
        if (cnt < min)
            min = cnt;
        
        sum = sum + cnt;
    }
    avg = sum / 10000;
    sprintf( buffer, "Mulitply: min: %lu max: %lu avg: %f\n\r", min, max, avg );
    UART_PutString( buffer );
    for (int i = 0; i < 10000; i++) {
        CNT_START_RES;  // reset counter
        TestMethod3();
        CNT_LAP_STOP_32( cnt );   // get counter value, stop counter
        if (cnt > max)
            max = cnt;
        if (cnt < min)
            min = cnt;
        
        sum = sum + cnt;
    }
    avg = (sum / 10000);
    sprintf( buffer, "Call Method: min: %lu max: %lu avg: %f\n\r", min, max, avg);
    UART_PutString( buffer );
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++   Candidates for new include and implementation file   ++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ------------------------------------------------------------------------------------

/**
 * Basic project with UART, RX-Interrupt, user-Button and -LED. With HW realization
 * of execution time counter. See the TopDesign and the macros in etCount.h
 * for FreeSoC2-5LP
 * 
 * Help and diverse functionalities
 * @see help.c
 *
 * @author David Welsch
 * @date 2018-05-24
 * @version 1.2
 */
int main()
{
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */   
    UART_Start();                       // start UART
    isr_UART_RX_StartEx( MyIsrUartRX ); // enable itr uart
    
    
    // +++ Initialize Counter and Registers +++
    Control_Reg_Write( CNT_DISABLE_RESET );  // reset, disable
    uint32 cnt, cnt1, cnt2,cnt3, cnt4;       // counter values
    
    CyGlobalIntEnable; /* Enable global interrupts. */

    sprintf( buffer, "\n\rWelcome! %s\n\r", _VERSTR_);
    UART_PutString( buffer );
    // help text
    prtHelp( UART_PutString );
        
    // +++ Test the execution time counter +++
    CNT_START_RES;      // reset and start counter
    CNT_LAP( cnt );     // get counter value
    CyDelay(1);         // test delay in ms
    CNT_LAP( cnt1 );    // get counter value
    CyDelay(2);         // test delay in ms
    CNT_LAP( cnt2 );    // get counter value
    CyDelay(5);         // test delay in ms
    CNT_LAP( cnt3 );    // get counter value
    CyDelay(10);        // test delay in ms
    CNT_LAP_STOP( cnt4 );   // get counter value, stop counter
    CNT_START_RES;      // reset and start counter
    sprintf( buffer, "Delay 0, 1, 2, 5, 10ms: %lu, %lu, %lu, %lu, %lu\n\r",
        cnt, cnt1, cnt2, cnt3, cnt4 );
    UART_PutString( buffer );
    CNT_LAP_STOP( cnt );   // get counter value, stop counter
    sprintf( buffer, "Time for 'printf' %lu\n\r", cnt );
    UART_PutString( buffer );
    // --- End Test the execution time counter ---
    
    char cInput = 0;        // UART RX char copy
    int i;
    volatile int vi;
    int lj;                 // a lokal var
    int test = 0;
    float lf = 0.0;
    double ld = 0.0;
    // Main loop
    for(;;)     // forever
    {
        /* Place your application code here. */
        // uart char received?
        if ( cRx != 0 ) {           // char received
            isr_UART_RX_Disable();  // disable Itr
            cInput = cRx;           // copy char
            cRx = 0;                // reset
            isr_UART_RX_Enable();   // enable Itr
        }

        if ( cInput != 0 ) {        // has input
            switch ( cInput ) {     // handle input
                case 'h':
                case 'H':
                    prtHelp( UART_PutString );
                    break;
                    
                case 'L':
                    usrLED( 1u );   // on
                    break;
                case 'l':
                    usrLED( 0u );   // off
                    break;
                    
                case 'I':
                    CNT_START_RES;  // reset counter
                    for ( i = 0; i < 10000; i++ );
                    CNT_LAP_STOP( cnt );   // get counter value, stop counter
                    sprintf( buffer, "Time for loop i 0...10000: %lu\n\r", cnt );
                    UART_PutString( buffer );
                    break;
                case 'i':
                    CNT_START_RES;  // reset counter
                    for ( vi = 0; vi < 10000; vi++ );
                    CNT_LAP_STOP( cnt );   // get counter value, stop counter
                    sprintf( buffer, "Time for loop vi 0...10000: %lu\n\r", cnt );
                    UART_PutString( buffer );
                    break;
                case 'J':
                    CNT_START_RES;  // reset counter
                    for ( i = 0; i < 10000; i++ ) {
                        gj = i;
                    }
                    CNT_LAP_STOP( cnt );   // get counter value, stop counter
                    sprintf( buffer, "Time for loop global=i 0...10000: %lu\n\r", cnt );
                    UART_PutString( buffer );
                    break;
                case 'j':
                    CNT_START_RES;  // reset counter
                    for ( i = 0; i < 10000; i++ ) {
                        lj = i;
                    }
                    CNT_LAP_STOP( cnt );   // get counter value, stop counter
                    sprintf( buffer, "Time for loop local=i 0...10000: %lu\n\r", cnt );
                    UART_PutString( buffer );
                    break;
                    
                case 'k':
                    CNT_START_RES;  // reset counter
                    for ( i = 0; i < 1000000; i++ ) {
                        lj = i;
                    }
                    CNT_LAP_STOP_32( cnt );   // get counter value, stop counter
                    sprintf( buffer, "Time for loop local=i 0...1000000: %lu\n\r", cnt );
                    UART_PutString( buffer );
                    break;
                case 'K':
                    CNT_START_RES;  // reset counter
                    test = 1;
                    break;
                case 'm':
                    CNT_LAP_STOP_32( cnt );   // get counter value, stop counter
                    sprintf( buffer, "Time complete: %lu\n\r", cnt );
                    UART_PutString( buffer );
                    test = 0;
                    break;
                case 'b':
                    CNT_START_RES;  // reset counter
                    if (1 == 2){
                        ld = 0.2;
                    }
                    else
                    {
                        ld = 0.4;
                    }
                    CNT_LAP_STOP_32( cnt );   // get counter value, stop counter
                    sprintf( buffer, "If/Else: %lu\n\r", cnt );
                    UART_PutString( buffer );
                    CNT_START_RES;  // reset counter
                    for (int i = 0; i < 1; i++)
                    {
                        ld = 0.2;
                    }
                    CNT_LAP_STOP_32( cnt );   // get counter value, stop counter
                    sprintf( buffer, "For: %lu\n\r", cnt );
                    UART_PutString( buffer );
                    break;
                case 'f':
                    CNT_START_RES;  // reset counter
                    lf = (float)(0.2);
                    CNT_LAP_STOP_32( cnt );   // get counter value, stop counter
                    CNT_START_RES;  // reset counter
                    gf = (float)(0.2);
                    CNT_LAP_STOP_32( cnt1 );   // get counter value, stop counter
                    sprintf( buffer, "Time for Float: %lu,%lu\n\r", cnt, cnt1 );
                    UART_PutString( buffer );
                    CNT_START_RES;  // reset counter
                    ld = (double)(0.2);
                    CNT_LAP_STOP_32( cnt );   // get counter value, stop counter
                    CNT_START_RES;  // reset counter
                    gd = (double)(0.2);
                    CNT_LAP_STOP_32( cnt1 );   // get counter value, stop counter
                    sprintf( buffer, "Time for Double: %lu,%lu\n\r", cnt, cnt1 );
                    UART_PutString( buffer );
                    break;
                case 'c':
                    CNT_START_RES;  // reset counter
                    int x = TestMethod1(1,2,3);
                    CNT_LAP_STOP_32( cnt );   // get counter value, stop counter
                    sprintf( buffer, "Methode mit 3 Params: %lu\n\r", cnt );
                    UART_PutString( buffer );
                    CNT_START_RES;  // reset counter
                    x = TestMethod2(1,2,3,1,2,3);
                    CNT_LAP_STOP_32( cnt );   // get counter value, stop counter
                    sprintf( buffer, "Methode mit 6 Params: %lu\n\r", cnt );
                    UART_PutString( buffer );
                    CNT_START_RES;  // reset counter
                    x = 1+2+3+1+2+3;
                    CNT_LAP_STOP_32( cnt );   // get counter value, stop counter
                    sprintf( buffer, "Inline: %lu\n\r", cnt );
                    UART_PutString( buffer );
                    break;
                case 'e':
                    BCETWCET();
                    break;
                default:    // repeat char
                    UART_PutChar( cInput );
                    break;
            } // end switch
            cInput = 0;             // reset, don't forget!
        }
        
        if (test > 0) {
            CNT_LAP_WORES_32( cnt );   // get counter value, stop counter
            sprintf( buffer, "Loop...: %lu\n\r", cnt );
            UART_PutString( buffer );
            CyDelay(1000);
        }
        
        // for Test Only!
        if ( !Pin_USR_SW1_Read() ) { // Button pressed
            usrLED( 1u );   // on
            CyDelay( 50 );
            usrLED( 0u );   // off          
        }
            
    } // end for ever
} // end main

/* [] END OF FILE */
