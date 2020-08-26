/* =========================================================
 * TaskEvent.h
 *
 *  Created on: 2016/09/18
 *      Author: t.miki
 *
 *  Copyright (C) 2016 Telepower .inc
 * =========================================================*/

#ifndef SRC_TASK_TASKEVENT_H_
#define SRC_TASK_TASKEVENT_H_

// =========================================================
// task event
typedef enum {
	// =====================================================
	// internal command
	Get_VersionInfo = 0x01 ,
	Get_W_WLAN_Beacon_Interval ,
	Set_W_WLAN_Beacon_Interval ,

	// =====================================================
	// Packet send Event
	W_TIMER_Socket_Ble_Send ,
	W_TIMER_Socket_WiSUN_Send ,
	//
	W_BUTTON_Socket_Ble_Send ,
	W_BUTTON_Socket_WiSUN_Send ,
	// =====================================================
	// Packet send
	W_WLAN_Socket_Send ,
	W_WLAN_Socket_Receive ,
	// =====================================================
	// sensor
	W_AK9750_INT,
	W_MMA8652FC_INT,

} eTaskEvent ;

#endif /* SRC_TASK_TASKEVENT_H_ */
// =========================================================
// - end of file
// =========================================================
