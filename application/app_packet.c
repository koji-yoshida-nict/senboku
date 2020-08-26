/*
 * app_packet.c
 *
 *  Created on: 2017/03/16
 *      Author: t.miki
 */

#include <app_version.h>
#include <env.h>
#include <string.h>
#include <TaskDefine.h>
#include <TaskInit.h>
#include <uif_pkt.h>
#include <driverlib/sys_ctrl.h>
#include "app_packet.h"
#include "i2c_LIS2DH12.h"

// =========================================================
// define
#define HD_TYPE                 WISUN_HD_TYPE
#define HD_LENG                 WISUN_HD_LENG
#define HD_SIZE                 WISUN_HD_SIZE

//#define WI_BASE_INFO_SIZE   20
#define WI_BASE_INFO_SIZE       24  // 制御情報は本当は、24Byte 2017.06.30 H.Ohuchi
#define WI_CONTOPT_DATA_SIZE    206
#define WI_APP_DATA_SIZE        206
#define BLE_APP_DATA_SIZE       16

#define WI_OPTDATA_START        6
#define WI_OPTDATA_END_NORMAL   10
#define WI_APPDATA_START        10
#define WI_APPDATA_END_EVENT    14
#define BLE_APPDATA_START       0
#define BLE_APPDATA_END_NORMAL  14
#define BLE_APPDATA_END_EVENT   14

// マクロ
#define B2H(a)  (uint8_t)(((a) >= 0x0a)?((a) - 0x0a + 'A'):((a) + '0'))

static uint8_t bi[WI_BASE_INFO_SIZE];
static uint8_t oi[WI_CONTOPT_DATA_SIZE];
static uint8_t ad[WI_APP_DATA_SIZE];
static uint8_t wi_pkid = 1;
static uint8_t bl_pkid = 1;

#if 0 // Classic Version
// =========================================================
//
static int ble_edit_payload(uint8_t *pfrm);
static int wisun_edit_frame_header(uint8_t *pfrm);
static int wisun_edit_base_info(uint8_t *pfrm);
static int wisun_edit_payloads(uint8_t *pfrm,uint8_t *pPayloadIE,int payloadIE_len,uint8_t *pPayload,int payload_len);

/***************************************************************************
 *!
 * \brief   wisun_edit_frame_header
 * \note
 *
 ***************************************************************************/
static int wisun_edit_frame_header(uint8_t *pfrm)
{
    int len;

    /* コントロールコード */
    *pfrm++ = 0x00;
    *pfrm++ = 0xea;
    len = 2;

    /* シーケンス番号 */
    *pfrm++ = wisun_seqno++;
    len += 1;

    /* PAN ID */
    *pfrm++ = (uint8_t)(wisun_panid & 0xff);
    *pfrm++ = (uint8_t)(wisun_panid >> 8);
    len  += 2;

    /* 宛先アドレス */
    *pfrm++ = 0xff;
    *pfrm++ = 0xff;
    len += 2;

    /* 送信元アドレス */
    memcpy(pfrm, wisun_mac, MACADDR_SIZE);
    pfrm += MACADDR_SIZE;
    len  += MACADDR_SIZE;

    return(len);
}

/***************************************************************************
 *!
 * \brief   wisun_edit_base_info
 * \note
 *
 ***************************************************************************/
static int wisun_edit_base_info(uint8_t *pfrm)
{
    int len;

    /* Local Name */
    *pfrm++ = wisun_localName[0];
    *pfrm++ = wisun_localName[1];
    *pfrm++ = wisun_localName[2];
    *pfrm++ = wisun_localName[3];
#define B2H(a) (uint8_t)(((a) >= 0x0a)?((a) - 0x0a + 'A'):((a) + '0'))
    *pfrm++ = B2H((wisun_panid >> 12) & 0xf);
    *pfrm++ = B2H((wisun_panid >>  8) & 0xf);
    *pfrm++ = B2H((wisun_panid >>  4) & 0xf);
    *pfrm++ = B2H((wisun_panid >>  0) & 0xf);
    len = 8;

    /* Flag */
#ifdef _TP_V10700
    *pfrm++ = (uint8_t)(wisun_flag & 0xff);
    *pfrm++ = (uint8_t)(wisun_flag >> 8);
#else
    *pfrm++ = 0x01;     /* bit9 fixed data: 0, bit8: 1 (mobile) */
    *pfrm++ = 0x40;     /* bit7:bit5: 1 (Wi-SUN), bit4-3: 0, bit2-0: 0 */
#endif
    len += 2;

    /* Hops */
    *pfrm++ = 0;
    len += 1;

    /* Sequence No. */
    *pfrm++ = (uint8_t)(wisun_base_seqno >> 8);
    *pfrm++ = (uint8_t)(wisun_base_seqno & 0xff);
    wisun_base_seqno++;
    len += 2;

    /* 発信周期 */
    *pfrm++ = (uint8_t)(wisun_base_period >> 8);
    *pfrm++ = (uint8_t)(wisun_base_period & 0xff);
    len += 2;

    /* 送信元デバイスID */
    *pfrm++ = wisun_mac[7];
    *pfrm++ = wisun_mac[6];
    *pfrm++ = wisun_mac[5];
    *pfrm++ = wisun_mac[4];
    *pfrm++ = wisun_mac[3];
    *pfrm++ = wisun_mac[2];
    *pfrm++ = wisun_mac[1];
    *pfrm++ = wisun_mac[0];
    len += MACADDR_SIZE;

    /* サービスネットワークID */
#define SERVICE_NETWORK_ID  1
    *pfrm++ = SERVICE_NETWORK_ID;
    len += 1;

    return(len);
}

/***************************************************************************
 *!
 * \brief   wisun_edit_payloads
 * \note
 *
 ***************************************************************************/
static int wisun_edit_payloads(uint8_t *pfrm,uint8_t *pPayloadIE,int payloadIE_len,uint8_t *pPayload,int payload_len)
{
    int len;

    /* Head IE */
    *pfrm++ = 0;
    *pfrm++ = 0x3f;
    len = 2;

    /* Descriptor */
    *pfrm++ = payloadIE_len + 3;
    *pfrm++ = 0x90;
    len += 2;

    /* Vendor OUI */
    *pfrm++ = vendorOUI[0];
    *pfrm++ = vendorOUI[1];
    *pfrm++ = vendorOUI[2];
    len += 3;

    /* Payload IE */
    memcpy(pfrm,pPayloadIE,payloadIE_len);
    pfrm += payloadIE_len;
    *pfrm++ = 0x00;
    *pfrm++ = 0xf8;
    len += (payloadIE_len + 2);

    /* Payload */
    if(payload_len){
        memcpy(pfrm,pPayload,payload_len);
        len += payload_len;
    }

    return(len);
}
/***************************************************************************
 *!
 * \brief   base_wi_beacon
 * \note
 *
 ***************************************************************************/
