//=============================================================================================
/**
 * @file  btl_pospoke_state.c
 * @brief ポケモンWB バトルシステム  場にいるポケモンの生存確認用構造体と処理ルーチン
 * @author  taya
 *
 * @date  2009.07.27  作成
 */
//=============================================================================================

#include "btl_common.h"
#include "btl_pokeparam.h"
#include "btl_pospoke_state.h"



/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void set_pos_state( BTL_POSPOKE_WORK* wk, const BTL_MAIN_MODULE* mainModule, const BTL_POKE_CONTAINER* pokeCon, BtlPokePos pos );
static void checkConfrontRec( BTL_POSPOKE_WORK* wk, BtlPokePos pos, BTL_POKE_CONTAINER* pokeCon );
static void updateLastPos( BTL_POSPOKE_WORK* wk, BtlPokePos pos );



void BTL_POSPOKE_InitWork( BTL_POSPOKE_WORK* wk, const BTL_MAIN_MODULE* mainModule, const BTL_POKE_CONTAINER* pokeCon, BtlRule rule )
{
  u32 i, endPos;

  for(i=0; i<NELEMS(wk->state); ++i){
    wk->state[i].fEnable = FALSE;
    wk->state[i].existPokeID = BTL_POKEID_NULL;
  }
  for(i=0; i<NELEMS(wk->state); ++i){
    wk->lastPos[i] = BTL_POS_NULL;
  }

  endPos = BTL_MAIN_GetEnablePosEnd( mainModule );
  for(i=0; i<=endPos; ++i){
    set_pos_state( wk, mainModule, pokeCon, i );
  }
}
static void set_pos_state( BTL_POSPOKE_WORK* wk, const BTL_MAIN_MODULE* mainModule, const BTL_POKE_CONTAINER* pokeCon, BtlPokePos pos )
{
  const BTL_POKEPARAM* bpp;
  u8 clientID, memberIdx;

  BTL_MAIN_BtlPosToClientID_and_PosIdx( mainModule, pos, &clientID, &memberIdx );
  bpp = BTL_POKECON_GetClientPokeDataConst( pokeCon, clientID, memberIdx );

  if( bpp != NULL && !BPP_IsDead(bpp) )
  {
    u8 pokeID = BPP_GetID( bpp );
    wk->state[ pos ].existPokeID = pokeID;
    wk->lastPos[ pokeID ] = pos;
  }
  else
  {
    wk->state[ pos ].existPokeID = BTL_POKEID_NULL;
  }
  wk->state[ pos ].clientID = clientID;
  wk->state[ pos ].fEnable = TRUE;
}

//=============================================================================================
/**
 * ポケモン退場の通知受け取り
 *
 * @param   wk
 * @param   pokeID
 */
//=============================================================================================
void BTL_POSPOKE_PokeOut( BTL_POSPOKE_WORK* wk, u8 pokeID )
{
  u32 i;
  for(i=0; i<NELEMS(wk->state); ++i)
  {
    if( wk->state[i].fEnable && (wk->state[i].existPokeID == pokeID) ){
      wk->state[i].existPokeID = BTL_POKEID_NULL;
      BTL_N_Printf( DBGSTR_POSPOKE_Out, pokeID, i );
      return;
    }
  }

  GF_ASSERT_MSG(0, "not exist pokeID=%d\n", pokeID);
  TAYA_Printf("set break point\n");
}
//=============================================================================================
/**
 * ポケモン入場の通知受け取り
 *
 * @param   wk
 * @param   pokeID
 */
//=============================================================================================
void BTL_POSPOKE_PokeIn( BTL_POSPOKE_WORK* wk, BtlPokePos pos,  u8 pokeID, BTL_POKE_CONTAINER* pokeCon )
{
  GF_ASSERT_MSG(wk->state[pos].fEnable, "pos=%d\n", pos);
  wk->state[pos].existPokeID = pokeID;
  wk->lastPos[ pokeID ] = pos;
  BTL_N_Printf( DBGSTR_POSPOKE_In, pokeID, pos );

  checkConfrontRec( wk, pos, pokeCon );
}
//=============================================================================================
/**
 * ポケモンローテーションの通知受け取り
 *
 * @param   wk
 * @param   dir
 * @param   clientID
 * @param   inPokeID
 * @param   pokeCon
 */
