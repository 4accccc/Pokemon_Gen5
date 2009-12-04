//=============================================================================================
/**
 * @file    btl_pokeset.h
 * @brief   ポケモンWB バトルシステム 特定目的のポケモンパラメータセット管理
 * @author  taya
 *
 * @date  2009.12.11  作成
 */
//=============================================================================================

#include "btl_pokeset.h"

/**
 *  指定ポケモンと同陣営のデータのみをコピー
 *
 *  @return u32 コピーしたポケモン数
 */
u32 BTL_POKESET_CopyFriends( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* bpp, TARGET_POKE_REC* dst )
{
  BTL_POKEPARAM* bpp;
  u32 max, i;
  u8 ID1, ID2;

  BTL_POKESET_Clear( dst );
  ID1 = BPP_GetID( bpp );

  max = BTL_POKESET_GetCount( rec );
  for(i=0; i<max; ++i)
  {
    bpp = BTL_POKESET_Get( rec, i );
    ID2 = BPP_GetID( bpp );
    if( BTL_MAINUTIL_IsFriendPokeID(ID1, ID2) ){
      BTL_POKESET_Add( dst, bpp );
    }
  }
  return BTL_POKESET_GetCount( dst );
}
/**
 *  指定ポケモンと敵チームのポケモンのみを別ワークにコピー
 *
 *  @return u32 コピーしたポケモン数
 */
u32 BTL_POKESET_CopyEnemys( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* bpp, TARGET_POKE_REC* dst )
{
  BTL_POKEPARAM* bpp;
  u32 max, i;
  u8 ID1, ID2;

  BTL_POKESET_Clear( dst );
  ID1 = BPP_GetID( bpp );

  max = BTL_POKESET_GetCount( rec );
  for(i=0; i<max; ++i)
  {
    bpp = BTL_POKESET_Get( rec, i );
    ID2 = BPP_GetID( bpp );
    if( !BTL_MAINUTIL_IsFriendPokeID(ID1, ID2) ){
      BTL_POKESET_Add( dst, bpp );
    }
  }
  return BTL_POKESET_GetCount( dst );
}
/**
 *  死んでるポケモンを削除
 */
void BTL_POKESET_RemoveDeadPokemon( TARGET_POKE_REC* rec )
{
  BTL_POKEPARAM* bpp;

  BTL_POKESET_GetStart( rec );

  while( (bpp = BTL_POKESET_GetNext(rec)) != NULL )
  {
    if( BPP_IsDead(bpp) ){
      BTL_POKESET_Remove( rec, bpp );
    }
  }
}

/**
 *  当ターンの計算後素早さ順でソート（当ターンに行動していないポケは素の素早さ）
 */
void BTL_POKESET_SortByAgility( TARGET_POKE_REC* rec )
{

}

