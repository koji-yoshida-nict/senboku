/***** Includes *****/
#include <xdc/std.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

#if QUEUE_TEST
#include <ti/sysbios/knl/Queue.h>
#endif

//#define _LOC_DEB
/* Drivers */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/PIN.h>

/* Board Header files */
#include "application/Board.h"

#include <stdlib.h>
#include <driverlib/trng.h>
#include <driverlib/aon_batmon.h>

#include "application/RadioProtocol.h"
#include "application/TaskInit.h"
#include "application/app_packet.h"
#ifndef _NO_linkNp
#include "easylink/EasyLink.h"
#endif
#include "rf_includes/bleBeacon.h"
#include "rf_includes/wiSunBeacon.h"
#include "DataClass/container.h"

#include "mw/uif_pkt.h"
#include "mw/env.h"

#include "app_power_mng.h"
#include "taskdefine.h"
#include "taskInit.h"

/***** Defines *****/
#define DUAL_RF_TASK_STACK_SIZE     1024
#define DUAL_RF_TASK_PRIORITY       2

#define HD_TYPE                     WISUN_HD_TYPE
#define HD_LENG                     WISUN_HD_LENG
#define HD_SIZE                     WISUN_HD_SIZE

#define IEEE_ADDRESS_SIZE           6

#define WISUN_DATA_HDR              2
#define WISUN_MAC_HD_SRC_ADDRESS    (2+1+2+2)
#define WISUN_MAC_DATA_SRC_ADDRESS  (2+1+2+8+2+(2+3)+(4+2+1+2+2)+2)
//#define QUEUE_TEST 1

#if QUEUE_TEST
Queue_Handle msgQueue;
#endif

// =========================================================
//
static Task_Params dualRfTaskParamsB, dualRfTaskParamsW;
Task_Struct dualRfTaskB, dualRfTaskW; /* not static so you can see in ROV */
static uint8_t dualRfTaskStackB[DUAL_RF_TASK_STACK_SIZE];
static uint8_t dualRfTaskStackW[DUAL_RF_TASK_STACK_SIZE];

// Task Semaphore
Semaphore_Struct dualRfSemB, dualRfSemW, dualRfSemTasks; /* not static so you can see in ROV */
//static Semaphore_Handle dualRfSemHandle=NULL;

// Task Semaphore
Semaphore_Struct dualRfSem; /* not static so you can see in ROV */
static Semaphore_Handle dualRfSemHandleB = NULL;
static Semaphore_Handle dualRfSemHandleW = NULL;
static Semaphore_Handle dualRfSemHandleTasks = NULL;

//bleBeacon_Frame propAdvFrame;
bleBeacon_Frame localNameAdvFrame;
bleBeacon_Frame propAdvFrame;

// =========================================================
// BST mode Timer
extern int StartTimer(int timer, int timnumber);
//#define WISUN_SLEEP_DEV (17* (Base_Timer100/10))
#define WISUN_SLEEP_DEV (13* (Base_Timer100/10))             // 2017.05.19 H.Ohuchi 送信時間分誤差修正
#define BLE_SLEEP_DEV   (0* (Base_Timer100/10))
// =========================================================
// NICT specification
extern app_EnvInit();

// =========================================================
//
#define _IRAN

wiSunBeacon_Frame WisunAdvFrame;

// =========================================================
/***** Function definitions *****/
static void dualRfTaskFunctionB(UArg arg0, UArg arg1);
static void dualRfTaskFunctionW(UArg arg0, UArg arg1);

void dualRfTask_init(void)
{
    Semaphore_Params semParamB, semParamW, semParamTasks;

    // =========================================================
    /* Create semaphore used for callers to wait for result */
    Semaphore_Params_init(&semParamB);
    Semaphore_construct(&dualRfSemB, 0, &semParamB);
    dualRfSemHandleB = Semaphore_handle(&dualRfSemB);
    // =========================================================
    /* Create semaphore used for callers to wait for result */
    Semaphore_Params_init(&semParamW);
    Semaphore_construct(&dualRfSemW, 0, &semParamW);
    dualRfSemHandleW = Semaphore_handle(&dualRfSemW);
    // =========================================================
    /* Create semaphore used for tasks */
    Semaphore_Params_init(&semParamTasks);
    Semaphore_construct(&dualRfSemTasks, 0, &semParamTasks);
    dualRfSemHandleTasks = Semaphore_handle(&dualRfSemTasks);
    Semaphore_post(dualRfSemHandleTasks);					// まず初めにPost しておく

#if QUEUE_TEST
    // ========================================================
    /* Create the message queue */
    msgQueue = Queue_create(NULL,NULL);
#endif

    // =========================================================
    /* Create the radio protocol task */
    Task_Params_init(&dualRfTaskParamsB);
    dualRfTaskParamsB.stackSize = DUAL_RF_TASK_STACK_SIZE;
    dualRfTaskParamsB.priority = DUAL_RF_TASK_PRIORITY;
    dualRfTaskParamsB.stack = &dualRfTaskStackB;
    Task_construct(&dualRfTaskB, dualRfTaskFunctionB, &dualRfTaskParamsB, NULL);
    // =========================================================
    /* Create the radio protocol task */
    Task_Params_init(&dualRfTaskParamsW);
    dualRfTaskParamsW.stackSize = DUAL_RF_TASK_STACK_SIZE;
    dualRfTaskParamsW.priority = DUAL_RF_TASK_PRIORITY;
    dualRfTaskParamsW.stack = &dualRfTaskStackW;
    Task_construct(&dualRfTaskW, dualRfTaskFunctionW, &dualRfTaskParamsW, NULL);

    app_EnvInit();
}

