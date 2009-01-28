//=============================================================================================
/**
 * @file	btl_pokeselect.c
 * @brief	ポケモンWB バトルシステム	ポケモン選択画面用パラメータ
 * @author	taya
 *
 * @date	2008.09.25	作成
 */
//=============================================================================================

#include "btl_common.h"
#include "btl_main.h"

#include "btl_pokeselect.h"



//------------------------------------------------------
/**
 *	ポケモン選択画面用パラメータ
 */
//------------------------------------------------------
typedef struct {

	const BTL_PARTY*	party;				///< パーティデータ
	u8								numSelect;		///< 選択するポケモン数
	u8								aliveOnly;		///< TRUEだと生きているポケモンしか選べない

	u8								prohibit[ BTL_PARTY_MEMBER_MAX ];	///< 選択不可理由

}BTL_POKESELECT_PARAM;

//------------------------------------------------------
/**
 *	結果格納用構造体
 */
//------------------------------------------------------
typedef struct {

	u8			selIdx[ BTL_POSIDX_MAX ];	///< 選択したポケモンの並び順（現在の並び順で先頭を0と数える）
	u8			cnt;											///< 選択した数

}BTL_POKESELECT_RESULT;



//=============================================================================================
/**
 * パラメータ構造体を初期化
 *
 * @param   param				[out]
 * @param   party				パーティデータポインタ
 * @param   numSelect		選択する必要のあるポケモン数
 * @param   aliveOnly		生きているポケモンしか選択させない場合TRUE
 *
 */
//=============================================================================================
void BTL_POKESELECT_PARAM_Init( BTL_POKESELECT_PARAM* param, const BTL_PARTY* party, u8 numSelect, u8 aliveOnly )
{
	int i;
	param->party = party;
	param->numSelect = numSelect;
	param->aliveOnly = aliveOnly;
	for(i=0; i<NELEMS(param->prohibit); i++)
	{
		param->prohibit[i] = BTL_POKESEL_CANT_NONE;
	}
}
//=============================================================================================
/**
 * 既に戦闘に出ているポケモンを選べないように設定する
 *
 * @param   param				[out] 
 * @param   numCover		戦闘に出ているポケモン数
 *
 */
//=============================================================================================
void BTL_POKESELECT_PARAM_SetProhibitFighting( BTL_POKESELECT_PARAM* param, u8 numCover )
{
	u8 i;
	for(i=0; i<numCover; i++)
	{
		BTL_POKESELECT_PARAM_SetProhibit( param, BTL_POKESEL_CANT_FIGHTING, i );
	}
}
//=============================================================================================
/**
 * パラメータ構造体に対し、追加情報として個別に選択できない理由を設定する
 *
 * @param   param		
 * @param   reason		
 * @param   idx		
 *
 */
//=============================================================================================
void BTL_POKESELECT_PARAM_SetProhibit( BTL_POKESELECT_PARAM* param, BtlPokeSelReason reason, u8 idx )
{
	GF_ASSERT(idx<NELEMS(param->prohibit));
	param->prohibit[ idx ] = reason;
}



//=============================================================================================
/**
 * リザルト構造体を初期化
 *
 * @param   result		[out]
 *
 */
//=============================================================================================
void BTL_POKESELECT_RESULT_Init( BTL_POKESELECT_RESULT *result, const BTL_POKESELECT_PARAM* param )
{
	result->cnt = 0;
	result->max = param->numSelect;
}
//=============================================================================================
/**
 * リザルト構造体に選択結果を１件追加
 *
 * @param   result		[out]
 * @param   idx				選択結果
 *
 */
//=============================================================================================
void BTL_POKESELECT_RESULT_Push( BTL_POKESELECT_RESULT *result, u8 idx )
{
	if( result->cnt < result->max )
	{
		result->selIdx[ result->cnt++ ] = idx;
	}
	else
	{
		GF_ASSERT(0);
	}
}
//=============================================================================================
/**
 * リザルト構造体から選択結果を１件削除
 *
 * @param   result		[out]
 *
 */
//=============================================================================================
void BTL_POKESELECT_RESULT_Pop( BTL_POKESELECT_RESULT* result )
{
	if( result->cnt )
	{
		result->cnt--;
	}
}
//=============================================================================================
/**
 * 選択し終わったか判定
 *
 * @param   result		リザルト構造体ポインタ
 *
 * @retval  BOOL		
 */
//=============================================================================================
BOOL BTL_POKESELECT_IsDone( const BTL_POKESELECT_RESULT* result )
{
	return result->cnt == result->max;
}


//=============================================================================================
/**
 * 選択しても良いポケモンかどうか判定
 *
 * @param   param			[in]
 * @param   result		[in]
 * @param   idx				選択ポケモンインデックス
 *
 * @retval  BtlPokeselReason		不可理由（不可でなければ BTL_POKESEL_CANT_NONE）
 */
//=============================================================================================
BtlPokeselReason BTL_POKESELECT_CheckProhibit( const BTL_POKESELECT_PARAM* param, const BTL_POKESELECT_RESULT* result, u8 idx )
{
	// 生きてるポケしか選べない場合のチェック
	if( param->aliveOnly )
	{
		const BTL_POKEPARAM* bpp = BTL_PARTY_GetMemberDataConst(param, idx);

		if( BTL_POKEPARAM_IsDead(bpp) )
		{
			return BTL_POKESEL_CANT_DEAD;
		}
	}

	// 個別理由チェック
	if( param->prohibit[idx] != BTL_POKESEL_CANT_NONE )
	{
		return param->prohibit[idx];
	}

	// 既に選んだポケは選べない
	{
		u8 i;
		for(i=0; i<result->cnt; i++)
		{
			if( result->selIdx[i] == idx )
			{
				return BTL_POKESEL_CANT_SELECTED;
			}
		}
	}

	return BTL_POKESEL_CANT_NONE;
}
				break;
			}
		}
		if( i != res->cnt )
		{
			BTL_STR_MakeWarnStr( wk->strbuf, bpp, BTLSTR_UI_WARN_SelectedPoke );
			(*seq) = SEQ_WARNING_START;
			break;
		}
	}

}


