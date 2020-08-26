/*
 * TaskDefine.h
 *
 *  Created on: 2016/11/26
 *      Author: t.miki
 */

#ifndef APPLICATION_TASKDEFINE_H_
#define APPLICATION_TASKDEFINE_H_

// =========================================================
#if 1
// timer base 10msec
//#define  Base_Timer       10
// Timer 間隔を100ms に変更			2017.07.06 H.Ohuchi
#define  Base_Timer       100

#define  Base_Timer100    1000

//#define  BLE_TIME_OFFSET  5
#define  BLE_TIME_OFFSET  0
// Ble　WiSUN Timer オフセット　（微妙にずらす）
// 500( msec)
#else
// timer base 100msec
#define  Base_Timer       100
#define  Base_Timer100    100

#endif
// =========================================================
// application

#define    BleBeaconType  0x01
#define    WisunBeaconType  0x02
//    BSTmodeBeaconYype = 0x04,
#define    BSTmodeWisunBeaconType 0x04
#define    BSTmodeBleBeaconType 0x08

typedef enum {
    PowerLow_LEVEL = 0xff ,     // batt level low
    LowPower_mode  = 0x00 ,
    Power_mode     = 0x01 ,
    Power_CTSmode  = 0x02 ,
}ePowerMode;

// =========================================================
//
#define fecMode_CW         0
#define fecMode_GFSK       0
#define fecMode_2GFSK      0

// Tx Power
#define txPowe_14           0xA73F
#define txPowe_8            0x24CB
#define txPowe_4            0x18C6

// Frequency: 920.00000 MHz
#define Frequency_920       0x0398

// Frequency: 868.00000 MHz
#define Frequency_868       0x0364

// Frequency: 2.3GHz
#define Frequency_2440      2440

#define RF_MODE_920         0
#define RF_MODE_2400        1

#define FEC_MODE_CW         0   // CW
#define FEC_MODE_GFSK       1   // GFSK
#define FEC_MODE_2GFSK      2   // 2-GFSK

// =========================================================
//
#define RF_CHANNEL_BUNDLE_1	0
#define RF_CHANNEL_BUNDLE_2	1
#define RF_CHANNEL_BUNDLE_5	5

///////////////////////////////////////////////////////////////////////////////
// SUB-1GHz
#define PREAMBLE_LENGTH		4
#define SYNCWORD_LENGTH		4
#define LENGTH_LENGTH		2
#define HEADER_LENGTH		0
#define CRC_LENGTH			4

#define BPS_050				50
#define BPS_100				100
#define BPS_500				500
#define PAYLOAD_LEN_050		1024
#define PAYLOAD_LEN_100		2048
#define PAYLOAD_LEN_500		2048
#define TIME_THRESHOLD_050	6
#define TIME_THRESHOLD_100	3
#define TIME_THRESHOLD_500	2
#define TIME_LIMIT_050		200000
#define TIME_LIMIT_100		200000
#define TIME_LIMIT_500		100000

#define TIME_COEFFICIENT	1
//#define TIME_COEFFICIENT    2.5
#define SEND_STOCK_MAX		360000000 * TIME_COEFFICIENT
/**
 * TX-POWER
 *   14:0xAB3F
 *   12:0xBC2B
 *   11:0x90E5
 *   10:0x58D8
 *    9:0x40D2
 *    8:0x32CE
 *    7:0x2ACB
 *    6:0x24C9
 *    5:0x20C8
 *    4:0x1844
 *    3:0x1CC6
 *    2:0x18C5
 *    1:0x16C4
 *    0:0x12C3
 *  -10:0x04C0
 */
#define SUB1G_TXPOWER_P14	0xAB3F
#define SUB1G_TXPOWER_P12	0xBC2B
#define SUB1G_TXPOWER_P11	0x90E5
#define SUB1G_TXPOWER_P10	0x58D8
#define SUB1G_TXPOWER_P09	0x40D2
#define SUB1G_TXPOWER_P08	0x32CE
#define SUB1G_TXPOWER_P07	0x2ACB
#define SUB1G_TXPOWER_P06	0x24C9
#define SUB1G_TXPOWER_P05	0x20C8
#define SUB1G_TXPOWER_P04	0x1844
#define SUB1G_TXPOWER_P03	0x1CC6
#define SUB1G_TXPOWER_P02	0x18C5
#define SUB1G_TXPOWER_P01	0x16C4
#define SUB1G_TXPOWER_P00	0x12C3
#define SUB1G_TXPOWER_M10	0x04C0

// =========================================================
// FREQUENCY
//