// =========================================================
//
// Dual RF task function
//
static void dualRfTaskFunctionB(UArg arg0, UArg arg1)
{
// =========================================================
//
//  uint16_t cmdType = 0;
//    uint8_t tictack = 0;
    uint8_t *dbuf;
    uint8_t chan;
    uint8_t blp;
    uint8_t BeaconType;

//    bleBeacon_Status bs;
//    wiSunBeacon_Status ws;

    // 定期送信モード
    if(env_nictparam.deviceRunmode & DEVICE_RUNMODE_TIMER )
    //    if(env_nictparam.deviceRunmode == 0x00)
    // NICT #MOD_190917
    {
        if (env_nictparam.beaconExist & BleBeaconType)
        {
            if (env_nictparam.beaconExist & BSTmodeBleBeaconType)
            {
                ble_burst_mode = 1;
            }
            else
            {
                ble_burst_mode = 0;
            }
            if (env_nictparam.bleBaseBeaconEn)
            {
                // Ble enable
                // =========================================================
                // TODO: makeup data
#if 0
                update_ble_beacon(ble_burst_mode);
#else   // 可変長対応
                updateBleBeaconData();
#endif
                // =========================================================
                // Timer Start (100msec)
                StartTimer((ble_interval + BLE_TIME_OFFSET) * (100 / Base_Timer), BleBeaconType);    // n*10msec
                //StartTimer(env_nictparam.BleBaseInterval *(100/Base_Timer), BleBeaconType) ;    // n*10msec
            }
        }
    }
    /* Enter main task loop */
    while (1)
    {
        // =========================================================
        // wait Semaphore From NpTask
        Semaphore_pend(dualRfSemHandleB, BIOS_WAIT_FOREVER);
        Semaphore_pend(dualRfSemHandleTasks, BIOS_WAIT_FOREVER);     // タスク間排他処理

        BeaconType = g_BeaconType;
        Hwi_enable();
        //cmdType = g_BeaconType & (BleBeaconType | WisunBeaconType) ;
        // =========================================================
        // Payload set
        // =========================================================
        // ble type

        if (BeaconType & BleBeaconType)
        {
            bleBeacon_init(TRUE);
            // 定期送信モード
            if(env_nictparam.deviceRunmode & DEVICE_RUNMODE_TIMER )
            //    if(env_nictparam.deviceRunmode == 0x00)
            // NICT #MOD_190917
            {
                // =========================================================
                // Timer Start Next(100msec)
                // auto repeat ?
                StartTimer((ble_interval + BLE_TIME_OFFSET) * (100 / Base_Timer), BleBeaconType);    // n*10msec
            }
            Hwi_disable();
            g_BeaconType &= ~BleBeaconType;
            Hwi_enable();

            dbuf = &sendBle_dp[HD_SIZE];
            //bleBeacon_init(TRUE);
            //memcpy(ieeeAddr,dbuf,IEEE_ADDRESS_SIZE);
            //propAdvFrame.deviceAddress = (uint8_t*) ieeeAddr ;
            propAdvFrame.deviceAddress = (uint8_t*) &dbuf[BLE_DATA_HDR];
            // ble mac address set bug fix
            propAdvFrame.length = g_bledatalength - (IEEE_ADDRESS_SIZE + BLE_DATA_HDR); // data size
            propAdvFrame.pAdvData = &dbuf[IEEE_ADDRESS_SIZE + BLE_DATA_HDR]; // data address
            if (BeaconType & BSTmodeBleBeaconType)
            {
                // Start BST Timer
                g_BeaconType &= ~BSTmodeBleBeaconType;
#if 0
                for (blp = 0; blp < g_bst_blemodeCount; blp++)
#else
                for (blp = 0; blp < ble_burst_count; blp++)
#endif
                {
                    //advertisement
                    for (chan = 37; chan < 40; chan++)
                    {
                        bleBeacon_sendFrame(propAdvFrame, 1, (uint64_t) 1 << chan);
                    }
#if 0
                    Task_sleep(((uint32_t) env_nictparam.bleburstPeriod * Base_Timer100) + BLE_SLEEP_DEV);
#else
                    Task_sleep(((uint32_t) ble_burst_period * Base_Timer100) + BLE_SLEEP_DEV);
#endif
                }
            }
            else
            {
                //advertisement
                for (chan = 37; chan < 40; chan++)
                {
                    bleBeacon_sendFrame(propAdvFrame, 1, (uint64_t) 1 << chan);
                }
            }
            bleBeacon_close();
        }
        Semaphore_post(dualRfSemHandleTasks);
    }
}

