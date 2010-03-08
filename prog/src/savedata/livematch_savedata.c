//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		livematch_btlscore_data.c
 *	@brief  ライブマッチ戦績セーブデータ
 *	@author	Toru=Nagihashi
 *	@data		2010.03.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//セーブデータ
#include "savedata/save_tbl.h"

//外部公開
#include "savedata/livematch_savedata.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	対戦相手情報
//=====================================
typedef struct 
{
  u8  mac_addr[6];
  u8  param[LIVEMATCH_FOEDATA_PARAM_MAX];
} LIVEMATCH_FOEDATA;

//-------------------------------------
///	セーブデータ不完全型
//=====================================
struct _LIVEMATCH_DATA
{ 
  //自分の情報
  u8  mac_addr[6];
  u8  param[LIVEMATCH_MYDATA_PARAM_MAX];
  
  //対戦相手のデータ
  LIVEMATCH_FOEDATA foe[LIVEMATCH_FOEDATA_MAX];
};
//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//----------------------------------------------------------
//セーブデータシステムに依存する関数
//----------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  構造体サイズ取得
 *
 *	@return ワークサイズ
 */
//-----------------------------------------------------------------------------
int LIVEMATCH_GetWorkSize( void )
{
  return sizeof( LIVEMATCH_DATA );
}

//----------------------------------------------------------------------------
/**
 *	@brief  セーブデータ初期化
 *
 *	@param	LIVEMATCH_DATA *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void LIVEMATCH_Init( LIVEMATCH_DATA *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(LIVEMATCH_DATA) );
  OS_GetMacAddress( p_wk->mac_addr );
}

//----------------------------------------------------------
//セーブデータ取得のための関数
//----------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  ライブマッチのセーブデータ取得  const版
 *
 *	@param	const SAVE_CONTROL_WORK * cp_sv ワーク
 *
 *	@return セーブデータ
 */