int update_wisun_beacon(int burst)
{
    uint8_t *pf;
    int wi_len,bi_l,ad_l;

    pf = (uint8_t *)&sendWiSUN_dp[WISUN_HD_SIZE];
    /* frame head */
    wi_len = wisun_edit_frame_header(pf);
    pf += wi_len;

    /* base info */
    bi_l = wisun_edit_base_info(bi);

    /* app data */
#if 0
    ad_l = wisun_edit_sensor_info(ad);
#else
    ad_l = 0 ;
#endif
    /* make frame */
    wi_len += wisun_edit_payloads(pf,bi,bi_l,ad,ad_l);

    sendWiSUN_dp[0] = wi_len + 7 ;       // packet total length
    sendWiSUN_dp[1] = PKCOM_BEACON ;    //
    sendWiSUN_dp[2] = 0 ;       //
    sendWiSUN_dp[3] = wi_pkid ; //
    sendWiSUN_dp[4] = 0x03 ;    //
    sendWiSUN_dp[5] = 1 ;       //
    if( burst )
        sendWiSUN_dp[HD_TYPE] = WisunBeaconType | BSTmodeWisunBeaconType;     //
    else
        sendWiSUN_dp[HD_TYPE] = WisunBeaconType ;     //

    sendWiSUN_dp[HD_LENG] = wi_len ;       //

    wi_pkid ++ ;

    // packet length
    g_WiSUNdatalength = sendWiSUN_dp[HD_LENG];  // data size

    return wi_len;
}

/***************************************************************************
 *!
 * \brief   ble_edit_payload
 * \note
 *
 ***************************************************************************/
static int ble_edit_payload(uint8_t *pfrm)
{
    int len = 0;

    /* BLE MACアドレス設定 */
#if 1
    memcpy(pfrm, ble_mac, BLE_MACADDR_SIZE);
    pfrm += BLE_MACADDR_SIZE;
#else
    *pfrm++ = ble_mac[5];
    *pfrm++ = ble_mac[4];
    *pfrm++ = ble_mac[3];
    *pfrm++ = ble_mac[2];
    *pfrm++ = ble_mac[1];
    *pfrm++ = ble_mac[0];
#endif
    len  += BLE_MACADDR_SIZE;

    /* AD1 */
    *pfrm++ = 2;            // Len
    *pfrm++ = 0x01;         // Type
    *pfrm++ = 0x05;         // Data
    len += 3;

    /* AD2 */
#if 0
    *(pfrm + 1) = 0xff;     // TYpe
    l = wisun_edit_sensor_info(pfrm + 2);
    *pfrm = l + 1;          // Len
#endif

    /* AD3 */
#define B2H(a)  (uint8_t)(((a) >= 0x0a)?((a) - 0x0a + 'A'):((a) + '0'))
    *pfrm++ = 9;            // Len
    *pfrm++ = 0x09;         // Type
    *pfrm++ = ble_localName[0];
    *pfrm++ = ble_localName[1];
    *pfrm++ = ble_localName[2];
    *pfrm++ = ble_localName[3];
    *pfrm++ = B2H((ble_panid >> 12) & 0xf);
    *pfrm++ = B2H((ble_panid >>  8) & 0xf);
    *pfrm++ = B2H((ble_panid >>  4) & 0xf);
    *pfrm++ = B2H((ble_panid >>  0) & 0xf);
    len += 10;

    return(len);
}

/***************************************************************************
 *!
 * \brief   base_bl_beacon
 * \note
 *
 ***************************************************************************/
int update_ble_beacon(int burst)
{
    int bl_len;

    msgBleData_t* pble = (msgBleData_t*)&sendBle_dp[WISUN_HD_SIZE] ;

    // ble data
    pble->hdr = 0x1140 ;
    bl_len = ble_edit_payload((uint8_t *)pble->payload);

    // Header size add
    bl_len += BLE_DATA_HDR ;

    sendBle_dp[0] = bl_len + 7 ;       // packet total length
    sendBle_dp[1] = PKCOM_BEACON ;    //
    sendBle_dp[2] = 0 ;       //
    sendBle_dp[3] = bl_pkid ; //
    sendBle_dp[4] = 0x03 ;    //
    sendBle_dp[5] = 1 ;       //
    if( burst )
    {
        sendBle_dp[HD_TYPE] = BleBeaconType  | BSTmodeBleBeaconType;   //
    }
    else
    {
        sendBle_dp[HD_TYPE] = BleBeaconType ;   //
    }
    sendBle_dp[HD_LENG] = bl_len ;       //

    bl_pkid ++ ;

    // packet length
    g_bledatalength = sendBle_dp[WISUN_HD_LENG];  // data size

    return bl_len ;
}

#else // 2次開発版
/***************************************************************************
 *
 *  AD2 DATA SET METHOD
 *
 ***************************************************************************/
/* CODE */
#define APPDATA_LOCALNAME_CODE  0x09

#define APPDATA_FLAG_CODE       0x41
#define APPDATA_HOP_CODE        0x42
#define APPDATA_SEQUENCE_CODE   0x43
#define APPDATA_PERIOD_CODE     0x44
#define APPDATA_WS_SENDID_CODE  0x45
#define APPDATA_BLE_SENDID_CODE 0x45
#define APPDATA_DESTID_CODE     0x46
#define APPDATA_SERVID_CODE     0x47
#define APPDATA_HOPMAX_CODE     0x48
#define APPDATA_DISTANCE_CODE   0x49
#define APPDATA_TIME_CODE       0x4A
#define APPDATA_ACCDATA_CODE    0x66
#define APPDATA_VOLTAGE_CODE    0x71
#define APPDATA_ACTSTAT_CODE    0x72
#define APPDATA_ANOMALYLEVEL_CODE 0x6A

/* LENGTH */
#define APPDATA_FLG_LEN         1
#define APPDATA_LEN_LEN         1