#define FIRST_CHANNNEL      33
#define LAST_CHANNEL        60
#define SUB1G_FREQ_922      922
#define SUB1G_FREQ_923      923
#define SUB1G_FREQ_924      924
#define SUB1G_FREQ_925      925
#define SUB1G_FREQ_926      926
#define SUB1G_FREQ_927      927
#define SUB1G_FREQ_928      928

// =========================================================
//

#define SUB1G_CORRECT		0x0333
//#define SUB1G_CORRECT		0x0000
#define SUB1G_FRACT_0		0x0000 + SUB1G_CORRECT
#define SUB1G_FRACT_1		0x1999 + SUB1G_CORRECT
#define SUB1G_FRACT_2		0x3333 + SUB1G_CORRECT
#define SUB1G_FRACT_3		0x4CCC + SUB1G_CORRECT
#define SUB1G_FRACT_4		0x6666 + SUB1G_CORRECT
#define SUB1G_FRACT_5		0x8000 + SUB1G_CORRECT
#define SUB1G_FRACT_6		0x9999 + SUB1G_CORRECT
#define SUB1G_FRACT_7		0xB333 + SUB1G_CORRECT
#define SUB1G_FRACT_8		0xCCCC + SUB1G_CORRECT
#define SUB1G_FRACT_9		0xE666 + SUB1G_CORRECT
/**
 * Symbol Rate
 *  preScale [0...3]
 *  rateWord [8...28]
 *
 *   50 kBaud  preScale:0xF rateWord:0x8000
 *  100 kBaud  preScale:0xF rateWord:0x10000
 *  200 kBaud  preScale:0xF rateWord:0x20000
 *  250 kBaud  preScale:0x6 rateWord:0x10000
 *  400 kBaud  preScale:0xF rateWord:0x40000
 *  500 kBaud  preScale:0x6 rateWord:0x20000
 */
#define PRESCALE_050K		0xF
#define PRESCALE_100K		0xF
#define PRESCALE_200K		0xF
#define PRESCALE_250K		0x6
#define PRESCALE_400K		0xF
#define PRESCALE_500K		0x6

#define RATEWORD_050K		0x8000
#define RATEWORD_100K		0x10000
#define RATEWORD_200K		0x20000
#define RATEWORD_250K		0x10000
#define RATEWORD_400K		0x40000
#define RATEWORD_500K		0x20000

#define MODTYPE_050K		0x1
#define MODTYPE_100K		0x1
#define MODTYPE_500K		0x1

#define DEVIATION_050K		0x64
#define DEVIATION_100K		0xC8
//#define DEVIATION_500K		0x640
#define DEVIATION_500K		0x1F4

/**
 * BLE TX-POWER
 *   9:0x3D3F
 *   8:0x6763
 *   7:0x772E
 *   6:0x6326
 *   5:0x5F3C
 *   4:0x7734
 *   3:0x7F2C
 *   2:0x6F26
 *   1:0x6321
 *   0:0x5B1D
 *  -3:0x2DEF
 *  -6:0x25E3
 *  -9:0x1DDA
 * -12:0x19D4
 * -15:0x15CE
 * -18:0x0DCB
 * -21:0x0DC8
 */
#define BLE_TXPOWER_P09		0x3D3F
#define BLE_TXPOWER_P08		0x6737
#define BLE_TXPOWER_P07		0x772E
#define BLE_TXPOWER_P06		0x6326
#define BLE_TXPOWER_P05		0x5F3C
#define BLE_TXPOWER_P04		0x7734
#define BLE_TXPOWER_P03		0x7F2C
#define BLE_TXPOWER_P02		0x6F26
#define BLE_TXPOWER_P01		0x6321
#define BLE_TXPOWER_P00		0x5B1D
#define BLE_TXPOWER_M03		0x2DEF
#define BLE_TXPOWER_M06		0x25E3
#define BLE_TXPOWER_M09		0x1DDA
#define BLE_TXPOWER_M12		0x19D4
#define BLE_TXPOWER_M15		0x15CE
#define BLE_TXPOWER_M18		0x0DCB
#define BLE_TXPOWER_M21		0x0DC8


// =========================================================
/* TX Configuration */
#define DATA_ENTRY_HEADER_SIZE 8  /* Constant header size of a Generic Data Entry */
//#define MAX_LENGTH             243 /* Max length byte the radio will accept */
#define MAX_LENGTH             31 /* Max length byte the radio will accept */
#define NUM_DATA_ENTRIES       8  /* NOTE: Only two data entries supported at the moment */
//#define NUM_DATA_ENTRIES       2  /* NOTE: Only two data entries supported at the moment */
#define NUM_APPENDED_BYTES     1  /* Length byte included in the stored packet */

#endif /* APPLICATION_TASKDEFINE_H_ */
