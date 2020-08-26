#ifdef ADD_ACCLERATION_2018
#ifndef I2C_LIS2DH12_H
#define I2C_LIS2DH12_H

typedef struct
{
    uint32_t    sleepFlag;
    uint8_t     sleepCounter;
    uint32_t    walkFlag;
    uint8_t     walkCounter;
    uint32_t    runFlag;
    uint8_t     runCounter;

    uint8_t     counter;
} judgement_s;

void i2cTask_init(void);
void setLIS2DH12_RegisterActivemode();

#endif /* I2C_LIS2DH12_H */
#endif
