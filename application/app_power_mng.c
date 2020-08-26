/*
 * app_power_mng.c
 *
 *  Created on: 2017/03/18
 *      Author: t.miki
 */
// ==========================================================
// POWER Management
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

#define    Power_SB_DISALLOW        PowerCC26XX_SD_DISALLOW
#define    Power_IDLE_PD_DISALLOW   PowerCC26XX_IDLE_PD_DISALLOW

#include <driverlib/vims.h>
#include <driverlib/aon_batmon.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>
#include <ti/drivers/UART.h>

/* Example/Board Header files */
#include "application/Board.h"
#include "app_power_mng.h"
/*
 * Application Sleep control CTS/RTS pin configuration table:
 *   -
 */

#ifdef _TP_POWER_MNG
#define _LOCAL_PIN
#ifdef _LOCAL_PIN
static PIN_Config PinTable[] = {
    Board_POWER_MNG  | PIN_INPUT_EN | PIN_PULLDOWN | PIN_HYSTERESIS,          /* POWER Btn is active Hi          */
    Board_UART_CTS   | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_POSEDGE | PIN_HYSTERESIS,            /* CTS is active low          */
    Board_UART_RTS   | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_TEST_PIN   | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

/* Pin driver handle */
static PIN_Handle PinHandle=0;
static PIN_State  PinState;

// ==========================================================
//
static void CTS_PinIntr(PIN_Handle _PinHandle, PIN_Id _PinId)
{
    if(_PinId == Board_UART_CTS){
        WakeUp();
    }
}
#endif

// ==========================================================
// Power Up
#define RTS_HIGH (1)
#define RTS_LOW (!(RTS_HIGH))

int WakeUp()
{
    // ==========================================================
    // Power UP
    Power_setConstraint(Power_SB_DISALLOW);
    Power_setConstraint(Power_IDLE_PD_DISALLOW);

    // ==========================================================
    // RTS High
#ifdef _LOCAL_PIN
    PIN_setOutputValue(PinHandle, Board_UART_RTS, RTS_HIGH);
#else
    GPIO_write(Board_UART_RTS,RTS_HIGH)
#endif
    return 0;
}

void setTestPin(uint8_t state ) {
	PIN_setOutputValue(PinHandle, Board_TEST_PIN, state );
}
// ==========================================================
// Power Manage
int GoSleep()
{
    // ==========================================================
    //
    Power_releaseConstraint(Power_SB_DISALLOW);
    Power_releaseConstraint(Power_IDLE_PD_DISALLOW);

    // ==========================================================
    // RTS Low
#ifdef _LOCAL_PIN
    PIN_setOutputValue(PinHandle, Board_UART_RTS, RTS_LOW);
#else
    GPIO_write(Board_UART_RTS,RTS_LOW)
#endif
    return 0;
}

/*
 *  ======== gpioButtonFxn0 ========
 *  Callback function for the GPIO interrupt on Board_GPIO_BUTTON0.
 */
void gpioCTSFxn(uint_least8_t index)
{
    WakeUp();
}

// ==========================================================
// Power Ctrl initialize
int PowerCtrlInit()
{
    // ==========================================================
    // Power mon func on
    AONBatMonEnable();

#ifdef _LOCAL_PIN
    PinHandle = PIN_open(&PinState, PinTable);
    if(!PinHandle) {
//        System_abort("Error initializing board LED pins\n");
        return -1;
    }


//	PIN_setOutputValue(PinHandle, Board_TEST_PIN, 1 );



    /* Register Interrupt */
    PIN_registerIntCb(PinHandle, CTS_PinIntr);
    PIN_setInterrupt(PinHandle, Board_UART_CTS | PIN_IRQ_POSEDGE);

#else
    /* Call driver init functions */
    GPIO_init();

    /* install Button callback */
    GPIO_setCallback(Board_UART_CTS, gpioCTSFxn);

    /* Enable interrupts */
    GPIO_enableInt(Board_UART_CTS);

 #endif
    return 0;
}

// ==========================================================
// Power Ctrl close
int PowerCtrlClose()
{
    if(PinHandle)
        PIN_close(PinHandle);
    return 0;
}

#endif

// ==========================================================
//
int PowerCtrlCheck()
{
    int val ;
    val =  PIN_getInputValue(Board_POWER_MNG) ;

    return (val) ;
}

// ==========================================================
//
int PowerLevelCheck()
{
    uint32_t v;

    v = AONBatMonBatteryVoltageGet();
    v = ((v * 125) >> 5) ;
//    v = ((v * 125) >> 5) - 100;       // -100 (0.1v)　margin ?

    return (v < BATT_LOW_LEVEL);
}

// =========================================================
// - end of file
// =========================================================
