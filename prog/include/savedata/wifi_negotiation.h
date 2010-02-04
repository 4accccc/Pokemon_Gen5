#pragma once
//============================================================================================
/**
 * @file	  wifi_negotiation.h
 * @brief	  WIFIネゴシエーション用ともだち履歴データ
 * @author	k.ohno
 * @date	  2009.10.16
 */
//============================================================================================

#include <gflib.h>
#include <dwc.h>
#include "savedata/save_control.h"	//SAVE_CONTROL_WORK
#include "savedata/mystatus.h"


//----------------------------------------------------------
/**
 * @brief	C-GEARの保存情報
 */
//----------------------------------------------------------
typedef struct _WIFI_NEGOTIATION_SAVEDATA WIFI_NEGOTIATION_SAVEDATA;

#define WIFI_NEGOTIATION_DATAMAX   (100)



//----------------------------------------------------------
//	セーブデータシステムが依存する関数
//----------------------------------------------------------
extern int WIFI_NEGOTIATION_SV_GetWorkSize(void);
extern WIFI_NEGOTIATION_SAVEDATA* WIFI_NEGOTIATION_SV_AllocWork(HEAPID heapID);
extern void WIFI_NEGOTIATION_SV_Init(WIFI_NEGOTIATION_SAVEDATA* pSV);

extern void WIFI_NEGOTIATION_SV_SetFriend(WIFI_NEGOTIATION_SAVEDATA* pSV,const MYSTATUS* pMyStatus);
extern s32 WIFI_NEGOTIATION_SV_GetFriend(WIFI_NEGOTIATION_SAVEDATA* pSV,u32 index);
extern MYSTATUS* WIFI_NEGOTIATION_SV_GetMyStatus(WIFI_NEGOTIATION_SAVEDATA* pSV,u32 index);
extern BOOL WIFI_NEGOTIATION_SV_IsCheckFriend(WIFI_NEGOTIATION_SAVEDATA* pSV,s32 profile);
extern int WIFI_NEGOTIATION_SV_GetCount(WIFI_NEGOTIATION_SAVEDATA* pSV);

//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------
extern WIFI_NEGOTIATION_SAVEDATA* WIFI_NEGOTIATION_SV_GetSaveData(SAVE_CONTROL_WORK* pSave);


