/* =========================================================
 * TaskError.h
 *
 *  Created on: 2016/09/05
 *      Author: t.miki
 *
 *  Copyright (C) 2016 Telepower .inc
 * ========================================================= */

#ifndef MPI_WIFI_SRC_TASK_TASKERROR_H_
#define MPI_WIFI_SRC_TASK_TASKERROR_H_

// =========================================================
// ----- 共通エラー番号
typedef enum {
	COMMON_ERROR_NONE = 0x00,		// 0x00 : エラー無し
	// MainRoutine
	COMMON_ERROR_MAIN_0 = 0x10,		// 0x10 : 無線送信タスク生成失敗
	COMMON_ERROR_MAIN_1,			// 0x11 : 無線受信タスク生成失敗
	COMMON_ERROR_MAIN_2,			// 0x12 : ネットワーク層タスク生成失敗
	// タイマ制御 Task
	COMMON_ERROR_OSTIM_0 = 0x20,	// 0x20 : タイマ生成失敗
	COMMON_ERROR_OSTIM_1,			// 0x21 : タイマ起動失敗
	COMMON_ERROR_OSTIM_2,			// 0x22 : タイマ停止失敗

	// 無線受信 Task
	COMMON_ERROR_CCRCV_0 = 0x30,	// 0x30 : 初期化失敗
	COMMON_ERROR_CCRCV_1,			// 0x31 : メッセージ受信失敗
	COMMON_ERROR_CCRCV_2,			// 0x32 : メッセージ送信失敗
	COMMON_ERROR_CCRCV_3,			// 0x33 : コールバック関数未設定
	COMMON_ERROR_CCRCV_4,			// 0x34 : セマフォ破棄失敗

	// 無線送信 Task
	COMMON_ERROR_CCSND_0 = 0x40,	// 0x40 : 初期化失敗
	COMMON_ERROR_CCSND_1,			// 0x41 : メッセージ受信失敗
	COMMON_ERROR_CCSND_2,			// 0x42 : メッセージ送信失敗
	COMMON_ERROR_CCSND_3,			// 0x43 : セマフォ生成失敗
	COMMON_ERROR_CCSND_4,			// 0x44 : セマフォ破棄失敗

	// Ctrl層 Task
	COMMON_ERROR_CTRL_0 = 0x50,		// 0x50 : 初期化失敗
	COMMON_ERROR_CTRL_1,			// 0x51 : メッセージ受信失敗
	COMMON_ERROR_CTRL_2,			// 0x52 : メッセージ送信失敗

	// SPI
	COMMON_ERROR_SPI_0 = 0x60,		// 0x60 : 初期化失敗
	COMMON_ERROR_SPI_1,				// 0x61 : 受信タスク キュー生成失敗
	COMMON_ERROR_SPI_2,				// 0x62 : 受信タスク メモリーアロケーションエラー
	COMMON_ERROR_SPI_3,				// 0x63 : 受信タスク メッセージ受信失敗
	COMMON_ERROR_SPI_4,				// 0x64 : 受信タスク メッセージ送信失敗
	COMMON_ERROR_SPI_5,				// 0x65 : 送信タスク メッセージ受信失敗
	COMMON_ERROR_SPI_6,				// 0x66 : 送信タスク メッセージ送信失敗

	// WireFrame
	COMMON_ERROR_WIRE_0 = 0x70,		// 0x70 : 初期化失敗
	COMMON_ERROR_WIRE_1,			// 0x71 : メッセージ受信失敗
	COMMON_ERROR_WIRE_2,			// 0x72 : メッセージ送信失敗

	// AirFrame
	COMMON_ERROR_AIR_0 = 0x80,		// 0x80 : 初期化失敗
	COMMON_ERROR_AIR_1,				// 0x81 : メッセージ受信失敗
	COMMON_ERROR_AIR_2,				// 0x82 : メッセージ送信失敗

	// stack overflow
	COMMON_ERROR_STACK = 0xf8,		// 0xf8 : スタックオーバフロー
	// For PanelBord
	COMMON_ERROR_RESERVE1 = 0xff	// 未使用 : 予約

} eCommonError;

// =========================================================
// Application specific status/error codes
typedef enum{
     _ERROIR_BASE = 0x7D0,
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
	TASK_CREATE_ERROR = _ERROIR_BASE -1 ,

	SOCKET_CREATE_ERROR = _ERROIR_BASE -2 ,
    BIND_ERROR = _ERROIR_BASE -3 ,
    SEND_ERROR = _ERROIR_BASE -4,
    RECV_ERROR = _ERROIR_BASE -5,
    SOCKET_CLOSE = _ERROIR_BASE -6,
    DEVICE_NOT_IN_STATION_MODE = _ERROIR_BASE -7,

	AirFrameSemaphore_ERROR =  _ERROIR_BASE -10 ,
	WireFramSemaphore_ERROR =  _ERROIR_BASE -11 ,

	STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;


#endif /* MPI_WIFI_SRC_TASK_TASKERROR_H_ */
// =========================================================
// - end of file
// =========================================================
