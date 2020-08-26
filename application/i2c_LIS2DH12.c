#ifdef ADD_ACCLERATION_2018

#include <unistd.h>
#include <string.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>

#include "util.h"

/* Example/Board Header files */
#include "Board.h"

#include "mw/env.h"

#include "app_packet.h"
#include "TaskInit.h"
#include "TaskDefine.h"
#include "i2c_LIS2DH12.h"
#include "dualRfTask.h"

#define I2C_TASK_STACKSIZE              640
#define I2C_TASK_PRIORITY               1

// Sensor I2C address
#define LIS2DH12_ADDR                   0x19

// Registers
#define LIS2DH12_REG_OUT_TEMP_L         0x0C // Temperature
#define LIS2DH12_REG_OUT_TEMP_H         0x0D // Temperature
#define LIS2DH12_REG_WHO_AM_I           0x0F // Who am I
#define LIS2DH12_REG_CTRL_REG0          0x1E
#define LIS2DH12_REG_TEMP_CONFIG        0x1F
#define LIS2DH12_REG_CTRL_REG1          0x20 // Control register
#define LIS2DH12_REG_CTRL_REG2          0x21 // Control register
#define LIS2DH12_REG_CTRL_REG3          0x22 // Control register
#define LIS2DH12_REG_CTRL_REG4          0x23 // Control register
#define LIS2DH12_REG_CTRL_REG5          0x24 // Control register
#define LIS2DH12_REG_CTRL_REG6          0x25 // Control register
#define LIS2DH12_REG_OUT_STATUS_REG     0x27 //
#define LIS2DH12_REG_OUT_X_L            0x28 //
#define LIS2DH12_REG_OUT_X_H            0x29 //
#define LIS2DH12_REG_OUT_Y_L            0x2A //
#define LIS2DH12_REG_OUT_Y_H            0x2B //
#define LIS2DH12_REG_OUT_Z_L            0x2C //
#define LIS2DH12_REG_OUT_Z_H            0x2D //
#define LIS2DH12_REG_SERID_1            0xFB // Serial ID 1
#define LIS2DH12_REG_SERID_2            0xFC // Serial ID 2
#define LIS2DH12_REG_SERID_3            0xFD // Serial ID 3
#define LIS2DH12_REG_MANUFACTURER_ID    0xFE // Manufacturer ID
#define LIS2DH12_REG_DEV_ID             0xFF // Device ID

// Fixed values
#define LIS2DH12_VAL_WHO_AM_I           0x33
#define LIS2DH12_VAL_MANF_ID            0x5449
#define LIS2DH12_VAL_DEV_ID             0x1050
#define LIS2DH12_VAL_CONFIG             0x1000 // 14 bit, acquired in sequence

#define LIMIT_VOLTAGE                   0x0248 // 2.28125(V)

// ----- Beacon Send Control Status
typedef enum {
    SND_CONTROL_NONE = 0,         // 無効
    SND_CONTROL_WALK,             // 歩行
    SND_CONTROL_RUN,              // 走行

} beaconSndState;

static Task_Struct i2cTask;
static Char i2cTaskStack[I2C_TASK_STACKSIZE];

/* Pin driver handles */
static PIN_Handle acceleraterIntrruptPinHandle;

/* Global memory storage for a PIN_Config table */
static PIN_State acceleraterIntrruptPinState;

static I2C_Handle      i2c;
static I2C_Transaction i2cTransaction;
static uint8_t         txBuffer[1];
static uint8_t         rxBuffer[1];

// Task Semaphore
Semaphore_Struct sleepSemStruct;
Semaphore_Struct ActiveSemStruct;
Semaphore_Handle sleepSemHandle=NULL;
static Semaphore_Handle activeSemHandle=NULL;

uint8_t isAccSleeping = 0;

static uint8_t dataCounter;
static judgement_s judgement;

PIN_Config acceleraterIntrruptPinTable[] = {
    Board_I2C_INT1  | PIN_INPUT_EN | PIN_NOPULL | PIN_IRQ_POSEDGE,
    Board_I2C_INT2  | PIN_INPUT_EN | PIN_NOPULL | PIN_IRQ_POSEDGE,
    PIN_TERMINATE
};

