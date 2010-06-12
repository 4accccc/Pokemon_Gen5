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
  u16             sortWork[ BTL_POS_MAX ];
  u8              fMigawariDamage[ BTL_POS_MAX ];
  u8              count;
  u8              countMax;
  u8              getIdx;
  u8              targetPosCount;
}BTL_POKESET;


typedef BTL_POKESET  TARGET_POKE_REC;

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/


/**
 *  初期化
 */
extern void BTL_POKESET_Clear( BTL_POKESET* set );


/**
 *  ポケモン１体登録（ダメージ記録）
 */
extern void BTL_POKESET_AddWithDamage( BTL_POKESET* rec, BTL_POKEPARAM* bpp, u16 damage, BOOL fMigawariHit );

/**
 *  ポケモン１体登録
 */
extern void BTL_POKESET_Add( BTL_POKESET* rec, BTL_POKEPARAM* bpp );

/**
 *  ポケモン１体除外
 */
extern void BTL_POKESET_Remove( BTL_POKESET* rec, BTL_POKEPARAM* bpp );

/**
 *  ポケモンデータ取得
 */
extern BTL_POKEPARAM* BTL_POKESET_Get( const BTL_POKESET* rec, u32 idx );

/**
 *  生きてるポケモンだけ
 */
extern u32 BTL_POKESET_CopyAlive( const BTL_POKESET* src, BTL_POKESET* dst );

/**
 *  味方だけコピー
 */
extern u32 BTL_POKESET_CopyFriends( const BTL_POKESET* rec, const BTL_POKEPARAM* bpp, BTL_POKESET* dst );

/**
 *  相手だけコピー
 */
extern u32 BTL_POKESET_CopyEnemys( const BTL_POKESET* rec, const BTL_POKEPARAM* bpp, BTL_POKESET* dst );

/**
 *  死んでるのを除外
 */
extern void BTL_POKESET_RemoveDeadPoke( BTL_POKESET* rec );

/**
 *  素早さ順ソート
 */
extern void BTL_POKESET_SortByAgility( BTL_POKESET* rec, BTL_SVFLOW_WORK* flowWk );

/**
 *  順番アクセス開始
 */
extern void BTL_POKESET_SeekStart( BTL_POKESET* rec );

/**
 *  順番アクセス（NULLが返ったら修了）
 */
extern BTL_POKEPARAM* BTL_POKESET_SeekNext( BTL_POKESET* rec );

/**
 *  ダメージ記録取得（実体・みがわりとも）
 */
extern u32 BTL_POKESET_GetDamage( const BTL_POKESET* rec, const BTL_POKEPARAM* bpp );

/**
 *  ダメージ記録取得（実体のみ）
 */
extern u32 BTL_POKESET_GetDamageReal( const BTL_POKESET* rec, const BTL_POKEPARAM* bpp );


/**
 *  現在登録されているポケモン総数を取得
 */
extern u32 BTL_POKESET_GetCount( const BTL_POKESET* rec );

/**
 *  初期化後、記録されたポケモン総数を取得
 */
extern u32 BTL_POKESET_GetCountMax( const BTL_POKESET* rec );

/**
 *
 */
 extern void BTL_POKESET_SetDefaultTargetCount( BTL_POKESET* rec, u8 posCnt );

/**
 *  本来は１体以上いたハズのターゲットが現在は0になってしまったケースをチェック
 */
extern BOOL BTL_POKESET_IsRemovedAll( const BTL_POKESET* rec );

