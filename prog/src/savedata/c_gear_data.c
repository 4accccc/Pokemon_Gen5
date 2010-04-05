//============================================================================================
/**
 * @file	  c_gear_data.c
 * @brief	  CGEARでセーブしておくデータ
 * @author	k.ohno
 * @date	  2009.05.14
 */
//============================================================================================

#include "gflib.h"

#include "savedata/save_tbl.h"
#include "savedata/c_gear_data.h"
#include "system/gf_date.h"

//----------------------------------------------------------
/**
 * @brief  構造体定義
 */
//----------------------------------------------------------

struct _CGEAR_SAVEDATA {
#if 0
  u16 CGearPictCRC;
  u8 gearType;
  u8 dummy;
	u8 type[C_GEAR_PANEL_WIDTH][C_GEAR_PANEL_HEIGHT];
  u8 irMarkx;
  u8 irMarky;
  u8 wirlessMarkx;
  u8 wirlessMarky;
  u8 wifiMarkx;
  u8 wifiMarkx;
  u8 CGearPictureON;
  u8 CGearON;
#else
	u8 type[C_GEAR_PANEL_WIDTH][C_GEAR_PANEL_HEIGHT];
  u16 CGearPictCRC;
  u8 CGearPictureON;
  u8 CGearON;
#endif
};


static u8 _inittable[C_GEAR_PANEL_WIDTH][C_GEAR_PANEL_HEIGHT]={
	{CGEAR_PANELTYPE_NONE,
		CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_NONE,
		CGEAR_PANELTYPE_NONE} ,
	{CGEAR_PANELTYPE_NONE,
		CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_NONE},
	{CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_NONE},
	{CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_BASE},
	{CGEAR_PANELTYPE_GET_ICON_TYPE(CGEAR_PANELTYPE_IR),
		CGEAR_PANELTYPE_GET_ICON_TYPE(CGEAR_PANELTYPE_WIFI),
		CGEAR_PANELTYPE_GET_ICON_TYPE(CGEAR_PANELTYPE_WIRELESS),
		CGEAR_PANELTYPE_NONE},
	//--------------------------------------
	{CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_BASE},
	{CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_NONE},
	{CGEAR_PANELTYPE_NONE,
		CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_NONE},
	{CGEAR_PANELTYPE_NONE,
		CGEAR_PANELTYPE_BASE,
		CGEAR_PANELTYPE_NONE,
		CGEAR_PANELTYPE_NONE},
};



//--------------------------------------------------------------------------------------------
/**
 * @brief   構造体サイズを得る
 * @return	サイズ
 */