// Beacon Send Control Clock
static beaconSndState wiSUNSendOnControl;
static beaconSndState bleSendOnControl;
static uint8_t wiSUNSendDataAvailable;
static uint8_t bleSendDataAvailable;
static Clock_Struct wiSUNSendControlClock;
static Clock_Struct bleSendControlClock;

extern void putUartStr(unsigned char *p ,int length);

uint8_t GetCtrlReg1Value()
{
    uint8_t val = 0x00;

    switch(env_nictparam.accSampringPeriod)
    {
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
        val = ((env_nictparam.accSampringPeriod << 4) & 0xF0) + 0x0F;
        break;
    default:
        val = 0x4F;
        break;
    }

    return val;
}

void setLIS2DH12_RegisterInitialize()
{
    uint8_t         txBuffer[2];

#if 1
    // CTRL_REG0 0x1E
    txBuffer[0] = LIS2DH12_REG_CTRL_REG0;
    txBuffer[1] = 0x10;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // TEMP_CFG_REG 0x1F
    txBuffer[0] = LIS2DH12_REG_TEMP_CONFIG;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // CTRL_REG1 0x20
    //  0x4F 00001111b
    /*
     * 0000 power off
     * 0001    1Hz
     * 0010   10Hz
     * 0011   25Hz
     * 0100   50Hz
     * 0101  100Hz
     * 0110  200Hz
     * 0111  400Hz
     * 1000 1620Hz
     * 1001 5376Hz
     *   + 1111 (Low-Power & XYZ)
     */
    txBuffer[0] = LIS2DH12_REG_CTRL_REG1;
    txBuffer[1] = 0x0F;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // CTRL_REG2 0x21
    txBuffer[0] = 0x21;
    txBuffer[1] = 0x0f;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // CTRL_REG3 0x22
    txBuffer[0] = 0x22;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // CTRL_REG4 0x23
    //  0x10 00010000b
    /*
     * 00 +
     *  00  +-2g
     *  01  +-4g
     *  10  +-8g
     *  11 +-16g
     *   + 0000
     */
    txBuffer[0] = 0x23;
    switch(env_nictparam.accDynamicRange)
    {
    // +-2g
    case 0x00:
        txBuffer[1] = 0x00;
        break;
    // +-4g
    case 0x01:
        txBuffer[1] = 0x10;
        break;
    // +-8g
    case 0x02:
        txBuffer[1] = 0x20;
        break;
    // +-16g
    case 0x03:
        txBuffer[1] = 0x30;
        break;
    default:
        txBuffer[1] = 0x10;
        break;
    }

    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;
    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // CTRL_REG5 0x24
    txBuffer[0] = 0x24;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // CTRL_REG6 0x25
    txBuffer[0] = 0x25;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // REFERENCE    0x26
    txBuffer[0] = 0x26;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // FIFO_CTRL_REG 0x2E
    txBuffer[0] = 0x2E;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // INT1_CFG 0x30
    txBuffer[0] = 0x30;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // INT1_THS     0x32
    txBuffer[0] = 0x32;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // INT1_DURATION 0x33
    txBuffer[0] = 0x33;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // INT2_CFG 0x34
    txBuffer[0] = 0x34;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // INT2_THS     0x36
    txBuffer[0] = 0x36;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // INT2_DURATION 0x37
    txBuffer[0] = 0x37;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // CLOCK_CFG 0x38
    txBuffer[0] = 0x38;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // CLOCK_THS 0x3A
    txBuffer[0] = 0x3A;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // TIME_LIMIT 0x3B
    txBuffer[0] = 0x3B;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // TIME_LATENCY 0x3C
    txBuffer[0] = 0x3C;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // TIME_WINDOW 0x3D
    txBuffer[0] = 0x3D;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // ACTTHS 0x3E
    txBuffer[0] = 0x3E;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 1
    // ACT_DUR 0x3F
    txBuffer[0] = 0x3F;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
#if 0
    // CTRL_REG1 0x20
    //  0x4F 01001111b
    /*
     * 0000 power off
     * 0001    1Hz
     * 0010   10Hz
     * 0011   25Hz
     * 0100   50Hz Default
     * 0101  100Hz
     * 0110  200Hz
     * 0111  400Hz
     * 1000 1620Hz
     * 1001 5376Hz
     *   + 1111 (Low-Power & XYZ)
     */
    txBuffer[0] = LIS2DH12_REG_CTRL_REG1;
    txBuffer[1] = 0x4F;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
#endif
}

