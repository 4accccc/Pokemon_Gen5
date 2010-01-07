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



void BTL_POSPOKE_InitWork( BTL_POSPOKE_WORK* wk, const BTL_MAIN_MODULE* mainModule, const BTL_POKE_CONTAINER* pokeCon, BtlRule rule )
{
  u32 i, endPos;

  for(i=0; i<NELEMS(wk->state); ++i){
    wk->state[i].fEnable = FALSE;
    wk->state[i].existPokeID = BTL_POKEID_NULL;
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
    wk->state[ pos ].existPokeID = BPP_GetID( bpp );
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
      BTL_Printf(" poke[%d] out from pos[%d]\n", pokeID, i );
      return;
    }
  }
  GF_ASSERT_MSG(0, "not exist pokeID=%d\n", pokeID);
  BTL_Printf("DummyString..\n");
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
  BTL_Printf(" poke[%d] in to pos[%d]\n", pokeID, pos );

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
  enum {
    ROT_MAX = BTL_ROTATE_NUM-1,
  };

  if( dir == BTL_ROTATEDIR_STAY ){
    return;
  }
  else
  {
    u8 idx[ ROT_MAX ];
    u8 cnt, i;

    // 該当クライアントの担当位置インデックスを保存
    for(cnt=0, i=0; i<NELEMS(wk->state); ++i)
    {
      if( wk->state[i].clientID == clientID ){
        idx[cnt++] = i;
        if( cnt >= ROT_MAX ){
          break;
        }
      }
    }

    // スライド方向に併せて状態更新
    if( cnt == ROT_MAX )
    {
      BtlPokePos slideInPos = BTL_POS_MAX;
      u8 slideInPokeID = BTL_POKEID_NULL;

      if( dir == BTL_ROTATEDIR_R )
      {
        wk->state[ idx[1] ] = wk->state[ idx[0] ];
        slideInPos = idx[0];
      }
      if( dir == BTL_ROTATEDIR_L )
      {
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

      BTL_Printf("Client[%d] rotate_%d, front pokeID= %d, %d\n", clientID, dir,
        wk->state[idx[0]].existPokeID, wk->state[idx[1]].existPokeID );
    }
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
  return BTL_POS_MAX;
}

