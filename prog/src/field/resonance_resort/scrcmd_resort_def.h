//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_scrcmd.h
 *	@brief  スクリプトコマンド
 *	@author	Toru=Nagihashi
 *	@date		2011.02.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/*
 *					スクリプトコマンドとソースの共通定義
*/
//=============================================================================
//_RESORT_GET_DATA用
#define RESORT_GET_PARAM_FRONT_OBJID  (0)
//_RESORT_JOB_CHANGE用
#define RESORT_JOBCHANGE_RESIDENT_AUTO      (0)
#define RESORT_JOBCHANGE_RESIDENT_LOT       (1)
#define RESORT_JOBCHANGE_RESIDENT_PARLOR    (2)
#define RESORT_JOBCHANGE_RESIDENT_BREEDER   (3)
#define RESORT_JOBCHANGE_RESIDENT_MARKET    (4)
#define RESORT_JOBCHANGE_RESIDENT_FLORIST   (5)
#define RESORT_JOBCHANGE_RESIDENT_EXRCICE   (6)
#define RESORT_JOBCHANGE_RESIDENT_NURSERY   (7)
#define RESORT_JOBCHANGE_RESIDENT_JEWELLEY  (8)
#define RESORT_JOBCHANGE_RESIDENT_ANTIQUE   (9)
#define RESORT_JOBCHANGE_RESIDENT_CAFE      (10)
#define RESORT_JOBCHANGE_RESIDENT_GYM       (11)
#define RESORT_JOBCHANGE_RESIDENT_HOUSE     (12)
#define RESORT_JOBCHANGE_SECETARY       (13)
#define RESORT_JOBCHANGE_VISITOR        (14)
///_RESORT_ACTION用
#define RESORT_ACTION_LEAVE           (0)
//_RESORT_SET_WORD用
#define RESORT_WORD_PEOPLE            (0)
#define RESORT_WORD_SHOP              (1)
#define RESORT_WORD_SHOPLABEL         (2)