//--------------------------------------------------------------------------------------------
int CGEAR_SV_GetWorkSize(void)
{
	return sizeof(CGEAR_SAVEDATA);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   構造体を得る
 * @return	構造体
 */
//--------------------------------------------------------------------------------------------

CGEAR_SAVEDATA* CGEAR_SV_AllocWork(HEAPID heapID)
{
	return GFL_HEAP_AllocClearMemory(heapID, CGEAR_SV_GetWorkSize());
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   中身を初期化する
 * @return	サイズ
 */
//--------------------------------------------------------------------------------------------

void CGEAR_SV_Init(CGEAR_SAVEDATA* pSV)
{
	GFL_STD_MemClear(pSV, CGEAR_SV_GetWorkSize());
	GFL_STD_MemCopy(_inittable,pSV->type,sizeof(_inittable));
}


//--------------------------------------------------------------------------------------------
/**
 * @brief   CGEARのONOFFをおこなう
 * @param   pSV     CGEARセーブ構造体
 * @param   bON     ONならTRUE OFFならFALSE
 */
//--------------------------------------------------------------------------------------------
void CGEAR_SV_SetCGearONOFF(CGEAR_SAVEDATA* pSV,BOOL bON)
{
  pSV->CGearON = bON;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   CGEARのONOFFを得る
 * @param   pSV     CGEARセーブ構造体
 * @return   ONならTRUE OFFならFALSE
 */
//--------------------------------------------------------------------------------------------
BOOL CGEAR_SV_GetCGearONOFF(CGEAR_SAVEDATA* pSV)
{
  return pSV->CGearON;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   CGEARの絵が入っているかどうかONOFFをおこなう
 * @param   pSV     CGEARセーブ構造体
 * @param   bON     ONならTRUE OFFならFALSE
 */
//--------------------------------------------------------------------------------------------
void CGEAR_SV_SetCGearPictureONOFF(CGEAR_SAVEDATA* pSV,BOOL bON)
{
  pSV->CGearPictureON = bON;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   CGEARの絵があるかONOFFを得る
 * @param   pSV     CGEARセーブ構造体
 * @return   ONならTRUE OFFならFALSE
 */
//--------------------------------------------------------------------------------------------
BOOL CGEAR_SV_GetCGearPictureONOFF(CGEAR_SAVEDATA* pSV)
{
  return pSV->CGearPictureON;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief    CGEARの絵が入っているかどうか確認の為CRC保存
 * @param    pSV     CGEARセーブ構造体
 * @param    pictureCRC     CRC
 */
//--------------------------------------------------------------------------------------------
void CGEAR_SV_SetCGearPictureCRC(CGEAR_SAVEDATA* pSV,u16 pictureCRC)
{
  pSV->CGearPictCRC = pictureCRC;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief    CGEARの絵があるか確認の為CRCGET
 * @param    pSV     CGEARセーブ構造体
 * @return   PictureのCRC
 */
//--------------------------------------------------------------------------------------------
u16 CGEAR_SV_GetCGearPictureCRC(CGEAR_SAVEDATA* pSV)
{
  return pSV->CGearPictCRC;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   CGEARパネルタイプを得る
 * @param   x     パネル横位置
 * @param   y     パネル縦位置
 * @return	type  パネルタイプ CGEAR_PANELTYPE_ENUM
 */
//--------------------------------------------------------------------------------------------
CGEAR_PANELTYPE_ENUM CGEAR_SV_GetPanelType(const CGEAR_SAVEDATA* pSV,int x, int y)
{
	GF_ASSERT(x < C_GEAR_PANEL_WIDTH);
  if(!(x < C_GEAR_PANEL_WIDTH)){
    return CGEAR_PANELTYPE_NONE;
  }
  GF_ASSERT(y < C_GEAR_PANEL_HEIGHT);
  if(!(y < C_GEAR_PANEL_HEIGHT)){
    return CGEAR_PANELTYPE_NONE;
  }
  // 上位1ビットはカット
	return pSV->type[x][y] & 0x7f;
}

//----------------------------------------------------------------------------
/**
 *	@brief  CGEAR パネルタイプがICONつきかチェック
 *
 *	@param	pSV セーブ
 *	@param	x   ｘ
 *	@param	y　 ｙ
 *
 *	@retval TRUE  ICONつきの場所
 *	@retval FALSE ICONつきじゃないばしょ
 */
//-----------------------------------------------------------------------------
BOOL CGEAR_SV_IsPanelTypeIcon(const CGEAR_SAVEDATA* pSV,int x, int y)
{
	GF_ASSERT(x < C_GEAR_PANEL_WIDTH);
	GF_ASSERT(y < C_GEAR_PANEL_HEIGHT);
  if(!(x < C_GEAR_PANEL_WIDTH)){
    return FALSE;
  }
  if(!(y < C_GEAR_PANEL_HEIGHT)){
    return FALSE;
  }
  // 上位1ビットはカット
  if( pSV->type[x][y] & 0x80 ){
  	return TRUE;
  }

  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   CGEARパネルタイプを設定
 * @param   x     パネル横位置
 * @param   y     パネル縦位置
 * @param   type  パネルタイプ CGEAR_PANELTYPE_ENUM
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CGEAR_SV_SetPanelType(CGEAR_SAVEDATA* pSV,int x, int y, CGEAR_PANELTYPE_ENUM type)
{
  u32 org_type;
  org_type = type & 0x7f;
	GF_ASSERT(org_type < CGEAR_PANELTYPE_MAX);
  if(!(org_type < CGEAR_PANELTYPE_MAX)){
    return;
  }
	GF_ASSERT(x < C_GEAR_PANEL_WIDTH);
  if(!(x < C_GEAR_PANEL_WIDTH)){
    return;
  }
	GF_ASSERT(y < C_GEAR_PANEL_HEIGHT);
  if(!(y < C_GEAR_PANEL_HEIGHT)){
    return;
  }
	pSV->type[x][y] = type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  パネルタイプのセーブバッファを取得
 *
 *	@param	pSV   ワーク
 *
 *	@return バッファ
 */
//-----------------------------------------------------------------------------
u8* CGEAR_SV_GetPanelTypeBuff( CGEAR_SAVEDATA* pSV )
{
  return &pSV->type[0][0];
}

//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------
CGEAR_SAVEDATA* CGEAR_SV_GetCGearSaveData(SAVE_CONTROL_WORK* pSave)
{
	CGEAR_SAVEDATA* pData;
	pData = SaveControl_DataPtrGet(pSave, GMDATA_ID_CGEAR);
	return pData;

}

