//============================================================================================
/**
 * @file		staff_roll_main.h
 * @brief		エンディング・スタッフロール画面
 * @author	Hiroyuki Nakamura
 * @date		10.04.19
 *
 *	モジュール名：SRMAIN
 */
//============================================================================================
#pragma	once

#include "demo/staff_roll.h"

/*
typedef struct {
	PRINTSYS_LSB	fontColor;
	u16	bgColor;
}SRMAIN_VER_DATA;
*/

typedef struct {
//	SRMAIN_VER_DATA	ver;		// バージョン別データ

	GFL_TCB * vtask;		// TCB ( VBLANK )

	int	mainSeq;
	int	nextSeq;

}SRMAIN_WORK;

typedef int (*pSRMAIN_FUNC)(SRMAIN_WORK*);


extern BOOL SRMAIN_Main( SRMAIN_WORK * wk );