void setLIS2DH12_RegisterSleepmode()
{
    uint8_t txBuffer[2];

    // CTRL_REG1 0x20
    //  0x0F 00001111b
    /*
     * 0000 power off
     *   + 1111 (Low-Power & XYZ)
     */
    txBuffer[0] = LIS2DH12_REG_CTRL_REG1;
    txBuffer[1] = 0x0F;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);

    // CTRL_REG3 0x22
    //  0x00 00000000b
    txBuffer[0] = 0x22;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);

    // CTRL_REG6 0x25
    //  0x40 01000000b
    txBuffer[0] = 0x25;
    txBuffer[1] = 0x40;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);

    // INT1_CFG 0x30
    //  0x2A 00101010b
    txBuffer[0] = 0x30;
    txBuffer[1] = 0x2A;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);

    // INT1_THS     0x32
    /*
     * 1LSb 16mg  @FS = 2g
     * 1LSb 32mg  @FS = 4g
     * 1LSb 62mg  @FS = 8g
     * 1LSb 186mg @FS = 16g
     */
    txBuffer[0] = 0x32;
    txBuffer[1] = env_nictparam.sleepWakeupValue;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);

    // CTRL_REG1 0x20
    //  0x4F 01001111b
    /*
     * 0100   50Hz
     *   + 1111 (Low-Power & XYZ)
     */
    txBuffer[0] = LIS2DH12_REG_CTRL_REG1;
    txBuffer[1] = GetCtrlReg1Value();
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
}

void setLIS2DH12_RegisterActivemode()
{
    uint8_t         txBuffer[2];

    // CTRL_REG1 0x20
    //  0x0F 00001111b
    /*
     * 0000 power off
     *   + 1111 (Low-Power & XYZ)
     */
    txBuffer[0] = LIS2DH12_REG_CTRL_REG1;
    txBuffer[1] = 0x0F;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);

    // CTRL_REG3 0x22
    //  0x10 00010000b
    /*
     * ZYXDA INT1
     */
    txBuffer[0] = 0x22;
    txBuffer[1] = 0x10;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);

    // CTRL_REG6 0x25
    //  0x00 00000000b
    txBuffer[0] = 0x25;
    txBuffer[1] = 0x00;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;
    I2C_transfer(i2c, &i2cTransaction);

    // CTRL_REG1 0x20
    //  0x4F 01001111b
    /*
     * 0100   50Hz
     *   + 1111 (Low-Power & XYZ)
     */
    txBuffer[0] = LIS2DH12_REG_CTRL_REG1;
    txBuffer[1] = GetCtrlReg1Value();
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
}

/*
 *  ======== acceleraterIntrruptCallbackFxn ========
 */
void acceleraterIntrruptCallbackFxn(PIN_Handle handle, PIN_Id pinId)
{
    /* Debounce logic, only toggle if the button is still pushed (low) */
//    CPUdelay(8000*50);

    if (PIN_getInputValue(pinId))
    {
        /* Toggle LED based on the button pressed */
        switch (pinId)
        {
        case Board_I2C_INT1:
            Semaphore_post(activeSemHandle);
            break;

        case Board_I2C_INT2:
            Semaphore_post(sleepSemHandle);
            break;

        default:
            /* Do nothing */
            break;
        }
    }
}

static void sq(accData* acc)
{
    int16_t x, y, z;
    uint32_t a1, a2, a3;

    x = (int16_t)acc->xData * 10;
    y = (int16_t)acc->yData * 10;
    z = (int16_t)acc->zData * 10;

    a3 = a2 = a1 = (x * x) + (y * y) + (z * z);

    if( a1 == 0 )
    {
        acc->scalarData = 0;
        return;
    }

    while(1)
    {
        a2 = (a2 + a1 / a2) >> 1;
        if( a2/10 == a3/10 )
        {
            acc->scalarData = a3 / 10;
            return;
        }
        a3 = a2;
    }
}

