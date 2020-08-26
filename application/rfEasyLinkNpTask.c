/*
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== empty_min.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/IHeap.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/Error.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>
#include <ti/drivers/UART.h>

/* Board Header files */
#include "application/Board.h"

/* EasyLink API Header files */
#ifndef _NO_linkNp
#include "easylink/EasyLink.h"
#endif
/* AT Header files */
#include "at/platform/inc/AtTerm.h"
#include "at/AtProcess.h"
#include "at/AtParams.h"

// =========================================================
// Application #includes
#include "application/dualRfTask.h"
#include "application/TaskEvent.h"
#include "application/TaskError.h"
#include "application/Taskinit.h"
#include "rf_includes/bleBeacon.h"
#include "DataClass/container.h"

#include "mw/uif_pkt.h"
#include "mw/reg.h"
#include "mw/fnet_i.h"
#include "mw/uif_pkt.h"

#include "rf_includes/wiSunBeacon.h"

// =========================================================
//

/*
 * Application LED pin configuration table:
 *   - All LEDs board LEDs are off.
 */

#ifdef CC1350_LAUNCHXL
PIN_Config pinTable[] = {
    Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_LED2 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};
/* Pin driver handle */
static PIN_Handle pinHandle;
static PIN_State pinState;
#else
PIN_Config pinTable[] = {
    PIN_TERMINATE
};
/* Pin driver handle */
static PIN_Handle pinHandle;
//static PIN_State pinState;
#endif

/***** Defines *****/
#define atnpTaskStackSize       1048
#define atnpTaskPriority        2

/***** Variable declarations *****/
static Task_Params atnpTaskParams;
Task_Struct atnpTask;    /* not static so you can see in ROV */
static uint8_t atnpTaskStack[atnpTaskStackSize];

bleBeacon_Frame propAdvFrame;
wiSunBeacon_Frame WisunAdvFrame;
bleBeacon_Frame localNameAdvFrame;

extern void uart_u2l_taskmain();

static void atnpFxn(UArg arg0, UArg arg1)
{
    static uint8_t _bst_modeEn = 0;  // bst mode

#define HD_TYPE WISUN_HD_TYPE
#define HD_LENG WISUN_HD_LENG
#define HD_SIZE WISUN_HD_SIZE

    AtTerm_init();
    AtParams_init(pinHandle, pinTable);

    while (1)
    {
        // =========================================================
        // wait Semaphore From RfTask
//        Semaphore_pend(msgQueus_sem, BIOS_WAIT_FOREVER);
        uart_u2l_taskmain();
        // =========================================================
        // バースモード対応
#ifdef _TP_BST_MODE
// =========================================================
// v1.06.31 対応
#ifdef _TP_V10631
        // =========================================================
        // Commnad type Ble Or Wisun
        g_BeaconType = que_dp[HD_TYPE];     // data type
        if( g_BeaconType & BleBeaconType ){
            // =========================================================
            // Get queue data
            memcpy(sendBle_dp,que_dp,sizeof(que_dp));
            g_bledatalength = que_dp[HD_LENG];     // data size
        }
        if( g_BeaconType & WisunBeaconType ){
            // =========================================================
            // Get queue data
            memcpy(sendWiSUN_dp,que_dp,sizeof(que_dp));
            g_WiSUNdatalength = que_dp[HD_LENG];     // data size
        }
        if(g_BeaconType & BSTmodeWisunBeaconType){
#else
        // Get queue data
        memcpy(send_dp,que_dp,sizeof(send_dp));
        //
        if(send_dp[WISUN_HD_TYPE] & BSTmodeBeaconYype){
#endif
            _bst_modeEn = TRUE ; // bst mode
        }else{
            _bst_modeEn = FALSE ;
        }
        if(_bst_modeEn){        // Start BST Timer
// =========================================================
// v1.06.31 対応  wisun
#ifdef _TP_V10631
            if( que_dp[WISUN_HD_COUNT] >= 1)
            {
#if 0
                g_bst_modeCount = que_dp[WISUN_HD_COUNT] ;
#endif
            }
#else
            if( send_dp[WISUN_HD_COUNT] >= 1){
                g_bst_modeCount = send_dp[WISUN_HD_COUNT] -1;
            }
#endif
        }
#else
//        memcpy(send_dp,que_dp,sizeof(send_dp));
#endif
        SendRf();   // post RF Semaphore
    }
}

/*
 *  ======== main task  ========
 */
int rfEasyLinkNpTaskInit(void)
{
    Semaphore_Params semParam;
    Semaphore_Params_init(&semParam);
// =========================================================
// _TP_V10631
#ifndef _TP_V10631
    // =========================================================
    /* Create semaphore used for callers to wait for result */
    Semaphore_construct(&semStruct, 0, &semParam);
    msgQueus_sem = Semaphore_handle(&semStruct);
#endif
    // =========================================================
    Task_Params_init(&atnpTaskParams);
    atnpTaskParams.stackSize = atnpTaskStackSize;
    atnpTaskParams.priority = atnpTaskPriority;
    atnpTaskParams.stack = &atnpTaskStack;
    atnpTaskParams.arg0 = (UInt) 1000000;

    Task_construct(&atnpTask, atnpFxn, &atnpTaskParams, NULL);

    return (0);
}

// =========================================================
// - end of file
// =========================================================
