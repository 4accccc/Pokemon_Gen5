//=============================================================================================
/**
 * @file    btl_pokeset.h
 * @brief   ポケモンWB バトルシステム 特定目的のポケモンパラメータセット管理
 * @author  taya
 *
 * @date  2009.12.11  作成
 */
//=============================================================================================
#pragma once

#include "btl_common.h"
#include "btl_pokeparam.h"
#include "btl_server_flow.h"

//----------------------------------------------------------------------
/**
 * 対象となるポケモンパラメータを記録しておくためのワーク
 */
//----------------------------------------------------------------------
typedef struct {

  BTL_POKEPARAM*  bpp[ BTL_POS_MAX ];
  u16             damage[ BTL_POS_MAX ];
  u8              count;
  u8              countMax;
  u8              getIdx;
}BTL_POKESET;


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
extern void BTL_POKESET_SortByAgility( TARGET_POKE_REC* rec );
extern u32 BTL_POKESET_CopyFriends( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* bpp, TARGET_POKE_REC* dst );
extern u32 BTL_POKESET_CopyEnemys( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* bpp, TARGET_POKE_REC* dst );
extern void BTL_POKESET_RemoveDeadPokemon( TARGET_POKE_REC* rec );



/**
 *  初期化
 */
static inline BTL_POKESET_Clear( BTL_POKESET* set )
{
  GFL_STD_MemClear( set, sizeof(BTL_POKESET) );
}
/**
 *  ポケモン１体登録
 */
static inline void BTL_POKESET_Add( TARGET_POKE_REC* rec, BTL_POKEPARAM* bpp )
{
  BTL_POKESET_AddWithDamage( rec, bpp, 0 );
}
/**
 *  ポケモン１体登録（ダメージ記録）
 */
static inline void BTL_POKESET_AddWithDamage( TARGET_POKE_REC* rec, BTL_POKEPARAM* bpp, u16 damage )
{
  if( rec->count < NELEMS(rec->bpp) )
  {
    rec->bpp[ rec->count ] = bpp;
    rec->damage[ rec->count ] = damage;
    rec->count++;
    if( rec->count > rec->countMax ){
      rec->countMax = rec->count;
    }
  }
  else
  {
    GF_ASSERT(0);
  }
}
/**
 *  ポケモン１体除外
 */
static inline void BTL_POKESET_Remove( TARGET_POKE_REC* rec, BTL_POKEPARAM* bpp )
{
  u32 i;
  for(i=0; i<rec->count; ++i)
  {
    if( rec->bpp[i] == bpp )
    {
      u32 j;
      for(j=i+1; j<rec->count; ++j)
      {
        rec->bpp[j-1] = rec->bpp[j];
      }
      rec->count--;
      rec->idx--;
      break;
    }
  }
}
/**
 *  ポケモンデータ取得
 */
static inline BTL_POKEPARAM* BTL_POKESET_Get( const TARGET_POKE_REC* rec, u32 idx )
{
  if( idx < rec->count )
  {
    return (BTL_POKEPARAM*)(rec->bpp[ idx ]);
  }
  return NULL;
}
/**
 *  順番アクセス開始
 */
static inline void BTL_POKESET_SeekStart( TARGET_POKE_REC* rec )
{
  rec->idx = 0;
}
/**
 *  順番アクセス（NULLが返ったら修了）
 */
static inline BTL_POKEPARAM* BTL_POKESET_SeekNext( TARGET_POKE_REC* rec )
{
  if( rec->idx < rec->count )
  {
    return rec->bpp[ rec->idx++ ];
  }
  else
  {
    return NULL;
  }
}
/**
 *  ダメージ記録取得
 */
static inline u32 BTL_POKESET_GetDamage( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* bpp )
{
  u32 i;
  for(i=0; i<rec->count; ++i)
  {
    if( rec->bpp[i] == bpp )
    {
      return rec->damage[i];
    }
  }
  GF_ASSERT(0); // ポケモン見つからない
  return 0;
}
/**
 *  現在登録されているポケモン総数を取得
 */
static inline u32 BTL_POKESET_GetCount( const TARGET_POKE_REC* rec )
{
  return rec->count;
}
/**
 *  初期化後、記録されたポケモン総数を取得
 */
static inline u32 BTL_POKESET_GetCountMax( const TARGET_POKE_REC* rec )
{
  return rec->countMax;
}


/**
 *  本来は１体以上いたハズのターゲットが現在は0になってしまったケースをチェック
 */
static inline BOOL BTL_POKESET_IsTargetRemoved( const TARGET_POKE_REC* rec )
{
  return (rec->countMax > 0) && (rec->count == 0);
}
