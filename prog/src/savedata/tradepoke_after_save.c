//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		tradepoke_after_save.c
 *	@brief  ポケモン交換　その後　セーブデータ
 *	@author	tomoya takahashi
 *	@date		2010.03.26
 *
 *	モジュール名：TRPOKE_AFTER_SAVE
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "savedata/tradepoke_after_save.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
#define SPEABINO_THREE_NONE (0xff)  // 第３特性なし　定義

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	1データ
//=====================================
typedef struct {
  u32 personal_rnd;
	STRCODE	nickname[MONS_NAME_SIZE+EOM_SIZE];	//16h	ニックネーム(MONS_NAME_SIZE=10)+(EOM_SIZE=1)=11
  u8  indata:1;
  u8  level:7;
  u8  speabino;
  u32 id;
} TRPOKE_AFTER_DATA;


//-------------------------------------
///	交換ポケモン　その後　セーブワーク
//=====================================
struct _TRPOKE_AFTER_SAVE 
{
  TRPOKE_AFTER_DATA data[TRPOKE_AFTER_SAVE_TYPE_MAX];
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//----------------------------------------------------------
//	セーブデータシステムが依存する関数
//----------------------------------------------------------
int TRPOKE_AFTER_SV_GetWorkSize(void)
{
  return sizeof(TRPOKE_AFTER_SAVE);
}

//----------------------------------------------------------------------------
/**
 *	@brief  メモリ確保
 */
//-----------------------------------------------------------------------------
TRPOKE_AFTER_SAVE* TRPOKE_AFTER_SV_AllocWork(HEAPID heapID)
{
  TRPOKE_AFTER_SAVE* sv;
  sv = GFL_HEAP_AllocClearMemory( heapID, sizeof(TRPOKE_AFTER_SAVE) );
  return sv;
}

//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 */
//-----------------------------------------------------------------------------
void TRPOKE_AFTER_SV_Init(TRPOKE_AFTER_SAVE* sv)
{
  GFL_STD_MemClear( sv, sizeof(TRPOKE_AFTER_SAVE) );
}


//----------------------------------------------------------
//	データアクセス
//----------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  データ有無
 *
 *	@param	sv      セーブ
 *	@param	type    タイプ
 *
 *	@retval TRUE    セーブあり
 *	@retval FALSE   セーブなし
 */
//-----------------------------------------------------------------------------
BOOL TRPOKE_AFTER_SV_IsData( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type )
{
  GF_ASSERT( type < TRPOKE_AFTER_SAVE_TYPE_MAX );
  return sv->data[ type ].indata;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レベルを取得
 *
 *	@param	sv    ワーク
 *	@param	type  タイプ
 *
 *	@return レベル
 */
//-----------------------------------------------------------------------------
u8 TRPOKE_AFTER_SV_GetLevel( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type  )
{
  u8 ret;
  GF_ASSERT( type < TRPOKE_AFTER_SAVE_TYPE_MAX );
  if( sv->data[ type ].level < TRPOKE_AFTER_LEVEL_DEF ){
    ret = TRPOKE_AFTER_LEVEL_DEF;
  }else{
    ret = sv->data[ type ].level + TRPOKE_AFTER_LEVEL_ADD;
  }
  if( ret > TRPOKE_AFTER_LEVEL_MAX ){
    ret = TRPOKE_AFTER_LEVEL_MAX;
  }
  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  第３とくせいがあるかチェック
 *
 *	@param	sv      ワーク
 *	@param	type    タイプ
 *
 *	@retval TRUE  ある
 *	@retval FALSE ない
 */
//-----------------------------------------------------------------------------
BOOL TRPOKE_AFTER_SV_IsSpeabino3( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type  )
{
  GF_ASSERT( type < TRPOKE_AFTER_SAVE_TYPE_MAX );
  if( sv->data[ type ].speabino == SPEABINO_THREE_NONE )
  {
    return FALSE;
  }
  return TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  第３とくせいを取得
 *
 *	@param	sv      ワーク
 *	@param	type    タイプ
 *
 *	@return 特性
 */
//-----------------------------------------------------------------------------
u8 TRPOKE_AFTER_SV_GetSpeabino3( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type  )
{
  GF_ASSERT( type < TRPOKE_AFTER_SAVE_TYPE_MAX );
  return sv->data[ type ].speabino;
}

//----------------------------------------------------------------------------
/**
 *	@brief  交換　モンスターナンバーを取得
 *
 *	@param	sv          ワーク
 *	@param	type        トレードタイプ
 *	@param	rom_version ROMバージョン
 *
 *	@return モンスターナンバー
 */
//-----------------------------------------------------------------------------
u32 TRPOKE_AFTER_SV_GetTradeMonsNo( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type, u32 rom_version )
{
  u32 monsno = MONSNO_533;
  GF_ASSERT( type < TRPOKE_AFTER_SAVE_TYPE_MAX );
  GF_ASSERT( (rom_version == VERSION_WHITE) || (rom_version == VERSION_BLACK) );

  if( type == TRPOKE_AFTER_SAVE_C05 )
  {
    monsno = MONSNO_533;
  }
  else
  {
    if( rom_version == VERSION_BLACK ){
      monsno = MONSNO_521;
    }else{
      monsno = MONSNO_517;
    }
  }
  return monsno;
}

//----------------------------------------------------------------------------
/**
 *	@brief  固体乱数を取得
 *
 *	@param	sv      ワーク
 *	@param	type    タイプ
 *
 *	@return 固体乱数
 */
//-----------------------------------------------------------------------------
u32 TRPOKE_AFTER_SV_GetRnd( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type  )
{
  GF_ASSERT( type < TRPOKE_AFTER_SAVE_TYPE_MAX );
  return sv->data[ type ].personal_rnd;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ニックネームを取得
 *
 *	@param	sv      ワーク
 *	@param	type    タイプ
 *	@param	str     文字列格納先
 */
//-----------------------------------------------------------------------------
const STRCODE* TRPOKE_AFTER_SV_GetNickName( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type )
{
  GF_ASSERT( type < TRPOKE_AFTER_SAVE_TYPE_MAX );
  return sv->data[type].nickname;
}

//----------------------------------------------------------------------------
/**
 *	@brief  IDnoの取得
 *
 *	@param	sv
 *	@param	type  
 *
 *	@return ID　no
 */
//-----------------------------------------------------------------------------
u32 TRPOKE_AFTER_SV_GetID( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type  )
{
  GF_ASSERT( type < TRPOKE_AFTER_SAVE_TYPE_MAX );
  return sv->data[type].id;
}


//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン情報を設定
 *
 *	@param	sv      ワーク
 *	@param	type    タイプ
 *	@param	pp      ポケモン情報
 */
//-----------------------------------------------------------------------------
void TRPOKE_AFTER_SV_SetData( TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type, const POKEMON_PARAM * pp )
{
  GF_ASSERT( type < TRPOKE_AFTER_SAVE_TYPE_MAX );

  sv->data[type].level = PP_Get( pp, ID_PARA_level, NULL );
  if( PP_Get( pp, ID_PARA_tokusei_3_flag, NULL ) ){
    sv->data[type].speabino = PP_Get( pp, ID_PARA_speabino, NULL );
  }else{
    sv->data[type].speabino = SPEABINO_THREE_NONE;
  }
  sv->data[type].personal_rnd = PP_Get( pp, ID_PARA_personal_rnd, NULL );
  sv->data[type].id = PP_Get( pp, ID_PARA_id_no, NULL );
  PP_Get( pp, ID_PARA_nickname_raw, sv->data[type].nickname );
  sv->data[type].indata = TRUE;
}



