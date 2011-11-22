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

#include "c_gear_data_local.h"    // struct _CGEAR_SAVEDATA
#include "options_local.h"    //OPTIONS_SAVEDATA

#if 0
#define CGEAR_PANELTYPE_GET_ICON_TYPE(type) ((type) | 0x80)
#define CGEAR_PANELTYPE_GET_LAST_IR(type) ((type) | 0x40)
#define CGEAR_PANELTYPE_GET_LAST_WIFI(type) ((type) | 0x20)
#define CGEAR_PANELTYPE_GET_LAST_WIRELESS(type) ((type) | 0x10)
#endif

//ポジション情報はC-GEAR修正仕様.xls->[ボタン配置位置]シートを参照のこと
#define PANEL_INIT_POS_IR       (12)
#define PANEL_INIT_POS_WIRELESS (21)
#define PANEL_INIT_POS_WIFI     (9)

//初期カスタムカラー
#define INITIAL_RING_COLOR  (0) //0がデフォルトカラー(男:青、女:ピンク)

SDK_COMPILER_ASSERT( sizeof(CGEAR_SAVEDATA) == 52 );

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
  int i;

	GFL_STD_MemClear(pSV, CGEAR_SV_GetWorkSize());

  for( i = 0;i < CGEAR_CUSTOM_COLOR_NUM;i++ ){
    pSV->custom_col[i] = INITIAL_RING_COLOR;
  }

  pSV->panel_pos[CGEAR_PANEL_IR] = PANEL_INIT_POS_IR;
  pSV->panel_pos[CGEAR_PANEL_WIRELESS] = PANEL_INIT_POS_WIRELESS;
  pSV->panel_pos[CGEAR_PANEL_WIFI] = PANEL_INIT_POS_WIFI;
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
 * @brief   CGEAR カスタムカラーをセット
 * @param   custom_id  カスタムID(内側から外に向かって0～)
 * @param   color カラーNo
 */
//--------------------------------------------------------------------------------------------
void CGEAR_SV_SetCGearCustomColor( CGEAR_SAVEDATA* pSV, u8 custom_id, u8 color )
{
  if( custom_id >= CGEAR_CUSTOM_COLOR_NUM || color >= CGEAR_RING_COLOR_MAX ){
    GF_ASSERT(0);
    return;
  }
  pSV->custom_col[custom_id] = color;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief   CGEAR カスタムカラーを得る
 * @param   custom_id  カスタムカラーID(内側から外に向かって0～)
 * @return  カラーNo
 */
//--------------------------------------------------------------------------------------------
u8 CGEAR_SV_GetCGearCustomColor(const CGEAR_SAVEDATA* pSV, u8 custom_id )
{
  if( custom_id >= CGEAR_CUSTOM_COLOR_NUM ){
    GF_ASSERT(0);
    return 0;
  }
  return pSV->custom_col[custom_id];
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   CGEAR パネル位置をセット
 * @param   panel_id CGEAR_PANEL_ID型
 * @param   pos ポジションNo
 */
//--------------------------------------------------------------------------------------------
void CGEAR_SV_SetCGearPanelPos( CGEAR_SAVEDATA* pSV, CGEAR_PANEL_ID panel_id, u8 pos )
{
  if( panel_id >= CGEAR_PANEL_MAX ){
    GF_ASSERT(0);
    return;
  }
  pSV->panel_pos[panel_id] = pos;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief   CGEAR パネルポジションを得る
 * @param   panel_id CGEAR_PANEL_ID型
 * @return  ポジションNo
 */
//--------------------------------------------------------------------------------------------
u8 CGEAR_SV_GetCGearPanelPos(const CGEAR_SAVEDATA* pSV, CGEAR_PANEL_ID panel_id )
{
  if( panel_id >= CGEAR_PANEL_MAX ){
    GF_ASSERT(0);
    return 0;
  }
  return pSV->panel_pos[panel_id];
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
  OPTIONS_SAVEDATA * opt;
	CGEAR_SAVEDATA* pData;
  //OPTIONS_SAVEDATAのメンバーとして保持している：呼び出し側は意識する必要はない
  opt = SaveControl_DataPtrGet(pSave, GMDATA_ID_OPTIONS);
  pData = &opt->cgear_savedata;
	return pData;

}

CGEAR_SAVEDATA* GAMEDATA_GetCGearSaveData(GAMEDATA* pGameData)
{
	return CGEAR_SV_GetCGearSaveData(GAMEDATA_GetSaveControlWork(pGameData));
}

