/*
 * app_power_mng.h
 *
 *  Created on: 2017/03/18
 *      Author: t.miki
 */

#ifndef APPLICATION_APP_POWER_MNG_H_
#define APPLICATION_APP_POWER_MNG_H_

// =========================================================
//

#define POWER_MODE_COUNT (50)         //
#define BATT_LOW_LEVEL   1900         //

int WakeUp();
int GoSleep();

int PowerCtrlInit();
int PowerCtrlClose();
int PowerLevelCheck();
int PowerCtrlCheck();

#endif /* APPLICATION_APP_POWER_MNG_H_ */
