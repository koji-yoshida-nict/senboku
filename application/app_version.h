/*
 * app_version.h
 *
 *  Created on: 2016/11/26
 *      Author: t.miki
 */

#ifndef APP_VERSION_H_
#define APP_VERSION_H_

//#define VERSION "1.07.00"
//#define VERSION "1.08.00"   // WiSUNとBLEがぶつかってしまった場合止まってしまう問題を修正    2017.05.22
//#define VERSION "1.09.00"   // 省電力機能の追加   2017.07.07    H.Ohuchi
//#define VERSION "2.00.00"   // イベント送信モードを追加 追加に伴いレジスタマップを変更
//#define VERSION "2.01.00"   // APPLICATION DATA 可変対応
//#define VERSION "2.02.00"   // 歩行走行モード毎にPANID・バースト時間・回数設定可能に変更 その他不具合対応
//#define VERSION "2.02.01"   // ダイナミックレンジが変更できない不具合を修正
//#define VERSION "2.03.00"   // ビーコン送信間隔を歩行と走行で別に判定するように修正(例：歩行から走行に切り替わると即時発信)
//#define VERSION "2.03.01"   // IEEE MACヘッダのMAC ADDRESSエンディアンを変更
//#define VERSION "3.00.00"   // TP より　NICT ソースコート提供　2019.9.17

// =====================================================
// #define VERSION "3.01.00"   // NICT #MOD_190917
                            // env_nictparam.deviceRunmode /DEVICE_RUNMODE_TIMER

// #define VERSION "3.02.00"   // ライフカウントモード
// #define VERSION "3.03.03"   // BLEも異常レベル（0x6a）に対応
// #define VERSION "3.04.00"       // 強制リブート追加
#define VERSION "3.04.08"       // 加速度検知アルゴリズム修正

#define _MOD_190917

// =====================================================
// VERSION "1.05.02"  fast release            2017/02/20
// VERSION "1.06.00"  Power Management        2017/02/21
//                    UART CTS/RTC
// VERSION "1.06.01"  burst mode              2017/02/22
//                    BST mode                2017/02/22
// VERSION "1.06.01a" overwrite Stop          2017/02/24
// VERSION "1.06.02"  BST Register            2017/02/22
#define _TP_BST_MODE
//
//                      smartrf_setting_ble.c
//                      smartrf_setting.c
//                      reg.c
//                      regcmd.h
//                      env.c
//                      env_nict.h
//
// =====================================================
// VERSION "1.06.22"  BST Register            2017/03/14
//                      uif_pkt.c bug  line 784, 800:
//                      WiSUN delay
//                      ble mac address set bug fix
#define _TP_V10622
//
// =====================================================
// VERSION "1.06.23"  NICT specification      2017/03/15
//                      Ble Queue
//                      WiSUN Queue
//                      dual Timer
//
#define _TP_V10631
//
//
// =====================================================
// VERSION "1.06.40"  POWER_MNG
//                      Power_mode 0: Low Power mode
//                      Power_mode 1: GPIO Control
//                      Power_mode 2: CTS/RTS Control
//                      Low power stop (1.9v) 50 count
// =====================================================
// =====================================================
// VERSION "1.07.00"  ADD PACKETDATA variable FLAG & VendorOUI
//
#define _TP_V10700
// param define

#define WISUN_HD_COUNT      5   // Bst count
#define WISUN_HD_TYPE       6   // Command type
#define WISUN_HD_LENG       7   // Payload length
#define WISUN_HD_SIZE       8   // Payload start point

#define BLE_DATA_HDR        2   // ble header size

#define configESY_HEAP_SIZE 1000
//#define configESY_HEAP_SIZE 8000
#define CONTAINER_MAX       16

//#define UART_baudRate       9600
//#define UART_baudRate       57600
#define UART_baudRate       115200

#ifdef _TP_DEBUG
#define UART_PRINTF(x)    System_printf(x)
#define DEBUG_PRINTF(x)   System_printf(x)
#define ABORT(x)          System_abort(x)
#else
#define UART_PRINTF(x)
#define DEBUG_PRINTF(x)
#define ABORT(x)
#endif

// =========================================================
// Heep size
//#define configESY_HEAP_SIZE (256*8)

#endif /* APP_VERSION_H_ */
