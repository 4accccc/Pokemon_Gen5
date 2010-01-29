//=============================================================================================
/**
 * @file  btl_pokeselect.c
 * @brief ポケモンWB バトルシステム ポケモン選択画面用パラメータ
 * @author  taya
 *
 * @date  2008.09.25  作成
 */
//=============================================================================================

#include "btl_common.h"
#include "btl_main.h"

#include "btl_pokeselect.h"


//=============================================================================================
/**
 * パラメータ構造体を初期化
 *
 * @param   param       [out]
 * @param   party       パーティデータポインタ
 * @param   numSelect   選択する必要のあるポケモン数
 * @param   bplMode     バトルリスト画面モード
 *
 */
//=============================================================================================
void BTL_POKESELECT_PARAM_Init( BTL_POKESELECT_PARAM* param, const BTL_PARTY* party, u8 numSelect, u8 bplMode )
{
  int i;
  param->party = party;
  param->numSelect = numSelect;
  param->bplMode = bplMode;
  for(i=0; i<NELEMS(param->prohibit); i++)
  {
    param->prohibit[i] = BTL_POKESEL_CANT_NONE;
  }
}
//=============================================================================================
/**
 * 既に戦闘に出ているポケモンを選べないように設定する
 *
 * @param   param       [out]
 * @param   numCover    戦闘に出ているポケモン数
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
void BTL_POKESELECT_PARAM_SetProhibit( BTL_POKESELECT_PARAM* param, BtlPokeselReason reason, u8 idx )
{
  GF_ASSERT(idx<NELEMS(param->prohibit));
  param->prohibit[ idx ] = reason;
}
//=============================================================================================
/**
 * 選択するべき数を取得
 *
 * @param   param
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_POKESELECT_PARAM_GetNumSelect( const BTL_POKESELECT_PARAM* param )
{
  return param->numSelect;
}



//=============================================================================================
/**
 * リザルト構造体を初期化
 *
 * @param   result    [out]
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
 * @param   result    [out]
 * @param   idx       選択結果
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
    GF_ASSERT_MSG(0, "cnt=%d, max=%d", result->cnt,result->max);
  }
}
//=============================================================================================
/**
 * リザルト構造体から選択結果を１件削除
 *
 * @param   result    [out]
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
 * キャンセルされたか判定
 *
 * @param   result
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_POKESELECT_IsCancel( const BTL_POKESELECT_RESULT* result )
{
  return result->fCancel;
}
//=============================================================================================
/**
 * 選択し終わったか判定
 *
 * @param   result    リザルト構造体ポインタ
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
 * 選んだ数を取得
 *
 * @param   result
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_POKESELECT_RESULT_GetCount( const BTL_POKESELECT_RESULT* result )
{
  return result->cnt;
}

//=============================================================================================
/**
 * 最後に選んだポケモンの並び順を返す
 *
 * @param   result
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_POKESELECT_RESULT_GetLast( const BTL_POKESELECT_RESULT* result )
{
  if( result->cnt > 0 )
  {
    return result->selIdx[ result->cnt - 1 ];
  }
  else
  {
    return BTL_PARTY_MEMBER_MAX;
  }
}
//=============================================================================================
/**
 * 選んだポケモンの並び順を返す
 *
 * @param   result
 * @param   idx       何番目に選んだ？
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_POKESELECT_RESULT_Get( const BTL_POKESELECT_RESULT* result, u8 idx )
{
  if( idx < result->cnt ){
    return result->selIdx[ idx ];
  }
  return BTL_PARTY_MEMBER_MAX;
}



