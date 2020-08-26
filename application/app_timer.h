/*
 * app_timer.h
 *
 *  Created on: 2017/03/18
 *      Author: t.miki
 */

#ifndef APPLICATION_APP_TIMER_H_
#define APPLICATION_APP_TIMER_H_

#include <stdint.h>

// ==========================================================
// timer
int StartTimer(int timer, int tNumber) ;

// ==========================================================
// Clock Task initialize
//
int ClockTaskInit() ;

#endif /* APPLICATION_APP_TIMER_H_ */
