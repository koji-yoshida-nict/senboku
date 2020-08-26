/*
 * app_packet.h
 *
 *  Created on: 2017/03/16
 *      Author: t.miki
 */

#ifndef APPLICATION_APP_PACKET_H_
#define APPLICATION_APP_PACKET_H_

#include <stdint.h>

#include "i2c_LIS2DH12.h"

#if 0
// =========================================================
// WiSUN Format
typedef struct __attribute__((__packed__)) tagUIF_WiSUN
{
//     Machdr_S_t mac_h ;
//     uint8_t  MacPayload[230];
     uint8_t  data[255];

//    MacPay_t mac_p ;
//    uint16_t FCS;
} msgWiSUNData_t ;

// =========================================================
// BLE Format
typedef struct __attribute__((__packed__)) tagUIF_BLE
{
//    uint8_t     mac_address ;
    uint16_t    hdr ;
    uint8_t     payload[37] ;
} msgBleData_t ;
#endif

int update_wisun_beacon(int burst);
int update_ble_beacon(int burst);

uint16_t updateWiSUNBeaconData();
uint16_t updateBleBeaconData();

#endif /* APPLICATION_APP_PACKET_H_ */