//=========================================================================
// utoa
//=========================================================================
uint8_t *utoa(uint16_t value, uint8_t *s, uint8_t radix)
{
    uint8_t *s1 = s;
    uint8_t *s2 = s;

    do
    {
        *s2++ = "0123456789abcdefghijklmnopqrstuvwxyz"[value % radix];
        value /= radix;
    }
    while (value > 0);

    *s2-- = '\0';

    while (s1 < s2)
    {
        uint8_t c = *s1;
        *s1++ = *s2;
        *s2-- = c;
    }
    return s;
}
//=========================================================================
// itoa
//=========================================================================
uint8_t *intToAscii(int16_t value, uint8_t *s, uint8_t radix)
{
    uint16_t t = value;
    uint8_t *ss = s;

    if (value < 0 && radix == 10)
    {
        *ss++ = '-';
        t = -t;
    }

    utoa(t, ss, radix);

    return s;
}

static void sendAccleratorDataToUart(accData* acc, eModeTransition mode, uint16_t voltage)
{
    uint8_t str[7] = { 0 };
/*
    str[0] = x;
    str[1] = y;
    str[2] = z;
    str[3] = 0x0D;
    str[4] = 0x0A;
*/
    switch(voltage)
    {
    case 0x00:
        putUartStr("H", 1);
        break;
    case 0x01:
        putUartStr("L", 1);
        break;
    default:
        putUartStr("E", 1);
        break;
    }
    switch(mode)
    {
    case MODE_TRANSITION_NONE:
        putUartStr("N", 1);
        break;
    case MODE_TRANSITION_TO_RUN:
        putUartStr("R", 1);
        break;
    case MODE_TRANSITION_TO_WALK:
        putUartStr("W", 1);
        break;
    case MODE_TRANSITION_TO_SLEEP:
        putUartStr("S", 1);
        break;
    default :
        putUartStr("E", 1);
        break;
    }
    intToAscii(acc->xData, str, 10 );
    putUartStr(str, strlen((const char*)str));
    putUartStr(",", 1);
    intToAscii(acc->yData, str, 10 );
    putUartStr(str, strlen((const char*)str));
    putUartStr(",", 1);
    intToAscii(acc->zData, str, 10 );
    putUartStr(str, strlen((const char*)str));
    putUartStr(",", 1);
    intToAscii(acc->scalarData, str, 10 );
    putUartStr(str, strlen((const char*)str));
    putUartStr("\r", 1);
}

static eModeTransition transitionAnalysis()
{
    uint16_t scalarMax = 0;
    uint8_t loopCnt;
    eModeTransition mt = MODE_TRANSITION_NONE;

    // 1窓内の最大値を取得
    for(loopCnt = 0; loopCnt < env_nictparam.activeWindowData; loopCnt++)
    {
        if(scalarMax < g_accDataBuffer[loopCnt].scalarData)
        {
            scalarMax = g_accDataBuffer[loopCnt].scalarData;
        }
    }

    // 各モードの閾値判定
    // SLEEP
    if(scalarMax <= env_nictparam.sleepJudgeValue)
    {
        if(judgement.sleepFlag & (1 << env_nictparam.activeWindowNumber - 1))
        {
            judgement.sleepCounter--;
        }
        judgement.sleepFlag = ((judgement.sleepFlag << 1) & 0xFFFFFFFF) + 1;
        judgement.sleepCounter++;
    }
    else
    {
        if(judgement.sleepFlag & (1 << env_nictparam.activeWindowNumber - 1))
        {
            judgement.sleepCounter--;
        }
        judgement.sleepFlag = (judgement.sleepFlag << 1) & 0xFFFFFFFE;
    }
    if(judgement.sleepCounter >= env_nictparam.sleepJudgeNumber)
    {
        mt = MODE_TRANSITION_TO_SLEEP;
    }

    // WALK
    if(scalarMax >= env_nictparam.walkJudgeValue)
    {
        if(judgement.walkFlag & (1 << env_nictparam.activeWindowNumber - 1))
        {
            judgement.walkCounter--;
        }
        judgement.walkFlag = ((judgement.walkFlag << 1) & 0xFFFFFFFF) + 1;
        judgement.walkCounter++;
    }
    else
    {
        if(judgement.walkFlag & (1 << env_nictparam.activeWindowNumber - 1))
        {
            judgement.walkCounter--;
        }
        judgement.walkFlag = (judgement.walkFlag << 1) & 0xFFFFFFFE;
    }
    if(judgement.walkCounter >= env_nictparam.walkJudgeNumber)
    {
        mt = MODE_TRANSITION_TO_WALK;
    }
    
    // RUN
    if(scalarMax >= env_nictparam.runJudgeValue)
    {
        if(judgement.runFlag & (1 << env_nictparam.activeWindowNumber - 1))
        {
            judgement.runCounter--;
        }
        judgement.runFlag = ((judgement.runFlag << 1) & 0xFFFFFFFF) + 1;
        judgement.runCounter++;
    }
    else
    {
        if(judgement.runFlag & (1 << env_nictparam.activeWindowNumber - 1))
        {
            judgement.runCounter--;
        }
        judgement.runFlag = (judgement.runFlag << 1) & 0xFFFFFFFE;
    }
    if(judgement.runCounter >= env_nictparam.runJudgeNumber)
    {
      if (mt == MODE_TRANSITION_TO_WALK){ /* WalkとRunを同時に満たすときのみRunとみなす */
        mt = MODE_TRANSITION_TO_RUN;
      } // if mt == MODE_TRANSITION_TO_WALK
    }

    return mt;
}

