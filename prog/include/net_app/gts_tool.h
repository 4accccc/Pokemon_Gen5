//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		gts_tool.h
 *	@brief  GTSNEGOとの共有ツール
 *	@author	Toru=Nagihashi
 *	@date		2010.03.18
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "poke_tool/poke_tool.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  禁止リボンもちかチェック
 *
 *	@param	POKEMON_PARAM *p_pp   ポケパラ
 *
 *	@return TRUEならば禁止リボンもち  FALSEならばそれ以外
 */
//-----------------------------------------------------------------------------
static inline BOOL GTS_TOOL_IsForbidRibbon( POKEMON_PARAM *p_pp )
{ 
  return PP_Get( p_pp, ID_PARA_country_ribbon, NULL )          //ポケモンリーグ優勝記念(全国大会地区優勝者
    || PP_Get( p_pp, ID_PARA_national_ribbon, NULL )           //ダークポケモン リライブ完了の証
    || PP_Get( p_pp, ID_PARA_earth_ribbon, NULL )              //バトル山 100人抜き達成
    || PP_Get( p_pp, ID_PARA_world_ribbon, NULL )              //ポケモンリーグ優勝記念(全国大会で日本一になった者のみ)
    || PP_Get( p_pp, ID_PARA_sinou_classic_ribbon, NULL )      //外部配布用
    || PP_Get( p_pp, ID_PARA_sinou_premiere_ribbon, NULL )     //外部配布用
    || PP_Get( p_pp, ID_PARA_sinou_history_ribbon, NULL )      //外部配布用：
    || PP_Get( p_pp, ID_PARA_sinou_green_ribbon, NULL )        //外部配布用：
    || PP_Get( p_pp, ID_PARA_sinou_blue_ribbon, NULL )         //外部配布用：
    || PP_Get( p_pp, ID_PARA_sinou_festival_ribbon, NULL )     //外部配布用：
    || PP_Get( p_pp, ID_PARA_sinou_carnival_ribbon, NULL )     //外部配布用：
    || PP_Get( p_pp, ID_PARA_marine_ribbon, NULL )             //汎用バトル大会
    || PP_Get( p_pp, ID_PARA_land_ribbon, NULL )               //WCS地方大会優勝者配布用
    || PP_Get( p_pp, ID_PARA_sky_ribbon, NULL )                //WCS国大会優勝者配布用
    || PP_Get( p_pp, ID_PARA_sinou_red_ribbon, NULL );         //WCS世界大会優勝者配布用                                   

}
