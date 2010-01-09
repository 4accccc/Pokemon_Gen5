//============================================================================================
/**
 * @file	  c_gear_picture.c
 * @brief	  CGEARの画像をセーブする場所
 * @author	k.ohno
 * @date	  2009.11.17
 */
//============================================================================================

#include "gflib.h"

#include "savedata/save_tbl.h"
#include "savedata/c_gear_picture.h"

//--------------------------------------------------------------------------------------------
/**
 * @brief   構造体サイズを得る
 * @return	サイズ
 */
//--------------------------------------------------------------------------------------------
int CGEAR_PICTURE_SAVE_GetWorkSize(void)
{
	return sizeof(CGEAR_PICTURE_SAVEDATA);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   構造体を得る
 * @return	構造体
 */
//--------------------------------------------------------------------------------------------

CGEAR_PICTURE_SAVEDATA* CGEAR_PICTURE_SAVE_AllocWork(HEAPID heapID)
{
	return GFL_HEAP_AllocClearMemory(heapID, CGEAR_PICTURE_SAVE_GetWorkSize());
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   中身を初期化する
 * @return	サイズ
 */
//--------------------------------------------------------------------------------------------

void CGEAR_PICTURE_SAVE_Init(CGEAR_PICTURE_SAVEDATA* pSV)
{
  //デフォルトは無色
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   CGEARのキャラクターを得る
 * @param   CGEAR_PICTURE_SAVEDATA  
 * @return  絵のアドレス
 */
//--------------------------------------------------------------------------------------------
void* CGEAR_PICTURE_SAVE_GetPicture(CGEAR_PICTURE_SAVEDATA* pSV)
{
  return &pSV->picture[0];
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   CGEARのONOFFを得る
 * @param   pSV     CGEARセーブ構造体
 * @return   ONならTRUE OFFならFALSE
 */
//--------------------------------------------------------------------------------------------
void* CGEAR_PICTURE_SAVE_GetPalette(CGEAR_PICTURE_SAVEDATA* pSV)
{
  return &pSV->palette[0];
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   パレットデータが入っているかどうか
 * @param   pSV     CGEARセーブ構造体
 * @return   TRUEならはいっている
 */
//--------------------------------------------------------------------------------------------
BOOL CGEAR_PICTURE_SAVE_IsPalette(CGEAR_PICTURE_SAVEDATA* pSV)
{
  u8 compbuff[CGEAR_PICTURTE_PAL_SIZE];

  GFL_STD_MemClear(compbuff,CGEAR_PICTURTE_PAL_SIZE);

  if(0==GFL_STD_MemComp(compbuff,pSV->palette,CGEAR_PICTURTE_PAL_SIZE)){
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   CGEARパネルタイプ設定する
 * @param   picture     キャラクターデータ
 * @param   palette     パレットデータ
 * @return	
 */
//--------------------------------------------------------------------------------------------
void CGEAR_PICTURE_SAVE_SetData(CGEAR_PICTURE_SAVEDATA* pSV, void* picture, void* palette)
{
  GFL_STD_MemCopy(picture, pSV->picture, CGEAR_PICTURTE_CHAR_SIZE);
  GFL_STD_MemCopy(palette, pSV->palette, CGEAR_PICTURTE_PAL_SIZE);
}

//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------
/*
CGEAR_PICTURE_SAVEDATA* CGEAR_PICTURE_SAVE_GetCGearSaveData(SAVE_CONTROL_WORK* pSave)
{
	CGEAR_PICTURE_SAVEDATA* pData;
	pData = SaveControl_DataPtrGet(pSave, GMDATA_ID_CGEAR);
	return pData;

}
*/