//-----------------------------------------------------------------------------
const LIVEMATCH_DATA * SaveData_GetLiveMatchConst( const SAVE_CONTROL_WORK * cp_sv )
{
  return SaveData_GetLiveMatch( (SAVE_CONTROL_WORK*)cp_sv );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ライブマッチのセーブデータ取得
 *
 *	@param	const SAVE_CONTROL_WORK * cp_sv ワーク
 *
 *	@return セーブデータ
 */
//-----------------------------------------------------------------------------
LIVEMATCH_DATA * SaveData_GetLiveMatch( SAVE_CONTROL_WORK * p_sv )
{
  return SaveControl_DataPtrGet(p_sv, GMDATA_ID_LIVEMATCH );
}


//----------------------------------------------------------
//取得、操作する関数
//----------------------------------------------------------
//自分に関係するデータ
//取得
//----------------------------------------------------------------------------
/**
 *	@brief  自分のマックアドレス受け取り
 *
 *	@param	const LIVEMATCH_DATA *cp_wk   ワーク
 *	@param	p_mac_addr                    受け取り（u8[6]）
 */
//-----------------------------------------------------------------------------
void LIVEMATCH_DATA_GetMyMacAddr( const LIVEMATCH_DATA *cp_wk, u8* p_mac_addr )
{
  GFL_STD_MemCopy( cp_wk->mac_addr, p_mac_addr, sizeof(u8)*6 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  自分の情報受け取り
 *
 *	@param	const LIVEMATCH_DATA *cp_wk ワーク
 *	@param	param                       受け取る情報インデックス
 *
 *	@return 情報
 */
//-----------------------------------------------------------------------------
int LIVEMATCH_DATA_GetMyParam( const LIVEMATCH_DATA *cp_wk, LIVEMATCH_MYDATA_PARAM param )
{
  GF_ASSERT( param < LIVEMATCH_MYDATA_PARAM_MAX );
  return cp_wk->param[ param ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  自分の情報設定
 *
 *	@param	const LIVEMATCH_DATA *cp_wk ワーク
 *	@param	param                       受け取る情報インデックス
 *	@param	data                        データ
 */
//-----------------------------------------------------------------------------
void LIVEMATCH_DATA_SetMyParam( LIVEMATCH_DATA *p_wk, LIVEMATCH_MYDATA_PARAM param, int data )
{
  GF_ASSERT( param < LIVEMATCH_MYDATA_PARAM_MAX );
  p_wk->param[ param ]  = data;
}

//----------------------------------------------------------------------------
/**
 *	@brief  自分の情報へ足しこみ
 *
 *	@param	const LIVEMATCH_DATA *cp_wk ワーク
 *	@param	param                       受け取る情報インデックス
 *	@param	data                        足し込む値
 */
//-----------------------------------------------------------------------------
void LIVEMATCH_DATA_AddMyParam( LIVEMATCH_DATA *p_wk, LIVEMATCH_MYDATA_PARAM param, int data )
{
  GF_ASSERT( param < LIVEMATCH_MYDATA_PARAM_MAX );
  p_wk->param[ param ]  += data;
}


//相手に関係するデータ
//取得
//----------------------------------------------------------------------------
/**
 *	@brief  対戦相手のマックアドレスを取得
 *
 *	@param	const LIVEMATCH_DATA *cp_wk ワーク
 *	@param	index                       対戦相手インデックス
 *	@param	p_mac_addr                  受け取り（u8[6]）
 */
//-----------------------------------------------------------------------------
void LIVEMATCH_DATA_GetFoeMacAddr( const LIVEMATCH_DATA *cp_wk, u32 index, u8* p_mac_addr )
{
  const LIVEMATCH_FOEDATA *cp_data;

  GF_ASSERT( index < LIVEMATCH_FOEDATA_MAX );

  cp_data  = &cp_wk->foe[ index ];

  GFL_STD_MemCopy( cp_data->mac_addr, p_mac_addr, sizeof(u8)*6 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  対戦相手の情報を取得
 *
 *	@param	const LIVEMATCH_DATA *cp_wk ワーク
 *	@param	index                       対戦相手のインデックス
 *	@param	param                       受け取る情報インデックス
 *
 *	@return 情報
 */
//-----------------------------------------------------------------------------
int LIVEMATCH_DATA_GetFoeParam( const LIVEMATCH_DATA *cp_wk, u32 index, LIVEMATCH_FOEDATA_PARAM param )
{
  const LIVEMATCH_FOEDATA *cp_data;

  GF_ASSERT( param < LIVEMATCH_FOEDATA_PARAM_MAX );
  GF_ASSERT( index < LIVEMATCH_FOEDATA_MAX );

  cp_data  = &cp_wk->foe[ index ];
  return cp_data->param[ param ];
}

//設定
//----------------------------------------------------------------------------
/**
 *	@brief  対戦相手のマックアドレスを設定
 *
 *	@param	LIVEMATCH_DATA *p_wk  ワーク
 *	@param	index                 対戦相手のインデックス
 *	@param	u8* cp_mac_addr       設定するマックアドレス（u8[6]）
 */
//-----------------------------------------------------------------------------
void LIVEMATCH_DATA_SetFoeMacAddr( LIVEMATCH_DATA *p_wk, u32 index, const u8* cp_mac_addr )
{
  LIVEMATCH_FOEDATA *p_data;

  GF_ASSERT( index < LIVEMATCH_FOEDATA_MAX );

  p_data  = &p_wk->foe[ index ];

  GFL_STD_MemCopy( cp_mac_addr, p_data->mac_addr, sizeof(u8)*6 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  対戦相手の情報を設定
 *
 *	@param	LIVEMATCH_DATA *p_wk  ワーク
 *	@param	index                 対戦相手のインデックス
 *	@param	param                 設定する情報インデックス
 *	@param	data                  でーた
 */
//-----------------------------------------------------------------------------
void LIVEMATCH_DATA_SetFoeParam( LIVEMATCH_DATA *p_wk, u32 index, LIVEMATCH_FOEDATA_PARAM param, int data )
{
  LIVEMATCH_FOEDATA *p_data;

  GF_ASSERT( param < LIVEMATCH_FOEDATA_PARAM_MAX );
  GF_ASSERT( index < LIVEMATCH_FOEDATA_MAX );

  p_data  = &p_wk->foe[ index ];
  p_data->param[ param ]  = data;
}


