//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\app\name_input\namein_setup.c
 *	@brief  名前入力設定関数
 *	@author	Toru=Nagihashi
 *	@data		2010.03.31
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//  lib
#include <gflib.h>

//  SYSTEM
#include "system/main.h"  //HEAPID
#include "system/gfl_use.h"

//  module
#include "app/app_menu_common.h"
#include "pokeicon/pokeicon.h"

//  savedata
#include "savedata/misc.h"
#include "app/name_input.h" //外部参照
#include "namein_local.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *    外部参照
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  NAMEINに渡すPARAM構造体作成
 *
 *  @param  HEAPID heapId         ヒープID
 *  @param  mode                  入力の種類
 *  @param  param1                種類ごとに違う引数１
 *  @param  param2                種類ごとに違う引数２
 *  @param  wordmax               入力文字最大数
 *  @param  STRBUF *default_str   デフォルトで入力されている文字列
 *
 *  @retura NAMEIN_PARAM
 */
//-----------------------------------------------------------------------------
NAMEIN_PARAM *NAMEIN_AllocParam( HEAPID heapId, NAMEIN_MODE mode, int param1, int param2, int wordmax, const STRBUF *default_str, MISC *p_misc )
{ 
  NAMEIN_PARAM *p_param;
  p_param = GFL_HEAP_AllocMemory( heapId, sizeof(NAMEIN_PARAM) );
  GFL_STD_MemClear( p_param, sizeof(NAMEIN_PARAM) );
  p_param->mode     = mode;
  p_param->wordmax  = wordmax;
  p_param->param1 = param1;
  p_param->param2 = param2;
  p_param->p_misc = p_misc;

  //バッファ作成
  p_param->strbuf = GFL_STR_CreateBuffer( NAMEIN_STR_BUFFER_LENGTH + 1, heapId );

  //デフォルト入力文字コピー
  if( default_str )
  { 
    GFL_STR_CopyBuffer( p_param->strbuf, default_str );
  }

  return p_param;
}
//----------------------------------------------------------------------------
/**
 *  @brief  NAMEINに渡すPARAM構造体作成 ポケモンモード固定＆PP指定版
 *
 *  @param  HEAPID heapId         ヒープID
 *  @param  POKEMON_PARAM *pp     ポケモンパラム
 *  @param  wordmax               入力文字最大数
 *  @param  STRBUF *default_str   デフォルトで入力されている文字列
 *
 *  @return NAMEIN_PARAM
 */
//-----------------------------------------------------------------------------
NAMEIN_PARAM *NAMEIN_AllocParamPokemonByPP( HEAPID heapId, const POKEMON_PARAM *pp, int wordmax, const STRBUF *default_str, MISC *p_misc )
{ 
  NAMEIN_PARAM *p_param;
  p_param = GFL_HEAP_AllocMemory( heapId, sizeof(NAMEIN_PARAM) );
  GFL_STD_MemClear( p_param, sizeof(NAMEIN_PARAM) );
  p_param->mode     = NAMEIN_POKEMON;
  p_param->wordmax  = wordmax;
  p_param->pp       = pp;
  p_param->p_misc = p_misc;

  //PPの場合引数を設定
  if( p_param->pp )
  { 
    p_param->mons_no  = PP_Get( pp, ID_PARA_monsno, NULL );
    p_param->form     = PP_Get( pp, ID_PARA_form_no, NULL ) | ( PP_Get( pp, ID_PARA_sex, NULL ) << 8 );
  }

  //バッファ作成
  p_param->strbuf = GFL_STR_CreateBuffer( NAMEIN_STR_BUFFER_LENGTH + 1, heapId );

  //デフォルト入力文字コピー
  if( default_str )
  { 
    GFL_STR_CopyBuffer( p_param->strbuf, default_str );
  }

  return p_param;
}

//----------------------------------------------------------------------------
/**
 *  @brief  NAMEINに渡すPARAM構造体作成 ポケモンモード固定＆PP指定版
 *          ポケモン捕獲時はこちらを使用してください
 *
 *  @param  HEAPID heapId         ヒープID
 *  @param  POKEMON_PARAM *pp     ポケモンパラム
 *  @param  wordmax               入力文字最大数
 *  @param  STRBUF *default_str   デフォルトで入力されている文字列
 *  @param  box_strbuf   [ポケモンニックネーム]は○○のパソコンの[ボックス名]に転送された！
 *  @param  box_manager  ボックスマネージャ(box_strbuf!=NULLのときしか使われない) 
 *  @param  box_tray     ボックストレイナンバー(box_strbuf!=NULLのときしか使われない) 
 *
 *  @return NAMEIN_PARAM
 */
//-----------------------------------------------------------------------------
NAMEIN_PARAM *NAMEIN_AllocParamPokemonCapture( HEAPID heapId, const POKEMON_PARAM *pp, int wordmax, const STRBUF *default_str,
                                               const STRBUF *box_strbuf, const BOX_MANAGER *box_manager, u32 box_tray, MISC *p_misc )
{
  NAMEIN_PARAM *p_param = NAMEIN_AllocParamPokemonByPP( heapId, pp, wordmax, default_str, p_misc );
  p_param->box_strbuf = box_strbuf;
  p_param->box_manager = box_manager;
  p_param->box_tray = box_tray;
  return p_param;
}

//----------------------------------------------------------------------------
/**
 *  @brief  NAMEIN_PARAMを破棄
 *
 *  @param  *param  ワーク
 */
//-----------------------------------------------------------------------------
void NAMEIN_FreeParam(NAMEIN_PARAM *param)
{ 
  GFL_STR_DeleteBuffer( param->strbuf );
  GFL_HEAP_FreeMemory( param );
}
//----------------------------------------------------------------------------
/**
 *  @brief  NAMEINPARAMから、入力された文字をコピー
 *
 *  @param  const NAMEIN_PARAM *param   ワーク
 *  @param  *strbuf   コピー受け取りバッファ
 */
//-----------------------------------------------------------------------------
void NAMEIN_CopyStr( const NAMEIN_PARAM *param, STRBUF *strbuf )
{ 
  GFL_STR_CopyBuffer( strbuf, param->strbuf );
}
//----------------------------------------------------------------------------
/**
 *  @brief  NAMEINPARAMから、入力された文字をAllocかつコピーして返す
 *
 *  @param  const NAMEIN_PARAM *param ワーク
 *  @param  heapID                    ヒープID
 *
 *  @return コピーされた文字列
 */
//-----------------------------------------------------------------------------
STRBUF* NAMEIN_CreateCopyStr( const NAMEIN_PARAM *param, HEAPID heapID )
{ 
  return GFL_STR_CreateCopyBuffer( param->strbuf, heapID );
}
//----------------------------------------------------------------------------
/**
 *  @brief  NAMEINPARAMから、キャンセルされたかのフラグを取得
 *
 *  @param  const NAMEIN_PARAM *param   ワーク
 *
 *  @retval TRUEならば0文字でOKをおされたorデフォルト文字と同じ
 *  @retval FALSEならばキャンセルされていない
 */
//-----------------------------------------------------------------------------
BOOL NAMEIN_IsCancel( const NAMEIN_PARAM *param )
{ 
  return param->cancel;
}


