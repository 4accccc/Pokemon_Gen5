//============================================================================================
/**
 * @file		gage_tool.c
 * @brief		ゲージ関連処理
 * @author	Hiroyuki Nakamura
 * @date		09.09.30
 *
 *	モジュール名：GAUGETOOL
 */
//============================================================================================
#include <gflib.h>

#include "poke_tool/gauge_tool.h"



//--------------------------------------------------------------------------------------------
/**
 * @brief	 現在値のゲージドット数を取得
 *
 * @param		prm_now		現在値
 * @param		prm_max		最大値
 * @param		dot_max		最大ドット数
 *
 * @return	ドット数
 */
//--------------------------------------------------------------------------------------------
u8 GAUGETOOL_GetNumDotto( u32 prm_now, u32 prm_max, u8 dot_max )
{
	u8 put_dot;
	
	put_dot = prm_now * dot_max / prm_max;
	if( put_dot == 0 && prm_now > 0 )   // ﾄﾞｯﾄ計算では0でも実際の値が1以上なら1ﾄﾞｯﾄにする
  {
		put_dot = 1;
	}
	return put_dot;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   表示ドットと最大ドットからHPゲージの色を取得する
 *
 * @param   put_dot		表示ドット数
 * @param   max_dot		最大ドット数
 *
 * @retval  ゲージカラー
 */
//--------------------------------------------------------------------------------------------
u8 GAUGETOOL_GetGaugeDottoColor( u32 put_dot, u32 max_dot )
{
	put_dot <<= 8;		//割り算使用の為、小数レベルまで見れるように固定小数化
	max_dot <<= 8;
	
	if( put_dot > (max_dot/2) )
  {
		return GAUGETOOL_HP_DOTTO_GREEN;		// 緑
	}
  else if( put_dot > ( max_dot / 5 ) )
  {
		return GAUGETOOL_HP_DOTTO_YELLOW;	// 黄
	}
  else if( put_dot > 0 )
  {
		return GAUGETOOL_HP_DOTTO_RED;			// 赤
	}
	return GAUGETOOL_HP_DOTTO_NULL;			// HP=0
}
