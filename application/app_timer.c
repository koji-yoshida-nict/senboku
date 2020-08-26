/*
 * app_timer.c
 *
 *  Created on: 2017/03/18
 *      Author: t.miki
 */

// ==========================================================
// TIMER control
// ==========================================================

#include <stdint.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>

// Power management
//#include <ti/sysbios/family/arm/cc26xx/Power.h>
#include <ti/drivers/Gpio.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

#define Power_SB_DISALLOW       PowerCC26XX_SD_DISALLOW
#define Power_IDLE_PD_DISALLOW  PowerCC26XX_IDLE_PD_DISALLOW

#include <driverlib/vims.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>
#include <ti/drivers/UART.h>

/* Example/Board Header files */
#include "application/Board.h"

// =========================================================
#include "app_version.h"
#include "taskdefine.h"

// =========================================================
extern void app_TimerEventFunc(int time);

//
static Void clk0Fxn(UArg arg0);
#define MAX_TIMER 5
static Clock_Struct clk0Struct;
static Clock_Handle clk0Handle = NULL;

// =========================================================
// BLE,WiSUN でタイマーを分割
// H.Ohuchi 2017/05/11
static Clock_Struct clk1Struct;
static Clock_Handle clk1Handle = NULL;
// end  H.Ohuchi

// =========================================================
// _TP_V10631
#ifdef _TP_V10631
static Uint32 conutTimer[MAX_TIMER] = {0,0,0,0,0} ;
static Uint32 waitTimer[MAX_TIMER] = {0,0,0,0,0} ; //
#else
static Uint32 conutTimer = 0 ;
static Uint32 waitTimer = 1000 ; // 1000
#endif

// ==========================================================
// create timer

// ==========================================================
// timer
int StartTimer(int timer, int tNumber)
{
// =========================================================
// _TP_V10631
#ifdef _TP_V10631
    if(tNumber>0 && tNumber<MAX_TIMER )
    {
        conutTimer[tNumber] = 0;
        waitTimer[tNumber] = timer;
    }
#else
    conutTimer = 0;
    waitTimer = timer;
#endif
    if(!clk0Handle )
    {
        clk0Handle = Clock_handle(&clk0Struct);
        Clock_start(clk0Handle);
    }

    return (0);
}

/*
 *  ======== clk0Fxn =======
 */
static Void clk0Fxn(UArg arg0)
{
    uint32_t    time;
    uint8_t     flag = 0;
// =========================================================
// _TP_V10631
    for(time = 0; time < MAX_TIMER; time++)
    {
        if(waitTimer[time])
        {
            if( ++conutTimer[time] >= waitTimer[time] )
            {
                conutTimer[time] = 0;
// auto repeat
#if 1
                waitTimer[time] = 0;
#endif
                app_TimerEventFunc(time);
                flag = 1;
            }
        }
    }
}

// ==========================================================
// Clock Task initialize
//
int ClockTaskInit()
{
#if 0
    // ==========================================================
    /* semaphores for command and response */
    Semaphore_Params semParams;
    Semaphore_Params_init(&semParams);
    timer_sem = Semaphore_create(0, &semParams, NULL);
    rf_rx_sem = Semaphore_create(0, &semParams, NULL);
#endif
    // ==========================================================
    /* Construct BIOS Objects */
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = (Base_Timer*1000)/Clock_tickPeriod;
//    clkParams.startFlag = TRUE;
    clkParams.startFlag = FALSE;

    // ==========================================================
    /* Construct a periodic Clock Instance */
    //  1000/Clock_tickPeriod 1msec now 10msec
    Clock_construct(&clk0Struct, (Clock_FuncPtr)clk0Fxn, (Base_Timer*1000)/Clock_tickPeriod, &clkParams);

    return(0);
}

// =========================================================
// - end of file
// =========================================================