//=============================================================================================
void BTL_POSPOKE_Rotate( BTL_POSPOKE_WORK* wk, BtlRotateDir dir, u8 clientID, const BTL_POKEPARAM* inPoke, BTL_POKE_CONTAINER* pokeCon )
{
  if( (dir == BTL_ROTATEDIR_STAY) || (dir == BTL_ROTATEDIR_NONE) ){
    return;
  }
  else
  {
    #ifdef ROTATION_NEW_SYSTEM
    u8  idx[ BTL_ROTATE_NUM ];
    u32 i, cnt;

    // 該当クライアントの担当位置インデックスを保存
    for(i=cnt=0; i<NELEMS(wk->state); ++i)
    {
      if( wk->state[i].clientID == clientID ){
        idx[  cnt++ ] = i;
      }
    }

    {
      u8 inIdx  = BTL_MAINUTIL_GetRotateInPosIdx( dir );
      u8 outIdx = BTL_MAINUTIL_GetRotateOutPosIdx( dir );

      BTL_POSPOKE_STATE  frontState = wk->state[ idx[0] ];

      wk->state[ idx[inIdx] ]  = wk->state[ idx[outIdx] ];
      wk->state[ idx[outIdx] ] = frontState;

      if( !BPP_IsDead(inPoke) ){
        BTL_POSPOKE_PokeIn( wk, idx[0], BPP_GetID(inPoke), pokeCon );
      }else{
        wk->state[ i ].existPokeID = BTL_POKEID_NULL;
      }
    }

    for(i=0; i<cnt; ++i){
      updateLastPos( wk, idx[i] );
    }

    #else

    u8  idx[ BTL_ROTATION_FRONTPOS_NUM ];
    BtlPokePos slideInPos = BTL_POS_MAX;
    u32 cnt, i;

    // 該当クライアントの担当位置インデックスを保存
    for(cnt=0, i=0; i<NELEMS(wk->state); ++i)
    {
      if( wk->state[i].clientID == clientID ){
        idx[cnt++] = i;
        if( cnt >= BTL_ROTATION_FRONTPOS_NUM ){
          break;
        }
      }
    }

    if( dir == BTL_ROTATEDIR_R ){
      wk->state[ idx[1] ] = wk->state[ idx[0] ];
      slideInPos = idx[0];
    }
    if( dir == BTL_ROTATEDIR_L ){
      wk->state[ idx[0] ] = wk->state[ idx[1] ];
      slideInPos = idx[1];
    }

    if( slideInPos != BTL_POS_MAX )
    {
      // 後衛から前衛に出たポケが生きていたら入場と同じ処理
      if( !BPP_IsDead(inPoke) ){
        BTL_POSPOKE_PokeIn( wk, slideInPos, BPP_GetID(inPoke), pokeCon );
      // 死んでいたらその位置を空ける
      }else{
        wk->state[ slideInPos ].existPokeID = BTL_POKEID_NULL;
      }
    }
    #endif
  }
}
//----------------------------------------------------------------------------------
/**
 * 対面レコード更新
 *
 * @param   wk
 * @param   pos
 * @param   pokeCon
 */
//----------------------------------------------------------------------------------
static void checkConfrontRec( BTL_POSPOKE_WORK* wk, BtlPokePos pos, BTL_POKE_CONTAINER* pokeCon )
{
  u32 i;
  u8 pokeID = wk->state[pos].existPokeID;

  if( pokeID != BTL_POKEID_NULL )
  {
    for(i=0; i<NELEMS(wk->state); ++i)
    {
      if( (wk->state[i].existPokeID != BTL_POKEID_NULL)
      &&  (wk->state[i].existPokeID != pokeID )
      ){
        if( !BTL_MAINUTIL_IsFriendPokeID(wk->state[i].existPokeID, pokeID) )
        {
          BTL_POKEPARAM* bpp;

          BTL_Printf("%d と%d が対面\n", pokeID, wk->state[i].existPokeID);

          bpp = BTL_POKECON_GetPokeParam( pokeCon, wk->state[i].existPokeID );
          BPP_CONFRONT_REC_Set( bpp, pokeID );

          bpp = BTL_POKECON_GetPokeParam( pokeCon, pokeID );
          BPP_CONFRONT_REC_Set( bpp, wk->state[i].existPokeID );
        }
      }
    }
  }
}

