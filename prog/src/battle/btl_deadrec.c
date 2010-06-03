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


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void clearTurnRecord( BTL_DEADREC_UNIT* unit );



/**
 *  ターン記録単位の初期化
 */
static void clearTurnRecord( BTL_DEADREC_UNIT* unit )
{
  u32 i;

  unit->cnt = 0;
  for(i=0; i<BTL_POKEID_MAX; ++i)
  {
    unit->deadPokeID[ i ] = BTL_POKEID_NULL;
    unit->fExpChecked[ i ] = FALSE;
  }
}

//--------------------------------------------------
/**
 *  バトル開始時の初期化
 */
//--------------------------------------------------
void BTL_DEADREC_Init( BTL_DEADREC* wk )
{
  u32 i;
  for(i=0; i<BTL_DEADREC_TURN_MAX; ++i)
  {
    clearTurnRecord( &wk->record[i] );
  }
}
//--------------------------------------------------
/**
 *  ターン開始時の初期化
 */
//--------------------------------------------------
void BTL_DEADREC_StartTurn( BTL_DEADREC* wk )
{
  u32 i;
  for(i=BTL_DEADREC_TURN_MAX-1; i>0; --i)
  {
    wk->record[i] = wk->record[i-1];
  }
  clearTurnRecord( &wk->record[0] );
}


//--------------------------------------------------
/**
 *  死亡ポケID追加
 */
//--------------------------------------------------
void BTL_DEADREC_Add( BTL_DEADREC* wk, u8 pokeID )
{
  u8 cnt = wk->record[0].cnt;
  if( cnt < BTL_POKEID_MAX )
  {
    BTL_N_Printf( DBGSTR_DEADREC_Add, cnt, pokeID );
    wk->record[0].deadPokeID[ cnt ] = pokeID;
    wk->record[0].fExpChecked[ cnt ] = FALSE;
    wk->record[0].cnt++;
  }
  else{
    GF_ASSERT(0);
  }
}

//--------------------------------------------------
/**
 *  指定ターン内に死んだポケ数を取得
 */
//--------------------------------------------------
u8 BTL_DEADREC_GetCount( const BTL_DEADREC* wk, u8 turn )
{
  if( turn < BTL_DEADREC_TURN_MAX ){
    return wk->record[ turn ].cnt;
  }else{
    GF_ASSERT(0);
    return 0;
  }
}

//--------------------------------------------------
/**
 *  指定ターン内に死んだポケIDを取得
 */
//--------------------------------------------------
u8 BTL_DEADREC_GetPokeID( const BTL_DEADREC* wk, u8 turn, u8 idx )
{
  if( turn < BTL_DEADREC_TURN_MAX )
  {
    if( idx < wk->record[turn].cnt )
    {
      return wk->record[turn].deadPokeID[ idx ];
    }
  }
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
    if( idx < wk->record[turn].cnt )
    {
      return wk->record[turn].fExpChecked[ idx ];
    }
  }
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
    if( idx < wk->record[turn].cnt )
    {
      wk->record[turn].fExpChecked[ idx ] = TRUE;
    }
  }
}

//=============================================================================================
/**
 * 最後に死亡したポケモンIDを取得
 *
 * @param   wk
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_DEADREC_GetLastDeadPokeID( const BTL_DEADREC* wk )
{
  u32 i;
  for(i=0; i<BTL_DEADREC_TURN_MAX; ++i)
  {
    if( wk->record[i].cnt )
    {
      u32 n = wk->record[i].cnt - 1;
      return wk->record[i].deadPokeID[n];
    }
  }
  return BTL_POKEID_NULL;
}