static void readAccData(accData* acc)
{
    // X
    txBuffer[0] = LIS2DH12_REG_OUT_X_H;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;

    I2C_transfer(i2c, &i2cTransaction);
    acc->xData = rxBuffer[0];

    // Y
    txBuffer[0] = LIS2DH12_REG_OUT_Y_H;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;

    I2C_transfer(i2c, &i2cTransaction);
    acc->yData = rxBuffer[0];

    // Z
    txBuffer[0] = LIS2DH12_REG_OUT_Z_H;
    i2cTransaction.slaveAddress = LIS2DH12_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;

    I2C_transfer(i2c, &i2cTransaction);
    acc->zData = rxBuffer[0];
}

static void setWiSUNBeaconData()
{
    // =========================================================
    // Flag を立てる。 立てたフラグは実行時に dualRfTaskFunction にて解除
    //
    g_BeaconType |= WisunBeaconType;
    if(env_nictparam.beaconExist & BSTmodeWisunBeaconType)
    {
        g_BeaconType |= BSTmodeWisunBeaconType;
    }
    updateWiSUNBeaconData();
    wiSUNSendDataAvailable = TRUE;
}
static void setBleBeaconData()
{
    // =========================================================
    // Flag を立てる。立てたFlagは、実行時にdualRfTaskFunction にて解除
    //
    g_BeaconType |= BleBeaconType;
    if(env_nictparam.beaconExist & BSTmodeBleBeaconType)
    {
        g_BeaconType |= BSTmodeBleBeaconType;
    }
    updateBleBeaconData();
    bleSendDataAvailable = TRUE;
}
static void wiSUNSendControl_clockHandler(UArg arg)
{
    if(wiSUNSendDataAvailable)
    {
        g_BeaconType |= WisunBeaconType;
        if(env_nictparam.beaconExist & BSTmodeWisunBeaconType)
        {
            g_BeaconType |= BSTmodeWisunBeaconType;
        }
        SendRf(WisunBeaconType);
        wiSUNSendDataAvailable = FALSE;
        wisun_base_seqno++;

        if(g_mode == MODE_TRANSITION_TO_WALK)
        {   // MODE_TRANSITION_TO_WALK
            Util_rescheduleClock(&wiSUNSendControlClock, ((uint32_t)env_nictparam.walkWisunInterval * 100));
        }
        else
        {   // MODE_TRANSITION_TO_RUN
            Util_rescheduleClock(&wiSUNSendControlClock, ((uint32_t)env_nictparam.runWisunInterval * 100));
        }
    }
    else
    {
        Util_stopClock(&wiSUNSendControlClock);
        wiSUNSendOnControl = SND_CONTROL_NONE;
    }
}
static void bleSendControl_clockHandler(UArg arg)
{
    if(bleSendDataAvailable)
    {
        g_BeaconType |= BleBeaconType;
        if(env_nictparam.beaconExist & BSTmodeBleBeaconType)
        {
            g_BeaconType |= BSTmodeBleBeaconType;
        }
        SendRf(BleBeaconType);
        bleSendDataAvailable = FALSE;
        ble_seqno++;
#if 0
        if(g_mode == MODE_TRANSITION_TO_WALK)
        {   // MODE_TRANSITION_TO_WALK
            Util_rescheduleClock(&bleSendControlClock, (env_nictparam.walkBleInterval * 100));
        }
        else
        {   // MODE_TRANSITION_TO_RUN
            Util_rescheduleClock(&bleSendControlClock, (env_nictparam.runBleInterval * 100));
        }
#endif
    }
    else
    {
        Util_stopClock(&bleSendControlClock);
        bleSendOnControl = SND_CONTROL_NONE;
    }
}

