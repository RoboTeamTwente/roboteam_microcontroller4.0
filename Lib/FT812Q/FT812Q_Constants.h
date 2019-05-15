/*
 * FT812Q_Constants.h
 *
 *  Created on: May 15, 2019
 *      Author: selina
 */

#ifndef FT812Q_FT812Q_CONSTANTS_H_
#define FT812Q_FT812Q_CONSTANTS_H_

/* HOST COMMANDS */
#define DISPLAY_SET_ACTIVE 				0x000000
#define DISPLAY_GET_ACTIVE				0x302000
#define DISPLAY_SET_STANDBY				0x000000
#define DISPLAY_SET_SLEEP				0x000000
#define DISPLAY_SET_POWERDOWN			0x000000

/* REGISTERS */
#define REG_PWM_HZ 			0x3020D0
#define REG_PWM_DUTY		0x3020D4
#define REG_GPIO			0x302094
#define REG_GPIO_DIR		0x302090
#define REG_HCYCLE			0x30202C
#define REG_HOFFSET			0x302030
#define REG_HSYNC0			0x302038
#define REG_HSYNC1			0x30203C
#define REG_VCYCLE			0x302040
#define REG_VOFFSET			0x302044
#define	REG_VSYNC0			0x30204C
#define	REG_VSYNC1			0x302050
#define	REG_SWIZZLE			0x302064
#define	REG_PCLK_POL		0x30206C
#define	REG_CSPREAD			0x302068
#define	REG_HSIZE			0x302034
#define	REG_VSIZE			0x302048
#define REG_DLSWAP			0x302054
#define REG_PCLK			0x302070

/* MEMORY MAP*/
#define RAM_DL				0x300000

#endif /* FT812Q_FT812Q_CONSTANTS_H_ */