#define APPDATA_FLAG_LEN        2
#define APPDATA_SEQUENCE_LEN    2
#define APPDATA_HOP_LEN         1
#define APPDATA_PERIOD_LEN      2
#define APPDATA_SERVID_LEN      1
#define APPDATA_DESTID_LEN      8
#define APPDATA_HOPMAX_LEN      1
#define APPDATA_DISTANCE_LEN    2
#define APPDATA_TIME_LEN        3
#define APPDATA_WS_SENDID_LEN   8
#define APPDATA_BLE_SENDID_LEN  6
#define APPDATA_VOLTAGE_LEN     1
#define APPDATA_ACTSTAT_LEN     1
#define APPDATA_ANOMALYLEVEL_LEN  2

#define APPDATA_ACC_ONE_LEN     3

#define MAX_DEATH_COUNT_INC     0xFF
#define MAX_SUBLIFE_COUNT_INC   0xFF

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
/** CONTROLE INFORMATION OPTIONS **/
uint8_t setWiSUNFlag(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_FLAG_LEN;

    if(WI_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_FLAG_CODE;
        *pfrm++ = (uint8_t)((env_nictparam.WiSUNPayloadFlag >> 8) & 0xFF);
        *pfrm++ = (uint8_t)(env_nictparam.WiSUNPayloadFlag & 0xFF);
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setWiSUNHop(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_HOP_LEN;

    if(WI_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_HOP_CODE;
        *pfrm++ = env_nictparam.WiSUNPayloadHop;
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setWiSUNSequence(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_SEQUENCE_LEN;

    if(WI_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_SEQUENCE_CODE;
        *pfrm++ = (uint8_t)((wisun_base_seqno >> 8) & 0xFF);
        *pfrm++ = (uint8_t)(wisun_base_seqno & 0xFF);
        wisun_base_seqno++;
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setWiSUNPeriod(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_PERIOD_LEN;

    if(WI_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_PERIOD_CODE;
        *pfrm++ = (uint8_t)((wisun_interval >> 8) & 0xFF);
        *pfrm++ = (uint8_t)(wisun_interval & 0xFF);
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setWiSUNSendid(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_WS_SENDID_LEN;

    if(WI_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_WS_SENDID_CODE;
#if 1
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[0];
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[1];
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[2];
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[3];
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[4];
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[5];
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[6];
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[7];
#else   // REVERSE
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[7];
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[6];
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[5];
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[4];
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[3];
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[2];
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[1];
        *pfrm++ = env_nictparam.WiSUNPayloadSendid[0];
#endif
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setWiSUNServid(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_SERVID_LEN;

    if(WI_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_SERVID_CODE;
        *pfrm++ = env_nictparam.WiSUNPayloadServid;
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setWiSUNDestid(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_DESTID_LEN;

    if(WI_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_DESTID_CODE;
#if 1
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[0];
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[1];
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[2];
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[3];
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[4];
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[5];
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[6];
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[7];
#else   // REVERSE
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[7];
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[6];
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[5];
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[4];
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[3];
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[2];
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[1];
        *pfrm++ = env_nictparam.WiSUNPayloadDestid[0];
#endif
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setWiSUNHopmax(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_HOPMAX_LEN;

    if(WI_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_HOPMAX_CODE;
        *pfrm++ = env_nictparam.WiSUNPayloadHopmax;
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setWiSUNDistance(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_DISTANCE_LEN;

    if(WI_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_DISTANCE_CODE;
        *pfrm++ = (uint8_t)((env_nictparam.WiSUNPayloadDistance >> 8) & 0xFF);
        *pfrm++ = (uint8_t)(env_nictparam.WiSUNPayloadDistance & 0xFF);
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setWiSUNTime(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_TIME_LEN;

    if(WI_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_TIME_CODE;
        *pfrm++ = env_nictparam.WiSUNPayloadTime[0];
        *pfrm++ = env_nictparam.WiSUNPayloadTime[1];
        *pfrm++ = env_nictparam.WiSUNPayloadTime[2];
    }
    else
    {
        len = 0;
    }
    return len;
}
/** APPLICATION DATA **/
uint8_t setWiSUNVoltage(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_VOLTAGE_LEN;

    if(WI_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_VOLTAGE_CODE;
        *pfrm++ = g_voltage;
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setWiSUNActStat(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_ACTSTAT_LEN;

    if(WI_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_ACTSTAT_CODE;
        *pfrm++ = g_mode;
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setWiSUNAccdata(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = 0;
    uint8_t cnt = 0;
    uint8_t loopCnt = 0;

    cnt = (WI_APP_DATA_SIZE - APPDATA_FLG_LEN - APPDATA_LEN_LEN - length) / APPDATA_ACC_ONE_LEN;

    if(cnt > 0)
    {
        if(env_nictparam.activeWindowData < cnt)
        {
            cnt = env_nictparam.activeWindowData;
        }
        *pfrm++ = (cnt * APPDATA_ACC_ONE_LEN) + APPDATA_LEN_LEN;
        *pfrm++ = APPDATA_ACCDATA_CODE;

        for(loopCnt = 1; loopCnt <= cnt; loopCnt++)
        {
            memcpy(pfrm, (uint8_t*)&g_accDataBuffer[env_nictparam.activeWindowData-loopCnt], APPDATA_ACC_ONE_LEN);
            pfrm += APPDATA_ACC_ONE_LEN;
        }

        len += (cnt * APPDATA_ACC_ONE_LEN) + APPDATA_FLG_LEN + APPDATA_LEN_LEN;
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setWiSUNAnomalyLevelData(uint8_t *pfrm, uint8_t length)
{
  uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_ANOMALYLEVEL_LEN;
  if(WI_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_ANOMALYLEVEL_CODE;
        *pfrm++ = (uint8_t)g_DeathCount_increment;
        *pfrm++ = (uint8_t)g_SublifeCount_increment;
    }
    else
    {
        len = 0;
    }
    return len;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
uint8_t setBleFlag(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_FLAG_LEN;

    if(BLE_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_FLAG_CODE;
        *pfrm++ = (uint8_t)((env_nictparam.blePayloadFlag >> 8) & 0xFF);
        *pfrm++ = (uint8_t)(env_nictparam.blePayloadFlag & 0xFF);
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setBleHop(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_HOP_LEN;

    if(BLE_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_HOP_CODE;
        *pfrm++ = env_nictparam.blePayloadHop;
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setBleSequence(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_SEQUENCE_LEN;

    if(BLE_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_SEQUENCE_CODE;
        *pfrm++ = (uint8_t)((ble_seqno >> 8) & 0xFF);
        *pfrm++ = (uint8_t)(ble_seqno & 0xFF);
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setBlePeriod(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_PERIOD_LEN;

    if(BLE_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_PERIOD_CODE;
        *pfrm++ = (uint8_t)((ble_interval >> 8) & 0xFF);
        *pfrm++ = (uint8_t)(ble_interval & 0xFF);
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setBleSendid(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_BLE_SENDID_LEN;

    if(BLE_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_BLE_SENDID_CODE;
#if 1
        *pfrm++ = env_nictparam.blePayloadSendid[0];
        *pfrm++ = env_nictparam.blePayloadSendid[1];
        *pfrm++ = env_nictparam.blePayloadSendid[2];
        *pfrm++ = env_nictparam.blePayloadSendid[3];
        *pfrm++ = env_nictparam.blePayloadSendid[4];
        *pfrm++ = env_nictparam.blePayloadSendid[5];
#else   // REVERSE
        *pfrm++ = env_nictparam.blePayloadSendid[5];
        *pfrm++ = env_nictparam.blePayloadSendid[4];
        *pfrm++ = env_nictparam.blePayloadSendid[3];
        *pfrm++ = env_nictparam.blePayloadSendid[2];
        *pfrm++ = env_nictparam.blePayloadSendid[1];
        *pfrm++ = env_nictparam.blePayloadSendid[0];
#endif
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setBleServid(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_SERVID_LEN;

    if(BLE_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_SERVID_CODE;
        *pfrm++ = env_nictparam.blePayloadServid;
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setBleDestid(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_DESTID_LEN;

    if(BLE_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_DESTID_CODE;
#if 1
        *pfrm++ = env_nictparam.blePayloadDestid[0];
        *pfrm++ = env_nictparam.blePayloadDestid[1];
        *pfrm++ = env_nictparam.blePayloadDestid[2];
        *pfrm++ = env_nictparam.blePayloadDestid[3];
        *pfrm++ = env_nictparam.blePayloadDestid[4];
        *pfrm++ = env_nictparam.blePayloadDestid[5];
        *pfrm++ = env_nictparam.blePayloadDestid[6];
        *pfrm++ = env_nictparam.blePayloadDestid[7];
#else   // REVERSE
        *pfrm++ = env_nictparam.blePayloadDestid[7];
        *pfrm++ = env_nictparam.blePayloadDestid[6];
        *pfrm++ = env_nictparam.blePayloadDestid[5];
        *pfrm++ = env_nictparam.blePayloadDestid[4];
        *pfrm++ = env_nictparam.blePayloadDestid[3];
        *pfrm++ = env_nictparam.blePayloadDestid[2];
        *pfrm++ = env_nictparam.blePayloadDestid[1];
        *pfrm++ = env_nictparam.blePayloadDestid[0];
#endif
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setBleHopmax(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_HOPMAX_LEN;

    if(BLE_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_HOPMAX_CODE;
        *pfrm++ = env_nictparam.blePayloadHopmax;
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setBleDistance(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_DISTANCE_LEN;

    if(BLE_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_DISTANCE_CODE;
        *pfrm++ = (uint8_t)((env_nictparam.blePayloadDistance >> 8) & 0xFF);
        *pfrm++ = (uint8_t)(env_nictparam.blePayloadDistance & 0xFF);
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setBleTime(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_TIME_LEN;

    if(BLE_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_TIME_CODE;
        *pfrm++ = env_nictparam.blePayloadTime[0];
        *pfrm++ = env_nictparam.blePayloadTime[1];
        *pfrm++ = env_nictparam.blePayloadTime[2];
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setBleVoltage(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_VOLTAGE_LEN;

    if(BLE_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_VOLTAGE_CODE;
        *pfrm++ = g_voltage;
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setBleActStat(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_ACTSTAT_LEN;
    if(BLE_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_ACTSTAT_CODE;
        *pfrm++ = g_mode;
    }
    else
    {
        len = 0;
    }
    return len;
}
uint8_t setBleAccdata(uint8_t *pfrm, uint8_t length)
{
    uint8_t len = 0;
    uint8_t cnt = 0;
    uint8_t loopCnt = 0;

    cnt = (BLE_APP_DATA_SIZE - APPDATA_FLG_LEN - APPDATA_LEN_LEN - length) / APPDATA_ACC_ONE_LEN;

    if(cnt > 0)
    {
        if(env_nictparam.activeWindowData < cnt)
        {
            cnt = env_nictparam.activeWindowData;
        }
        *pfrm++ = (cnt * APPDATA_ACC_ONE_LEN) + APPDATA_LEN_LEN;
        *pfrm++ = APPDATA_ACCDATA_CODE;

        for(loopCnt = 1; loopCnt <= cnt; loopCnt++)
        {
            memcpy(pfrm, (uint8_t*)&g_accDataBuffer[env_nictparam.activeWindowData-loopCnt], APPDATA_ACC_ONE_LEN);
            pfrm += APPDATA_ACC_ONE_LEN;
        }
        len += (cnt * APPDATA_ACC_ONE_LEN) + APPDATA_FLG_LEN + APPDATA_LEN_LEN;
    }
    else
    {
        len = 0;
    }
    return len;
}

uint8_t setBleAnomalyLevelData(uint8_t *pfrm, uint8_t length)
{
  uint8_t len = APPDATA_FLG_LEN + APPDATA_LEN_LEN + APPDATA_ANOMALYLEVEL_LEN;
  if(BLE_APP_DATA_SIZE-length >= len)
    {
        *pfrm++ = len - APPDATA_FLG_LEN;
        *pfrm++ = APPDATA_ANOMALYLEVEL_CODE;
        *pfrm++ = (uint8_t)g_DeathCount_increment;
        *pfrm++ = (uint8_t)g_SublifeCount_increment;
    }
    else
    {
        len = 0;
    }
    return len;
}


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
/* 関数ﾎﾟｲﾝﾀ型の定義 */
typedef uint8_t (*editFuncTable)(uint8_t*, uint8_t);
editFuncTable editWiSUN[] =
{
 setWiSUNFlag,
 setWiSUNHop,
 setWiSUNSequence,
 setWiSUNPeriod,
 setWiSUNSendid,
 setWiSUNServid,
 setWiSUNDestid,
 setWiSUNHopmax,
 setWiSUNDistance,
 setWiSUNTime,
 setWiSUNVoltage,
 setWiSUNActStat,
 setWiSUNAccdata,
 setWiSUNAnomalyLevelData
};
editFuncTable editBle[] =
{
 setBleFlag,
 setBleHop,
 setBleSequence,
 setBlePeriod,
 setBleSendid,
 setBleServid,
 setBleDestid,
 setBleHopmax,
 setBleDistance,
 setBleTime,
 setBleVoltage,
 setBleActStat,
 setBleAccdata,
 setBleAnomalyLevelData
};
/***************************************************************************
 *
 *  Wi-SUN BEACON DATA PACKET EDIT
 *
 ***************************************************************************/
/* デスカウントモード用にPANIDをインクリメントする */
extern Semaphore_Handle sleepSemHandle;
extern uint8_t isAccSleeping;
static void prepareLifecountParameters()
{
  if (g_LifeCounter < env_nictparam.WiSUNLifeCount){
    g_DeathCount_increment = 0;
  } // if g_LifeCounter < env_nictparam.WiSUNLifeCount
  else{
    /* ライフカウントモードから初めてデスカウントモードになったとき */
    if (g_DeathCount_increment == 0){
      g_DeathCounter = 0;
      g_DeathCount_increment = 1;
      g_SublifeCounter = 0;
      g_SublifeCount_increment = 0;

      if (isAccSleeping){
        Semaphore_post(sleepSemHandle); /* 加速度センサーを起こす */
      } // if isAccSleeping
      
    } // if g_DeathCount_increment == 0
    if (g_DeathCounter >= env_nictparam.WiSUNDeathCount){
      /* g_DeathCounter はデスカウントまで到達したらゼロリセット */
      g_DeathCounter = 0;

      if (isAccSleeping){
        Semaphore_post(sleepSemHandle); /* 加速度センサーを起こす */
      } // if isAccSleeping
      
      if (g_DeathCount_increment < MAX_DEATH_COUNT_INC){        
        g_DeathCount_increment++;
        
        g_SublifeCounter = 0;
        g_SublifeCount_increment = 0;
      } // if  g_DeathCount_increment < MAX_DEATH_COUNT_INC
    } // if g_DeathCounter >= env_nictparam.WiSUNDeathCount
    
    if (env_nictparam.WiSUNDeathCount == 0){
      g_DeathCount_increment = 1;
    } // if env_nictparam.WiSUNDeathCount == 0
    
  } // else

  if (g_SublifeCounter >= env_nictparam.WiSUNSublifeCount){
    g_SublifeCount_increment++;
    g_SublifeCounter = 0;    
  } // if g_SublifeCounter >= env_nictparam.WiSUNSublifeCount
  if (g_SublifeCount_increment > MAX_SUBLIFE_COUNT_INC){
    g_SublifeCount_increment = MAX_SUBLIFE_COUNT_INC;
  } // if g_SublifeCount_increment > MAX_SUBLIFE_COUNT_INC
}

static void updateLifecountParameters()
{
  static uint16_t sysRebootCounter = 0;
  
  if (g_LifeCounter < env_nictparam.WiSUNLifeCount){
    g_LifeCounter++;
  } // if g_LifeCounter < env_nictparam.WiSUNLifeCount
  else {
    if (g_DeathCounter < env_nictparam.WiSUNDeathCount){
      g_DeathCounter++;
    } // if env_nictparam.WiSUNDeathCount > g_DeathCount
  }

  if (g_SublifeCounter < env_nictparam.WiSUNSublifeCount){
    g_SublifeCounter++;
  } // if g_SublifeCount < env_nictparam.WiSUNSublifeCount

  /* 指定回数ビーコン発信したら強制リセット */
  if (sysRebootCounter >= env_nictparam.WiSUNSystemRebootCount){
    SysCtrlSystemReset();
  } // if sysRebootCounter >= env_nictparam.WiSUNSystemRebootCount
  
  sysRebootCounter++;
  
  
}

static uint16_t currentPanid_Lifecount(uint16_t inpanid)
{
  uint16_t outpanid = inpanid + g_DeathCount_increment;

  /* 0xf 以上なら0xfに補正する */
  if ((inpanid & 0xf) + g_DeathCount_increment > 0xf){
    outpanid = (inpanid & 0xfff0) + 0xf;
  } // if (inpanid & 0xf) + g_DeathCount_increment > 0xf

  return outpanid;
}

static int wisunEditFrameHeader(uint8_t *pfrm)
{
    int len;

    /* コントロールコード */
    *pfrm++ = 0x00;
    *pfrm++ = 0xea;
    len = 2;

    /* シーケンス番号 */
    *pfrm++ = wisun_seqno++;
    len += 1;

    /* PAN ID */
#if 0
    *pfrm++ = (uint8_t)(wisun_panid & 0xff);
    *pfrm++ = (uint8_t)(wisun_panid >> 8);
#else
    if(env_nictparam.deviceRunmode & DEVICE_RUNMODE_TIMER )
    //    if(env_nictparam.deviceRunmode == 0x00)
    // NICT #MOD_190917
    {   
        // ------------------------------------------------------------------------------
        // NICT #MOD_190917
      if (env_nictparam.deviceRunmode & DEVICE_RUNMODE_EVENT)
        {
          uint16_t panid = currentPanid_Lifecount(env_nictparam.WiSUNPayloadPanidNormal);
          *pfrm++ = (uint8_t)(panid & 0xff);
          *pfrm++ = (uint8_t)(panid >> 8);
        }
      else{// 通常モード
        *pfrm++ = (uint8_t)(env_nictparam.WiSUNPayloadPanidNormal & 0xff);
        *pfrm++ = (uint8_t)(env_nictparam.WiSUNPayloadPanidNormal >> 8);
      } // else 
    }
    else
    {   // イベントモード
        if(g_mode == MODE_TRANSITION_TO_WALK)
        {   // MODE_TRANSITION_TO_WALK
            *pfrm++ = (uint8_t)(env_nictparam.WiSUNPayloadPanidWalk & 0xff);
            *pfrm++ = (uint8_t)(env_nictparam.WiSUNPayloadPanidWalk >> 8);
        }
        else
        {   // MODE_TRANSITION_TO_RUN
            *pfrm++ = (uint8_t)(env_nictparam.WiSUNPayloadPanidRun & 0xff);
            *pfrm++ = (uint8_t)(env_nictparam.WiSUNPayloadPanidRun >> 8);
        }
    }
#endif
    len += 2;

    /* 宛先アドレス */
    *pfrm++ = 0xff;
    *pfrm++ = 0xff;
    len += 2;

    /* 送信元アドレス */
#if 0
    memcpy(pfrm, wisun_mac, MACADDR_SIZE);
    pfrm += MACADDR_SIZE;
    len  += MACADDR_SIZE;
#else
#if 0
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[0];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[1];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[2];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[3];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[4];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[5];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[6];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[7];
#else   // Reverse
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[7];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[6];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[5];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[4];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[3];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[2];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[1];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[0];
#endif
#endif
    len  += MACADDR_SIZE;

    return(len);
}
static uint16_t wisunEditBaseInformation(uint8_t *pfrm)
{
    uint16_t len;

    /* Local Name */
    *pfrm++ = wisun_localName[0];
    *pfrm++ = wisun_localName[1];
    *pfrm++ = wisun_localName[2];
    *pfrm++ = wisun_localName[3];
    if(env_nictparam.deviceRunmode & DEVICE_RUNMODE_TIMER )
    //    if(env_nictparam.deviceRunmode == 0x00)
    // NICT #MOD_190917
    {   // 通常モード
        // ------------------------------------------------------------------------------
        // NICT #MOD_190917
      if (env_nictparam.deviceRunmode & DEVICE_RUNMODE_EVENT)
        {
          uint16_t panid = currentPanid_Lifecount(env_nictparam.WiSUNPayloadPanidNormal);
          *pfrm++ = B2H((panid >> 12) & 0xf);
          *pfrm++ = B2H((panid >>  8) & 0xf);
          *pfrm++ = B2H((panid >>  4) & 0xf);
          *pfrm++ = B2H((panid >>  0) & 0xf);
        }
      else{
        *pfrm++ = B2H((env_nictparam.WiSUNPayloadPanidNormal >> 12) & 0xf);
        *pfrm++ = B2H((env_nictparam.WiSUNPayloadPanidNormal >>  8) & 0xf);
        *pfrm++ = B2H((env_nictparam.WiSUNPayloadPanidNormal >>  4) & 0xf);
        *pfrm++ = B2H((env_nictparam.WiSUNPayloadPanidNormal >>  0) & 0xf);
      } // else 
    }
    else
    {   // イベントモード
        if(g_mode == MODE_TRANSITION_TO_WALK)
        {   // MODE_TRANSITION_TO_WALK
            *pfrm++ = B2H((env_nictparam.WiSUNPayloadPanidWalk >> 12) & 0xf);
            *pfrm++ = B2H((env_nictparam.WiSUNPayloadPanidWalk >>  8) & 0xf);
            *pfrm++ = B2H((env_nictparam.WiSUNPayloadPanidWalk >>  4) & 0xf);
            *pfrm++ = B2H((env_nictparam.WiSUNPayloadPanidWalk >>  0) & 0xf);
        }
        else
        {   // MODE_TRANSITION_TO_RUN
            *pfrm++ = B2H((env_nictparam.WiSUNPayloadPanidRun >> 12) & 0xf);
            *pfrm++ = B2H((env_nictparam.WiSUNPayloadPanidRun >>  8) & 0xf);
            *pfrm++ = B2H((env_nictparam.WiSUNPayloadPanidRun >>  4) & 0xf);
            *pfrm++ = B2H((env_nictparam.WiSUNPayloadPanidRun >>  0) & 0xf);
        }
    }
    len = 8;

    /* Flag */
#ifdef _TP_V10700
    *pfrm++ = (uint8_t)(wisun_flag >> 8);
    *pfrm++ = (uint8_t)(wisun_flag & 0xff);
#else
    *pfrm++ = 0x01;     /* bit9 fixed data: 0, bit8: 1 (mobile) */
    *pfrm++ = 0x40;     /* bit7:bit5: 1 (Wi-SUN), bit4-3: 0, bit2-0: 0  */
#endif
    len += 2;

    /* Hops */
    *pfrm++ = env_nictparam.WiSUNPayloadHop;
    len += 1;

    /* Sequence No. */
    *pfrm++ = (uint8_t)(wisun_base_seqno >> 8);
    *pfrm++ = (uint8_t)(wisun_base_seqno & 0xff);
//    wisun_base_seqno++;
    len += 2;

    /* 発信周期 */
#if 0
    *pfrm++ = (uint8_t)(wisun_interval >> 8);
    *pfrm++ = (uint8_t)(wisun_interval & 0xff);
#else
    *pfrm++ = (uint8_t)(env_nictparam.WiSUNPayloadPepiod >> 8);
    *pfrm++ = (uint8_t)(env_nictparam.WiSUNPayloadPepiod & 0xff);
#endif
    len += 2;

    /* 送信元デバイスID */
#if 1
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[0];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[1];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[2];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[3];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[4];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[5];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[6];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[7];
#else   // Reverse
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[7];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[6];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[5];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[4];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[3];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[2];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[1];
    *pfrm++ = env_nictparam.WiSUNPayloadSendid[0];
#endif
    len += MACADDR_SIZE;

    /* サービスネットワークID */
    *pfrm++ = env_nictparam.WiSUNPayloadServid;
    len += 1;

    return(len);
}
static uint16_t wisunEditControleOptionData(uint8_t *pfrm)
{
    uint16_t len = 0;
    uint8_t cnt = 0;
    uint8_t retLen = 0;
    uint8_t ret = 0;

    for(cnt=WI_OPTDATA_START; cnt<WI_OPTDATA_END_NORMAL; cnt++)
    {
        if(cnt <= 7)
        {
            ret = (env_nictparam.WiSUNExist[1] >> cnt) & 0x01;
        }
        else
        {
            ret = (env_nictparam.WiSUNExist[0] >> (cnt-8)) & 0x01;
        }
        if(ret)
        {
            retLen = (*editWiSUN[cnt])(pfrm, len);
            if(retLen != 0)
            {
                pfrm += retLen;
                len += retLen;
            }
            else
            {
                break;
            }
        }
    }

    return len;
}
static uint16_t wisunEditApplicationData(uint8_t *pfrm)
{
    uint16_t len = 0;
    uint8_t cnt = 0;
    uint8_t retLen = 0;
    uint8_t ret = 0;

    for(cnt=WI_APPDATA_START; cnt<WI_APPDATA_END_EVENT; cnt++)
    {
        if(cnt <= 7)
        {
            ret = (env_nictparam.WiSUNExist[1] >> cnt) & 0x01;
        }
        else
        {
            ret = (env_nictparam.WiSUNExist[0] >> (cnt-8)) & 0x01;
        }
        if(ret)
        {
            retLen = (*editWiSUN[cnt])(pfrm, len);
            if(retLen != 0)
            {
                pfrm += retLen;
                len += retLen;
            }
            else
            {
                break;
            }
        }
    }

    return len;
}
static uint16_t wisunEditPayloads(uint8_t *pfrm, uint8_t *bi, uint16_t bi_len, uint8_t *oi, uint16_t oi_len, uint8_t *ad, uint16_t ad_len)
{
    uint16_t len;

    /* Head IE */
    *pfrm++ = 0;
    *pfrm++ = 0x3f;
    len = 2;

    /* Descriptor */
    *pfrm++ = 3 + bi_len + oi_len;
    *pfrm++ = 0x90;
    len += 2;

    /* Vendor OUI */
    *pfrm++ = vendorOUI[0];
    *pfrm++ = vendorOUI[1];
    *pfrm++ = vendorOUI[2];
    len += 3;

    /* Base Infomation */
    memcpy(pfrm, bi, bi_len);
    pfrm += bi_len;
    len += bi_len;

    /* Option Infomation */
    memcpy(pfrm, oi, oi_len);
    pfrm += oi_len;
    len += oi_len;

    /* Payload IE */
    *pfrm++ = 0x00;
    *pfrm++ = 0xf8;
    len += 2;

    /* Payload */
    if(ad_len)
    {
        memcpy(pfrm, ad, ad_len);
        len += ad_len;
    }

    return(len);
}
uint16_t updateWiSUNBeaconData()
{
    uint8_t *pf;
    uint16_t all_len;
    uint16_t wisunedit_length;
    uint16_t frameheader_length;
    uint16_t baseinformation_length;
    uint16_t optioninformation_length;
    uint16_t applicationdata_length;

    pf = (uint8_t *)&sendWiSUN_dp[WISUN_HD_SIZE];

    if (env_nictparam.deviceRunmode == (DEVICE_RUNMODE_EVENT | DEVICE_RUNMODE_TIMER)) {
      prepareLifecountParameters();
    }
    
    // モード毎の設定値
    if(env_nictparam.beaconExist & BSTmodeWisunBeaconType)
    {
        if(env_nictparam.deviceRunmode & DEVICE_RUNMODE_TIMER )
        //    if(env_nictparam.deviceRunmode == 0x00)
        // NICT #MOD_190917
        {
          // ライフカウントモードと通常モード
          wisun_interval = env_nictparam.WiSUNBaseInterval;
          wisun_burst_period = env_nictparam.WiSUNBurstPeriod;
          wisun_burst_count = env_nictparam.WiSUNBurstCount;
          
        }
        else
        {   // イベントモード
            if(g_mode == MODE_TRANSITION_TO_WALK)
            {   // MODE_TRANSITION_TO_WALK
                wisun_interval = env_nictparam.walkWisunInterval;
                wisun_burst_period = env_nictparam.walkWisunBurstPeriod;
                wisun_burst_count = env_nictparam.walkWisunBurstCount;
            }
            else
            {   // MODE_TRANSITION_TO_RUN
                wisun_interval = env_nictparam.runWisunInterval;
                wisun_burst_period = env_nictparam.runWisunBurstPeriod;
                wisun_burst_count = env_nictparam.runWisunBurstCount;
            }
        }
    }
    
    /* frame head */
    frameheader_length = wisunEditFrameHeader(pf);
    pf += frameheader_length;

    /* base information */
    baseinformation_length = wisunEditBaseInformation(bi);

    /* Option Information */
    optioninformation_length = wisunEditControleOptionData(oi);

    /* application data */
    applicationdata_length = wisunEditApplicationData(ad);

    /* make frame */
    wisunedit_length = wisunEditPayloads(pf, bi, baseinformation_length, oi, optioninformation_length, ad, applicationdata_length);

    all_len = frameheader_length + wisunedit_length;

    sendWiSUN_dp[0] = all_len + 7 ;     // packet total length
    sendWiSUN_dp[1] = PKCOM_BEACON ;    //
    sendWiSUN_dp[2] = 0 ;               //
    sendWiSUN_dp[3] = wi_pkid ;         //
    sendWiSUN_dp[4] = 0x03 ;            //
    sendWiSUN_dp[5] = 1 ;               //

    if(env_nictparam.beaconExist & BSTmodeWisunBeaconType)
    {
        sendWiSUN_dp[HD_TYPE] = WisunBeaconType | BSTmodeWisunBeaconType;
    }
    else
    {
        sendWiSUN_dp[HD_TYPE] = WisunBeaconType;
    }
    sendWiSUN_dp[HD_LENG] = all_len;

    wi_pkid++;

    // packet length
    g_WiSUNdatalength = sendWiSUN_dp[HD_LENG];  // data size

    if (env_nictparam.deviceRunmode == (DEVICE_RUNMODE_EVENT | DEVICE_RUNMODE_TIMER)) {
      updateLifecountParameters();
    }
    
    return all_len;
}

/***************************************************************************
 *
 * BLE BEACON DATA PACKET EDIT
 *
 ***************************************************************************/
#define AD2_MAX_DATA_LENGTH 17

static uint8_t bleEditPayloadAD2(uint8_t *pfrm)
{
    uint8_t len = 0;
    uint8_t cnt = 0;
    uint8_t retLen = 0;
    uint8_t ret = 0;
    uint8_t limit = 0;

    if(env_nictparam.deviceRunmode & DEVICE_RUNMODE_TIMER )
    //    if(env_nictparam.deviceRunmode == 0x00)
    // NICT #MOD_190917
    {
        limit = BLE_APPDATA_END_NORMAL;
    }
    else
    {
        limit = BLE_APPDATA_END_EVENT;
    }

    for(cnt=BLE_APPDATA_START; cnt<limit; cnt++)
    {
        if(cnt <= 7)
        {
            ret = (env_nictparam.bleExist[1] >> cnt) & 0x01;
        }
        else
        {
            ret = (env_nictparam.bleExist[0] >> (cnt-8)) & 0x01;
        }
        if(ret)
        {
            retLen = (*editBle[cnt])(pfrm, len);
            if(retLen != 0)
            {
                pfrm += retLen;
                len += retLen;
            }
            else
            {
                break;
            }
        }
    }

    return(len);
}
static uint8_t bleEditPayload(uint8_t *pfrm)
{
    uint8_t ad2_data[20];
    uint8_t len = 0;
    uint8_t count = 0;

    /* BLE MACアドレス設定 */
#if 1
    memcpy(pfrm, ble_mac, BLE_MACADDR_SIZE);
    pfrm += BLE_MACADDR_SIZE;
#else   // Reverse
    *pfrm++ = ble_mac[5];
    *pfrm++ = ble_mac[4];
    *pfrm++ = ble_mac[3];
    *pfrm++ = ble_mac[2];
    *pfrm++ = ble_mac[1];
    *pfrm++ = ble_mac[0];
#endif
    len += BLE_MACADDR_SIZE;

    /* AD1 */
    {
        *pfrm++ = 2;            // Len
        *pfrm++ = 0x01;         // Type
        *pfrm++ = 0x05;         // Data
        len += 3;
    }

    /* AD2 */
#if 1
    count = bleEditPayloadAD2(ad2_data);
    *pfrm++ = count + 1;    // Len
    *pfrm++ = 0xFF;         // Type
    len += 2;

    memcpy(pfrm, ad2_data, count);
    pfrm += count;
    len += count;
#else
    count = bleEditPayloadAD2(pfrm);
    pfrm += count;
    len += count;
#endif
    /* AD3 */
    {
        *pfrm++ = 9;                            // Len
        *pfrm++ = 0x09;                         // Type
        *pfrm++ = ble_localName[0];
        *pfrm++ = ble_localName[1];
        *pfrm++ = ble_localName[2];
        *pfrm++ = ble_localName[3];
        if(env_nictparam.deviceRunmode & DEVICE_RUNMODE_TIMER )
        //    if(env_nictparam.deviceRunmode == 0x00)
        // NICT #MOD_190917
        {   // 通常モード
            // ------------------------------------------------------------------------------
             // NICT #MOD_190917
          if (env_nictparam.deviceRunmode & DEVICE_RUNMODE_EVENT)
             {
               uint16_t panid = currentPanid_Lifecount(env_nictparam.blePayloadPanidNormal);
               *pfrm++ = B2H((panid >> 12) & 0xf);
               *pfrm++ = B2H((panid >>  8) & 0xf);
               *pfrm++ = B2H((panid >>  4) & 0xf);
               *pfrm++ = B2H((panid >>  0) & 0xf);
             }
          else{
            *pfrm++ = B2H((env_nictparam.blePayloadPanidNormal >> 12) & 0xf);
            *pfrm++ = B2H((env_nictparam.blePayloadPanidNormal >>  8) & 0xf);
            *pfrm++ = B2H((env_nictparam.blePayloadPanidNormal >>  4) & 0xf);
            *pfrm++ = B2H((env_nictparam.blePayloadPanidNormal >>  0) & 0xf);
          } // else 
        }
        else
        {   // イベントモード
            if(g_mode == MODE_TRANSITION_TO_WALK)
            {   // MODE_TRANSITION_TO_WALK
                *pfrm++ = B2H((env_nictparam.blePayloadPanidWalk >> 12) & 0xf);
                *pfrm++ = B2H((env_nictparam.blePayloadPanidWalk >>  8) & 0xf);
                *pfrm++ = B2H((env_nictparam.blePayloadPanidWalk >>  4) & 0xf);
                *pfrm++ = B2H((env_nictparam.blePayloadPanidWalk >>  0) & 0xf);
            }
            else
            {   // MODE_TRANSITION_TO_RUN
                *pfrm++ = B2H((env_nictparam.blePayloadPanidRun >> 12) & 0xf);
                *pfrm++ = B2H((env_nictparam.blePayloadPanidRun >>  8) & 0xf);
                *pfrm++ = B2H((env_nictparam.blePayloadPanidRun >>  4) & 0xf);
                *pfrm++ = B2H((env_nictparam.blePayloadPanidRun >>  0) & 0xf);
            }
        }
        len += 10;
    }


    return(len);
}
uint16_t updateBleBeaconData()
{
    uint16_t bl_len;

    msgBleData_t* pble = (msgBleData_t*)&sendBle_dp[WISUN_HD_SIZE] ;

    // モード毎の設定値
    if(env_nictparam.beaconExist & BSTmodeBleBeaconType)
    {
        if(env_nictparam.deviceRunmode & DEVICE_RUNMODE_TIMER )
        //    if(env_nictparam.deviceRunmode == 0x00)
        // NICT #MOD_190917
        {
          // ------------------------------------------------------------------------------
          ble_interval = env_nictparam.bleBaseInterval;
          ble_burst_period = env_nictparam.bleBurstPeriod;
          ble_burst_count = env_nictparam.bleBurstCount;
        }
        else
        {   // イベントモード
            if(g_mode == MODE_TRANSITION_TO_WALK)
            {   // MODE_TRANSITION_TO_WALK
                ble_interval = env_nictparam.walkBleInterval;
                ble_burst_period = env_nictparam.walkBleBurstPeriod;
                ble_burst_count = env_nictparam.walkBleBurstCount;
            }
            else
            {   // MODE_TRANSITION_TO_RUN
                ble_interval = env_nictparam.runBleInterval;
                ble_burst_period = env_nictparam.runBleBurstPeriod;
                ble_burst_count = env_nictparam.runBleBurstCount;
            }
        }
    }

    // ble data
    pble->hdr = 0x1140 ;
    bl_len = bleEditPayload((uint8_t *)pble->payload);

    // Header size add
    bl_len += BLE_DATA_HDR ;

    sendBle_dp[0] = bl_len + 7;     // packet total length
    sendBle_dp[1] = PKCOM_BEACON;   //
    sendBle_dp[2] = 0;              //
    sendBle_dp[3] = bl_pkid;        //
    sendBle_dp[4] = 0x03;           //
    sendBle_dp[5] = 1;              //

    if(env_nictparam.beaconExist & BSTmodeBleBeaconType)
    {
        sendBle_dp[HD_TYPE] = BleBeaconType | BSTmodeBleBeaconType;
    }
    else
    {
        sendBle_dp[HD_TYPE] = BleBeaconType;
    }
    sendBle_dp[HD_LENG] = bl_len;

    bl_pkid++;

    // packet length
    g_bledatalength = sendBle_dp[WISUN_HD_LENG];  // data size

    return bl_len ;
}
#endif
// =========================================================
// - end of file
// =========================================================