//=============================================================================================
/**
 * ポケモン位置交換
 *
 * @param   wk
 * @param   pos1
 * @param   pos2
 */
//=============================================================================================
void BTL_POSPOKE_Swap( BTL_POSPOKE_WORK* wk, BtlPokePos pos1, BtlPokePos pos2 )
{
  BTL_POSPOKE_STATE tmp = wk->state[ pos1 ];
  wk->state[ pos1 ] = wk->state[ pos2 ];
  wk->state[ pos2 ] = tmp;

  updateLastPos( wk, pos1 );
  updateLastPos( wk, pos2 );
}
//----------------------------------------------------------------------------------
/**
 * ポケモン最終位置ワークを更新
 *
 * @param   wk
 * @param   pos
 */
//----------------------------------------------------------------------------------
static void updateLastPos( BTL_POSPOKE_WORK* wk, BtlPokePos pos )
{
  u8 pokeID = wk->state[ pos ].existPokeID;
  if( pokeID != BTL_POKEID_NULL )
  {
    wk->lastPos[ pokeID ] = pos;
  }
}


//=============================================================================================
/**
 * 指定クライアントの担当している「位置」の内、空きになっている数を返す
 *
 * @param   wk
 * @param   clientID
 * @param   pos         [out] 空き位置ID（BtlPokePos）
 *
 * @retval  u8    空き位置数
 */
//=============================================================================================
u8 BTL_POSPOKE_GetClientEmptyPos( const BTL_POSPOKE_WORK* wk, u8 clientID, u8* pos )
{
  u32 i, cnt=0;
  for(i=0; i<NELEMS(wk->state); ++i)
  {
    if( wk->state[i].fEnable && (wk->state[i].clientID == clientID) )
    {
      if( wk->state[i].existPokeID == BTL_POKEID_NULL ){
        pos[ cnt++ ] = i;
      }
    }
  }
  return cnt;
}
//=============================================================================================
/**
 * 指定ポケモンが戦闘の場に出ているか判定
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_POSPOKE_IsExist( const BTL_POSPOKE_WORK* wk, u8 pokeID )
{
  return ( BTL_POSPOKE_GetPokeExistPos(wk, pokeID) != BTL_POS_NULL );
}
//=============================================================================================
/**
 * 指定ポケモンが戦闘の場に出ているか判定
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_POSPOKE_IsExistFrontPos( const BTL_POSPOKE_WORK* wk, const BTL_MAIN_MODULE* mainModule, u8 pokeID )
{
  BtlPokePos pos = BTL_POSPOKE_GetPokeExistPos( wk, pokeID );
  if( pos != BTL_POS_NULL )
  {
    return BTL_MAIN_IsFrontPos( mainModule, pos );
  }
  return FALSE;
}
//=============================================================================================
/**
 * 指定ポケモンがいる場所IDを取得
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  BtlPokePos    出ている場合は位置ID、出ていない場合は BTL_POS_NULL
 */
//=============================================================================================
BtlPokePos BTL_POSPOKE_GetPokeExistPos( const BTL_POSPOKE_WORK* wk, u8 pokeID )
{
  u32 i;
  for(i=0; i<NELEMS(wk->state); ++i)
  {
    if( wk->state[i].fEnable && (wk->state[i].existPokeID == pokeID) ){
      return i;
    }
  }
  return BTL_POS_NULL;
}
//=============================================================================================
/**
 * 指定ポケモンが最後に居た位置を返す（死んでいる場合にも有効な値が返る）
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  BtlPokePos    最後に居た位置／一度も場に出ていないポケモンの場合 BTL_POS_NULL
 */
//=============================================================================================
BtlPokePos BTL_POSPOKE_GetPokeLastPos( const BTL_POSPOKE_WORK* wk, u8 pokeID )
{
  return wk->lastPos[ pokeID ];
}

//=============================================================================================
/**
 * 指定位置にいるポケモンIDを返す（誰もいなければ BTL_POKEID_NULL）
 *
 * @param   wk
 * @param   pos
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_POSPOKE_GetExistPokeID( const BTL_POSPOKE_WORK* wk, BtlPokePos pos )
{
  GF_ASSERT(pos<BTL_POS_MAX);
  GF_ASSERT(wk->state[pos].fEnable);

  return wk->state[ pos ].existPokeID;
}
