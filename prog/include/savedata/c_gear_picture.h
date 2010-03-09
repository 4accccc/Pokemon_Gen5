#pragma once
//============================================================================================
/**
 * @file	  c_gear_picture.h
 * @brief	  CGEARでセーブしておく絵のデータ
 * @author	k.ohno
 * @date	  2009.11.17
 */
//============================================================================================

#include <gflib.h>
#include "savedata/save_control.h"	//SAVE_CONTROL_WORK


//----------------------------------------------------------
/**
 * @brief	C-GEARの保存情報
 */
//----------------------------------------------------------
typedef struct _CGEAR_PICTURE_SAVEDATA CGEAR_PICTURE_SAVEDATA;


#define CGEAR_PICTURTE_WIDTH   (17)
#define CGEAR_PICTURTE_HEIGHT  (15)

#define CGEAR_PICTURTE_CHAR_SIZE (CGEAR_PICTURTE_WIDTH*CGEAR_PICTURTE_HEIGHT*32)
#define CGEAR_PICTURTE_PAL_SIZE  (16*2)
#define CGEAR_PICTURTE_SCR_SIZE (2*32*24)
#define CGEAR_PICTURTE_ALL_SIZE (CGEAR_PICTURTE_CHAR_SIZE+CGEAR_PICTURTE_PAL_SIZE+CGEAR_PICTURTE_SCR_SIZE)


//----------------------------------------------------------
//	セーブデータシステムが依存する関数
//----------------------------------------------------------

extern int CGEAR_PICTURE_SAVE_GetWorkSize(void);
extern CGEAR_PICTURE_SAVEDATA* CGEAR_PICTURE_SAVE_AllocWork(HEAPID heapID);
extern void CGEAR_PICTURE_SAVE_Init(CGEAR_PICTURE_SAVEDATA* pSV);
extern void* CGEAR_PICTURE_SAVE_GetPicture(CGEAR_PICTURE_SAVEDATA* pSV);
extern void* CGEAR_PICTURE_SAVE_GetPalette(CGEAR_PICTURE_SAVEDATA* pSV);
extern BOOL CGEAR_PICTURE_SAVE_IsPalette(CGEAR_PICTURE_SAVEDATA* pSV);
extern void CGEAR_PICTURE_SAVE_SetData(CGEAR_PICTURE_SAVEDATA* pSV, void* picture, void* palette);

//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------
extern CGEAR_PICTURE_SAVEDATA* CGEAR_PICTURE_SAVE_GetCGearSaveData(SAVE_CONTROL_WORK* pSave);




//----------------------------------------------------------
/**
 * @brief  構造体定義
 */
//----------------------------------------------------------

struct _CGEAR_PICTURE_SAVEDATA {
	u8 picture[CGEAR_PICTURTE_CHAR_SIZE]; //キャラクター
  u8 palette[CGEAR_PICTURTE_PAL_SIZE];     //パレット
  u8 scr[CGEAR_PICTURTE_SCR_SIZE];   //スクリーン
};