static void i2cTaskFxn(UArg a0, UArg a1)
{
  accData         acc;
  I2C_Params      i2cParams;
  uint16_t        wisunClockTime = 0;
  uint16_t        bleClockTIme = 0;
  uint16_t        voltage = 0x0000;

  // 電圧チェック
  HWREG(0x40095000) = 3;

  /* Call driver init functions */
  I2C_init();

  /* Create I2C for usage */
  I2C_Params_init(&i2cParams);
  //    i2cParams.bitRate = I2C_100kHz;
  i2cParams.bitRate = I2C_400kHz;
  i2c = I2C_open(Board_I2C, &i2cParams);
  if (i2c == NULL)
    {
      while (1);
    }

  // Accelerator Interrupter
  acceleraterIntrruptPinHandle = PIN_open(&acceleraterIntrruptPinState, acceleraterIntrruptPinTable);
  if(!acceleraterIntrruptPinHandle)
    {
      /* Error initializing button pins */
      while(1);
    }
  /* Setup callback for button pins */
  if (PIN_registerIntCb(acceleraterIntrruptPinHandle, &acceleraterIntrruptCallbackFxn) != 0)
    {
      /* Error registering button callback function */
      while(1);
    }

  txBuffer[0] = LIS2DH12_REG_WHO_AM_I;
  i2cTransaction.slaveAddress = LIS2DH12_ADDR;
  i2cTransaction.writeBuf = txBuffer;
  i2cTransaction.writeCount = 1;
  i2cTransaction.readBuf = rxBuffer;
  i2cTransaction.readCount = 1;
  if (I2C_transfer(i2c, &i2cTransaction))
    {
      if(rxBuffer[0] != LIS2DH12_VAL_WHO_AM_I)
        {
          while(1);
        }
    }

  // 加速度センサ設定レジスタ初期化
  setLIS2DH12_RegisterInitialize();

  // 1窓用カウンタ
  dataCounter = 0;

  // 判定用変数初期化
  judgement.sleepFlag = 0;
  judgement.sleepCounter = 0;
  judgement.walkFlag = 0;
  judgement.walkCounter = 0;
  judgement.runFlag = 0;
  judgement.runCounter = 0;
  judgement.counter = 0;

  g_mode = MODE_TRANSITION_NONE;


#if 0
  /* Take 20 samples and print them out onto the console */
  while(1)
    {
      /* Sleep for 1 second */
      sleep(1);
      // 加速度センサデータ読み込み
      readAccData(&acc);
      // スカラー計算
      sq(&acc);
      if(g_Power_mode == Power_mode )
        {
          sendAccleratorDataToUart(&acc);
        }
    }
#else
  
  
  wiSUNSendOnControl = SND_CONTROL_NONE;
  bleSendOnControl = SND_CONTROL_NONE;
  wiSUNSendDataAvailable = FALSE;
  bleSendDataAvailable = FALSE;
  wisunClockTime = (wisun_interval * 100);
  bleClockTIme = (ble_interval * 100);
  Util_constructClock(&wiSUNSendControlClock, wiSUNSendControl_clockHandler, wisunClockTime, wisunClockTime, false, 0);
  Util_constructClock(&bleSendControlClock, bleSendControl_clockHandler, bleClockTIme, bleClockTIme, false, 0);

  setLIS2DH12_RegisterSleepmode();
  //        test();
  //        readAccData(&acc);
  //        Semaphore_post(sleepSemHandle);

  while(1)
    {
      isAccSleeping = 1;
      Semaphore_pend(sleepSemHandle, BIOS_WAIT_FOREVER);
      isAccSleeping = 0;
      
      setLIS2DH12_RegisterActivemode();

      while(1)
        {
          Semaphore_pend(activeSemHandle, BIOS_WAIT_FOREVER);

          // 加速度センサデータ読み込み
          readAccData(&acc);
          // スカラー計算
          sq(&acc);
          // バッファ格納
          g_accDataBuffer[dataCounter] = acc;
          dataCounter++;

          // WRITE UART for Debug Mode
          if(g_Debug_Flag && (g_Power_mode == Power_mode))
            {
              // UART送信
              sendAccleratorDataToUart(&acc, g_mode, g_voltage);
            }
          
          if(dataCounter >= env_nictparam.activeWindowData)
            {                  
              g_mode = transitionAnalysis();
              dataCounter = 0;
              // 電圧読み込み
              voltage = HWREG(0x40095000 + 0x00000028);
              if(voltage < LIMIT_VOLTAGE)
                {
                  g_voltage = 0x01;
                }
              else
                {
                  g_voltage = 0x00;
                }

              if (env_nictparam.deviceRunmode & DEVICE_RUNMODE_TIMER) { /* ライフカウントモード */
                if(g_mode == MODE_TRANSITION_TO_RUN)
                  {
                    g_LifeCounter = 0 ;         // Life Count Clear
                    g_DeathCounter = 0;
                    g_DeathCount_increment = 0;
                    g_SublifeCounter = 0;
                    g_SublifeCount_increment = 0;
                    
//                    if((env_nictparam.beaconExist & WisunBeaconType) && env_nictparam.WiSUNBaseBeaconEn)
//                      {setWiSUNBeaconData();}
//                    if((env_nictparam.beaconExist & BleBeaconType) && env_nictparam.bleBaseBeaconEn)
//                      {setBleBeaconData();}
                  } else if(g_mode == MODE_TRANSITION_TO_WALK)
                  {
                    /* walkのときはパラメータリセットしない */
                    
//                    if((env_nictparam.beaconExist & WisunBeaconType) && env_nictparam.WiSUNBaseBeaconEn)
//                      {setWiSUNBeaconData();}
//                    if((env_nictparam.beaconExist & BleBeaconType) && env_nictparam.bleBaseBeaconEn)
//                      {setBleBeaconData();}
                  } else if(g_mode == MODE_TRANSITION_NONE)
                  {
//                    if((env_nictparam.beaconExist & WisunBeaconType) && env_nictparam.WiSUNBaseBeaconEn)
//                      {setWiSUNBeaconData();}
//                    if((env_nictparam.beaconExist & BleBeaconType) && env_nictparam.bleBaseBeaconEn)
//                      {setBleBeaconData();}
                  } else if(g_mode == MODE_TRANSITION_TO_SLEEP)
                  {
                    // 1窓用カウンタ
                    dataCounter = 0;

                    // 判定用変数初期化
                    judgement.sleepFlag = 0;
                    judgement.sleepCounter = 0;
                    judgement.walkFlag = 0;
                    judgement.walkCounter = 0;
                    judgement.runFlag = 0;
                    judgement.runCounter = 0;
                    judgement.counter = 0;

                    setLIS2DH12_RegisterSleepmode();
                    break;
                  }
              }
              else{             /* 通常イベントモード */
                if(g_mode == MODE_TRANSITION_TO_RUN)
                  {
                    if((env_nictparam.beaconExist & WisunBeaconType) && env_nictparam.WiSUNBaseBeaconEn)
                      {
                        setWiSUNBeaconData();
                        if(wiSUNSendOnControl != SND_CONTROL_RUN)
                          {
                            wiSUNSendOnControl = SND_CONTROL_RUN;
                            SendRf(WisunBeaconType);
                            wiSUNSendDataAvailable = FALSE;
                            wisun_base_seqno++;
                            Util_rescheduleClock(&wiSUNSendControlClock, ((uint32_t)env_nictparam.runWisunInterval * 100));
                            Util_startClock(&wiSUNSendControlClock);
                          }
                      }
                    // send Ble Beacon
                    if((env_nictparam.beaconExist & BleBeaconType) && env_nictparam.bleBaseBeaconEn)
                      {
                        setBleBeaconData();
                        if(bleSendOnControl != SND_CONTROL_RUN)
                          {
                            bleSendOnControl = SND_CONTROL_RUN;
                            SendRf(BleBeaconType);
                            bleSendDataAvailable = FALSE;
                            ble_seqno++;
                            Util_rescheduleClock(&bleSendControlClock, ((uint32_t)env_nictparam.runBleInterval * 100));
                            Util_startClock(&bleSendControlClock);
                          }
                      }                  
                  } 
                else if(g_mode == MODE_TRANSITION_TO_WALK)
                  {
                    // send Wi-SUN Beacon
                    if((env_nictparam.beaconExist & WisunBeaconType) && env_nictparam.WiSUNBaseBeaconEn)
                      {
                        setWiSUNBeaconData();
                        if(wiSUNSendOnControl != SND_CONTROL_WALK)
                          {
                            wiSUNSendOnControl = SND_CONTROL_WALK;
                            SendRf(WisunBeaconType);
                            wisun_base_seqno++;
                            Util_rescheduleClock(&wiSUNSendControlClock, ((uint32_t)env_nictparam.walkWisunInterval * 100));
                            Util_startClock(&wiSUNSendControlClock);
                          }
                      }

                    // send Ble Beacon
                    if((env_nictparam.beaconExist & BleBeaconType) && env_nictparam.bleBaseBeaconEn)
                      {
                        setBleBeaconData();
                        if(bleSendOnControl != SND_CONTROL_WALK)
                          {
                            bleSendOnControl = SND_CONTROL_WALK;
                            SendRf(BleBeaconType);
                            ble_seqno++;
                            Util_rescheduleClock(&bleSendControlClock, ((uint32_t)env_nictparam.walkBleInterval * 100));
                            Util_startClock(&bleSendControlClock);
                          }
                      }

                  }
                else if(g_mode == MODE_TRANSITION_TO_SLEEP)
                  {
                    // 1窓用カウンタ
                    dataCounter = 0;

                    // 判定用変数初期化
                    judgement.sleepFlag = 0;
                    judgement.sleepCounter = 0;
                    judgement.walkFlag = 0;
                    judgement.walkCounter = 0;
                    judgement.runFlag = 0;
                    judgement.runCounter = 0;
                    judgement.counter = 0;

                    setLIS2DH12_RegisterSleepmode();
                    break;
                  }
              }// else 

            }
        }
    }
 
#endif
  /* Deinitialized I2C */
  //    I2C_close(i2c);

  //    return;
}

void i2cTask_init(void)
{
    // CREATE SEMAPHORE
    {
        Semaphore_Params sleepSemParam;
        Semaphore_Params ActiveSemParam;

        // =========================================================
        /* Create semaphore used for callers to wait for result */
        Semaphore_Params_init(&sleepSemParam);
        Semaphore_construct(&sleepSemStruct, 0, &sleepSemParam);
        sleepSemHandle = Semaphore_handle(&sleepSemStruct);
        // =========================================================
        /* Create semaphore used for callers to wait for result */
        Semaphore_Params_init(&ActiveSemParam);
        Semaphore_construct(&ActiveSemStruct, 0, &ActiveSemParam);
        activeSemHandle = Semaphore_handle(&ActiveSemStruct);
    }

    // CREATE TASK
    {
        Task_Params taskParames;

        Task_Params_init(&taskParames);
        taskParames.stackSize = I2C_TASK_STACKSIZE;
        taskParames.priority = I2C_TASK_PRIORITY;
        taskParames.stack = &i2cTaskStack;

        Task_construct(&i2cTask, i2cTaskFxn, &taskParames, NULL);
    }
}
#endif