// =========================================================
//
// Dual RF task function
//
static void dualRfTaskFunctionW(UArg arg0, UArg arg1)
{
    // =========================================================
    //
    //uint16_t cmdType = 0;
//    uint8_t tictack = 0;
    uint8_t *dbuf;
    uint8_t chan;
//    uint8_t blp;
    uint8_t BeaconType;
//    uint8_t wisunProcessTime = 0;

#define WISUN_ADDRESS_SIZE 8
//    static uint8_t ieeeAddr[WISUN_ADDRESS_SIZE];
    //static uint8_t _bst_modeEn = 0;  // bst mode

    // 定期送信モード
    if(env_nictparam.deviceRunmode & DEVICE_RUNMODE_TIMER )
    //    if(env_nictparam.deviceRunmode == 0x00)
    // NICT #MOD_190917
    {
        if (env_nictparam.beaconExist & WisunBeaconType)
        {
            if (env_nictparam.beaconExist & BSTmodeWisunBeaconType)
            {
                wisun_burst_mode = 1;
            }
            else
            {
                wisun_burst_mode = 0;
            }
            if (env_nictparam.WiSUNBaseBeaconEn)
            {
                // Wisun enable
                // =========================================================
                // TODO: makeup data
#if 0
                update_wisun_beacon(wisun_burst_mode);
#else           // 可変長対応
                updateWiSUNBeaconData();
#endif

                // =========================================================
                // Timer Start (100msec)
                StartTimer(wisun_interval * (100 / Base_Timer), WisunBeaconType);   // n*10msec
            }
        }
    }
    // =========================================================
    //
// =========================================================
// _TP_V10631
#ifdef _TP_V10631
#endif
    // =========================================================
    // _TP_V10631
    // =========================================================
    /* Enter main task loop */
    while (1)
    {
        // =========================================================
        // wait Semaphore From NpTask
        Semaphore_pend(dualRfSemHandleW, BIOS_WAIT_FOREVER);
        Semaphore_pend(dualRfSemHandleTasks, BIOS_WAIT_FOREVER);	// タスク間排他処理

        BeaconType = g_BeaconType;
        Hwi_enable();
        // =========================================================
        // Wisun type
        if (BeaconType & WisunBeaconType)
        {
            wiSunBeacon_init(TRUE);
            // =========================================================
            // Get queue data
            dbuf = &sendWiSUN_dp[HD_SIZE];
        }

        if (BeaconType & WisunBeaconType)
        {
            // VERSION "1.06.01a" overwrite Stop          2017/02/24
            //  if( ovw_address ){
            // =========================================================
            // _TP_V10631
            // 定期送信モード
            if(env_nictparam.deviceRunmode & DEVICE_RUNMODE_TIMER )
            //    if(env_nictparam.deviceRunmode == 0x00)
            // NICT #MOD_190917
            {
                // =========================================================
                // Timer Start Next(100msec)
                // auto repeat ?
                StartTimer(wisun_interval * (100 / Base_Timer),WisunBeaconType);   // n*10msec
            }
            Hwi_enable();
            g_BeaconType &= ~WisunBeaconType;
            Hwi_enable();

            WisunAdvFrame.length = g_WiSUNdatalength + WISUN_DATA_HDR; // data size
            WisunAdvFrame.pAdvData = dbuf;               // data address
            if (env_nictparam.wiSunCh1 != 0xff)
            {
                if (BeaconType & BSTmodeWisunBeaconType)
                {        // Start BST Timer
                    g_BeaconType &= ~BSTmodeWisunBeaconType;
                    // =========================================================
                    // _TP_V10631
#if 0
                    for (chan = 0; chan < g_bst_modeCount; chan++)
#else
                    for (chan = 0; chan < wisun_burst_count; chan++)
#endif
                    {
                        if (env_nictparam.burstCh == 0)
                        { // default
                            if (env_nictparam.wiSunCh1 != 0xff)
                            {
                                wiSunBeacon_init(TRUE);
                                wiSunBeacon_sendToFrame(&WisunAdvFrame, (uint32_t) env_nictparam.wiSunCh1);
                                wiSunBeacon_close();
                            }
                        }
                        else
                        {
                            if (env_nictparam.burstCh != 0xff)
                            {
                                wiSunBeacon_init(TRUE);
                                wiSunBeacon_sendToFrame(&WisunAdvFrame, (uint32_t) env_nictparam.burstCh);
                                wiSunBeacon_close();
                            }
                        }
#if 0
                        if ((env_nictparam.WiSUNBurstPeriod) >> 1 && (chan < g_bst_modeCount))
#else
                        if ((wisun_burst_period) >> 1 && (chan < wisun_burst_count))
#endif
                        {
                            // バースト発信間隔　Wait　　2017/3/13
#if 0
                            Task_sleep(((uint32_t) env_nictparam.WiSUNBurstPeriod * Base_Timer100) - WISUN_SLEEP_DEV);
#else
                            Task_sleep(((uint32_t) wisun_burst_period * Base_Timer100) - WISUN_SLEEP_DEV);
#endif
                        }
                    }
                }
                else
                {
                    if (env_nictparam.wiSunCh1 != 0xff)
                    {
                        wiSunBeacon_init(TRUE);
                        wiSunBeacon_sendToFrame(&WisunAdvFrame, (uint32_t) env_nictparam.wiSunCh1);
                        wiSunBeacon_close();
#if 0
                        if ((env_nictparam.WiSUNBurstPeriod >> 1) && ((env_nictparam.wiSunCh2 != 0xff) || (env_nictparam.wiSunCh3 != 0xff)))
#else
                        if ((wisun_burst_period >> 1) && ((env_nictparam.wiSunCh2 != 0xff) || (env_nictparam.wiSunCh3 != 0xff)))
#endif
                        {
                            // バースト発信間隔 Wait 2017/3/13
#if 0
                            Task_sleep(((uint32_t) env_nictparam.WiSUNBurstPeriod * Base_Timer100) - WISUN_SLEEP_DEV);
#else
                            Task_sleep(((uint32_t) wisun_burst_period * Base_Timer100) - WISUN_SLEEP_DEV);
#endif
                        }
                    }
                    if (env_nictparam.wiSunCh2 != 0xff)
                    {
                        wiSunBeacon_init(TRUE);
                        wiSunBeacon_sendToFrame(&WisunAdvFrame, (uint32_t) env_nictparam.wiSunCh2);
                        wiSunBeacon_close();
#if 0
                        if ((env_nictparam.WiSUNBurstPeriod >> 1) && (env_nictparam.wiSunCh3 != 0xff))
#else
                        if ((wisun_burst_period >> 1) && (env_nictparam.wiSunCh3 != 0xff))
#endif
                        {
                            // バースト発信間隔 Wait 2017/3/13
#if 0
                            Task_sleep(((uint32_t) env_nictparam.WiSUNBurstPeriod * Base_Timer100) - WISUN_SLEEP_DEV);
#else
                            Task_sleep(((uint32_t) wisun_burst_period * Base_Timer100) - WISUN_SLEEP_DEV);
#endif
                        }
                    }
                    if (env_nictparam.wiSunCh3 != 0xff)
                    {
                        wiSunBeacon_init(TRUE);
                        wiSunBeacon_sendToFrame(&WisunAdvFrame, (uint32_t) env_nictparam.wiSunCh3);
                        wiSunBeacon_close();
#if 0	// 最後は待つ必要がない　2017.07.01	H.Ohuchi
                        if( env_nictparam.burstPeriod >> 1 )
                        {
                            // バースト発信間隔　Wait　2017/3/13
                            Task_sleep(((uint32_t)env_nictparam.WiSUNBurstPeriod * Base_Timer100 ) - WISUN_SLEEP_DEV);
                        }
#endif
                    }
                }
            }
        }
        Semaphore_post(dualRfSemHandleTasks);
    }
}

void SendRf(int time)
{
    if (time == BleBeaconType)
    {
        Semaphore_post(dualRfSemHandleB);
    }
    else if (time == WisunBeaconType)
    {
        Semaphore_post(dualRfSemHandleW);
    }
}

// =========================================================
// - end of file
// =========================================================
