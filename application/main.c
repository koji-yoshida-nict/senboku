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
 *  ======== main.c ========
 abcabcabc
 defdefdef
 */

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
#define  Power_SB_DISALLOW        PowerCC26XX_SD_DISALLOW
#define  Power_IDLE_PD_DISALLOW   PowerCC26XX_IDLE_PD_DISALLOW

#include <driverlib/vims.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>
#include <ti/drivers/UART.h>

/* Example/Board Header files */
#include "application/Board.h"

#define TASKSTACKSIZE     768

// ==========================================================
#define  _PUBLIC
#include "DataClass/container.h"
#include "TaskDefine.h"
#include "TaskInit.h"
#include "app_packet.h"
#include "app_power_mng.h"
#include "app_timer.h"

#include "mw/env.h"

extern int rfEasyLinkNpTaskInit(void);
extern void NodeRadioTask_init(void);
extern void dualRfTask_init(void);

#ifdef ADD_ACCLERATION_2018
extern void i2cTask_init(void);
#endif

int StartTimer(int timer, int tNumber) ;
// ==========================================================
//
void app_DefaultInit()
{
    rf_mode = RF_MODE_920 ;
    fecMode = FEC_MODE_CW ;

//    rf_rx_sem = NULL ;
#ifdef _TP_V10631
// =========================================================
// _TP_V10631
    // =========================================================
    wisun_seqno = 0 ;
    wisun_base_seqno = 0 ;
    wisun_base_period = 100 ;

    ble_seqno = 0;
    ble_burst_mode = 0 ;
    wisun_burst_mode = 0 ;

#else
    msgQueus_sem = NULL ;       // magQue Semaphore
//    ovw_address = 1 ;       // MAC address over write
#endif
    // =========================================================
    // バースモード対応
#ifdef _TP_BST_MODE
#if 1
    wisun_interval = 50;
    wisun_burst_period = 10;
    wisun_burst_count = 1;
    ble_interval = 10;
    ble_burst_period = 10;
    ble_burst_count = 1;
#else
    g_bst_modeCount = 0;     // bst mode Count
    g_bst_blemodeCount = 0;  // bst blemode Count
//    g_bst_modeInterval = 30;  // bst mode Interval
#endif
#endif
    // =========================================================
    // Power Manage mode
    g_Power_mode = Power_mode ;  //

    // ---------------------------------------------------------
    // NICT #MOD_190917
    g_LifeCounter = 0;         // Life Count initial

    g_DeathCounter = 0;           // Death Count initial
    g_DeathCount_increment = 0;

    g_SublifeCounter = 0;         // Sub-life count initial
    g_SublifeCount_increment = 0;
}
// ==========================================================
// check GPIO power mode
void check_Powermode(int mode)
{
#ifdef _TP_POWER_MNG
    int val ;
    PowerCtrlInit();

    val = PowerCtrlCheck();
    if(!val){
        // ==========================================================
        // set low power mode
        g_Power_mode = LowPower_mode ;
        GoSleep();
        PowerCtrlClose();
    }else{
        if(mode)
            PowerCtrlClose();
    }
#endif
}


// ==========================================================
// application env setting
void app_EnvInit()
{
    // update env param
    memcpy(wisun_localName, env_nictparam.WiSUNPayloadName, sizeof(wisun_localName));
    memcpy(ble_localName, env_nictparam.blePayloadName, sizeof(ble_localName));

    memcpy(wisun_mac, env_nictparam.wiSunAddress, sizeof(wisun_mac));
#ifdef _TP_V10700
    wisun_flag = env_nictparam.WiSUNPayloadFlag;
    vendorOUI[0] = env_nictparam.vendor1;
    vendorOUI[1] = env_nictparam.vendor2;
    vendorOUI[2] = env_nictparam.vendor3;
#endif
    memcpy(ble_mac,env_nictparam.bleAddress, sizeof(ble_mac));
#if 1
    wisun_interval = env_nictparam.WiSUNBaseInterval;
    wisun_burst_period = env_nictparam.WiSUNBurstPeriod;
    wisun_burst_count = env_nictparam.WiSUNBurstCount;
    ble_interval = env_nictparam.bleBaseInterval;
    ble_burst_period = env_nictparam.bleBurstPeriod;
    ble_burst_count = env_nictparam.bleBurstCount;
#else
    g_bst_modeCount = env_nictparam.WiSUNBurstCount;
    g_bst_blemodeCount = env_nictparam.bleburstCount;
#endif

    // DEBUG FLAG
    g_Debug_Flag = 0x00;
}

