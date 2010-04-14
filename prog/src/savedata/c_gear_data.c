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
#include "gamesystem/game_data.h"
#include "system/gf_date.h"


#define CGEAR_PANELTYPE_GET_ICON_TYPE(type) ((type) | 0x80)
#define CGEAR_PANELTYPE_GET_LAST_IR(type) ((type) | 0x40)
#define CGEAR_PANELTYPE_GET_LAST_WIFI(type) ((type) | 0x20)
#define CGEAR_PANELTYPE_GET_LAST_WIRELESS(type) ((type) | 0x10)

//----------------------------------------------------------
/**
 * @brief  構造体定義
 */
//----------------------------------------------------------

struct _CGEAR_SAVEDATA {
	u8 type[C_GEAR_PANEL_WIDTH][C_GEAR_PANEL_HEIGHT];
  u16 CGearPictCRC;
  u8 CGearPictureON;
  u8 CGearON;
  u8 gearType;
  u8 CGearPowerON;
  u8 dummy2;
  u8 dummy3;
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
 * @brief   CGEARの絵柄の記録をおこなう
 * @param   pSV     CGEARセーブ構造体
 * @param   type    タイプ
 */
//--------------------------------------------------------------------------------------------
void CGEAR_SV_SetCGearType(CGEAR_SAVEDATA* pSV,u8 type)
{
  pSV->gearType = type;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   CGEARの絵柄を得る
 * @param   pSV     CGEARセーブ構造体
 * @return  type    タイプ
 */
//--------------------------------------------------------------------------------------------
u8 CGEAR_SV_GetCGearType(CGEAR_SAVEDATA* pSV)
{
  return pSV->gearType;
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
  // 上位4ビットはカット
	return (pSV->type[x][y] & 0x0f);
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

//----------------------------------------------------------------------------
/**
 *	@brief  １つまえか？
 *
 *	@param	pSV セーブ
 *	@param	x   ｘ
 *	@param	y   ｙ
 *
 *	@retval TRUE
 *	@retval FALSE
 */
//-----------------------------------------------------------------------------
BOOL CGEAR_SV_IsPanelTypeLast(const CGEAR_SAVEDATA* pSV,int x, int y, CGEAR_PANELTYPE_ENUM type )
{
  u32 bit;
	GF_ASSERT(x < C_GEAR_PANEL_WIDTH);
	GF_ASSERT(y < C_GEAR_PANEL_HEIGHT);
  if(!(x < C_GEAR_PANEL_WIDTH)){
    return FALSE;
  }
  if(!(y < C_GEAR_PANEL_HEIGHT)){
    return FALSE;
  }

  bit = 0;
  if( type == CGEAR_PANELTYPE_IR ){
    bit = 0x40;
  }else if( type == CGEAR_PANELTYPE_WIFI ){
    bit = 0x20;
  }else if( type == CGEAR_PANELTYPE_WIRELESS ){
    bit = 0x10;
  }

  // 上位1ビットはカット
  if( pSV->type[x][y] & bit ){
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
 * @param   icon  ICONをつける場所
 * @param   last  １つ前にICONがあった場所
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CGEAR_SV_SetPanelType(CGEAR_SAVEDATA* pSV,int x, int y, CGEAR_PANELTYPE_ENUM type, BOOL icon, BOOL last)
{
  u32 set_type;
	GF_ASSERT(type < CGEAR_PANELTYPE_MAX);
  if(!(type < CGEAR_PANELTYPE_MAX)){
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
  set_type = type;
  if( (icon == TRUE) && 
      ( (type == CGEAR_PANELTYPE_IR) ||
      (type == CGEAR_PANELTYPE_WIFI) ||
      (type == CGEAR_PANELTYPE_WIRELESS) ) ){

    set_type = CGEAR_PANELTYPE_GET_ICON_TYPE(set_type);
  }
  if( last ){
    if( type == CGEAR_PANELTYPE_IR ){
      set_type = CGEAR_PANELTYPE_GET_LAST_IR(set_type);
    }else if( type == CGEAR_PANELTYPE_WIFI ){
      set_type = CGEAR_PANELTYPE_GET_LAST_WIFI(set_type);
    }else if( type == CGEAR_PANELTYPE_WIRELESS ){
      set_type = CGEAR_PANELTYPE_GET_LAST_WIRELESS(set_type);
    }
  }
	pSV->type[x][y] = set_type;
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


//----------------------------------------------------------------------------
/**
 *	@brief  CGEAR 電源　設定
 *
 *	@param	pSV     セーブデータ
 *	@param	flag    TRUE：ON　　FALSE：OFF
 */
//-----------------------------------------------------------------------------
void CGEAR_SV_SetPowerFlag( CGEAR_SAVEDATA* pSV, BOOL flag )
{
  pSV->CGearPowerON = flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief  CGEARの電源ON・OFFを取得
 *
 *	@param	cpSV  セーブデータ
 *
 *	@retval TRUE    ON
 *	@retval FALSE   OFF
 */
//-----------------------------------------------------------------------------
BOOL CGEAR_SV_GetPowerFlag( const CGEAR_SAVEDATA* cpSV )
{
  return cpSV->CGearPowerON;
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

CGEAR_SAVEDATA* GAMEDATA_GetCGearSaveData(GAMEDATA* pGameData)
{
	return CGEAR_SV_GetCGearSaveData(GAMEDATA_GetSaveControlWork(pGameData));
}

