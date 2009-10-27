//============================================================================================
/**
 * @file	timezone.h
 * @brief	ŽžŠÔ‘Ñ‚Ì’è‹`
 * @author	tamada GAME FREAK inc.
 * @date	2006.02.13
 */
//============================================================================================
#pragma once

#ifndef ASM_CPP

typedef enum {
  TIMEZONE_MORNING =	0,
  TIMEZONE_NOON =		  1,
  TIMEZONE_EVENING =	2,
  TIMEZONE_NIGHT =		3,
  TIMEZONE_MIDNIGHT =	4,

  TIMEZONE_MAX,
}TIMEZONE;

#else

#define	TIMEZONE_MORNING	(0)
#define	TIMEZONE_NOON		(1)
#define	TIMEZONE_EVENING	(2)
#define TIMEZONE_NIGHT		(3)
#define TIMEZONE_MIDNIGHT	(4)

#endif
