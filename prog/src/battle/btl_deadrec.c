//=============================================================================================
/**
 * @file  btl_deadrec.c
 * @brief ポケモンWB 死亡ポケ記録
 * @author  taya
 *
 * @date  2009.11.23  作成
 */
//=============================================================================================

#include "btl_util.h"
#include "btl_deadrec.h"

/**
 *  バトル開始時の初期化
 */
void BTL_DEADREC_Init( BTL_DEADREC* wk )
{
  GFL_STD_MemClear( wk, sizeof(BTL_DEADREC) );
}

/**
 *  ターン開始時の初期化
 */
void BTL_DEADREC_StartTurn( BTL_DEADREC* wk )
{
  u32 i;
  for(i=BTL_DEADREC_TURN_MAX-1; i>0; --i)
  {
    wk->record[i] = wk->record[i-1];
  }
  GFL_STD_MemClear( &wk->record[0], sizeof(wk->record[0]) );
}

/**
 *  死亡ポケID追加
 */
void BTL_DEADREC_Add( BTL_DEADREC* wk, u8 pokeID )
{
  u8 cnt = wk->record[0].cnt;
  if( cnt < NELEMS(wk->record[0].fDead) ){
    BTL_N_Printf( DBGSTR_DEADREC_Add, cnt, pokeID );
    wk->record[0].fDead[ pokeID ] = TRUE;
    wk->record[0].fExpChecked[ pokeID ] = FALSE;
    wk->record[0].cnt++;
  }else{
    GF_ASSERT(0);
  }
}


static u8 seekPokeID( const BTL_DEADREC* wk, u8 turn, u8 idx )
{
  if( turn < BTL_DEADREC_TURN_MAX )
  {
    u32 i;
    for(i=0; i<BTL_POKEID_MAX; ++i)
    {
      if( wk->record[turn].fDead[i] )
      {
        if( idx ){
          --idx;
        }else{
          return i;
        }
      }
    }
  }
  return BTL_POKEID_NULL;
}


u8 BTL_DEADREC_GetCount( const BTL_DEADREC* wk, u8 turn )
{
  if( turn < BTL_DEADREC_TURN_MAX ){
    return wk->record[ turn ].cnt;
  }else{
    GF_ASSERT(0);
    return 0;
  }
}
u8 BTL_DEADREC_GetPokeID( const BTL_DEADREC* wk, u8 turn, u8 idx )
{
  if( turn < BTL_DEADREC_TURN_MAX )
  {
    u8 pokeID = seekPokeID( wk, turn, idx );
    return pokeID;
  }
  GF_ASSERT(0);
  return BTL_POKEID_NULL;
}

//=============================================================================================
/**
 * 経験値処理済みフラグ取得
 *
 * @param   wk
 * @param   turn    遡るターン数（現ターンなら0）
 * @param   idx     死亡ポケモンIndex
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_DEADREC_GetExpCheckedFlag( const BTL_DEADREC* wk, u8 turn, u8 idx )
{
  if( turn < BTL_DEADREC_TURN_MAX )
  {
    u8 pokeID = seekPokeID( wk, turn, idx );

    if( pokeID < BTL_POKEID_MAX )
    {
      return wk->record[ turn ].fExpChecked[ pokeID ];
    }
  }
  GF_ASSERT(0);
  return FALSE;
}
//=============================================================================================
/**
 * 経験値処理済みフラグセット
 *
 * @param   wk
 * @param   turn    遡るターン数（現ターンなら0）
 * @param   idx     死亡ポケモンIndex
 */
//=============================================================================================
void BTL_DEADREC_SetExpCheckedFlag( BTL_DEADREC* wk, u8 turn, u8 idx )
{
  if( turn < BTL_DEADREC_TURN_MAX )
  {
    u8 pokeID = seekPokeID( wk, turn, idx );
    if( pokeID < BTL_POKEID_MAX )
    {
      wk->record[ turn ].fExpChecked[ pokeID ] = TRUE;
      return;
    }
  }
  GF_ASSERT(0);
}


