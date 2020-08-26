/* =========================================================
 * TaskInit.h
 *
 *  Created on: 2016/09/06
 *      Author: t.miki
 * ========================================================= */

#ifndef SRC_TASK_TASKINIT_H_
#define SRC_TASK_TASKINIT_H_
#include <stdint.h>
#include "app_version.h"
#include <xdc/runtime/Error.h>

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Queue.h>

#include "DataClass/container.h"
#include "TaskDefine.h"

// =========================================================
// Simplelink includes

#ifndef _PUBLIC
#define _PUBLIC extern
#endif

// =========================================================

// =========================================================
// タスク優先順位
//#define CTL_TASK_PRIORITY			(tskIDLE_PRIORITY + 2)	// 制御タスク
//#define UDP_SERVER_TASK_PRIORITY	(tskIDLE_PRIORITY + 2)	// UdpServerタスク

// =========================================================
// スタックサイズ
// TODO UART出力プログラムでスタックを食ってしまうので一時的にサイズをあげる

//#define CTL_TASK_STACK_SIZE				OSI_STACK_SIZE	// 制御タスク
//#define UDP_SERVER_TASK_STACK_SIZE		OSI_STACK_SIZE	// UdpServerタスク


// =========================================================
// Queue element
#ifdef _TP_V10631

typedef struct tag_msgQueue {
    Queue_Elem elem;
    uint8_t que_dp [CONTAINER_MESSAGE_LEN] ;
} msgQueue_, *msgQueue_t ;

#endif

// MODE TRANSITION
typedef enum
{
    MODE_TRANSITION_NONE        = 0x00,
    MODE_TRANSITION_TO_SLEEP,
    MODE_TRANSITION_TO_WALK,
    MODE_TRANSITION_TO_RUN,
    MODE_TRANSITION_NUBER
} eModeTransition;

typedef struct
{
    int8_t      xData;
    int8_t      yData;
    int8_t      zData;
    uint16_t    scalarData;
} accData;

// =========================================================
//
// =========================================================
// PUBLIC
// =========================================================

// Register
_PUBLIC unsigned long RGS_PORT_NUM ;
_PUBLIC unsigned long RGS_IP_ADDR ;
_PUBLIC unsigned char REG_WLAN_MODE ;

// =========================================================
//
_PUBLIC unsigned long   rf_mode ; // 0: 920Mhz 1:ble
_PUBLIC unsigned short  fecMode ; // 0: cw 1: GFSK 2: 2-GFSK

// =========================================================
// Control

_PUBLIC uint8_t task_sem;
//_PUBLIC Semaphore_Handle timer_sem;
//_PUBLIC Semaphore_Handle rf_rx_sem;

_PUBLIC Semaphore_Struct semStruct;
_PUBLIC Semaphore_Handle msgQueus_sem;

#if QUEUE_TEST
_PUBLIC Queue_Handle msgQueue;
_PUBLIC sendQueue_t  master_d[4] ;
_PUBLIC sendQueue_t* master_dp ;
#endif

_PUBLIC uint8_t que_dp[CONTAINER_MESSAGE_LEN];  // Queue Data
_PUBLIC Error_Block g_eb;

// =========================================================
// _TP_V10631
#ifdef _TP_V10631
#if 0
#define _QUEUE_MAX  3
_PUBLIC Queue_Handle bleQueue;
_PUBLIC msgQueue_    master_ble[_QUEUE_MAX] ;
_PUBLIC msgQueue_t   master_blep ;

_PUBLIC Queue_Handle wiSUNQueue;
_PUBLIC msgQueue_    master_wiSUN[_QUEUE_MAX] ;
_PUBLIC msgQueue_t   master_wiSUNp ;
#endif

_PUBLIC uint8_t g_BeaconType ;
_PUBLIC uint16_t g_bledatalength ;
_PUBLIC uint16_t g_WiSUNdatalength ;

_PUBLIC uint8_t sendBle_dp[CONTAINER_MESSAGE_LEN]; // send BlE Data
_PUBLIC uint8_t sendWiSUN_dp[CONTAINER_MESSAGE_LEN]; // send WiSUN Data

#else
_PUBLIC Semaphore_Handle msgQueus_sem;
_PUBLIC uint8_t send_dp[CONTAINER_MESSAGE_LEN]; // send Data
_PUBLIC uint8_t ovw_address;                    // アドレス　OverWrite

#endif

// =========================================================
// Power Control mode
_PUBLIC ePowerMode g_Power_mode ;
// DEBUG ACCELERATOR DATA UART SEND
_PUBLIC uint8_t g_Debug_Flag;
// =========================================================
// バースモード対応
#ifdef _TP_BST_MODE
#if 0
_PUBLIC uint8_t g_bst_modeCount;        // bst mode Count wisen
_PUBLIC uint8_t g_bst_blemodeCount ;    // bst mode Count wisen
#endif
//_PUBLIC uint8_t g_bst_modeInterval ;  // bst mode Interval
#endif

_PUBLIC uint8_t g_voltage;
_PUBLIC eModeTransition g_mode;
_PUBLIC accData g_accDataBuffer[255];

// ------------------------------------------------------------------------------
// NICT #MOD_190917
_PUBLIC uint16_t  g_LifeCounter;         // Life Count

_PUBLIC uint16_t g_DeathCounter;  // Death Count
_PUBLIC uint16_t g_DeathCount_increment; // PANIDのインクリメント分

_PUBLIC uint16_t g_SublifeCounter; // Sub-life count
_PUBLIC uint16_t g_SublifeCount_increment;


// =========================================================
// packet
//
#define MACADDR_SIZE        8
#define BLE_MACADDR_SIZE    6

_PUBLIC unsigned short      wisun_base_seqno;
_PUBLIC unsigned short      wisun_base_period;
_PUBLIC unsigned short      ble_seqno;
_PUBLIC unsigned char       wisun_localName[8];
_PUBLIC unsigned char       ble_localName[8];
#ifdef _TP_V10700
_PUBLIC uint16_t            wisun_flag;
_PUBLIC uint8_t             vendorOUI[3];
#endif
_PUBLIC unsigned char       wisun_seqno;
_PUBLIC unsigned char       wisun_mac[MACADDR_SIZE];
_PUBLIC unsigned char       ble_mac[BLE_MACADDR_SIZE];

_PUBLIC unsigned char       wisun_burst_mode;
_PUBLIC unsigned char       ble_burst_mode;

_PUBLIC uint16_t            wisun_interval;
_PUBLIC uint8_t             wisun_burst_period;
_PUBLIC uint8_t             wisun_burst_count;
_PUBLIC uint16_t            ble_interval;
_PUBLIC uint8_t             ble_burst_period;
_PUBLIC uint8_t             ble_burst_count;

#endif /* SRC_TASK_TASKINIT_H_ */
// =========================================================
// - end of file
// =========================================================