// ==========================================================
// application Timer event func
void app_TimerEventFunc(int time)
{
    extern void SendRf(int);
    static uint8_t bt_flag = 0;

    // =========================================================
    // Check power level
    if( PowerLevelCheck() )
    {
        bt_flag ++;
        if(bt_flag > POWER_MODE_COUNT )
        {
            g_Power_mode = PowerLow_LEVEL;
        }
    }
    else
    {
        bt_flag = 0;
    }

    // =========================================================
    // Batt level low to goto stop func
    if(g_Power_mode == PowerLow_LEVEL)
    {
        return;
    }

    // =========================================================
    // Do send packet
    if(time == BleBeaconType)
    {
        // =========================================================
        // Flag を立てる。立てたFlagは、実行時にdualRfTaskFunction にて解除
        //
        g_BeaconType |= BleBeaconType;
        if(ble_burst_mode)
        {
            g_BeaconType |= BSTmodeBleBeaconType;
        }
#if 0
                update_ble_beacon(ble_burst_mode);
#else   // 可変長対応
                updateBleBeaconData();
                ble_seqno++;
#endif
        SendRf(time);
    }
    else if(time == WisunBeaconType)
    {

        // =========================================================
        // Flag を立てる。 立てたフラグは実行時に dualRfTaskFunction にて解除
        //
        g_BeaconType |= WisunBeaconType;
        if(wisun_burst_mode)
        {
            g_BeaconType |= BSTmodeWisunBeaconType;
        }
#if 0
        update_wisun_beacon(wisun_burst_mode);
#else   // 可変長対応
        updateWiSUNBeaconData();
        wisun_base_seqno++;
#endif
        SendRf(time);
    }
}

/*
 *  ======== main ========
 */
int main(void)
{
    // ==========================================================
    /* Disable iCache prefetching */
    VIMSConfigure(VIMS_BASE, TRUE, FALSE);
    /* Disable cache */
    VIMSModeSet(VIMS_BASE, VIMS_MODE_OFF);

    // ==========================================================
    /* Call board init functions */
    Board_initGeneral();

    // ==========================================================
    // application init
    app_DefaultInit() ;

    // ==========================================================
    // Power Ctrl initialize
    check_Powermode(1);

    // 環境変数取込み
    {
        environ_read_nict();
    }

#ifdef ADD_ACCLERATION_2018
    // イベント送信モード
    if(env_nictparam.deviceRunmode & DEVICE_RUNMODE_EVENT)
//    if(env_nictparam.deviceRunmode != DEVICE_RUNMODE_TIMER)
// NICT #MOD_190917
    {
        // ==========================================================
        // I2C TASK for ACCELERATION
        i2cTask_init();
    }
    // 定期送信モード
    if(env_nictparam.deviceRunmode & DEVICE_RUNMODE_TIMER)
//    else
// NICT #MOD_190917
    {
      // ==========================================================
      // clock task create
      ClockTaskInit();
    }
#else
    // ==========================================================
    // clock task create
    ClockTaskInit();
#endif

    // ==========================================================
    // EsayLink Task
    if(g_Power_mode == Power_mode )
    {
        Board_initUART();
        rfEasyLinkNpTaskInit();
    }
    // ==========================================================
    // BLE & Wi-sun Beacon SEND TASK
    dualRfTask_init();
    // ==========================================================
    /* 起動してまず最初に1発ビーコン発信 */
    if((env_nictparam.beaconExist & WisunBeaconType) && env_nictparam.WiSUNBaseBeaconEn) {
      app_TimerEventFunc(WisunBeaconType);
    }
    if((env_nictparam.beaconExist & BleBeaconType) && env_nictparam.bleBaseBeaconEn) {
      app_TimerEventFunc(BleBeaconType);
    }
    

    /* Start BIOS */
    BIOS_start();


    return (0);
}

// =========================================================
// - end of file
// =========================================================
