#pragma once
//============================================================================================
/**
 * @file	  c_gear_data.h
 * @brief	  CGEARでセーブしておくデータ
 * @author	k.ohno
 * @date	  2009.05.14
 */
//============================================================================================

#include <gflib.h>
#include "savedata/save_control.h"	//SAVE_CONTROL_WORK


//----------------------------------------------------------
/**
 * @brief	C-GEARの保存情報
 */
//----------------------------------------------------------
typedef struct _CGEAR_SAVEDATA CGEAR_SAVEDATA;

#define C_GEAR_PANEL_WIDTH   (9)
#define C_GEAR_PANEL_HEIGHT  (4)

/// 
typedef enum {
  CGEAR_PANELTYPE_NONE,
  CGEAR_PANELTYPE_IR,
  CGEAR_PANELTYPE_WIRELESS,
  CGEAR_PANELTYPE_WIFI,
  CGEAR_PANELTYPE_BASE,
  CGEAR_PANELTYPE_MAX,
} CGEAR_PANELTYPE_ENUM;



//----------------------------------------------------------
//	セーブデータシステムが依存する関数
//----------------------------------------------------------
extern int CGEAR_SV_GetWorkSize(void);
extern CGEAR_SAVEDATA* CGEAR_SV_AllocWork(HEAPID heapID);
extern void CGEAR_SV_Init(CGEAR_SAVEDATA* pSV);

extern void CGEAR_SV_SetCGearONOFF(CGEAR_SAVEDATA* pSV,BOOL bON);
extern BOOL CGEAR_SV_GetCGearONOFF(CGEAR_SAVEDATA* pSV);
extern void CGEAR_SV_SetCGearPictureONOFF(CGEAR_SAVEDATA* pSV,BOOL bON);
extern BOOL CGEAR_SV_GetCGearPictureONOFF(CGEAR_SAVEDATA* pSV);
extern void CGEAR_SV_SetCGearPictureCRC(CGEAR_SAVEDATA* pSV,u16 pictureCRC);
extern u16 CGEAR_SV_GetCGearPictureCRC(CGEAR_SAVEDATA* pSV);


extern CGEAR_PANELTYPE_ENUM CGEAR_SV_GetPanelType(CGEAR_SAVEDATA* pSV,int x, int y);
extern void CGEAR_SV_SetPanelType(CGEAR_SAVEDATA* pSV,int x, int y, CGEAR_PANELTYPE_ENUM type);

//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------
extern CGEAR_SAVEDATA* CGEAR_SV_GetCGearSaveData(SAVE_CONTROL_WORK* pSave);


